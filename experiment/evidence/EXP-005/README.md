# EXP-005 Evidence Index

- Experiment: EXP-005 — MOSFET driver module assembly and preliminary wiring
- Experiment date: 2026-11-19
- Status: evidence index created; original image files are **not yet present in this repository**.

## Evidence register

| Evidence ID | Original capture / source date | Description | Current repository status | Notes |
|---|---|---|---|---|
| EXP005-PHOTO-001 | 2026-11-14 19:33 (as visible in the previously shared image) | PIR module close-up; white Fresnel lens, two adjustment potentiometers and mode jumper; a separate board marked “Water Sensor” appears in the background | Not uploaded as an image file | Visual appearance is consistent with an HC-SR501-style PIR module; exact model is not confirmed |
| EXP005-PHOTO-002 | 2026-11-19 | Overall assembly view showing ESP32-S3, DHT11/PIR modules, fan and driver module | Not uploaded as an image file | Referenced in the experiment record |
| EXP005-PHOTO-003 | 2026-11-19 | MOSFET driver terminal and conductor close-up | Not uploaded as an image file | Terminal labels and wiring arrangement were documented; electrical correctness is not established |
| EXP005-PHOTO-004 | 2026-11-19 | Fan rear switch and cable | Not uploaded as an image file | No powered test is claimed |
| EXP005-PHOTO-005 | 2026-11-19 | ESP32-S3 board and connected jumper leads | Not uploaded as an image file | GPIO assignments remain subject to firmware audit |
| EXP005-PHOTO-006 | 2026-11-18 | Earlier breadboard/sensor arrangement | Not uploaded as an image file | Historical assembly-state evidence; do not treat as current wiring |

## Sensor inventory supported by current records

- DHT11 temperature/humidity module: previously tested on GPIO4; see EXP-001.
- PIR motion sensor module: photographed previously; appearance is consistent with an HC-SR501-style module, but exact model and electrical specifications are unconfirmed.
- A separate red board marked “Water Sensor” is visible in the 2026-11-14 photograph. It is recorded as an observed component only, not as an integrated SmartFan sensor.

## Evidence integrity note

This index preserves the descriptions and provenance currently available from the conversation record. It does **not** contain the original binary photographs. No image file is claimed to have been uploaded. If original image files become available later, add them without altering the historical descriptions, and record their original capture dates and SHA-256 hashes.

## Safety / verification boundary

The photographed assembly is not proof of electrical continuity, correct terminal topology, safe polarity, or successful operation. Keep the circuit unpowered until the checks listed in the EXP-005 experiment record are completed.
