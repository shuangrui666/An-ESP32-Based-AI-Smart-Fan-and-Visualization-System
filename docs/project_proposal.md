# SmartFan 2.0 Project Proposal

## 1. Project Background

Traditional fans usually rely on manual operation or simple control mechanisms. They provide limited environmental awareness, intelligent decision-making and remote monitoring capabilities.

With the development of the Internet of Things (IoT), embedded systems and artificial intelligence, it is possible to develop more intelligent and interactive fan systems.

SmartFan 2.0 aims to explore the integration of ESP32, environmental sensors, machine learning and web-based visualization to build an intelligent fan prototype.

## 2. Project Objectives

The project aims to:

1. Build an ESP32-based smart fan prototype.
2. Collect environmental temperature and humidity data.
3. Detect human movement using a motion sensor.
4. Implement automatic fan control.
5. Develop a Python-based machine learning model for fan operating-level recommendations.
6. Build a web dashboard for real-time monitoring and remote control.
7. Visualize historical sensor data and AI prediction results.
8. Evaluate system performance through practical experiments.

## 3. Proposed Functions

### 3.1 Environmental Monitoring
Collect and display temperature and humidity data.

### 3.2 Human Activity Detection
Use a PIR sensor to detect human movement and support automatic control strategies.

### 3.3 Intelligent Fan Control
Adjust fan operation according to environmental conditions and user preferences.

### 3.4 AI-Based Decision Support
Use a Decision Tree Classifier implemented with Python and scikit-learn to recommend fan operating levels.

### 3.5 Remote Control
Provide a mobile-friendly web interface for manual and automatic fan operation.

### 3.6 Data Visualization
Display real-time sensor readings, historical trends, fan operating states and AI model evaluation results.

## 4. Technical Approach

The proposed system consists of four layers:

- Hardware layer: ESP32, sensors, fan and motor driver.
- Communication layer: Wi-Fi and HTTP.
- AI and backend layer: Python, Flask and scikit-learn.
- Visualization layer: HTML, CSS, JavaScript and ECharts.

The Windows computer will initially run the AI model. The ESP32 will execute the received control commands.

A local rule-based control mechanism is planned as a fallback when the computer or network is unavailable.

## 5. Innovation and Project Characteristics

1. Combines embedded systems, IoT and machine learning.
2. Integrates real hardware control with a web visualization dashboard.
3. Supports both automatic and manual operating modes.
4. Records experimental data for analysis and evaluation.
5. Uses an evidence-based development process.

These are intended project characteristics and will be evaluated through implementation and testing.

## 6. Experimental Plan

The planned experiments include:

- Temperature and humidity measurement tests.
- Human activity detection tests.
- Fan control and operating-level tests.
- Wi-Fi communication tests.
- AI model evaluation.
- Dashboard data accuracy and update tests.
- System stability tests.

Experimental results will be recorded and analyzed.

## 7. Budget and Resource Constraints

The project will prioritize the use of available ESP-Claw kit components and borrowed laboratory equipment.

Additional purchases will be made only after checking the existing hardware inventory and verifying the fan's electrical specifications.

The project will follow the applicable component cost limit set by the instructor.

## 8. Expected Deliverables

1. A working SmartFan 2.0 prototype.
2. ESP32 firmware source code.
3. Python backend and machine learning code.
4. A responsive web visualization dashboard.
5. Experimental datasets and analysis charts.
6. Hardware documentation and wiring diagrams.
7. Project report and demonstration materials.

## 9. Current Status

The project is currently in the preparation and hardware verification stage.

The system architecture and initial technical approach have been planned. The fan electrical specifications and available kit components still require confirmation.

Functions will be marked as completed only after implementation and verification.
