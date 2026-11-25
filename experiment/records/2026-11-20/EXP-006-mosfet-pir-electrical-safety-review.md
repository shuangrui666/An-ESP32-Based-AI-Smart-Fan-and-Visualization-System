# EXP-006 — MOSFET and PIR Electrical Safety Review

- Date: 2026-11-20
- Scope: Read-only review of the previously supplied MOSFET product listing, recorded wiring, module markings, and the current GitHub firmware tree.
- Status: **partially verified; power-on hold remains**

## 1. Evidence reviewed

- Previously supplied product listing: https://e.tb.cn/h.jmhfQoTesXdEfsZ?tk=YkMM6fDJOJk
- EXP-005 assembly record and uploaded evidence index.
- Current main-branch firmware files: `src/main.cpp`, `src/sensors/dht_sensor.h`, `src/sensors/dht_sensor.cpp`, and `platformio.ini`.
- Matching D4184 / XY-MOS style module references:
  - https://www.tinytronics.nl/en/switches/mosfets/high-power-mosfet-module-5-36v-15a
  - https://elgrad.pro/modulj-mosfet-shim-regulyator-aod4184-dc-5-36v-15a-400vt
  - https://robotchip.ru/obzor-drayvera-s-paroy-mosfet-na-aod4184/

## 2. MOSFET module specification cross-check

The previously supplied listing was recorded as specifying:
- Load supply: DC 5–36 V
- Trigger input: DC 3.3–20 V
- PWM: 0–20 kHz
- Advertised maximum current/power: 15 A / 400 W

The user's photographs show terminal labels VIN+, VIN-, VOUT+, VOUT-, TRIG/PWM and GND. Matching D4184 / XY-MOS family references describe TRIG/PWM as high-level active and support 3.3 V control.

This is a strong family-level match, but the exact MOSFET chip marking and board revision have not been independently confirmed. The 15 A / 400 W marketing rating is not a verified operating rating for this assembly.

## 3. User-reported wiring cross-check

| Connection | Recorded wiring | Review |
|---|---|---|
| USB supply positive | VIN+ | Consistent with module input definition |
| USB supply negative | VIN- | Consistent with module input definition |
| Fan positive | VOUT+ | Consistent with module output definition |
| Fan negative | VOUT- | Consistent with module output definition |
| ESP32-S3 GND | Module control GND | Required common control reference |
| ESP32-S3 GPIO6 | TRIG/PWM | Within the listing's stated 3.3 V trigger range, subject to exact-board confirmation |

The reported wiring arrangement is topologically consistent with the matching module family documentation. This is not a continuity test and does not prove that every conductor strand is securely clamped.

## 4. Firmware/GPIO audit

- The current public `src` directory contains `main.cpp`, `test_dht11_main.cpp`, and `sensors/`.
- No `src/control/` directory is present on the reviewed branch.
- `DHT_SENSOR_PIN` is GPIO4.
- The published main program currently does not configure GPIO5 or GPIO6.
- No GPIO5/GPIO6 collision was found in the reviewed current source files. This does not establish the state of uncommitted local files or undocumented board peripherals.
- GPIO6 remains the planned MOSFET control pin, not yet hardware-tested.

## 5. PIR safety status

The photographed PIR board resembles an HC-SR501-style module, but its exact model/revision is not established by a readable marking. Its supply and OUT voltage have not been verified from an exact model datasheet or measurement. Do not connect its OUT pin to the ESP32 until this is resolved; ESP32-S3 GPIOs are not 5 V tolerant.

## 6. Remaining pre-power hold points

1. Inspect the existing terminal wiring before power-on: no loose copper strands may bridge adjacent terminals; each conductor must be securely clamped with insulation outside the terminal opening.
2. Confirm the actual module's chip/board marking matches the D4184 / XY-MOS family references, or identify the board revision from its markings.
3. Confirm the PIR module's actual supply and OUT logic levels before connecting OUT to GPIO5.
4. Confirm the fan's rear mechanical switch is ON for the later controlled test; do not cut or open the fan cable.
5. Keep all power disconnected until the remaining checks are complete.

## 7. Decision

**Partial pass only.** The recorded MOSFET wiring arrangement and 3.3 V control plan are consistent with matching module family documentation. The firmware source audit found no current GPIO5/GPIO6 conflict. Physical terminal clamping, exact module revision, and PIR OUT voltage remain unverified. EXP-006 does not authorize powering the fan-driver circuit yet.


## 后续测试状态补充（2026-11-25）

本记录的“尚未上电 / 尚未进行 PIR 实测”描述的是 EXP-006 审查当时的安全状态。后续实验已分别记录：PIR GPIO5 初步信号读取见 [EXP-009](EXP-009-pir-hardware-signal-test.md)；GPIO6/MOSFET 驱动 USB 风扇基本启停及 100% 连续运行见 [EXP-012](../2026-11-22/EXP-012-esp32s3-mosfet-fan-continuous-run-test.md)。

这些后续实验更新了“是否曾经测试”的状态，但没有消除本审查提出的全部电气核验事项：风扇启动电流、模块温升、准确板卡修订、端子夹持及中间 PWM 档位仍未测量。请结合 EXP-009 和 EXP-012 阅读，不要单独把本记录的历史安全门槛当作最终测试状态。

