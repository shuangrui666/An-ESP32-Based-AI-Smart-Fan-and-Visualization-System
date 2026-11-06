# An-ESP32-Based-AI-Smart-Fan-and-Visualization-System
# SmartFan 2.0
**An ESP32-Based AI Smart Fan and Visualization System**

## 1. Project Overview

SmartFan 2.0 is an intelligent fan prototype based on the ESP32 microcontroller, Python-based machine learning and a web-based visualization dashboard.

The project aims to transform an ordinary USB-powered fan into an intelligent system with environmental monitoring, human activity detection, AI-assisted fan control, Wi-Fi remote operation and real-time data visualization.

Artificial intelligence will be integrated throughout the development process and implemented as a real function in the final system.

The project emphasizes practical implementation, experimental verification, data visualization, system integration and engineering documentation.

## 2. Project Objectives

1. Develop an ESP32-based smart fan control system.
2. Monitor environmental temperature and humidity.
3. Detect human activity using a PIR motion sensor.
4. Implement automatic fan control according to environmental conditions.
5. Develop a machine learning model to recommend fan operating levels.
6. Enable remote operation through a mobile-friendly web interface.
7. Build a real-time visualization dashboard.
8. Record historical sensor data and fan operating states.
9. Evaluate system performance using experimental data.
10. Document the complete engineering development process.

## 3. Main System Functions

| Function | Description | Status |
|---|---|---|
| Environmental monitoring | Collect temperature and humidity | Planned |
| Human activity detection | Detect movement using a PIR sensor | Planned |
| Automatic fan control | Adjust fan operation according to control rules | Planned |
| AI-based control | Predict recommended fan operating levels | Planned |
| Wi-Fi communication | Exchange data between ESP32 and computer | Planned |
| Mobile remote control | Operate the fan through a web interface | Planned |
| Real-time dashboard | Display environmental and device information | Planned |
| Historical charts | Display changes over time | Planned |
| AI visualization | Display model predictions and evaluation results | Planned |
| Offline fallback | Use local ESP32 rules if the AI computer is unavailable | Planned |

## 4. System Architecture

The proposed system contains four main layers.

### 4.1 Hardware Layer

Planned hardware:

- ESP32 development board
- USB-powered DC fan
- Temperature and humidity sensor
- PIR motion sensor
- Suitable motor driver
- Optional TFT display or RGB LED module

The final hardware configuration will depend on the actual ESP-Claw kit inventory and verified fan specifications.

### 4.2 Communication and Backend Layer

The ESP32 will collect sensor data and communicate with a Windows computer through Wi-Fi and HTTP.

The Python backend will receive sensor information, manage data, call the machine learning model and provide control and monitoring interfaces.

### 4.3 Artificial Intelligence Layer

A Python-based Decision Tree Classifier using scikit-learn is planned for the initial implementation.

Potential inputs:

- Temperature
- Humidity
- Human activity status
- User-selected preferences

The model will recommend one of four operating levels:

- OFF
- LOW
- MEDIUM
- HIGH

The ESP32 will receive the recommended control command and execute it through a suitable motor driver.

If the computer or network connection becomes unavailable, local rule-based control will provide a fallback.

Training labels and model evaluation will be based on documented rules or actual collected data. Model performance will not be claimed before testing.

### 4.4 Visualization and User Interface Layer

A responsive web dashboard will support computer and mobile browsers.

Planned technologies:

- HTML
- CSS
- JavaScript
- ECharts

The dashboard will communicate with the Python backend to display real-time and historical information.

## 5. Data Visualization and Monitoring

### 5.1 Real-Time Monitoring Dashboard

The dashboard will display:

- Current temperature
- Current humidity
- Fan operating status
- Current fan control level
- Human activity detection status
- Automatic or manual mode
- Device connection status
- AI-recommended operating level

### 5.2 Historical Data Visualization

The system will record and visualize:

- Temperature changes over time
- Humidity changes over time
- Fan operating-level changes
- Human activity events
- Automatic control responses

### 5.3 AI Model Visualization

The project will provide:

- Decision tree structure visualization
- AI prediction results
- Model accuracy
- Confusion matrix
- Prediction and actual-label comparison

