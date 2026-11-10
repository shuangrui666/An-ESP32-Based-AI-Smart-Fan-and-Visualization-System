# SmartFan 2.0 — Project Proposal

## 1. Project Background

Conventional fans generally provide limited environmental awareness, data recording and remote monitoring. Internet of Things (IoT) devices, embedded controllers and machine learning offer an opportunity to explore more adaptive and observable control systems.

SmartFan 2.0 is a proposed prototype that combines an ESP32 controller, available environmental sensors, a Python-based machine-learning service and a web visualization dashboard. The project will first verify whether the purchased USB-powered fan can be controlled safely and reliably before finalizing the motor-driving circuit.

## 2. Project Objectives

1. Identify and document the available hardware, including the YD-ESP32-S3 Classic V1.0 board, DHT11 module and seller-confirmed 5 V / 1 A USB fan rating; verify remaining electrical characteristics before circuit design.
2. Build an ESP32-based sensor and control prototype.
3. Use the user-confirmed DHT11 module to attempt temperature and humidity measurement after the board pinout, wiring and software environment are verified.
4. Add movement detection if a compatible PIR sensor is available.
5. Implement fan control only after verifying electrical compatibility.
6. Develop a Python model that recommends fan operating levels.
7. Provide Wi-Fi communication and a responsive web dashboard.
8. Record experimental data and evaluate the system using repeatable tests.

## 3. Proposed Functions

### 3.1 Environmental Monitoring
Read and display temperature and, if supported by the selected sensor, humidity.

### 3.2 Movement Detection
Use a PIR sensor if available to detect movement events. A PIR sensor detects changes associated with movement; it does not reliably prove that a person is absent when no movement is detected.

### 3.3 Fan Control
Provide OFF/LOW/MEDIUM/HIGH commands only if the verified fan, motor driver and power arrangement support these levels safely and repeatably. If smooth speed control is not feasible, revise the control design and document the limitation.

### 3.4 AI-Based Recommendation
Initially use a scikit-learn Decision Tree Classifier running on a Windows computer. Candidate inputs include measured environmental values, available movement state and recorded user preferences. Labels must be documented and grounded in a defined rule set or actual observations.

### 3.5 Remote Operation
Provide a mobile-friendly page for status monitoring and manual/automatic control through the Python backend.

### 3.6 Data Visualization
Use a dashboard to display current readings, historical trends, fan command levels, movement events and AI recommendations. If actual fan RPM is not measured, show the commanded level rather than a measured speed.

## 4. Technical Approach

The proposed architecture contains four layers:

- **Hardware:** ESP32, verified sensors, fan and compatible motor driver.
- **Communication:** Wi-Fi and HTTP between ESP32 and the computer.
- **Backend and AI:** Python/Flask service and scikit-learn model running on the computer.
- **Visualization:** HTML, CSS, JavaScript and ECharts dashboard.

The ESP32 will perform sensor acquisition and execute validated commands. The computer will receive data and run the initial machine-learning model. A simple local rule-based fallback is planned for computer or network outages.

## 5. Project Characteristics

1. Combines embedded control, IoT communication, machine learning and visualization.
2. Connects physical device behavior with a web dashboard.
3. Supports manual and automatic operating modes, subject to verified hardware capability.
4. Uses recorded data and repeatable tests to evaluate system behavior.
5. Maintains traceable records of design decisions, tests and revisions.

These are intended characteristics, not claims of completed or novel research results.

## 6. Experimental Plan

Planned tests include:

- Sensor reading consistency and response.
- PIR detection behavior, if the sensor is available.
- Fan startup, stable operation and control-level behavior.
- Driver and power-supply temperature/current checks, using suitable lab instruments.
- Wi-Fi communication and reconnection behavior.
- Dashboard data correctness and update behavior.
- AI model evaluation on a held-out test set.
- Local fallback behavior when the computer or network is disconnected.

Each experiment will record setup, conditions, procedure, raw data, result and limitations.

## 7. Budget and Resources

The project will prioritize the purchased fan, available ESP-Claw kit components and borrowed laboratory tools. Additional components will only be purchased after the kit inventory and fan specifications are confirmed. The project will comply with the instructor's applicable component-cost limit; the exact interpretation of that limit must be confirmed.

## 8. Expected Deliverables

1. A tested smart-fan prototype, within verified hardware capabilities.
2. ESP32 firmware source code.
3. Python backend and machine-learning source code.
4. Responsive web dashboard.
5. Original experimental data and analysis scripts.
6. Wiring diagrams, component inventory and safety notes.
7. Experiment records, photos/videos and final report.

## 9. Current Status and Constraints

The project is in the preparation and hardware-verification stage. The GitHub repository and initial planning documents have been created. The currently identified hardware includes a YD-ESP32-S3 Classic V1.0 development board, an ESP32-S3-WROOM-1 module marking (full suffix not yet identified), a user-confirmed DHT11 module, and a USB fan.

The fan is rated 5 V / 1 A according to information the user previously confirmed with the seller; these are seller-provided/rated values, not measurements made in this project. Startup/stall current, internal speed-control electronics and external PWM compatibility remain unverified. The exact ESP32 module variant, board pin mapping, remaining kit inventory and motor-driver availability also require verification. Therefore, the final circuit, bill of materials and speed-control method are not yet fixed. No fan load should be connected to an ESP32 GPIO.

## 10. Acceptance Principle

A function will be marked complete only after implementation and verification. Planned functions, simulated dashboard data and actual sensor measurements will be clearly distinguished. No measurement, model metric or performance claim will be invented.
