#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Pins for V1 Prototype (ESP32-S3)
#define PIN_PUF_TAP      1  // PUF tape input (ADC1_CH0)
#define PIN_MQ135        2  // Gas sensor analog (ADC1_CH1)
#define PIN_BTN_1        4  // General Purpose Button 1
#define PIN_BTN_2        5  // General Purpose Button 2
#define PIN_I2C_SDA      8  
#define PIN_I2C_SCL      9  

// ~100k pulldown for the PUF voltage divider
const float R_PULLDOWN_OHMS = 97800.0f; 

// TrustTrace WebBLE UUIDs
#define SERVICE_UUID           "19b10000-e8f2-537e-4f6c-d104768a1214"
#define TELEMETRY_CHAR_UUID    "19b10001-e8f2-537e-4f6c-d104768a1214"
#define ALERT_CHAR_UUID        "19b10002-e8f2-537e-4f6c-d104768a1214"

Adafruit_BME280 bme;
bool bmeDetected = false;

BLEServer* pServer = nullptr;
BLECharacteristic* pTelemetryChar = nullptr;
BLECharacteristic* pAlertChar = nullptr;

bool deviceConnected = false;
bool oldDeviceConnected = false;

unsigned long lastTelemetryTime = 0;
const unsigned long TELEMETRY_INTERVAL = 500; 

// Button interrupt flags
volatile bool flagBtn1 = false;
volatile bool flagBtn2 = false;
volatile unsigned long lastBtn1Time = 0;
volatile unsigned long lastBtn2Time = 0;
const unsigned long DEBOUNCE_MS = 300;

// Oversampling to clean up the ADC noise natively on the edge
uint16_t readAnalogSmooth(uint8_t pin, uint8_t samples = 16) {
    uint32_t acc = 0;
    for (uint8_t i = 0; i < samples; i++) {
        acc += analogRead(pin);
        delayMicroseconds(50);
    }
    return (uint16_t)(acc / samples);
}

// Convert raw 12-bit ADC to actual resistance
float calculateResistanceKOhms(uint16_t adcVal) {
    if (adcVal <= 10) return 999.9f;
    if (adcVal >= 4090) return 0.1f;
    float rTape = R_PULLDOWN_OHMS * ((4095.0f - (float)adcVal) / (float)adcVal);
    return rTape / 1000.0f;
}

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* server) override {
        deviceConnected = true;
        Serial.println("Dashboard connected via BLE");
    }
    void onDisconnect(BLEServer* server) override {
        deviceConnected = false;
        Serial.println("Dashboard disconnected");
    }
};

void IRAM_ATTR isr_Btn1() {
    unsigned long now = millis();
    if (now - lastBtn1Time > DEBOUNCE_MS) {
        lastBtn1Time = now;
        flagBtn1 = true;
    }
}

void IRAM_ATTR isr_Btn2() {
    unsigned long now = millis();
    if (now - lastBtn2Time > DEBOUNCE_MS) {
        lastBtn2Time = now;
        flagBtn2 = true;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\nTrustTrace V1 Telemetry Node Booting...");

    analogReadResolution(12);
    pinMode(PIN_PUF_TAP, INPUT);
    pinMode(PIN_MQ135, INPUT);
    pinMode(PIN_BTN_1, INPUT_PULLUP);
    pinMode(PIN_BTN_2, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PIN_BTN_1), isr_Btn1, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_2), isr_Btn2, FALLING);

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    if (bme.begin(0x76) || bme.begin(0x77)) {
        bmeDetected = true;
        Serial.println("BME280 sensor initialized");
    } else {
        Serial.println("Warning: BME280 not detected on I2C bus");
    }

    Serial.println("Allowing analog sensors to stabilize (4s)...");
    delay(4000); 

    BLEDevice::init("TrustTrace-V2");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);

    pTelemetryChar = pService->createCharacteristic(
        TELEMETRY_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pTelemetryChar->addDescriptor(new BLE2902());

    pAlertChar = pService->createCharacteristic(
        ALERT_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pAlertChar->addDescriptor(new BLE2902());

    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    pAdvertising->start();

    Serial.println("BLE broadcasting active. Awaiting dashboard connection.");
}

void loop() {
    unsigned long currentMillis = millis();

    // Auto-resume advertising if dashboard drops connection
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); 
        pServer->startAdvertising();
        Serial.println("Re-advertising BLE...");
        oldDeviceConnected = false;
    }
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = true;
    }

    // Pass physical button presses directly to the dashboard alert stream
    if (flagBtn1) {
        flagBtn1 = false;
        if (deviceConnected) {
            pAlertChar->setValue("BTN_1_PRESSED");
            pAlertChar->notify();
        }
        Serial.println("Event: Button 1 pressed");
    }

    if (flagBtn2) {
        flagBtn2 = false;
        if (deviceConnected) {
            pAlertChar->setValue("BTN_2_PRESSED");
            pAlertChar->notify();
        }
        Serial.println("Event: Button 2 pressed");
    }

    // Main Telemetry Pipeline (500ms intervals)
    if (currentMillis - lastTelemetryTime >= TELEMETRY_INTERVAL) {
        lastTelemetryTime = currentMillis;

        float temp = 0.0f, hum = 0.0f;
        if (bmeDetected) {
            temp = bme.readTemperature();
            hum = bme.readHumidity();
            if (isnan(temp)) temp = 0.0f;
            if (isnan(hum)) hum = 0.0f;
        }

        uint16_t rawGas = readAnalogSmooth(PIN_MQ135, 16);
        uint16_t currentPuf = readAnalogSmooth(PIN_PUF_TAP, 16);
        float currentPufR = calculateResistanceKOhms(currentPuf);

        if (deviceConnected) {
            // Pure pass-through JSON payload.
            char telemetryJSON[200];
            int len = snprintf(telemetryJSON, sizeof(telemetryJSON),
                     "{\"temp\":%.1f,\"hum\":%.1f,\"gas\":%u,\"puf\":%u,\"pufR\":%.1f}",
                     temp, hum, rawGas, currentPuf, currentPufR);

            if (len > 0 && len < sizeof(telemetryJSON)) {
                pTelemetryChar->setValue((uint8_t*)telemetryJSON, len);
                pTelemetryChar->notify();
            }
        }
    }
}
