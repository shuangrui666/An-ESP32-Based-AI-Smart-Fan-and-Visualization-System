# SmartFan 2.0 — System Architecture

**Project:** An ESP32-Based AI Smart Fan and Visualization System  
**Document:** System Architecture Design  
**Version:** 1.0  
**Status:** Preliminary Design  
**Last Updated:** November 6, 2026

---

## 1. Introduction

### 1.1 Purpose

This document describes the overall architecture of SmartFan 2.0, an ESP32-based smart fan system integrating environmental sensing, AI-assisted decision-making, remote control, and real-time visualization.

It defines the responsibilities of hardware and software modules, communication methods, data flow, and system control logic.

This document represents the preliminary system design. Hardware specifications and implementation details will be updated after the actual components have been inspected and tested.

### 1.2 System Objectives

The system is designed to:

- Collect environmental data through sensors.
- Monitor temperature, humidity, and human activity when supported by the installed sensors.
- Use a machine learning model to recommend fan operation levels.
- Allow users to control the fan remotely.
- Display real-time measurements and historical data.
- Provide a local fallback control strategy when network communication is unavailable.

---

## 2. Overall System Architecture

SmartFan 2.0 is designed with five main layers:

1. Hardware Layer
2. Embedded Firmware Layer
3. Communication Layer
4. Backend and AI Layer
5. Visualization Layer

### 2.1 System Architecture Diagram

![Overall System Architecture](images/system_architecture.png)

**Figure 1. Overall System Architecture**

The ESP32 acts as the embedded controller and sensor data collection unit. It communicates with the backend through Wi-Fi.

The backend processes incoming data, executes the machine learning model, and generates fan control recommendations.

The dashboard provides real-time monitoring, historical visualization, and remote control functions.

---

## 3. Hardware Architecture

### 3.1 Main Hardware Components

| Component | Function | Status |
|---|---|---|
| ESP32 development board | Main embedded controller | Awaiting kit inspection |
| Environmental sensors | Temperature and humidity measurement | Exact models pending verification |
| PIR sensor | Detects movement | Availability pending verification |
| USB-powered fan | Airflow output device | Purchased; electrical specifications pending |
| Motor driver or switching circuit | Controls fan power | Selection pending electrical verification |
| Power supply | Provides stable power | To be verified |
| Connecting wires | Hardware connections | Pending inspection |

### 3.2 Hardware Architecture Diagram

![Hardware Architecture](images/hardware_architecture.png)

**Figure 2. Hardware Architecture**

The ESP32 will collect sensor readings and control the fan through a suitable driver or switching circuit.

The fan must not be connected directly to an ESP32 GPIO pin.

The motor driver and power arrangement must be selected only after verifying the fan's rated voltage, operating current, startup current, and control compatibility.

### 3.3 Hardware Safety Considerations

- Do not connect the fan motor directly to an ESP32 GPIO.
- Verify the fan's electrical specifications before selecting a driver.
- Ensure that the power supply can provide sufficient current.
- Verify voltage compatibility between the ESP32, sensors, and driver.
- Test the fan independently before integrating automatic control.

---

## 4. Embedded Firmware Architecture

### 4.1 ESP32 Responsibilities

The ESP32 firmware is responsible for:

- Initializing connected sensors.
- Reading environmental measurements.
- Monitoring supported activity signals.
- Connecting to the Wi-Fi network.
- Sending sensor data to the backend.
- Receiving fan control commands.
- Applying safe local fallback rules when communication fails.

### 4.2 Firmware Modules

The planned firmware will be organized into the following modules:

| Module | Responsibility |
|---|---|
| Sensor Module | Read sensor measurements |
| Wi-Fi Module | Establish network connection |
| Communication Module | Exchange JSON messages |
| Fan Control Module | Execute verified fan control commands |
| Safety Module | Handle abnormal conditions |
| Local Control Module | Provide fallback behavior |

---

## 5. Communication Architecture

### 5.1 Communication Method

The preliminary communication method is Wi-Fi with HTTP and JSON.

The ESP32 will communicate with a Python backend running on a computer connected to the same network.

The exact network configuration will be confirmed during implementation.

### 5.2 Data Flow

1. Sensors provide measurements to the ESP32.
2. The ESP32 packages the readings into a data message.
3. The backend receives and validates the message.
4. The AI module generates a fan-level recommendation.
5. The backend sends the control result to the ESP32.
6. The ESP32 executes the command if it passes safety checks.
7. The dashboard displays the latest system state.

### 5.3 Example JSON Message

The following is an illustrative message format, not actual experimental data:

```json
{
  "temperature": 28.5,
  "humidity": 65.0,
  "motion_detected": true,
  "device_id": "smartfan_01"
}
```

The actual message fields will depend on the installed sensors and firmware implementation.

---

## 6. Backend Architecture

### 6.1 Technology

The planned backend technology is:

- Python
- Flask
- JSON
- scikit-learn

### 6.2 Backend Responsibilities

The backend is designed to:

- Receive sensor data from the ESP32.
- Validate and process incoming measurements.
- Store historical records.
- Call the AI prediction module.
- Generate control recommendations.
- Provide data and control APIs for the dashboard.

