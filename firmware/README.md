# ⚡ Trust Trace: Edge Node Firmware

This directory houses the C++ firmware for the Trust Trace master edge nodes. 

To ensure a highly reliable build for SIH 2026, we are structuring our firmware development into two distinct phases. This iterative approach allows us to validate our core intellectual property the PUF tamper seals—before integrating the full industrial cold-chain communication stack.

## V1 Baseline Prototype (PoC)
**Status: Completed**

The `v1_poc_analog_telemetry` folder contains our active Proof of Concept. 

For this baseline, our primary focus is proving the PUF seal innovation. We engineered the ESP32-S3 to cleanly read the analog impedance signatures from our conductive ink tape and stream that raw telemetry directly to our local WebBLE dashboard. 

To isolate and test this core hardware trigger quickly, we deliberately utilized accessible commercial off-the-shelf (COTS) sensors for the environmental data. By bypassing complex industrial sensor drivers in this phase, we successfully locked in the edge-native telemetry pipeline, analog ADC oversampling, and dashboard UI parsing without unnecessary bottlenecks.

## V2 MVP & Industrial Deployment
**Status: Work in Progress**

With the PUF logic and base telemetry pipeline completely validated in V1, the V2 MVP focuses on integrating the enterprise-grade hardware defined in our architecture:
* **Industrial Sensor Swap:** Replacing the PoC sensors with our final targets—the **Sensirion SHT40** (climate) and **Winsen ME3-C2H4** (precision electrochemical ethylene tracking).
* **NB-IoT & Cloud:** Integrating the **Quectel NB-IoT** modem via AT commands to push CBOR-compressed payloads to Hyperledger Fabric.
* **Offline Cryptography:** Implementing local **SPI Flash SHA-256 hash-chaining** to guarantee zero data loss and immutable records during rural cellular dead zones.
