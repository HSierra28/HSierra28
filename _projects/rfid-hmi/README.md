# RFID Access Control with HMI

## 📌 Project Overview
This project demonstrates a **basic access control system** using an **ESP32**, an **RFID-RC522 reader**, and a **Nextion HMI display**.  
The system authenticates users via RFID tags and shows access status on the HMI screen.

**Purpose:** Showcase hardware-software integration for IoT and industrial automation applications.

---

## 🛠️ Hardware Used
- ESP32-WROOM-32  
- RFID-RC522 + 1 RFID card + 1 RFID key fob  
- Nextion Basic 7" HMI display  
- Breadboard, wires, and power supply  

---

## 🔌 Hardware Setup
- **RFID-RC522** → connected via SPI to ESP32  
- **Nextion HMI** → UART connection (TX/RX) with ESP32  
- **ESP32 Power**: 3.3V  
- Diagram available in `/hardware/wiring.png`  

---

## 💻 Software Overview
- Language: C / Arduino Framework (ESP-IDF can be used later)  
- Libraries:  
  - `MFRC522` for RFID reader  
  - `Nextion` library for HMI communication  
- Main features:  
  1. Read RFID UID  
  2. Compare UID with predefined list  
  3. Grant or deny access  
  4. Display status on Nextion screen  

---

## 📂 Repository Structure
- `/src`: Source code (C/C++)  
- `/docs`: Auto-generated documentation (Sphinx + Doxygen)  
- `/media`: Photos and demo videos  
- `README.md`: Project overview  

---

## ▶️ Demo
- **Video, photos, and screenshots:** See `/media`  

---

## 📖 Documentation
- [Generated documentation](../generated-docs/html/index.html)  
- Includes: code architecture, flowcharts, hardware datasheets

---

## 🚀 Future Improvements
- WiFi integration for logging access events  
- Multiple access levels with different tags  
- Cloud dashboard for remote monitoring