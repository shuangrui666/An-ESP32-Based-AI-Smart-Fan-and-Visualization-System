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
| 1. Project documentation | Create repository, README, proposal and schedule | Planning documents in GitHub | In progress |
| 2. Hardware inventory | Inspect ESP-Claw kit; identify board, sensors, driver, wires and tools | Dated inventory with photos/model numbers | Pending |
| 3. Fan verification | Record label and seller specifications; measure/check electrical behavior with suitable equipment | Fan specification and verification record | Pending |
| 4. Hardware proof of concept | Test ESP32 and sensors separately; select driver only after fan verification | Wiring record and bench-test results | Pending |
| 5. Firmware and communication | Read sensors, report status and receive commands over Wi-Fi | ESP32 source code and communication test | Pending |
| 6. Backend and data logging | Build Python/Flask service and store incoming readings | Backend source code and sample log | Pending |
| 7. Dashboard | Build responsive interface; first use clearly labeled simulated data, then verified live data | Dashboard source and screenshots/test record | Pending |
| 8. AI model | Define labels, prepare dataset, train and evaluate decision tree | Dataset description, model and evaluation report | Pending |
| 9. Integration and testing | Connect hardware, backend, model and dashboard; test fallback | Integrated test records | Pending |
| 10. Final delivery | Organize code, data, diagrams, photos/videos and report | Reviewed repository and presentation | Pending |

## 3. Detailed Tasks and Dependencies

### Stage 1 — Project Documentation
- [x] Create the GitHub repository.
- [x] Create the initial README.
- [x] Create the project proposal.
- [x] Create this development schedule.
- [ ] Review document paths and consistency.
- [ ] Create the system architecture document after hardware inventory.

### Stage 2 — Hardware Inventory
- [ ] Inspect and photograph the complete ESP-Claw kit.
- [ ] Record the exact ESP32 board model and USB cable type.
- [ ] Identify temperature/humidity sensor and PIR sensor models, if present.
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
- [ ] Test each sensor independently.
- [ ] Test the selected motor driver without connecting an incompatible load.
- [ ] Test fan startup and each intended control level.
- [ ] Record current, temperature and observed stability where instruments are available.

### Stage 5 — Firmware and Communication
- [ ] Implement sensor acquisition.
- [ ] Define a documented data format and update interval.
- [ ] Implement Wi-Fi communication.
- [ ] Test command reception and safe output states.
- [ ] Implement local fallback behavior.

### Stage 6 — Backend and Data Logging
- [ ] Set up the Python environment.
- [ ] Build the Flask service and API endpoints.
- [ ] Validate incoming data.
- [ ] Store timestamped readings and command history.
- [ ] Handle missing data and communication errors.

### Stage 7 — Visualization Dashboard
- [ ] Create responsive page layout.
- [ ] Display current sensor readings and connection state.
- [ ] Display mode and commanded fan level.
- [ ] Add temperature/humidity time-series charts where sensor data exists.
- [ ] Add motion-event and fan-command timelines where applicable.
- [ ] Display AI recommendation and its timestamp/source.
- [ ] Add manual/automatic controls after command safety is tested.
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

As of the latest planning update:

| Item | Status | Evidence / next action |
|---|---|---|
| GitHub repository | Completed | Repository created |
| README | Initial version created | Review and maintain as project evolves |
| Project proposal | Created | Review after hardware inventory |
| Development schedule | Created | Update when deadlines are confirmed |
| USB-powered fan | Purchased | Verify specifications and actual behavior |
| ESP-Claw kit inventory | Pending | Inspect with instructor |
| Fan electrical verification | Pending | Obtain specifications and test safely |
| Firmware/backend/AI/dashboard | Pending | Begin after dependencies are confirmed |
| Experimental evaluation | Pending | Requires working prototype |

## 5. Completion Criteria

A task is complete only when its output exists and has been checked. Simulated data must be labeled as simulated. Commanded fan level must not be described as measured RPM unless a speed sensor is installed. AI performance must be evaluated on data not used to train the model.

## 6. Schedule Maintenance

Update this document when dates, hardware availability, technical decisions or test results change. Record significant changes and their reasons so the development history remains traceable.
