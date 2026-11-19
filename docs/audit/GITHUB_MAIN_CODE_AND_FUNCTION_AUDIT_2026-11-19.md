# GitHub main 代码与功能核查报告

- 核查日期：2026-11-19
- 仓库：`shuangrui666/An-ESP32-Based-AI-Smart-Fan-and-Visualization-System`
- 核查分支：`main`
- 核查 commit：`79b3db7bb2e1dd8b2c3a004e61d6875f17044c04`
- 方法：只读 GitHub 文件与完整 Git tree；结合此前已提供的本地源码片段和 2026-11-14 的 EXP-002 代码审查记录
- 本次没有修改固件、后端、前端或硬件配置

## 1. 总结结论

**当前 GitHub `main` 不是此前曾经成功运行 Wi-Fi / Flask / Web 联调的完整代码版本。**

main 的完整 Git tree（GitHub 返回 `truncated=false`）中，与程序有关的文件只有：

- `src/main.cpp`
- `src/test_dht11_main.cpp`
- `src/sensors/dht_sensor.cpp`
- `src/sensors/dht_sensor.h`

未发现以下目录或源文件：

- `src/communication/`
- `src/control/`
- `src/ai/`
- `backend/`
- `frontend/`
- `firmware/`
- `dashboard/`
- `visualization/`

因此，main 当前不能作为完整 Wi-Fi / Flask / Web / 风扇控制系统的可复现源码快照。该结论不否认此前本地工作目录中的代码和历史联调成功记录。

## 2. GitHub main 文件核查

### 2.1 主固件

读取 `src/main.cpp`（blob SHA：`5b3fb8dfd331c0886291b9e1731144c43e78d448`）：

- 文件版本注释为 `0.1.0`、`Phase 1: Basic Test`。
- 只包含 `DHTSensor` 实例。
- `systemInit()` 初始化串口并初始化 DHT11。
- `printSystemStatus()` 读取并打印 DHT11 温湿度。
- `loop()` 每 2 秒打印一次状态，并延迟 100 ms。
- 未包含 Wi-Fi 管理器、Wi-Fi 连接、HTTP 上传、后端命令接收或风扇控制器调用。

判定：main 当前固件是 DHT11 基础测试程序，不是历史 Wi-Fi 联调时的完整固件。

### 2.2 PlatformIO 配置

读取 `platformio.ini`（blob SHA：`574151059ffb4b0ec29802b3a67094e7d86098cd`）：

- 主环境为 `esp32-s3-devkitc-1`。
- framework 为 Arduino。
- 主环境的 `build_src_filter` 包含 `main.cpp`、`sensors/`、`ai/`、`communication/`、`control/`。
- 但 main 的完整 tree 中实际不存在 `ai/`、`communication/`、`control/` 目录。
- 另有独立 `dht11-test` 环境，筛选 `test_dht11_main.cpp`。

判定：PlatformIO 配置仍引用当前 main tree 中不存在的功能目录。需要在补齐源文件或调整构建配置后，重新做干净编译验证；本报告不声称此次已运行编译。

### 2.3 传感器驱动

main 中存在 `src/sensors/dht_sensor.cpp` 和 `src/sensors/dht_sensor.h`。头文件定义 DHT11、GPIO4。历史 EXP-001 记录独立读取 22/22 成功及主程序串口读取成功；但当前 DHT11 已拆下的情况下，不能把历史结果当作当前接线状态或当前复测结果。

## 3. README 与 main 源码之间的状态冲突

当前 main 的 `README.md` 声称：

- ESP32-S3 已连接 Wi-Fi 并通过 HTTP 上传传感器数据；
- Flask 已接收真实温湿度；
- Web dashboard 已显示真实数据、图表和设备状态。

这些描述有此前同期项目记录和历史本地开发过程支持，应保留为**历史成功**。

但当前 main tree 不包含实现这些功能所需的 Wi-Fi 源码、Flask 后端和 Web 前端，当前 `src/main.cpp` 也没有 Wi-Fi 或 HTTP 调用。

