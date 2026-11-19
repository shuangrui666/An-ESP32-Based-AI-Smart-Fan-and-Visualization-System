# EXP-005 — MOSFET Driver Module Assembly and Preliminary Wiring Record

- Experiment ID: EXP-005
- Date: 2026-11-19
- Status: in_progress — assembly photographed; circuit not powered or electrically verified
- Activity: hardware assembly and pre-power wiring documentation

## 1. Objective

Preserve the actual hardware assembly state and wiring progress before any powered fan-driver test.

## 2. Hardware inventory visible in the 2026-11-19 photographs

| Component | Identification | Status |
|---|---|---|
| Controller | ESP32-S3 development board; board marking visible as VO-ESP32-S3 | Present; GPIO allocation still requires firmware audit |
| Temperature/humidity sensor | DHT11 module (blue vented housing) | GPIO4 was previously validated in EXP-001 |
| Motion sensor | PIR-type module with white Fresnel dome | Present; exact model/board marking not readable in current photos; wiring and operation not verified |
| Fan | USB-powered desktop fan, rear mechanical switch visible | Present; manual rating previously recorded as 5 V / 2.5 W |
| Driver | Blue dual-MOSFET PWM/electronic switch module | Board labels visible: TRIG/PWM, GND, VIN−, VIN+, VOUT−, VOUT+; exact circuit topology and input behavior still require confirmation |

No other sensor is claimed as identified from these photographs. In particular, the PIR module's exact model number is not established by appearance alone.

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
- The white-dome sensor is identified only as PIR-type; exact model and supply/output voltage remain pending confirmation.

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

Photographs supplied by the user in this conversation on 2026-11-19:
- Overall view of fan, ESP32-S3, DHT11/PIR modules and MOSFET driver.
- Close-up of MOSFET terminal labels and conductors.
- Fan rear switch and cable.
- ESP32-S3 board and connected jumper leads.
- Earlier breadboard/sensor arrangement photographed on 2026-11-18.

The original image files still need to be committed to the repository's evidence directory; this record does not claim that binary image files have already been uploaded.

## 7. Conclusion

The module header soldering and preliminary wiring have been reported and photographed. This is an assembly-progress record only. No powered test, fan operation test, PWM test, voltage/current measurement or electrical compatibility test has been performed. The circuit remains unpowered pending review.
