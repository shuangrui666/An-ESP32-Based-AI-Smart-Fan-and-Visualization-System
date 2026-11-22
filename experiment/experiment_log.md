# SmartFan 2.0 实验总日志

本文件是项目实验索引。每完成一次实验，在此追加一行，并链接到对应的独立记录。不得将计划中的实验写成已完成。

## 实验记录索引

| 实验编号 | 日期 | 实验名称 | 状态 | 独立记录 | 主要结论 / 待解决问题 |
|---|---|---|---|---|---|
| EXP-001 | 2026-11-11 | ESP32-S3 与 DHT11 温湿度传感器读取验证 | completed | [EXP-001](records/2026-11-11/EXP-001-esp32-dht11-validation.md) | 独立测试 22/22 次成功；修复主程序 DHT 初始化后，串口日志连续两轮读取到 25.0 °C、42.0%；风扇控制尚未测试 |
| EXP-002 | 2026-11-14 | ESP32 固件现状与风扇控制链路排查 | completed（代码审查） | [EXP-002](records/2026-11-14/EXP-002-firmware-fan-control-audit.md) | 确认 DHT11 使用 GPIO4；风扇控制 GPIO21 仍是临时默认值；PWM 初始化/写入被注释，主程序未调用风扇控制器；真实风扇控制尚未实现。记录为事后补记。 |
| EXP-003 | 2026-11-14 | README 项目状态校正与阶段工作记录 | completed（文档校正） | [EXP-003](records/2026-11-14/EXP-003-readme-status-reconciliation.md) | 校正项目真实进度、风扇参数和第11节状态；记录 PowerShell 修改过程与未完成事项。仅为文档工作，不代表实体风扇控制完成。 |
| EXP-004 | 2026-11-19 | ESP32-S3 Wi-Fi 连接与 Flask HTTP 数据上传验证 | completed（数据上传链路） | [EXP-004](records/2026-11-19/EXP-004-esp32-wifi-http-upload-verification.md) | Flask 日志确认 ESP32 192.168.137.171 向 /api/sensor/data POST 温湿度数据并收到 HTTP 200；断线自动重连、双向控制和长期稳定性仍待验证。 |
| EXP-005 | 2026-11-19 | MOSFET 驱动模块焊接与初步接线记录 | in_progress（已拍照；未上电、未完成电气核验） | [EXP-005](records/2026-11-19/EXP-005-mosfet-module-preliminary-wiring.md) | 已记录模块焊接和用户报告的初步连接；MOSFET端子拓扑、触发极性、GPIO兼容性及接线安全仍待核实；传感器照片已辨认出DHT11与PIR外观模块，PIR型号待确认。 |
| EXP-006 | 2026-11-20 | MOSFET 与 PIR 电气安全审查 | in_progress（部分核对通过；仍有上电前检查项） | [EXP-006](records/2026-11-20/EXP-006-mosfet-pir-electrical-safety-review.md) | 商品参数与同系列模块资料相符；GPIO5/6 当前公开源码未发现冲突；PIR OUT 电平、端子夹线状态及模块版本仍待确认。 |
| EXP-007 | 2026-11-20 | PIR 原始信号读取测试程序编写 | completed（代码编写；未编译/未上板） | [EXP-007](records/2026-11-20/EXP-007-pir-raw-signal-read-test-code.md) | 新增独立 GPIO5 原始电平读取测试入口及 PlatformIO 环境；不控制风扇。PIR OUT 电压未确认，禁止上传运行；无硬件测试结论。 |
| EXP-008 | 2026-11-20 | PIR 测试程序编译验证 | completed（编译验证；未上板） | [EXP-008](records/2026-11-20/EXP-008-pir-test-build-verification.md) | PIR 独立测试环境编译成功；因当时 PIR OUT 电平安全性未确认，记录明确未上传、未进行硬件测试。 |
| EXP-009 | 2026-11-20 | PIR 硬件信号测试 | completed（GPIO 信号初步验证） | [EXP-009](records/2026-11-20/EXP-009-pir-hardware-signal-test.md) | 记录显示 GPIO5 读取到 8 次 HIGH/LOW 周期，静止观察约 38 秒持续 LOW；仅验证 PIR 信号读取，不涉及 MOSFET 或风扇控制。 |
| EXP-012 | 2026-11-22 | ESP32-S3 GPIO6 / MOSFET 风扇连续运行验证 | completed（基本开关控制与连续运行现象已观察；电气测量及调速能力待验证） | [EXP-012](records/2026-11-22/EXP-012-esp32s3-mosfet-fan-continuous-run-test.md) | 用户确认 100% PWM 占空比下风扇能够持续运行；GPIO 波形、输出电压/电流、温升、中间占空比调速和正式固件集成仍待验证。独立测试源码与专用环境尚未纳入当前 GitHub 版本。 |
| EXP-011 | 2026-11-22 | DEF-001 LEDC 初始化错误处理与 DEF-003 HTTP 超时返修 | 本地返修核查与三环境编译据报告通过；源码未纳入 Git，运行时/硬件待验证 | [DEF-001_REPAIR_REPORT](records/2026-11-22/DEF-001_REPAIR_REPORT.md) | 后续只读 Git 核查确认 fan_controller.cpp、wifi_manager.cpp/h 均未被 Git 跟踪，相关文件无提交历史；三环境编译结果来自本地工作区报告，日志仍在本地 .qwen/temp/，不能视为 GitHub 源码构建证据。未烧录、未测 PWM、未验证 MOSFET/风扇；更正详见独立报告第 8 节。 |
| EXP-010 | 构建日期待确认；记录创建 2026-11-22 | ESP32-S3 主程序 LEDC API 兼容性修复与编译验证 | completed（编译验证；硬件待验证） | [EXP-010](records/2026-11-22/EXP-010-esp32s3-fan-controller-build-fix.md) | 据用户提供的 Qwen 报告，修复 LEDC API、PIR_SENSOR_PIN 定义及 5 处格式字符串问题；esp32-s3-devkitc-1 编译成功、无警告，RAM 14.4%、Flash 30.5%；未证明实际 PWM、MOSFET、风扇和 60 秒控制逻辑通过测试。 |

