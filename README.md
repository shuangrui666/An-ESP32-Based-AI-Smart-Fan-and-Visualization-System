# SmartFan 2.0
**An ESP32-Based AI Smart Fan and Visualization System**

> **Project stage:** Hardware received; inventory and electrical verification in progress  
> **Implementation status:** Planned functions are not yet verified on the physical prototype.

## 1. Project Overview

SmartFan 2.0 is an innovative smart-fan prototype combining an ESP32 controller, environmental sensing, a Python-based machine-learning service and a responsive web dashboard.

The project investigates how an ordinary USB-powered fan can be upgraded with environmental monitoring, activity-aware control, AI-assisted operating-level recommendations, Wi-Fi communication and data visualization. The project is being developed as an innovative response to the course project requirements. Its evidence package maps the teacher's four required deliverable categories to the actual SmartFan prototype and its documented development process.

The project will document design decisions, implementation, experiments, limitations and revisions. Features will only be described as completed after they have been implemented and tested. Hardware structures, chips, circuit connections and test results will be described only when supported by photographs, component markings, datasheets or measurements.

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
10. Produce traceable evidence for hardware inspection, circuit analysis, cost accounting, upgrade design and physical verification.

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

## 4. Course Project Requirements and Deliverable Mapping

The course brief lists four required process-data categories. SmartFan 2.0 addresses them through an innovative prototype-upgrade approach. This mapping describes the evidence to be produced; it does not claim that uncompleted experiments have already been performed.

| Teacher's required deliverable | SmartFan 2.0 evidence and planned output | Evidence location |
|---|---|---|
| 1. Physical structure / exploded-view documentation and step-by-step photographs (15 points) | Photograph the purchased fan, ESP32 board, sensors, driver and assembled prototype; document actual safe disassembly/inspection where applicable; create a clearly labeled system hardware structure/exploded-view diagram. Do not depict an internal component that has not been observed. | [evidence/course_project_05/](evidence/course_project_05/README.md), [diagrams/course_project_05/](diagrams/course_project_05/README.md) |
| 2. Chip-marking identification and power/control block diagram (15 points) | Photograph readable chip markings on the actual ESP32 board and other accessible modules; identify parts using official datasheets; document verified power paths and control-signal paths in block diagrams. Unknown markings or paths remain pending verification. | [docs/course_project_05/](docs/course_project_05/README.md), [evidence/course_project_05/](evidence/course_project_05/README.md), [diagrams/course_project_05/](diagrams/course_project_05/README.md) |
| 3. Batch BOM cost analysis (10 points) | Inventory actual components and quantities; record model/specification, supplier, public bulk-price source, quotation date, unit and calculation method; separate purchased, borrowed and already-owned items. | [data/](data/README.md) |
| 4. Smart upgrade proposal (20 points) | Document the original fan interface as actually verified, proposed power/control connection points, incremental components within the teacher's stated budget, control-state diagram, and physical wiring/test evidence. Actual connection points must be verified before wiring. | [docs/course_project_05/](docs/course_project_05/README.md), [data/](data/README.md), [diagrams/course_project_05/](diagrams/course_project_05/README.md), [evidence/course_project_05/](evidence/course_project_05/README.md) |

**Important scope and evidence note:** The purchased USB fan is the selected project device for the approved innovative upgrade approach. It must not be described as a second-hand appliance if it is new. If it has no accessible original control board or identifiable controller chip, the report will state that limitation and identify the actual ESP32 controller and added driver circuit instead. A system-level hardware structure diagram is not evidence of an internal fan teardown. Any physical disassembly will be performed only if the device can be opened safely and without damaging or misrepresenting it.

The teacher's brief specifies a **20-yuan limit for incremental components** in the upgrade proposal. The cost table will explicitly distinguish the incremental upgrade budget from the full prototype BOM. The interpretation of the budget (per device, total added parts, or another rule) should be confirmed with the teacher if unclear.

## 5. Proposed System Architecture

The initial architecture has four layers.

### 5.1 Hardware and sensing

Candidate components include:
- ESP32 development board
- Purchased USB-powered fan
- Temperature/humidity sensor available in the kit
- PIR motion sensor, if available
- Motor driver selected only after fan voltage, current, startup behavior and control method are verified
- Optional display or RGB indicator if included in the kit

The purchased fan's rated voltage and rated current have been confirmed as 5 V and 1 A, and the seller described it as a brushed DC motor. Startup current, internal control circuitry and reliable external speed-control compatibility remain to be verified. The rated current must not be treated as the startup current.

### 5.2 Firmware and communication

The ESP32 is planned to read sensors, report device state and receive control commands over Wi-Fi/HTTP. The exact communication protocol and update interval will be confirmed during implementation.

### 5.3 Python backend and AI

A Windows computer will initially run a Python service, planned with Flask, and a scikit-learn Decision Tree Classifier.

Potential inputs include measured temperature, humidity (if available), motion state (if available) and an explicitly recorded user preference. The model may recommend OFF, LOW, MEDIUM or HIGH.

Training labels must come from a documented rule set or actual user-labeled observations. The project will not describe rule-generated labels as independently measured comfort or claim model accuracy before evaluation. The ESP32 will retain a simple local fallback rule for loss of computer/network connectivity.

### 5.4 Web visualization

A responsive web dashboard is planned with HTML, CSS, JavaScript and ECharts. It will display current sensor values, connection state, operating mode, commanded fan level, historical trends and AI recommendations.

