<div align="center">

# 🛡️ Trust Trace
### Asymmetric Cold-Chain Telemetry & Cryptographic Vaulting Ecosystem
**Smart India Hackathon 2026 (SIH 2026) — MoFPI Problem Statement 26232**

[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32--S3-orange.svg?style=flat-square&logo=platformio)](https://platformio.org/)
[![Hyperledger Fabric](https://img.shields.io/badge/Ledger-Hyperledger%20Fabric-blue.svg?style=flat-square&logo=hyperledger)](https://www.hyperledger.org/use/fabric)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square)](LICENSE)
[![Team: Vision.exe](https://img.shields.io/badge/Team-Vision.exe%20(ID%3A%20162907)-purple.svg?style=flat-square)]()

</div>

---

## 🚀 Executive Summary
Global export markets demand rigorous compliance (APEDA/FSSAI) to combat food fraud, yet Small and Medium Enterprises (SMEs) are priced out by commercial loggers costing ₹300+ per crate. Traditional active trackers also suffer from severe physical vulnerabilities: lithium-ion batteries freeze below 0°C in refrigerated trucks, and cellular dead zones cause permanent data loss.

**Trust Trace** solves these bottlenecks through an asymmetric architecture: a single, reusable **ESP32-S3 Master Node** per pallet paired with sub-₹10 consumable **conductive ink PUF (Physical Unclonable Function) tamper seals**. By combining sub-zero thermal energy harvesting, hardware-level cryptographic key storage, and offline hash-chaining, Trust Trace delivers tamper-proof farm-to-fork traceability at a fraction of enterprise costs.

---

## 📐 System Architecture

```text
[Conductive Ink PUF Seals] ──► (ADC GPIO 1)  ┐
                                        ├─► [ESP32-S3 Edge Core] ──► [Offline SPI Flash Hash Chain] ──► [Quectel NB-IoT] ──► [EMQX MQTT] ──► [Hyperledger Fabric]
[Sensors (SHT40/ME3)] ──► (I2C / ADC)  ┘           ▲
                                                    │
                                        [TI BQ25570 MPPT + LTO Battery] (Solar/TEG Dual Harvest)
```




## 🛠️ Hardware & Firmware Specifications

| Subsystem | Component / Technology | Engineering Rationale |
| :--- | :--- | :--- |
| **Master Node** | ESP32-S3 (Dual-Core Xtensa LX7) | Handles deterministic sensor polling, local cryptographic hashing, and modem orchestration. |
| **Tamper Seals** | Screen-printed Conductive Ink PUF | Generates unique analog impedance signatures wired to ADC pins; physical breaches trigger instant hardware interrupts. |
| **Power Harvesting** | TI BQ25570 MPPT + Nichicon LTO | Hybrid power management switching between open-air solar and reefer TEGs (Thermoelectric Generators) to ensure indefinite autonomy down to -30°C. |
| **Gas & Climate** | Sensirion SHT40 & Winsen ME3-C2H4 | High-precision climate logging and zero-power electrochemical ethylene gas tracking (PPM) to catch ripening cascades early. |
| **Secure Vault** | Microchip ATECC608B + SPI Flash | Hardware root-of-trust (ECDSA P-256) combined with an immutable local SPI flash hash chain (`H[n] = SHA-256(H[n-1] || Data)`) for zero data loss during rural cellular dead zones. |
| **Cloud & Ledger** | Quectel NB-IoT + Hyperledger Fabric | CBOR-compressed payloads transmitted via TLS MQTT into private, permissioned decentralized ledger channels. |

---

## 👥 Team Vision.exe
* **Team ID:** 162907
* **Problem Statement ID:** 26232 (Ministry of Food Processing Industries - MoFPI)
* **Category:** Hardware / IoT & Electronics