## 阶段进度

| 阶段 | 状态 | 完成条件 |
|---|---|---|
| 硬件清点 | in_progress | 实际元件、型号、数量和参数均有证据；PIR准确型号待确认 |
| 电气与接线设计 | in_progress | 实际接线已拍照记录；电路拓扑、引脚和电源方案仍待核实 |
| 传感器单项测试（DHT11） | completed | 本次 GPIO4 温湿度读取验证通过；长期稳定性仍待持续测试 |
| ESP32 固件测试（基础状态与 DHT11） | completed | 主程序已编译、上传并输出有效温湿度；其他固件功能仍待测试 |
| ESP32-S3 主程序编译（LEDC API 修复后） | completed（编译） | 据用户提供报告，目标环境编译成功且无警告；固件烧录、PWM 波形和风扇实际控制仍待验证 |
| Wi-Fi 通信测试（ESP32 → Flask 上传） | completed | 已观察到真实温湿度 POST 请求并收到 HTTP 200；断线恢复和双向通信仍待单独测试 |
| MOSFET驱动模块装配 | in_progress | 已焊接排针并完成初步接线记录；尚未上电或验证 |
| 后端测试 | in_progress | 已验证温湿度接收；风扇控制接口仍待实现并验证 |
| AI 模型实验 | planned | 数据来源、划分方式、参数和评价结果可复现 |
| 前端与可视化测试 | in_progress | 此前已验证温湿度展示；实时折线图刷新仍需排查；真实风扇控制未验证 |
| 整机集成测试 | planned | 端到端功能和安全测试有证据 |

## 追加记录格式

新增实验时按以下字段填写：

- 实验编号：
- 日期：
- 实验名称：
- 状态：
- 实验记录链接：
- 核心结果：
- 异常与待解决问题：
- 对应 Git commit：

## 重要说明

- 本表只作为索引，不替代每次实验的详细记录。
- 记录日期使用实际实验日期。
- 若结论后来被新证据修正，应追加更正说明并链接新记录，不要悄悄抹去旧结论。