All model evaluation results will be generated from actual experiments.

### 5.4 Remote Control Interface

The web interface will provide:

- Fan ON/OFF control
- Automatic mode
- Manual mode
- Fan level selection
- Current device status

The actual fan control capability will be verified against the motor and driver specifications.

## 6. Technology Stack

| Area | Technology |
|---|---|
| Microcontroller | ESP32 |
| Firmware | C++ / Arduino framework |
| AI programming | Python |
| Machine learning | scikit-learn |
| Backend | Flask |
| Communication | Wi-Fi / HTTP |
| Frontend | HTML, CSS, JavaScript |
| Chart visualization | ECharts |
| Data processing | pandas, NumPy |
| Data analysis | Matplotlib |
| Version control | Git |
| Project hosting | GitHub |

## 7. Project Development Plan

### Stage 1: Project Preparation
- Create the GitHub repository.
- Inspect the purchased fan.
- Identify the ESP32 board and available sensor modules.
- Confirm laboratory equipment and borrowing arrangements.
- Finalize the hardware architecture.

### Stage 2: Hardware Development
- Connect the ESP32 and sensors.
- Test temperature and humidity measurements.
- Test PIR motion detection.
- Verify fan electrical specifications.
- Select and test a suitable motor driver.
- Implement basic fan control.

### Stage 3: Firmware and Backend Development
- Develop ESP32 sensor acquisition firmware.
- Implement Wi-Fi communication.
- Develop the Python backend.
- Create data receiving and control APIs.
- Implement data storage.

### Stage 4: AI Model Development
- Prepare a documented training dataset.
- Define and explain model labels.
- Train the initial Decision Tree Classifier.
- Evaluate the model.
- Visualize the decision tree and evaluation results.
- Connect model predictions to the control system.

### Stage 5: Visualization Dashboard Development
- Build the responsive dashboard.
- Display real-time environmental data.
- Display fan status and operating level.
- Add temperature and humidity time-series charts.
- Add fan operation and activity timelines.
- Display AI recommendations and model analysis.
- Implement manual and automatic remote control.

### Stage 6: System Integration and Testing
- Integrate hardware, firmware, backend, AI and dashboard.
- Test automatic and manual operating modes.
- Test communication reliability.
- Test offline fallback behavior.
- Evaluate control stability.

### Stage 7: Experimental Evaluation and Documentation
- Conduct temperature response tests.
- Conduct human activity detection tests.
- Evaluate fan speed control.
- Record experimental data.
- Generate data visualization charts.
- Prepare the final report and presentation.
- Organize photos, videos, source code and test evidence.

## 8. Repository Structure

```text
smartfan-2.0/
├── README.md
├── LICENSE
├── requirements.txt
├── docs/
├── hardware/
├── firmware/
├── backend/
├── ai/
├── dashboard/
├── data/
├── visualization/
├── experiments/
├── media/
└── ai_collaboration/
```

Each directory will contain relevant source code, data, documentation or experimental evidence as development progresses.

## 9. Safety and Engineering Principles

- Never connect a motor directly to an ESP32 GPIO pin.
- Select the motor driver according to the actual motor voltage and current requirements.
- Verify the power supply before connecting the motor.
- Disconnect power before modifying wiring.
- Do not modify or puncture lithium batteries.
- Distinguish planned functions from experimentally verified functions.
- Preserve original experimental data and document data processing.
- Do not fabricate measurements, model results or performance claims.

## 10. Current Project Status

**Current stage: Project preparation and hardware verification.**

Completed:
- Initial project concept
- Preliminary system architecture
- Initial AI implementation route
- Advanced visualization objectives
- Purchase of a USB-powered fan

Pending:
- Confirm fan voltage, current and motor specifications.
- Inspect the ESP-Claw kit.
- Confirm available sensors and motor driver modules.
- Finalize hardware and power supply design.
- Implement and test the hardware, AI model, backend and dashboard.

## 11. Development Principle

SmartFan 2.0 follows an evidence-based engineering approach.

All circuit designs, source code, experimental data, visualizations and performance claims will be supported by actual implementation or testing.

Unverified functions will remain marked as planned or pending until they have been successfully tested.
