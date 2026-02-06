# 😷 SentinAir: IoT Air Quality Monitor & Cloud Analytics

![Status](https://img.shields.io/badge/Status-Completed-success)
![Platform](https://img.shields.io/badge/Platform-ESP8266-blue)
![Architecture](https://img.shields.io/badge/Architecture-OOP-orange)

## 📖 Project Overview
Designed and implemented a scalable IoT solution to monitor hyper-local air quality indices (AQI). The system integrates precision hardware with a C++ firmware architecture to log environmental data (PM2.5, PM10, Temperature, Humidity) to the cloud for real-time analytics.

## 🚀 Key Features
* **OOP-Based Firmware:** Utilizes C++ classes to encapsulate sensor behaviors, ensuring modularity and ease of maintenance.
* **Precision Monitoring:** Deploys Laser Scattering technology (PMS5003) for accurate particulate counting.
* **Cloud Integration:** Asynchronous data transmission to ThingSpeak via RESTful API integration.
* **Dynamic Visualization:** Custom-built OLED dashboard with interrupt-based UI updates.

## 📸 Component Breakdown
<p align="center">
  <img src="project/1%20(3).jpeg" width="45%">
  <img src="project/1%20(11).jpeg" width="45%">
</p>
<p align="center">
  <img src="project/1%20(7).jpeg" width="30%">
  <img src="project/1%20(1).jpeg" width="30%">
  <img src="project/1%20(4).jpeg" width="30%">
</p>

## 🛠️ Technical Stack
* **Language:** C++ (Object-Oriented Design)
* **Hardware Abstraction:** Arduino Framework for ESP8266
* **Communication Protocols:** UART (SoftwareSerial), I2C, HTTP/REST
* **Hardware:** NodeMCU (ESP8266), Plantower PMS5003, DHT11.

## ⚡ Engineering Challenges Solved
* **Sensor Data Buffering:** Implemented rigorous input flushing to prevent UART buffer overflow.
* **Power Management:** Optimized sensor polling intervals to balance data granularity with power consumption.
* **Calibration Algorithms:** Mapped raw particulate density to Indian AQI standards using custom conversion logic.

## 👨‍💻 Developer
**B.Tech Computer Science & Engineering (3rd Year)**
Focused on IoT Architectures, Embedded Software, and Cloud Computing.
[![ThingSpeak](https://img.shields.io/badge/Dashboard-Live%20Data-red)](https://thingspeak.mathworks.com/channels/3239197)