因此建议使用如下准确表述：

- 历史开发阶段：Wi-Fi / HTTP / Flask / Web 联调曾成功（同期项目记录支持）。
- 当前 GitHub main：未归档这套联调所用的完整源码，不能从 main 单独复现。
- 当前本地工作目录：此前已确认存在相关源码文件；其版本与 main 的关系需要单独整理。
- 不应把历史成功改成失败，也不应声称 main 当前已经包含完整实现。

## 4. 本地代码与接口核对（基于此前已提供的本地源码片段）

本节描述此前检查过的本地 `E:\PlatformIOProjects\SmartFan2` 文件，不代表这些文件已进入 GitHub main。

### 4.1 ESP32 到 Flask 的传感器上传

此前提供的本地代码片段显示：

- `main.cpp` 初始化 `WifiManager` 并调用 `wifiManager.connect()`。
- 上传函数调用 `wifiManager.sendSensorData(temperature, humidity, -1)`。
- `wifi_manager.cpp` 使用 `HTTPClient`、JSON Content-Type 和 `http.POST(payload)`。
- `BACKEND_URL` 被用于初始化 `_backendURL`，但具体宏值尚未从已提交给本次核查的输出中确认。
- 此前片段中 `main.cpp` 的上传调度注释为每 5 秒；头文件另有 `SENSOR_DATA_INTERVAL 2000`。实际调用链需以宏定义和代码为准，不能混为同一参数。

### 4.2 Flask 接收接口

此前提供的本地 `backend/app.py` 片段显示：

- 路由：`POST /api/sensor/data`。
- 必填 JSON 字段：`temperature`、`humidity`、`fan_gear`。
- 可选字段：`wifi_signal`。
- 成功分支返回 HTTP 200，JSON 中包含 `status: success`、`message: Data received` 和 `record_id`。
- 服务启动配置为 `host='0.0.0.0'`、`port=5000`。

### 4.3 前端接口

此前提供的本地 `frontend/js/app.js` 片段显示：

- `BACKEND_URL = 'http://localhost:5000'`。
- 前端读取 `GET /api/status`、`GET /api/history` 和 `GET /api/fan/recommendation`。
- 前端控制函数向 `POST /api/fan/control` 发送 `action: 'set_gear'` 和 `gear`。

### 4.4 当前接口一致性判断

- 传感器上传：本地固件调用、Flask 接口字段之间存在预期对应关系；但本次尚未读取完整 `sendSensorData()` JSON 序列化实现，因此字段逐项一致性仍标记为**部分核实**。
- 前端读取：前端请求的 status/history/recommendation 路径与此前检查过的 Flask 路由清单相符。
- 风扇控制：前端有发送控制请求的代码，不等于后端已实现命令转发；此前 EXP-002 记录后端控制接口仍待实现。
- ESP32 命令接收：此前本地 `main.cpp` 检查未发现从后端接收风扇命令并执行的流程。
- 因此当前不能宣称网页按钮已经形成“网页 → Flask → ESP32 → 驱动器 → 风扇”的闭环。

## 5. 风扇控制功能现状

根据 main 源码和 2026-11-14 的本地 EXP-002 审查记录：

| 环节 | 核查结果 | 状态 |
|---|---|---|
| 前端档位按钮/请求 | 本地前端存在 `POST /api/fan/control` 请求代码 | 有界面/请求代码；不是实体控制 |
| Flask 控制接口 | 此前审查记录为待实现逻辑 | 未形成已验证转发 |
| ESP32 命令接收 | 本地主程序审查未发现接收并处理后端风扇命令的流程 | 未完成 |
| FanController 类 | EXP-002 审查的本地文件存在控制器实现 | 历史本地代码，不在当前 main |
| PWM 初始化 | EXP-002 发现 `ledcSetup()`、`ledcAttachPin()` 被注释 | 未启用 |
| PWM 输出 | EXP-002 发现 `ledcWrite()` 被注释，当前逻辑用 `digitalWrite(HIGH/LOW)` | 不能实现占空比调速 |
| 主程序集成 | EXP-002 未发现主程序实例化或调用 FanController | 未集成 |
| GPIO21 | EXP-002 中为临时默认值 | 未经硬件核实，不可直接作为接线依据 |
| 实体风扇驱动 | 当前未完成可靠接线和负载验证 | 未验证 |

