# 实验记录与证据管理

本目录用于保存 SmartFan 2.0 项目的实验计划、过程记录、原始证据、问题排查和测试结果。记录应真实、及时、可追溯。

## 目录说明

| 文件 / 目录 | 用途 |
|---|---|
| [experiment_log.md](experiment_log.md) | 全项目实验总日志与进度索引 |
| [hardware_inventory.md](hardware_inventory.md) | 实际硬件清单、型号与参数核实 |
| [hardware_connection_log.md](hardware_connection_log.md) | 接线方案、引脚分配、连接检查与变更 |
| [software_debug_log.md](software_debug_log.md) | 固件、后端、前端调试过程 |
| [ai_experiment_log.md](ai_experiment_log.md) | 数据集、模型训练、评价与对比 |
| [system_test_log.md](system_test_log.md) | 单元、集成、异常和整机测试 |
| [records/](records/README.md) | 每次实验的独立记录与模板 |

## 记录规则

1. 每次实验分配唯一编号，例如 `EXP-001`、`EXP-002`，编号一经使用不重复分配。
2. 实验开始前记录目的、设备、条件和步骤；实验后补充真实现象、结果、问题及结论。
3. 失败实验、异常数据和被放弃的方案也要保留，注明原因，不得伪装成成功结果。
4. 照片、视频、串口日志和数据文件使用实验编号关联，例如 `EXP-001-01.jpg`。
5. 原始数据只追加、不覆盖；处理结果另存，并记录处理脚本、参数和输入文件。
6. 所有参数必须来自实物标签、数据手册、测量或可追溯来源。未知内容写“待核实”，不可猜测。
7. 模拟数据必须明确标记 `SIMULATED`，不得与真实实验数据混淆。
8. 每次重要修改都通过 Git commit 留下记录，提交说明应概括实际变更。
9. 不上传 Wi-Fi 密码、API Key、访问令牌、个人隐私或其他秘密信息。
10. 大型视频可留存在本地或经批准的存储位置；仓库中至少记录文件名、时间、说明和存放位置。不要未经确认就提交超大文件。

## 建议的实验文件命名

`records/YYYY-MM-DD/EXP-NNN-short-description.md`

例如：`records/2026-11-09/EXP-001-hardware-inventory.md`

日期应使用实际实验日期，不要为了符合示例而填写未发生的日期。

## 实验状态定义

- `planned`：已计划，尚未执行。
- `in_progress`：正在执行。
- `completed`：按记录完成，证据已整理。
- `failed`：实验执行失败，保留失败证据。
- `pending_verification`：已有记录，但关键事实或证据仍待核实。

## 实验结论要求

结论应区分“观察到的事实”“根据事实作出的解释”和“尚不能确定的事项”。样本不足、参数未确认或证据缺失时，明确写出限制。
