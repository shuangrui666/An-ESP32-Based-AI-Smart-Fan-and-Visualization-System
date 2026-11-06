# SmartFan 2.0
**An ESP32-Based AI Smart Fan and Visualization System**

> **Project stage:** Preparation and hardware verification  
> **Implementation status:** Planned functions are not yet verified on the physical prototype.

## 1. Project Overview

SmartFan 2.0 is a planned smart-fan prototype combining an ESP32 controller, environmental sensing, a Python-based machine-learning service and a responsive web dashboard.

The project aims to investigate whether an ordinary USB-powered fan can be upgraded with environmental monitoring, activity-aware control, AI-assisted operating-level recommendations, Wi-Fi communication and data visualization.

The project will document design decisions, implementation, experiments, limitations and revisions. Features will only be described as completed after they have been implemented and tested.

## 2. Project Objectives

1. Build an ESP32-based control prototype.
2. Measure environmental temperature and, if supported by the available sensor, humidity.
3. Detect movement using a PIR sensor if one is available in the kit or can be borrowed.
4. Implement safe fan control after verifying the fan's electrical characteristics and selecting a compatible driver.
5. Develop a Python machine-learning model to recommend fan operating levels.
6. Exchange sensor data and control commands over Wi-Fi.
7. Build a mobile- and desktop-friendly monitoring dashboard.
8. Record historical data and visualize system behavior.
9. Evaluate the prototype with repeatable experiments and report limitations honestly.

## 3. Planned Features and Status

| Feature | Planned behavior | Status |
|---|---|---|
| Environmental monitoring | Display measured temperature and humidity when supported by the selected sensor | Planned |
| Motion detection | Detect movement with a compatible PIR sensor, if available | Pending hardware confirmation |
| Fan control | Control OFF/LOW/MEDIUM/HIGH levels if the fan and driver support reliable control | Pending electrical verification |
| AI recommendation | Python model recommends a fan operating level | Planned |
| Wi-Fi communication | Exchange data and commands between ESP32 and computer | Planned |
| Web dashboard | Show current readings, device status and control mode | Planned |
| Historical charts | Plot sensor readings, activity events and fan command history | Planned |
| AI visualization | Show decision-tree structure and evaluation results after model training | Planned |
| Offline fallback | ESP32 uses a documented local rule when the computer/network is unavailable | Planned |

## 4. Proposed System Architecture

The initial architecture has four layers.

### 4.1 Hardware and sensing

Candidate components include:
- ESP32 development board
- Purchased USB-powered fan
- Temperature/humidity sensor available in the kit
- PIR motion sensor, if available
- Motor driver selected only after fan voltage and current are verified
- Optional display or RGB indicator if included in the kit

The fan is advertised as USB-powered, and the seller has described it as using a brushed motor. Its rated voltage, operating current, startup current and exact motor construction still need verification. USB power must not be treated as proof of a confirmed electrical rating.

### 4.2 Firmware and communication

The ESP32 is planned to read sensors, report device state and receive control commands over Wi-Fi/HTTP. The exact communication protocol and update interval will be confirmed during implementation.

### 4.3 Python backend and AI

A Windows computer will initially run a Python service, planned with Flask, and a scikit-learn Decision Tree Classifier.

Potential inputs include measured temperature, humidity (if available), motion state (if available) and an explicitly recorded user preference. The model may recommend OFF, LOW, MEDIUM or HIGH.

Training labels must come from a documented rule set or actual user-labeled observations. The project will not describe rule-generated labels as independently measured comfort or claim model accuracy before evaluation. The ESP32 will retain a simple local fallback rule for loss of computer/network connectivity.

### 4.4 Web visualization

A responsive web dashboard is planned with HTML, CSS, JavaScript and ECharts. It will display current sensor values, connection state, operating mode, commanded fan level, historical trends and AI recommendations.

If no tachometer or speed sensor is installed, the dashboard will display the **commanded level**, not claim to show measured RPM.

## 5. Visualization Scope

### 5.1 Real-time dashboard
- Temperature and humidity readings, where supported
- Device connection state
- Motion state, if a PIR sensor is used
- Automatic/manual mode
- Fan commanded level and operating state
- AI-recommended level

### 5.2 Historical charts
- Temperature and humidity time series
- Fan command-level timeline
- Motion-event timeline
- Comparison of AI recommendations and executed commands

### 5.3 AI model analysis
- Decision-tree visualization
- Confusion matrix
- Accuracy and other suitable metrics
- Prediction-versus-reference comparison

Metrics will be calculated from a clearly described test set. Results will not be fabricated or inferred from training performance alone.

### 5.4 Remote control
- Fan ON/OFF
- Automatic/manual mode selection
- Manual level selection, only if supported by the verified hardware

## 6. Planned Technology Stack

| Area | Planned technology |
|---|---|
| Microcontroller | ESP32 (exact board to be identified) |
| Firmware | C++ / Arduino framework, subject to board compatibility |
| AI | Python, scikit-learn |
| Backend | Flask |
| Communication | Wi-Fi / HTTP |
| Frontend | HTML, CSS, JavaScript |
| Charts | ECharts |
| Data processing | pandas, NumPy |
| Offline analysis | Matplotlib |
| Version control | Git and GitHub |

## 7. Development Stages

1. Project documentation and hardware inventory.
2. Fan and kit verification; confirm power and driver requirements.
3. Sensor and fan-control bench tests.
4. ESP32 firmware and Wi-Fi communication.
5. Python backend and data logging.
6. Dashboard using simulated data first, then verified live data.
7. Dataset preparation, AI training and evaluation.
8. Integration, fallback behavior and system tests.
9. Final experiments, report, demonstration and repository review.

Detailed tasks and progress are tracked in [docs/project_schedule.md](docs/project_schedule.md).

## 8. Repository Structure

```text
.
├── README.md
├── docs/
│   ├── project_proposal.md
│   ├── project_schedule.md
│   └── system_architecture.md
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

Directories will be added as real work products are created; empty folders and unverified deliverables will not be used to imply progress.

## 9. Safety and Engineering Principles

- Never connect a motor directly to an ESP32 GPIO.
- Verify fan voltage, running current and startup current before choosing a driver or power arrangement.
- Ensure the motor supply and ESP32 share an appropriate reference ground when required by the driver circuit.
- Disconnect power before changing wiring.
- Do not modify or puncture batteries.
- Distinguish measured values from commanded values.
- Preserve original experimental data and document processing steps.
- Clearly label planned, pending, tested and completed functions.
- Do not fabricate measurements, images, test results or model metrics.

## 10. Current Project Status

### Completed
- Initial project concept and technical direction documented.
- GitHub repository created.
- Initial README, project proposal and development schedule drafted.
- Preliminary system architecture document created; hardware-specific details remain pending verification.
- USB-powered fan purchased.

### Pending verification
- Fan label/specifications: rated voltage, current, startup current and motor details.
- ESP-Claw kit inventory and exact ESP32 board model.
- Available sensors, motor driver, wiring materials and laboratory tools.
- Applicable additional-component budget limit.

### Not yet implemented
- Hardware wiring and fan driver
- ESP32 firmware
- Python backend and AI model
- Web dashboard and live data integration
- Experimental evaluation

## 11. Evidence-Based Development

This project follows an evidence-based engineering approach. Design choices, source code, measurements, visualizations and performance claims will be supported by implementation records or experiments. Unverified functions remain planned or pending until successfully tested.
