# 06_web_interface_and_software — 网页界面与软件系统证据

本目录用于集中展示 SmartFan 2.0 的网页前端、后端 API 和软件控制功能。当前提交包含功能说明、源代码入口和已有软件验证记录索引。

> **证据边界：** 本目录现已收录 4 张于 2026-11-25 上传的 JPG 图片。GitHub 文件清单可以确认图片存在及文件名、时间顺序，但仅凭文件清单不能判断每张图具体展示了哪项功能；因此在完成逐张内容核对前，不将它们擅自标注为仪表盘、手动控制、指令成功或端到端实机测试证据。
## 1.1 已上传图片清单（按文件名时间顺序）

| 顺序 | 原始文件名 | 上传目录 | 内容分类 |
|---:|---|---|---|
| 1 | `IMG_20261125_124345[1].jpg` | 本目录 | 待逐图核对 |
| 2 | `IMG_20261125_124405[1].jpg` | 本目录 | 待逐图核对 |
| 3 | `IMG_20261125_124422[1].jpg` | 本目录 | 待逐图核对 |
| 4 | `IMG_20261125_124430[1].jpg` | 本目录 | 待逐图核对 |

> 文件名暂时保留原样，避免在未查看图片内容前误命名或改变原始证据。逐张确认后，再按实际内容补充说明和索引；不以普通界面截图替代浏览器请求、后端日志或完整软硬件联动证据。

## 1. 网页入口与源代码

- 前端页面：[`frontend/index.html`](../../../frontend/index.html)
- 前端交互逻辑：[`frontend/js/app.js`](../../../frontend/js/app.js)
- 前端样式：[`frontend/css/style.css`](../../../frontend/css/style.css)
- 前端说明：[`frontend/README.md`](../../../frontend/README.md)
- Flask 后端：[`backend/app.py`](../../../backend/app.py)
- 后端说明：[`backend/README.md`](../../../backend/README.md)

## 2. 已实现的软件功能

| 功能 | 实现内容 | 代码/记录入口 | 当前证据状态 |
|---|---|---|---|
| 温度展示 | 显示 ESP32 上传的温度数据 | `frontend/js/app.js` | 前端实现；历史数据上传见 EXP-004 |
| 湿度展示 | 显示 ESP32 上传的湿度数据 | `frontend/js/app.js` | 前端实现；历史数据上传见 EXP-004 |
| 温湿度图表 | 使用 ECharts 展示数据 | `frontend/index.html`、`frontend/js/app.js` | 前端实现；缺少本目录网页截图 |
| 系统状态 | 展示设备/传感器及风扇相关状态 | `frontend/js/app.js` | 前端实现；真实状态一致性仍待端到端核验 |
| 手动控制 | MANUAL 模式与 0–5 档位控制界面 | `frontend/js/app.js` | 前端代码实现；网页到实体风扇闭环未验收 |
| 自动/手动模式 | AUTO / MANUAL 模式切换 | `frontend/js/app.js`、`backend/app.py` | 软件逻辑实现；自动控制实机联动待验证 |
| 指令状态 | 创建指令、保存 command_id、轮询状态并反馈 | `frontend/js/app.js` | 修复报告和 API 测试记录见下文 |
| 后端命令队列 | 接收命令、ESP32 拉取、ACK 与状态查询 | `backend/app.py` | Phase 3.2.1 软件测试报告记录 22 项测试通过；不是实体端到端验收 |

## 3. 软件运行方法

在 Windows PowerShell 中，从仓库根目录分别启动后端和前端。

### 终端 A：启动 Flask 后端

```powershell
Set-Location .\backend
python -m pip install -r requirements.txt
python app.py
```

### 终端 B：启动网页前端

```powershell
Set-Location <仓库根目录>
python -m http.server 8080 --directory frontend
```

浏览器访问：

`http://localhost:8080`

前端配置的后端地址必须与实际 Flask 服务地址一致。ESP32 与运行 Flask 的电脑也必须处于可互相访问的网络环境。

## 4. 已有软件验证与修复记录

- [前端指令状态管理修复报告](../../../FRONTEND_FIX_REPORT.md)：记录 command_id 保存、状态查询参数、嵌套 command 对象解析等修复，以及 JavaScript 语法检查和后端 API 验证。
- [前端手动控制故障排查报告](../../../FRONTEND_MANUAL_CONTROL_DEBUG_REPORT.md)：记录浏览器按钮请求未到达后端时的排查过程。
- [前端手动控制测试指南](../../../FRONTEND_MANUAL_CONTROL_TEST_GUIDE.md)：列出 MANUAL 模式、档位按钮、Console 和 Network 的实际浏览器检查步骤。
- [Phase 3.2.1 后端指令队列记录](../../../experiment/records/2026-11-23/EXP-013_PHASE_3.2.1_BACKEND_COMMAND_QUEUE.md)：软件命令队列与自动化测试记录。
- [EXP-004 ESP32 Wi-Fi / Flask HTTP 上传](../../../experiment/records/2026-11-19/EXP-004-esp32-wifi-http-upload-verification.md)：历史单向传感器数据上传验证。

## 5. 建议补拍的真实网页证据

请在实际运行网页时截取原始屏幕，不要用绘图软件制作“仿真截图”。建议保存：

1. `web_01_dashboard.png`：网页完整仪表盘，能看到温度、湿度、图表和设备状态。
2. `web_02_manual_control.png`：MANUAL 模式和档位按钮界面。
3. `web_03_auto_manual_mode.png`：AUTO / MANUAL 模式切换界面。
4. `web_04_command_status.png`：点击档位后显示 command_id 和状态的页面。
5. `web_05_network_requests.png`：浏览器 Network 中真实 POST 指令和状态查询请求。
6. `web_06_backend_log.png`：同一轮测试对应的 Flask 请求日志。
7. 如完成整机联调，再补充网页发令、ESP32 收令、风扇动作及 ACK 的连续视频/记录。

每张截图应保留原始文件，并在旁边的记录中注明拍摄日期、测试步骤、实际观察结果和对应实验编号。若某项没有真实运行结果，应标记为“待验证”，不能仅凭代码截图声称功能实测通过。

## 6. 当前验收结论

- 网页前端和 Flask 后端源代码已纳入仓库。
- 温湿度展示、图表、模式切换、手动档位控制和指令状态查询有软件实现。
- 历史 ESP32 → Flask 传感器上传有 EXP-004 记录。
- 后端命令队列有软件自动化测试记录。
- **尚无本目录可核验的网页运行截图。**
- **网页 → Flask → ESP32 → MOSFET → 实体风扇 → ACK/UI 状态一致的完整实机闭环仍未验收。**
