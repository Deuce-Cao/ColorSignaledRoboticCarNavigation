# Color Signaled Robotic Car Navigation

## Overview
This project is an advanced robotics application using the **ELEGOO Smart Robot Car Kit V4.0** with an **ESP32-CAM** module to simulate an autonomous transport system within a production facility. The robot follows mapped black-line paths on the ground to transport between multiple production lines and a packaging line. The system also includes a manually simulated signal light, which the ESP32-CAM detects to determine the robot’s next destination.

## Features
### **Basic Functions**
The ELEGOO kit provides several pre-built features, some of which are used in this project:
- **Line Tracking Mode:** The robot follows black lines on the ground.
- **Infrared Control Mode:** Reserved as a backup for manual override.

### **Enhanced & Custom Functions**
- **Color Recognition via ESP32-CAM:** Detects signal lights to decide the production location.
- **Route Selection & Navigation:** Uses a state machine to manage transport tasks.
- **Simulated Physical Loading Platform:** Represents goods transportation between stations.

## System Architecture
The project consists of two primary modules:
1. **Arduino Uno** (controls sensors, motors, and navigation logic)
2. **ESP32-CAM** (captures and processes images to detect color signals)

### **Workflow**
1. **Standby Mode:** The robot waits at the packaging station.
2. **Loading Signal:** The ESP32-CAM reads the light signal and the Arduino determines the route.
4. **Transit to Production Line:** The robot follows the black-line path to the assigned production station.
5. **Loading Simulation:** The robot waits for a simulated loading time.
6. **Return and Standby:** The robot moves back to the packaging station and resets.

## Hardware Components
- **ELEGOO Smart Robot Car Kit V4.0** (including line tracking sensors and motor drivers)
- **ESP32-CAM Module** (for image processing)
- **Manually Simulated Signal Light** (smartphone screen)

## Software & Development Environment
- **PlatformIO under VSCode** (for both Arduino Uno & ESP32-CAM programming)
- **Git for Version Control**

### **Project Repository Setup**
1. **Git**
   ```sh
   git clone https://github.com/Deuce-Cao/ColorSignaledRoboticCarNavigation.git
   ```

## File Structure
```
ColorSignaledRoboticCarNavigation/
├── README.md
├── docs/
│   └── Advanced Robotics Proposal.docx
├── lib/
│   └── README.md
├── src/
│   ├── Arduino/
│   │   └── main.ino
│   └── ESP32_CAM/
│       └── main.cpp
└── test/
    └── README.md
```

## Risks & Challenges
- Failures in the components of the robot car, such as motors, sensors, and cameras, can affect the normal operation of the car.
- Problems with wiring when assembling the car, loose or incorrect wiring when connecting peripherals can cause failures.
- The motor will overheat when working, affecting the operation of the car.
- Errors in the code that controls the operation of the car and implements various functions.
- In low-light conditions, the camera has difficulty capturing.
- Signal interference problem when the car is running.


## Contributors
- **[Hongqing Cao](https://www.github.com/Deuce-Cao)**
- **Leila Dabbaghzadeh**
- **Pengbo Ma**
- **Zhengrong Yu**

---
This project is part of the **SEP 780 - Advanced Robotics and Automation** course under the guidance of **Dr. Richard Ma**.

