# EXP-005 — MOSFET Driver Module Assembly and Preliminary Wiring Record

- Experiment ID: EXP-005
- Date: 2026-11-19
- Status: in_progress — assembly photographed; circuit not powered or electrically verified
- Activity: hardware assembly and pre-power wiring documentation

## 1. Objective

Preserve the actual hardware assembly state and wiring progress before any powered fan-driver test. This record also links the earlier PIR module identification photograph supplied on 2026-11-14.

## 2. Hardware inventory

| Component | Identification | Status |
|---|---|---|
| Controller | ESP32-S3 development board; board marking visible as VO-ESP32-S3 | Present; GPIO allocation still requires firmware audit |
| Temperature/humidity sensor | DHT11 module (blue vented housing) | GPIO4 was previously validated in EXP-001 |
| Motion sensor | PIR module with white Fresnel dome, two adjustment potentiometers and a mode jumper | Appearance is consistent with HC-SR501 or a compatible clone; exact manufacturer/model marking is not visible; wiring and operation not verified |
| Fan | USB-powered desktop fan, rear mechanical switch visible | Present; manual rating previously recorded as 5 V / 2.5 W |
| Driver | Blue dual-MOSFET PWM/electronic switch module | Board labels visible: TRIG/PWM, GND, VIN−, VIN+, VOUT−, VOUT+; exact circuit topology and input behavior still require confirmation |

### 2.1 Additional component visible in the earlier photograph

A red board marked "Water Sensor" is visible in the background of the 2026-11-14 PIR photograph. It is identified as a water-sensor module by its board marking, but the available evidence does not establish that it is connected to or used by the SmartFan 2.0 build. It is therefore recorded as an observed component only, not as an integrated project sensor.

## 3. Assembly and wiring work reported by the user

The user reported on 2026-11-19 that:
1. Header pins were soldered onto the MOSFET module.
2. USB supply red/black conductors were connected to VIN+/VIN−.
3. Fan red/black conductors were connected to VOUT+/VOUT−.
4. ESP32 GND was connected to the driver's control GND.
5. ESP32 GPIO6 was connected to TRIG/PWM.
6. The user reported that the wires were clamped and that there was no unintended contact between conductors.

These are user-reported connections. The photographs provide visual evidence of the assembly but do not conclusively establish terminal contact quality or the circuit's electrical correctness.

## 4. Visual observations and unresolved issues

- The close-up photograph shows exposed/twisted copper near the VIN and VOUT terminal areas. The extent to which the copper is securely clamped inside the terminals is not fully clear.
- The control header labels include TRIG/PWM and GND. The exact polarity and electrical requirements of the control input are not yet established.
- GPIO6 is a candidate control pin based on the current wiring, not yet confirmed free of conflicts in the complete local firmware project.
- GPIO5 is a candidate PIR signal pin, not yet confirmed free of conflicts.
- The PIR photograph dated 2026-11-14 shows a white Fresnel dome, two adjustment potentiometers and a mode jumper. This is consistent with an HC-SR501-style PIR module, but the exact model is not conclusively confirmed by a readable model marking.
- The PIR supply voltage and OUT signal voltage remain unverified.

## 5. Safety hold point

**Do not connect USB power or power the ESP32 for a fan-driver test yet.**

Before power-on:
1. Confirm the driver's actual circuit topology and terminal functions from reliable module documentation or inspection.
2. Confirm TRIG/PWM active polarity and compatibility with ESP32-S3 3.3 V logic.
3. Review the close-up wiring and ensure no exposed copper can bridge adjacent terminals.
4. Audit the local firmware for GPIO5/GPIO6 conflicts.
5. Confirm the PIR module's supply and OUT voltage are safe for the ESP32-S3 GPIO.
6. Establish a safe startup state and a controlled first-power-on procedure.

## 6. Evidence captured

Photographs supplied by the user:
- 2026-11-14 19:33: close-up of the PIR module, showing its Fresnel dome, two adjustment potentiometers and mode jumper; a separate red board marked "Water Sensor" is visible in the background.
- 2026-11-19: overall view of fan, ESP32-S3, DHT11/PIR modules and MOSFET driver.
- 2026-11-19: close-up of MOSFET terminal labels and conductors.
- 2026-11-19: fan rear switch and cable.
- 2026-11-19: ESP32-S3 board and connected jumper leads.
- 2026-11-18: earlier breadboard/sensor arrangement.

The original image files still need to be committed to the repository's evidence directory; this record does not claim that binary image files have already been uploaded.

## 7. Conclusion

The module header soldering and preliminary wiring have been reported and photographed. The PIR module's appearance is consistent with an HC-SR501-style board, but its exact model and electrical specifications remain unconfirmed. The Water Sensor board is only observed in the background and is not counted as an integrated SmartFan sensor. This is an assembly-progress record only. No powered test, fan operation test, PWM test, voltage/current measurement or electrical compatibility test has been performed. The circuit remains unpowered pending review.