If no tachometer or speed sensor is installed, the dashboard will display the **commanded level**, not claim to show measured RPM.

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

1. Hardware inventory, device profile and safety assessment.
2. Photograph and document actual hardware structure; identify accessible chips and modules.
3. Verify electrical specifications and draw evidence-based power/control diagrams.
4. Prepare component inventory and BOM cost analysis with dated supplier sources.
5. Design the upgrade interfaces, incremental-component budget and control-state diagram.
6. Conduct safe sensor and fan-driver bench tests; preserve wiring and test evidence.
7. Implement ESP32 firmware and Wi-Fi communication.
8. Develop the Python backend and data logging.
9. Build the dashboard using simulated data first, then verified live data.
10. Prepare the dataset, train and evaluate the AI model.
11. Integrate, test fallback behavior and review the complete evidence package.

Detailed tasks and progress are tracked in [docs/project_schedule.md](docs/project_schedule.md). The course-specific evidence mapping and working documents are indexed in [docs/course_project_05/](docs/course_project_05/README.md).

## 8. Repository Structure

~~~text
.
├── README.md
├── docs/
│   ├── project_proposal.md
│   ├── project_schedule.md
│   ├── system_architecture.md
│   └── course_project_05/
├── docs/images/
├── experiment/
│   ├── README.md
│   ├── experiment_log.md
│   ├── hardware_inventory.md
│   ├── hardware_connection_log.md
│   ├── software_debug_log.md
│   ├── ai_experiment_log.md
│   ├── system_test_log.md
│   └── records/
├── evidence/
│   └── course_project_05/
├── data/
├── diagrams/
│   └── course_project_05/
├── hardware/
├── firmware/
├── backend/
├── ai/
├── dashboard/
├── visualization/
└── ai_collaboration/
~~~

Directories are populated as real work products are created. Their README index files explain expected evidence; they do not imply that experiments or deliverables are already complete.

## 9. Experiment Records and Evidence Management

Every practical activity must have a unique experiment ID and a contemporaneous record. Use the template in [experiment/records/EXPERIMENT_RECORD_TEMPLATE.md](experiment/records/EXPERIMENT_RECORD_TEMPLATE.md).

For each experiment:
- Record the actual date, start/end time, purpose, equipment, conditions and pre-defined success criteria.
- Capture before/during/after photographs or video where relevant. Keep original files unchanged.
- Record actual readings, units, instrument/source and measurement conditions. Distinguish measured, rated, seller-provided, datasheet-derived and estimated values.
- Link each conclusion to its supporting evidence filename and repository path.
- Preserve failures, unexpected results, troubleshooting attempts and design changes.
- Keep raw data separate from processed data; record scripts, parameters and input files.
- Use planned, in_progress, completed, failed or pending_verification accurately.
- Do not claim a test passed unless the recorded success criteria and evidence support that conclusion.

Evidence naming convention: EXP-NNN-01-short-description.ext. Use the actual experiment date in record-folder names. For large videos stored outside GitHub, retain a manifest entry containing the original filename, file size if known, capture date, description, storage location and access note.

See [experiment/README.md](experiment/README.md) for the full rules and [evidence/course_project_05/README.md](evidence/course_project_05/README.md) for the course evidence folder guide.

## 10. Safety and Engineering Principles

- Never connect a motor directly to an ESP32 GPIO.
- Verify fan voltage, running current, startup current and internal control behavior before choosing a driver or power arrangement.
- Ensure the motor supply and ESP32 share an appropriate reference ground when required by the driver circuit.
- Disconnect power before changing wiring.
- Do not modify or puncture batteries.
- Do not open mains-powered equipment or lithium battery packs as a beginner exercise.
- Distinguish measured values from commanded values.
- Preserve original experimental data and document processing steps.
- Clearly label planned, pending, tested and completed functions.
- Do not fabricate measurements, images, test results, chip identities, supplier quotations or model metrics.

## 11. Current Project Status

### Completed
- Initial project concept and technical direction documented.
- GitHub repository created.
- Initial README, project proposal and development schedule drafted.
- Preliminary system architecture document created; hardware-specific details remain pending verification.
- Purchased USB fan received; rated 5 V / 1 A and brushed-motor description recorded from prior seller confirmation.
- ESP-Claw kit received; detailed component inventory and exact board/module identification are still in progress.
- ESP32-S3 development environment established with VS Code and PlatformIO; firmware Build and Upload verified on COM5.
- DHT11 sensor read verified on GPIO4: independent test reported 22/22 successful reads; after fixing the missing DHT initialization call in the main firmware, the main program reported 25.0 °C and 42.0% RH in consecutive serial outputs. See [EXP-001](experiment/records/2026-11-11/EXP-001-esp32-dht11-validation.md).
- Experiment log and record templates created.

### In progress
- Physical inventory and photography of the received kit and fan.
- Verification of ESP32 board model, sensor models and available driver/power components.
- Verification of fan startup current, internal control circuitry and external control compatibility.
- Course project evidence mapping and experiment-record improvements.

### Not yet implemented
- Verified fan-driver wiring and fan power/control circuit
- Temperature-based automatic fan control
- Wi-Fi communication
- Python backend and AI model
- Web dashboard and live data integration
- Experimental evaluation and final course evidence package

## 12. Evidence-Based Development

This project follows an evidence-based engineering approach. Design choices, source code, measurements, visualizations and performance claims will be supported by implementation records or experiments. Unverified functions remain planned or pending until successfully tested.