### 6.3 Planned API Endpoints

| Endpoint | Method | Purpose |
|---|---|---|
| `/api/sensor-data` | POST | Receive sensor readings |
| `/api/status` | GET | Return current system status |
| `/api/predict` | POST | Generate an AI recommendation |
| `/api/control` | POST | Submit a manual control command |
| `/api/history` | GET | Retrieve historical data |

These endpoints are planned interfaces and are not claimed to be implemented yet.

---

## 7. AI Decision-Making Architecture

### 7.1 Machine Learning Model

The preliminary model is a Decision Tree Classifier implemented using scikit-learn.

A decision tree is selected because it is relatively easy to understand, visualize, and explain.

### 7.2 Input Features

Potential input features include:

- Temperature
- Humidity
- Motion detection state
- User preference, if collected

Only features supported by actual hardware and recorded data will be used.

### 7.3 Output Classes

The planned output classes are:

- OFF
- LOW
- MEDIUM
- HIGH

The available output levels depend on the verified fan and driver capabilities.

### 7.4 Model Evaluation

The model will be evaluated using a separate test dataset.

Evaluation may include:

- Accuracy
- Precision
- Recall
- F1-score
- Confusion matrix

No model performance values will be reported until actual training and testing have been completed.

### 7.5 Training Data

Training labels must be based on documented control rules or recorded user preferences.

They must not be presented as objectively measured comfort labels unless supported by an appropriate measurement method.

---

## 8. Control Logic

The system is designed to support three operating modes.

### 8.1 Automatic Mode

1. The ESP32 collects available sensor readings.
2. The backend processes the data.
3. The AI model recommends a fan level.
4. The ESP32 validates and executes the command.
5. The dashboard displays the recommendation and control state.

### 8.2 Manual Mode

Users can select a supported fan operating level through the dashboard.

Manual commands must pass the ESP32 safety checks before execution.

### 8.3 Offline Mode

If the backend or Wi-Fi connection becomes unavailable, the ESP32 should switch to a predefined local fallback strategy.

The fallback behavior will be implemented and tested after the hardware and control capabilities are confirmed.

---

## 9. Visualization Architecture

### 9.1 Frontend Technology

The planned frontend technologies are:

- HTML
- CSS
- JavaScript
- ECharts

### 9.2 Dashboard Features

The dashboard is planned to provide:

**Real-Time Monitoring**
- Temperature
- Humidity
- Motion detection state, if supported
- Fan commanded operating level
- Automatic or manual mode
- Device connection status
- AI recommendation

**Historical Visualization**
- Temperature time-series chart
- Humidity time-series chart
- Fan control-level history
- Motion event timeline

**AI Visualization**
- Decision tree visualization
- Prediction results
- Model evaluation charts
- Prediction versus recorded labels

**Remote Control**
- Fan ON/OFF
- Automatic/manual mode switching
- Supported fan-level selection

Actual fan RPM will only be displayed if a suitable speed sensor is installed and verified. Otherwise, the dashboard will display the commanded fan level rather than measured rotational speed.

### 9.3 Visualization Workflow

![Software Workflow](images/software_workflow.png)

**Figure 3. Software and Data Workflow**

---

## 10. Data Management

The system will maintain records of:

- Sensor measurements
- Timestamps
- Fan control commands
- AI recommendations
- Operating modes
- Device connection status
- Model training and evaluation records

Data should use consistent field names and timestamps.

The project should distinguish between:

- Simulated data
- Real sensor data
- Training data
- Testing data

Simulated data must be clearly labeled and must not be presented as real experimental measurements.

---

## 11. Reliability and Safety

The following measures are planned:

- Validate sensor readings before processing.
- Handle Wi-Fi disconnections.
- Handle backend communication failures.
- Apply safe control limits.
- Prevent unsupported fan commands.
- Verify hardware power requirements.
- Record errors for troubleshooting.

The actual safety behavior will be documented after implementation and testing.

---

## 12. Current Development Status

| Subsystem | Current Status |
|---|---|
| USB fan procurement | Completed |
| Fan electrical specification verification | Pending |
| ESP32 board inspection | Pending kit collection |
| Sensor identification | Pending kit inspection |
| Motor driver selection | Pending electrical verification |
| Firmware development | Planned |
| Backend development | Planned |
| AI model development | Planned |
| Dashboard development | Planned |
| Hardware integration | Planned |
| System testing | Planned |

**Important:** A planned feature is not considered completed until it has been implemented and verified.

---

## 13. Future Improvements

Potential future improvements include:

- More advanced machine learning models.
- Improved sensor calibration.
- Better control strategies.
- Model retraining with additional real-world data.
- Enhanced dashboard interactions.
- Long-term system performance analysis.

These improvements will be considered after the basic system has been implemented and tested.

---

## 14. Conclusion

SmartFan 2.0 adopts a layered architecture combining embedded sensing, Wi-Fi communication, Python-based backend processing, machine learning recommendations, and web-based visualization.

The design aims to provide an understandable, extensible, and testable smart fan system.

The architecture will be revised as actual hardware specifications, implementation results, and experimental evidence become available.
