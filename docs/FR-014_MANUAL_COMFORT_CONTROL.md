# FR-014 — User-Initiated Comfort Control (MANUAL)

- **Requirement ID:** FR-014
- **Project:** SmartFan 2.0
- **Status:** Approved requirement; implementation and system-level verification pending
- **Priority:** High
- **Decision basis:** User explicitly approved this requirement in the project discussion on 2026-11-23.

## 1. Purpose

Allow a user who feels hot or uncomfortable to start the fan from the web dashboard even when the measured ambient temperature has not reached the AUTO-mode start threshold and the PIR sensor has not detected a person.

The web dashboard is the user-facing control interface. The command must travel through the backend command queue to the ESP32; a browser click alone must never be represented as proof that the physical fan has started.

## 2. User story

As a user, when I feel hot but the environment does not satisfy the automatic start conditions, I want to switch the system to MANUAL mode on the web dashboard and select a fan level so that I can request airflow immediately.

## 3. Functional requirements

| ID | Requirement | Expected behavior |
|---|---|---|
| FR-014-01 | Web mode selection | The dashboard provides AUTO and MANUAL mode selection. |
| FR-014-02 | Manual start independent of temperature | In MANUAL mode, a valid user command for gear 1–5 can request fan operation even below the AUTO start threshold (currently 28 °C). |
| FR-014-03 | Manual start independent of PIR | MANUAL start does not require PIR occupancy detection. |
| FR-014-04 | Manual gear selection | The dashboard provides gear selection from 0 to 5, using the project's existing and verified gear-to-output mapping. |
| FR-014-05 | Manual stop | Gear 0 requests fan stop. |
| FR-014-06 | Execution feedback | The dashboard distinguishes command submission/pending from actual device execution success or failure. It reports success only after the ESP32 has executed the command and the backend has received the corresponding acknowledgement. |
| FR-014-07 | Actual device state | The dashboard distinguishes the user's requested gear from the latest device-reported/acknowledged operating state. |
| FR-014-08 | Safety priority | Sensor faults and other defined device safety faults take priority over MANUAL requests. A manual request must not bypass the system's safety interlocks. |
| FR-014-09 | AUTO behavior preservation | MANUAL support must not silently alter the established AUTO thresholds, occupancy delay, hysteresis, or local safety behavior. |

## 4. Control policy

### 4.1 AUTO mode

AUTO remains governed by the project's established automatic policy:

- Start when the configured temperature and occupancy conditions are satisfied.
- Apply the configured no-occupancy delay before stopping.
- Apply the configured lower-temperature stop threshold and hysteresis.
- Enter the defined safe state when required sensor readings fail.

Current project settings recorded for AUTO are a 28 °C start threshold, a 26 °C lower stop threshold, and a 60-second no-occupancy delay. These are configuration values and must be verified against the active firmware before being treated as implementation evidence.

### 4.2 MANUAL mode

- The user selects MANUAL on the web dashboard.
- The user may request gear 1–5 without meeting the AUTO temperature threshold and without PIR occupancy detection.
- Gear 0 requests stop.
- The selected gear must use the existing, verified FanController gear mapping; do not assume that AUTO PWM duty values and MANUAL gear numbers are interchangeable.
- Safety faults remain authoritative. If a safety condition requires shutdown, the firmware rejects or overrides the manual request and reports the reason.
- MANUAL must not be implemented as a second independent writer to the GPIO/PWM output. AUTO and MANUAL requests must pass through one coordinated control authority to prevent competing control loops.

## 5. Web-to-device command flow

1. User opens the SmartFan 2.0 dashboard.
2. User selects MANUAL mode.
3. User selects a gear from 0–5.
4. Frontend submits the request to the Flask backend.
5. Backend creates a uniquely identified command in the command queue.
6. ESP32 polls for the command and validates its identity, mode, gear, expiry and safety conditions.
7. ESP32 passes the request to the unified fan-control logic.
8. Firmware determines whether the requested action was actually applied or rejected/overridden by a safety condition.
9. ESP32 sends an acknowledgement containing the command identity and execution result.
10. Backend updates command status; the dashboard displays the confirmed result and device-reported state.

The dashboard must not display “fan running” merely because the user clicked a gear button or because the backend accepted the request.

## 6. Acceptance tests

| Test ID | Setup / action | Expected result |
|---|---|---|
| FR014-T01 | Temperature is below 28 °C; AUTO mode; no qualifying PIR detection | AUTO does not start the fan. |
| FR014-T02 | Keep temperature below 28 °C and PIR inactive; switch to MANUAL and request gear 3 | ESP32 executes gear 3, provided no safety fault is active; dashboard reports success only after acknowledgement. |
| FR014-T03 | In MANUAL, request each supported gear from 1–5 | Each request maps to the documented output level and the device reports the executed level. |
| FR014-T04 | In MANUAL, request gear 0 | Fan is stopped and the execution result is acknowledged. |
| FR014-T05 | In MANUAL, trigger a defined sensor/device safety fault | Safety policy takes priority; unsafe operation is prevented and the dashboard reports rejection/override and reason. |
| FR014-T06 | Submit a command while network communication is interrupted | Dashboard does not falsely report execution success; local safety and AUTO fallback behavior remain available as designed. |
| FR014-T07 | Send a duplicate command ID | The physical action is not executed twice; acknowledgement/state handling remains consistent. |
| FR014-T08 | Return from MANUAL to AUTO | AUTO control resumes according to the documented mode-transition policy without stale manual requests overriding it. |

## 7. Implementation and verification status

This document records an approved product requirement, not a claim that the feature is already implemented.

At the time this requirement was approved:

- The web-based MANUAL interaction and complete end-to-end execution path still require implementation/integration and testing.
- The ESP32 command polling code has been reviewed, but the current review identified unfinished command execution, acknowledgement timing, acknowledgement retry, expiry validation and input validation.
- The actual gear-to-PWM mapping must be verified from the current FanController implementation and hardware tests.
- No system-level acceptance test in Section 6 is claimed as passed by this requirements document.

Implementation evidence must be recorded in a dated experiment record, with source changes, build results, device logs, backend responses and observed fan behavior.

## 8. Safety and scope notes

- This requirement does not authorize bypassing sensor-fault protection or other defined hardware safety interlocks.
- PIR absence and temperature below the AUTO threshold are not, by themselves, reasons to reject a MANUAL request.
- A five-minute command expiry in the backend queue is not the same as an automatic five-minute MANUAL-mode timeout. No MANUAL inactivity timeout is specified by this requirement.
- This requirement does not introduce a DEMO mode.
- Do not upload Wi-Fi credentials, `secrets.h`, passwords or other secrets as part of its implementation evidence.