**风扇控制结论：** 当前只能说存在过软件层的控制器草稿和网页请求入口；尚无证据证明 PWM 已实际输出、后端已把命令发给 ESP32，或实体风扇已实现开关/调速。

## 6. 硬件接好后仍需真正测试的项目

硬件接好前先完成电源、驱动器、接口和 GPIO 核实。之后按以下顺序测试：

1. **DHT11 回归测试**：重新接线后验证初始化、温度/湿度读取、连续读数和异常读取处理。历史 22/22 成功不代替本次复测。
2. **PIR 测试**：核实模块型号、VCC/GND/OUT 端子、供电电平和 ESP32 输入兼容性；分别测试静止 LOW、移动触发 HIGH、触发延时、重复触发。当前 main 没有 PIR 逻辑。
3. **Wi-Fi 回归测试（如需验证当前本地固件）**：记录 SSID（不记录密码）、ESP32 IP、电脑 LAN IP、实际后端 URL、连接日志和断线恢复；历史成功保留为历史结果。
4. **HTTP 上传测试**：记录请求 URL、JSON 字段、HTTP 状态码、Flask 接收日志和前端显示；使用同一条测试数据做端到端对照。
5. **后端接口测试**：合法数据、缺字段、错误类型、空 JSON、重复数据、历史记录上限、服务重启后的数据行为。
6. **网页显示测试**：status/history/recommendation 的正常、空数据、后端离线和错误响应状态。
7. **风扇驱动无负载测试**：先测控制信号和安全默认状态；核实逻辑电平、PWM 频率/占空比、急停/断电行为。不可让 GPIO 直接承担风扇电流。
8. **风扇负载测试**：确认风扇电气参数、启动电流、驱动器和电源额定值后，在教师指导及保护条件下测试 OFF/低/中/高档；记录实际电压、电流、异常发热和转速（若无转速传感器，只记录观察到的档位效果，不虚报 RPM）。
9. **端到端控制测试**：网页发命令 → Flask 校验 → ESP32 收到并确认 → 驱动器动作 → 风扇实际响应 → 状态回传网页。
10. **故障与稳定性测试**：Wi-Fi 断开、Flask 停止、无效档位、传感器掉线、重启恢复和持续运行；确认失联时输出进入预先定义的安全状态。

## 7. 当前工作决策

1. 不要直接把当前本地文件夹整体覆盖到 main；先确定唯一权威本地源码目录及版本。
2. 从本地工作目录整理一个可编译的完整源码快照，排除 `.venv`、密钥、缓存和构建产物。
3. 先让源码在独立分支中通过 PlatformIO 编译，并检查 Flask / 前端运行和接口一致性。
4. 补齐命令接收、后端转发和风扇驱动前，不把风扇控制标记为完成。
5. 将历史 Wi-Fi 成功记录与当前 main 缺少源码的事实同时保留；不要相互覆盖。

## 8. 证据与限制

- main tree 和文件内容来自 GitHub commit `79b3db7bb2e1dd8b2c3a004e61d6875f17044c04`。
- 历史本地通信源码片段来自 2026-11-19 对话中用户粘贴的 PowerShell 输出。
- 风扇控制器细节来自 `experiment/records/2026-11-14/EXP-002-firmware-fan-control-audit.md`，属于此前本地代码审查的事后记录；不是当前 main 中的源码。
- 本报告没有读取用户当前电脑上的实时文件，也没有运行编译、Flask 服务、网页或硬件测试。
- 本报告只新增文档，不修改现有源代码，不合并 PR。
