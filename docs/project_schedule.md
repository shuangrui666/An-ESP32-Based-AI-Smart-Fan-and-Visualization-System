# SmartFan 2.0 — Development Schedule

## 1. Schedule Principles

The project will proceed through verification gates. Hardware-dependent work must not move forward on unverified assumptions. Dates may be added after the instructor confirms the deadline and the kit inventory is known.

Status definitions:
- **Completed:** output exists and has been checked.
- **In progress:** work has started and evidence exists.
- **Pending:** not yet started.
- **Blocked:** cannot proceed until a dependency is resolved.

## 2. Development Stages

| Stage | Main tasks | Verifiable output | Current status |
|---|---|---|---|
| 1. Project documentation | Create repository, README, proposal, schedule and preliminary system architecture | Planning documents in GitHub | Completed (preliminary design) |
| 2. Hardware inventory | Inspect ESP-Claw kit; identify board, sensors, driver, wires and tools | Dated inventory with photos/model numbers | In progress (board, DHT11, PIR, fan and MOSFET identified; full inventory/electrical details pending) |
| 3. Fan verification | Record label and seller specifications; measure/check electrical behavior with suitable equipment | Fan specification and verification record | In progress (basic MOSFET switching/full-duty run observed; current/startup/electrical measurements and PWM levels pending) |
| 4. Hardware proof of concept | Test ESP32 and sensors separately; select driver only after fan verification | Wiring record and bench-test results | In progress (DHT11, PIR signal and independent fan basic operation tested; integrated proof pending) |
| 5. Firmware and communication | Read sensors, report status and receive commands over Wi-Fi | ESP32 source code and communication test | In progress (sensor upload and remote-command software implemented; physical command/ACK integration pending) |
| 6. Backend and data logging | Build Python/Flask service and store incoming readings | Backend source code and sample log | In progress (Flask APIs and command queue implemented/tested; in-memory persistence limitation remains) |
| 7. Dashboard | Build responsive interface; first use clearly labeled simulated data, then verified live data | Dashboard source and screenshots/test record | In progress (sensor dashboard and control UI/status polling implemented; physical control result not verified) |
| 8. AI model | Define labels, prepare dataset, train and evaluate decision tree | Dataset description, model and evaluation report | Pending (recommendation remains rule-based) |
| 9. Integration and testing | Connect hardware, backend, model and dashboard; test fallback | Integrated test records | In progress (component/software tests exist; end-to-end and fault-recovery tests pending) |
| 10. Final delivery | Organize code, data, diagrams, photos/videos and report | Reviewed repository and presentation | In progress (repository documentation/evidence reconciliation underway) |

## 3. Detailed Tasks and Dependencies

### Stage 1 — Project Documentation
- [x] Create the GitHub repository.
- [x] Create the initial README.
- [x] Create the project proposal.
- [x] Create this development schedule.
- [x] Review the main planning-document paths and consistency.
- [x] Create the preliminary system architecture document.
- [ ] Revise the hardware-specific architecture after the ESP-Claw kit and fan specifications are verified.

### Stage 2 — Hardware Inventory
- [ ] Inspect and photograph the complete ESP-Claw kit.
- [ ] Record the exact ESP32 board model and USB cable type.
- [x] Identify the DHT11 and confirm PIR module as HC-SR501 in the EXP-009 record; detailed electrical characteristics remain to be checked.
- [ ] Identify available motor driver modules and their ratings.
- [ ] Record breadboard, jumper wires and connectors.
- [ ] Confirm which laboratory tools can be borrowed.

### Stage 3 — Fan and Power Verification
- [ ] Photograph the fan label, connector and packaging.
- [ ] Obtain rated voltage/current and startup-current information if available.
- [ ] Confirm whether the fan contains a battery or internal control electronics.
- [ ] Verify operating behavior using suitable equipment and supervision.
- [ ] Choose the driver and power arrangement only after the above checks.

### Stage 4 — Hardware Proof of Concept
- [ ] Test ESP32 board and data cable.
- [x] Test DHT11 and PIR signal acquisition independently (EXP-001 and EXP-009); calibration and regression tests remain pending.
- [ ] Test the selected motor driver without connecting an incompatible load.
- [x] Observe basic fan startup and continuous full-duty operation in the independent test (EXP-012); intermediate PWM levels and electrical measurements remain pending.
- [ ] Record current, temperature and observed stability where instruments are available.

