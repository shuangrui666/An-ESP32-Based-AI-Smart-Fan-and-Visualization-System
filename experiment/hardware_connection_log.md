# 硬件连接与电路验证记录

本文件记录每次接线设计、连接变更和上电测试。未确认的引脚和电气参数不得填写为确定值。

## 1. 当前连接状态

- 当前状态：in_progress（已完成 PIR 信号读取及独立风扇基本驱动测试；电流、温升、保护和调速能力等电气核验仍未完成）
- ESP32 开发板：照片可见 VO-ESP32-S3 标识；完整板卡型号和引脚映射仍需结合本地工程与板卡资料核对
- 温湿度传感器：DHT11；GPIO4 已在 EXP-001 验证
- 人体活动传感器：HC-SR501（EXP-009 记录的用户确认型号）；GPIO5 信号读取已初步验证，OUT 电压及检测参数仍待核实
- 风扇：USB 风扇；说明书记录 5 V / 2.5 W，启动电流仍待核实
- 驱动 / 开关模块：蓝色双 MOSFET PWM 电子开关模块；可见端子标识 TRIG/PWM、GND、VIN−、VIN+、VOUT−、VOUT+
- 接线照片与阶段记录：[EXP-005](records/2026-11-19/EXP-005-mosfet-module-preliminary-wiring.md)

## 2. 引脚分配表

| 模块 | 模块引脚 | ESP32 引脚 | 信号类型 / 总线 | 电压兼容性 | 分配依据 | 状态 |
|---|---|---|---|---|---|---|
| DHT11 | DATA | GPIO4 | 数字单总线 | 已在当前配置下完成读取验证 | EXP-001 与固件 DHT 驱动 | verified for DHT11 read |
| HC-SR501 PIR | OUT | GPIO5 | 数字输入 | 已观察 HIGH/LOW；电压幅值未测 | EXP-009 烧录与串口信号记录 | signal read verified; voltage/calibration pending |
| MOSFET 驱动模块 | TRIG/PWM | GPIO6 | 数字/PWM 输出 | 独立测试已观察基本开关及 100% 连续运行；中间 PWM 未验证 | EXP-012 独立 fan-simple-test | basic switching/full-duty verified; electrical characterization and variable speed pending |
| MOSFET 驱动模块 | GND | ESP32 GND | 信号参考地 | 依据驱动电路拓扑最终确认 | 用户报告已连接 | pending_verification |

## 3. 电源与负载核对

| 检查项 | 核实结果 | 证据 / 依据 | 状态 |
|---|---|---|---|
| 风扇额定电压 | 5 V | 风扇说明书 | recorded |
| 风扇额定功率 | 2.5 W | 风扇说明书 | recorded |
| 风扇工作电流 | 按 P/V 计算约 0.5 A；卖家另提 1 A，具体含义未确认 | 说明书额定参数与卖家信息，二者性质需区分 | pending_verification |
| 风扇启动电流 | 未测量 | 无测量证据 | pending_verification |
| 驱动器额定电压 / 电流 | 商品宣传参数不能替代实物规格核验 | 模块型号及数据资料待确认 | pending_verification |
| ESP32 与驱动器逻辑电平兼容性 | 未确认 | TRIG/PWM 输入电气参数和触发极性待核实 | pending_verification |
| 共地要求 | 用户报告 ESP32 GND 已连接驱动模块 GND；是否符合该模块实际拓扑待核实 | EXP-005 用户报告及照片 | pending_verification |
| 电源容量与保护 | 尚未完成设计核对 | 尚无完整测量与保护方案 | pending_verification |

## 4. 每次连接实验记录

### EXP-005 — 2026-11-19 MOSFET 模块装配与初步接线

- 连接目的：为 USB 风扇建立由 ESP32 控制的驱动链路，先完成模块装配和初步接线记录。
- 使用的板卡和模块：ESP32-S3、DHT11、PIR 外观模块、双 MOSFET PWM 电子开关模块、USB 风扇。
- 实际连接（用户报告，尚未上电验证）：
  - USB 供电线红/黑线 → VIN+ / VIN−
  - 风扇红/黑线 → VOUT+ / VOUT−
  - ESP32 GND → 驱动模块 GND
  - ESP32 GPIO6 → 驱动模块 TRIG/PWM
- 上电状态：未上电；不得将本记录解释为接线已验证。
- 待核查：端子拓扑、铜线夹持状态、触发极性、GPIO 电平兼容性、GPIO5/GPIO6 冲突、PIR 型号及 OUT 电压。
- 详细过程与照片说明：[EXP-005](records/2026-11-19/EXP-005-mosfet-module-preliminary-wiring.md)

## 5. 安全检查清单

- [ ] 已确认所有模块的供电电压。
- [ ] 已确认 GPIO 逻辑电平兼容。
- [ ] 已确认负载电流不由 GPIO 直接承担。
- [ ] 已确认风扇与驱动器兼容。
- [ ] 已确认电源容量及极性。
- [ ] 上电前检查无短路和松动。
- [ ] 在教师指导或适当实验条件下进行必要的电气测量。
- [ ] 断电后再修改接线。

## 6. 接线变更历史

| 日期 | 版本 | 变更内容 | 变更原因 | 验证结果 | 实验编号 |
|---|---|---|---|---|---|
| — | v0.1 | 创建初始记录模板 | — | 未进行上电测试 | — |
| 2026-11-19 | v0.2 | 记录 DHT11、PIR 外观模块、MOSFET 驱动模块及风扇的当前装配与用户报告接线 | 保存实际实验过程和上电前待核查事项 | 仅完成照片记录；未上电、未验证 | EXP-005 |



## 7. 2026-11-25 状态更新（后续实验补充）

本节更新较早的 EXP-005 / EXP-006 上电前记录。旧记录保留其当时状态，不应覆盖后续已完成的独立测试事实。

- **DHT11：** GPIO4 已在 EXP-001 完成单项读取验证。
- **PIR：** GPIO5 已在 EXP-009 完成初步硬件信号读取；观察到 8 次 HIGH/LOW 周期及静止期间 LOW。该测试未控制风扇，且未完成检测距离、角度、误触发率和 OUT 电压测量。
- **风扇驱动：** EXP-012 已使用独立 fan-simple-test 程序，通过 GPIO6 / MOSFET 模块观察到风扇基本启停及 100% PWM 连续运行。此结果更新了 EXP-005 / EXP-006 当时“未上电”的历史状态，但不代表中间 PWM 调速或正式主固件集成已通过。
- **接线方案（用户确认）：** 独立 5V 负载电源正负分别接 VIN+ / VIN−；风扇正负接 VOUT+ / VOUT−；ESP32 GND 接模块 GND；GPIO6 接 TRIG/PWM。修改接线前必须断开负载电源，严禁把独立电源正极与 ESP32 USB 5V 并接。
- **仍待核验：** MOSFET 实际芯片/板卡修订、端子夹持状态、负载电流、启动电流、模块温升、供电保护及中间 PWM 档位效果。没有仪器读数的项目继续标记为待测。

