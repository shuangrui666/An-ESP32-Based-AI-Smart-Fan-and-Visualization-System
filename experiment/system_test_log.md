# 系统测试记录

用于记录模块测试、集成测试、异常测试和整机验收。测试结果必须对应明确的测试条件和证据。

## 1. 测试用例模板

### TEST-编号：测试名称

- 日期 / 时间：
- 关联实验编号：
- 测试版本 / Git commit：
- 测试类型：unit / integration / system / safety / performance
- 测试环境：
- 硬件配置：
- 软件配置：
- 前置条件：
- 测试步骤：
- 预期结果：
- 实际结果：
- 判定：PASS / FAIL / BLOCKED / NOT RUN
- 原始日志 / 图片 / 视频：
- 异常说明：
- 修复或后续行动：
- 复测编号：

## 2. 已有测试与验证索引

| 测试编号 | 日期 | 测试项目 | 实际结果 | 判定 | 证据 |
|---|---|---|---|---|---|
| TEST-001 | 2026-11-11 | DHT11 GPIO4 单项读取 | 22/22 次读取成功；主程序串口有有效读数 | PASS（单项范围） | [EXP-001](records/2026-11-11/EXP-001-esp32-dht11-validation.md) |
| TEST-002 | 2026-11-19 | ESP32 → Flask HTTP 上传 | Flask 收到 POST /api/sensor/data 并返回 HTTP 200 | PASS（单向上传） | [EXP-004](records/2026-11-19/EXP-004-esp32-wifi-http-upload-verification.md) |
| TEST-003 | 2026-11-20 | PIR GPIO5 原始信号 | 烧录成功；观察到 8 次 HIGH/LOW 周期，静止观察约 38 秒保持 LOW | PASS（初步信号读取；非标定） | [EXP-009](records/2026-11-20/EXP-009-pir-hardware-signal-test.md) |
| TEST-004 | 2026-11-22 | GPIO6 / MOSFET / USB 风扇独立驱动 | OFF/ON 周期与风扇动作对应；100% PWM 下连续运行 | PASS（独立程序、基本开关；非调速） | [EXP-012](records/2026-11-22/EXP-012-esp32s3-mosfet-fan-continuous-run-test.md) |
| TEST-005 | 2026-11-22 至 2026-11-24 | PlatformIO 固件构建 | 主环境修复构建成功；Phase 3.2.2 报告五个环境均成功 | PASS（构建验证） | [EXP-010](records/2026-11-22/EXP-010-esp32s3-fan-controller-build-fix.md)、[手动控制修复报告](../MANUAL_CONTROL_FINAL_FIX_REPORT.md) |
| TEST-006 | 2026-11-23 | Flask 指令队列自动化测试 | 22 项通过、0 项失败（仓库结果文件和交付报告） | PASS（后端软件测试） | [Phase 3.2.1 报告](../PHASE_3.2.1_DELIVERY_REPORT.md)、[test_results.json](../backend/test_results.json) |
| TEST-007 | 2026-11-23 至 2026-11-24 | 远程控制软件链路代码与接口 | 前端创建指令、后端队列、ESP32 轮询/ACK 软件实现及 API 验证有记录；未证明同一次真实网页到实体风扇动作 | BLOCKED / pending hardware verification | [Phase 3.2.2 报告](../PHASE_3.2.2_DELIVERY_REPORT.md)、[手动控制最终修复报告](../MANUAL_CONTROL_FINAL_FIX_REPORT.md) |
| TEST-008 | 2026-11-23 | PIR + 温度自动控制实机联动 | 固件逻辑和编译结果有报告；未找到完整实体风扇联动测试记录 | NOT RUN（整机联动） | [自动控制实现报告](../AUTO_FAN_CONTROL_IMPLEMENTATION.md) |

## 3. 尚待执行的测试范围

| 测试类别 | 检查内容 | 当前状态 |
|---|---|---|
| DHT11 回归 | 新接线后重复读取、异常处理和长期稳定性 | PENDING |
| PIR 标定 | OUT 电压、检测距离、角度、误触发率和长时间稳定性 | PENDING |
| 风扇电气安全 | 启动电流、驱动器输出、模块温升、供电余量和保护 | PENDING |
| PWM 调速 | 中间占空比启动能力、运行稳定性和可观察速度差异 | NOT RUN |
| 远程控制实机闭环 | 网页发令 → Flask → ESP32 → MOSFET → 风扇动作 → ACK/UI 状态一致 | NOT RUN |
| 自动控制实机闭环 | PIR + 温度阈值 + 60 秒超时 + 传感器故障安全关闭 | NOT RUN |
| 网络异常 | Wi-Fi/Flask 断开、恢复、ESP32 重启后的状态策略 | PENDING |
| AI 模型 | 数据集、训练、测试集评价和可复现指标 | NOT RUN |
| 整机稳定性 | 连续运行、重复性和故障恢复 | NOT RUN |

## 4. 测试判定规则

- PASS：按预先定义的步骤执行，实际结果满足预期，且证据可查。
- FAIL：执行后不满足预期，必须保留失败记录。
- BLOCKED：因硬件、环境或依赖未就绪而无法执行。
- NOT RUN：尚未执行。

不得将 BLOCKED 或 NOT RUN 写成 PASS。若测试条件变化，应建立新测试记录或清楚注明复测条件。