### Stage 5 — Firmware and Communication
- [x] Implement sensor acquisition.
- [x] Define and use the sensor HTTP data interface.
- [x] Implement Wi-Fi communication and verify one-way upload (EXP-004).
- [ ] Test command reception and safe output states.
- [ ] Implement local fallback behavior.

### Stage 6 — Backend and Data Logging
- [ ] Set up the Python environment.
- [x] Build the Flask service and API endpoints.
- [x] Implement and test the command queue (Phase 3.2.1; 22 reported tests passed).
- [ ] Store timestamped readings and command history.
- [ ] Handle missing data and communication errors.

### Stage 7 — Visualization Dashboard
- [x] Create responsive page layout.
- [x] Display current sensor readings and connection state.
- [x] Display mode and commanded fan level in the control UI.
- [x] Add temperature/humidity time-series charts where sensor data exists.
- [ ] Add motion-event and fan-command timelines where applicable.
- [ ] Display AI recommendation and its timestamp/source.
- [x] Implement manual/automatic controls and command-status polling in software; physical command-to-fan response remains to be verified.
- [ ] Verify displayed values against backend records.

### Stage 8 — AI Model
- [ ] Define the model's intended task and output labels.
- [ ] Document whether labels come from explicit rules or user observations.
- [ ] Collect and clean a dataset.
- [ ] Separate training and test data before evaluation.
- [ ] Train a baseline Decision Tree Classifier.
- [ ] Report suitable metrics and a confusion matrix.
- [ ] Visualize the tree and prediction results.
- [ ] Connect predictions to the control path only after validation.

### Stage 9 — Integration and Testing
- [ ] Integrate ESP32, backend, AI and dashboard.
- [ ] Test automatic and manual modes.
- [ ] Test network loss and recovery.
- [ ] Test local fallback behavior.
- [ ] Test long-duration stability.
- [ ] Record defects, fixes and remaining limitations.

### Stage 10 — Final Delivery
- [ ] Review repository structure and links.
- [ ] Preserve raw data and analysis scripts.
- [ ] Add wiring diagrams and component records.
- [ ] Add real prototype photos and demonstration videos.
- [ ] Complete final report and presentation.
- [ ] Check that all completion claims have supporting evidence.

## 4. Current Progress Snapshot

**Updated: 2026-11-25.** This table supersedes the original early-stage snapshot above.

| Item | Current status | Evidence / remaining action |
|---|---|---|
| GitHub repository and documentation | In progress | Main code, website, hardware records, experiments and evidence are present; final consistency and branch-history review ongoing |
| ESP32 board / firmware environment | Established | YD-ESP32-S3 Classic V1.0 and PlatformIO environments recorded |
| DHT11 acquisition | Completed (single test) | EXP-001: 22/22 reads; regression and long-term stability remain |
| PIR signal acquisition | Completed (initial signal test) | EXP-009: GPIO5 HIGH/LOW observations; calibration remains |
| Wi-Fi sensor upload | Completed (historical one-way test) | EXP-004: Flask received HTTP 200; reconnect/stability remain |
| Fan driver basic operation | Completed (independent full-duty test) | EXP-012: basic switching and 100% continuous run; intermediate PWM and electrical measurements remain |
| Firmware remote-command software | Implemented / compile-reported | Phase 3.2.2; physical web-to-fan verification remains |
| Flask backend command queue | Completed (software tests reported) | Phase 3.2.1: 22 passed, 0 failed; queue is in-memory |
| Web dashboard/control UI | Implemented in software | Sensor display, mode/gear controls and command polling; physical fan response remains |
| Integrated automatic fan control | Pending verification | Code and build report exist; combined PIR + temperature + fan test not recorded |
| AI model | Pending | Rule-based recommendation only; no trained/evaluated model |
| Final system acceptance | Pending | End-to-end, safety, stability and evidence checks remain |

## 5. Completion Criteria

A task is complete only when its output exists and has been checked. Simulated data must be labeled as simulated. Commanded fan level must not be described as measured RPM unless a speed sensor is installed. AI performance must be evaluated on data not used to train the model.

## 6. Schedule Maintenance

Update this document when dates, hardware availability, technical decisions or test results change. Record significant changes and their reasons so the development history remains traceable.
