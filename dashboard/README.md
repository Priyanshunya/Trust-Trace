# Trust Trace: V1 Edge Telemetry Dashboard

This directory contains the V1 Proof-of-Concept (PoC) dashboard for our Trust Trace edge nodes.

To validate our PUF seals in real-world offline environments (like warehouses or steel reefer trucks), we engineered a zero-dependency, serverless WebBLE interface rather than building a heavy cloud backend for the baseline prototype.

## Architecture & Engineering Rationale
The entire V1 dashboard operates locally from a single `index.html` file. It connects directly to the ESP32-S3 over GATT, completely bypassing the need for WiFi or cellular data during hardware validation.

By stripping the threshold logic out of the edge node and moving it into this frontend, we created a highly flexible testing environment:

* **Hardware-in-the-Loop:** The UI is physically bound to the prototype. Pressing Button 1 on the ESP32-S3 commands the dashboard to register a baseline, and pressing Button 2 executes the seal audit.
* **Dynamic Threshold Tuning:** PUF impedance can shift based on ambient variables. A threshold input in the UI allows us to tune the tamper sensitivity (e.g., 0.3 kΩ) on the fly without re-flashing the C++ firmware.
* **Multi-Package Registry:** The local JavaScript maps package IDs to their specific analog baselines in a dictionary, allowing us to scan and test multiple crates sequentially using just browser memory.

---
*This V1 dashboard serves strictly as a localized hardware testbench. Our final V2 production architecture deprecates WebBLE in favor of NB-IoT cellular uplinks pushing MQTT payloads to a decentralized Hyperledger Fabric backend.*
