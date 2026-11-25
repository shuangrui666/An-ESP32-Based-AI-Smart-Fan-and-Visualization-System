# SmartFan 2.0 实验总日志

本文件是项目实验索引。每完成一次实验，在此追加一行，并链接到对应的独立记录。不得将计划中的实验写成已完成。

## 实验记录索引

| 实验编号 | 日期 | 实验名称 | 状态 | 独立记录 | 主要结论 / 待解决问题 |
|---|---|---|---|---|---|
| EXP-001 | 2026-11-11 | ESP32-S3 与 DHT11 温湿度读取验证 | completed（单项读取） | [EXP-001](records/2026-11-11/EXP-001-esp32-dht11-validation.md) | GPIO4 读取 22/22 成功；主程序串口也有有效温湿度读数；不代表后续每次接线均已复测 |
| EXP-002 | 2026-11-14 | ESP32 固件与风扇控制链路审查 | completed（历史代码审查） | [EXP-002](records/2026-11-14/EXP-002-firmware-fan-control-audit.md) | 记录当时旧版本问题；GPIO21 等结论属于历史快照，不能当作当前 GPIO6 配置 |
| EXP-003 | 2026-11-14 | README 状态校正 | completed（文档工作） | [EXP-003](records/2026-11-14/EXP-003-readme-status-reconciliation.md) | 历史文档校正记录 |
| EXP-004 | 2026-11-19 | ESP32-S3 Wi-Fi / Flask HTTP 上传 | completed（单向数据上传） | [EXP-004](records/2026-11-19/EXP-004-esp32-wifi-http-upload-verification.md) | Flask 收到温湿度 POST 并返回 HTTP 200；断线重连、命令下发和长期稳定性未由本实验验证 |
| EXP-005 | 2026-11-19 | MOSFET 模块装配与初步接线 | completed（装配/接线记录） | [EXP-005](records/2026-11-19/EXP-005-mosfet-module-preliminary-wiring.md) | 保留当时上电前状态；后续独立运行测试见 EXP-012，不应把 EXP-005 本身改写为已验证 |
| EXP-006 | 2026-11-20 | MOSFET 与 PIR 电气安全审查 | completed（审查记录；保留当时限制） | [EXP-006](records/2026-11-20/EXP-006-mosfet-pir-electrical-safety-review.md) | 当时对模块版本、端子和 PIR OUT 电平的安全疑点仍需按当前实物复核 |
| EXP-007 | 2026-11-20 | PIR 原始信号读取测试代码 | completed（代码编写） | [EXP-007](records/2026-11-20/EXP-007-pir-raw-signal-read-test-code.md) | 建立 GPIO5 测试入口；实际硬件信号结果见 EXP-009 |
| EXP-008 | 2026-11-20 | PIR 测试程序编译验证 | completed（编译；未上板） | [EXP-008](records/2026-11-20/EXP-008-pir-test-build-verification.md) | 编译成功；该记录本身未烧录，后续硬件测试见 EXP-009 |
| EXP-009 | 2026-11-20 | PIR GPIO5 硬件信号测试 | completed（初步信号测试） | [EXP-009](records/2026-11-20/EXP-009-pir-hardware-signal-test.md) | 烧录成功并观察到 8 次 HIGH/LOW 周期及静止 LOW；未做距离/角度标定或 OUT 电压测量 |
| EXP-010 | 2026-11-22 | 主程序 FanController 兼容性修复与编译 | completed（编译验证） | [EXP-010](records/2026-11-22/EXP-010-esp32s3-fan-controller-build-fix.md) | ESP32-S3 主环境编译成功；不代表 PWM、PIR 联动或实体风扇已由主程序验证 |
| EXP-012 | 2026-11-22 | GPIO6 / MOSFET 风扇连续运行测试 | completed（独立程序、基本开关） | [EXP-012](records/2026-11-22/EXP-012-esp32s3-mosfet-fan-continuous-run-test.md) | 用户观察到 OFF/ON 周期与 100% 连续运行；中间占空比、电气测量和正式固件集成未验证 |
| EXP-013 | 2026-11-23 | Phase 3.2.1 Flask 指令队列 | completed（软件交付/测试摘要） | [EXP-013](records/2026-11-23/EXP-013_PHASE_3.2.1_BACKEND_COMMAND_QUEUE.md) | 指令状态机、AUTO/MANUAL 和 22 项自动化测试结果已归档；不是实体控制验收 |

> 编号 EXP-011 在当前仓库中没有对应独立记录，因此本索引不补造该编号。

## 阶段进度

| 阶段 | 当前状态 | 已有证据 / 尚缺内容 |
|---|---|---|
| 硬件清点 | in_progress | 开发板、DHT11、PIR、风扇和 MOSFET 已识别到一定程度；模块版本、完整器件清单及部分电气参数仍待核实 |
| DHT11 单项测试 | completed（历史单项测试） | EXP-001 22/22 成功；新接线后的回归和长期稳定性未记录 |
| PIR 单项测试 | completed（初步信号测试） | EXP-009 GPIO5 HIGH/LOW 观察通过；距离、角度、误触发率和 OUT 电压未测 |
| Wi-Fi → Flask 数据上传 | completed（历史单向链路） | EXP-004 有 HTTP 200 接收日志；断网恢复和长期稳定性待测 |
| MOSFET / 风扇基本驱动 | completed（独立程序、全开状态） | EXP-012 基本开关和 100% 连续运行已观察；中间 PWM、温升、电流和安全参数未测 |
| 固件构建 | completed（所记录环境） | EXP-010 主环境构建；Phase 3.2.2 报告五个环境构建通过；硬件功能不能由编译替代 |
| 后端指令队列 | completed（软件测试） | Phase 3.2.1 报告 22/22 测试通过；内存队列持久化不是当前能力 |
| 前端控制软件 | completed（代码/UI） | 模式、档位和指令状态查询代码已存在；实体风扇端到端响应未验证 |
| 远程控制实机联调 | pending_verification | 需要网页发令、ESP32 接收执行、ACK、风扇动作和网页状态一致的同一次实测证据 |
| PIR + 温度自动控制实机联调 | pending_verification | 自动控制代码已实现/编译报告；完整实体联动尚无测试记录 |
| AI 模型训练与评价 | planned | 目前为规则推荐，尚无训练和独立评价结果 |

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
