# EXP-004：ESP32-S3 Wi-Fi / HTTP / Flask / Web 数据链路历史验证

- 记录日期：2026-11-19
- 所述测试日期：截至 2026-11-14 的历史开发阶段；精确执行日期未从原始日志确认
- 类型：软件系统集成测试（历史测试回溯）
- 判定：**PASS（历史项目记录支持）；原始证据完整性受限**
- 项目工作目录：`E:\\PlatformIOProjects\\SmartFan2` 与 `E:\\PlatformIOProjects\\SmartFan2_GitHub`（后续扫描确认两目录均有相关源文件；测试当时具体使用哪一目录尚未由原始日志确认）

## 1. 实验目的

回溯此前已经完成的 ESP32-S3 无线数据链路验证，明确区分：
1. 当时项目记录所报告的成功功能；
2. 目前能够恢复的操作信息；
3. 尚未找到的原始日志和复现参数。

本记录不是 2026-11-19 的重新测试，也不代表当前 GitHub `main` 已包含全部运行代码。

## 2. 已有同期记录支持的结果

2026-11-14 的项目状态记录（`PROJECT_COMPLETE.md` 内容摘要及仓库中的 EXP-003）明确记录了以下完成状态：

- ESP32-S3 成功连接 Wi-Fi。
- ESP32-S3 通过 HTTP 上传传感器数据到 Flask 后端。
- Flask 后端接收到了 ESP32-S3 的实际温湿度读数。
- Web 前端显示了实际温湿度、图表和设备状态。

因此，历史 Wi-Fi/HTTP/Flask/Web 数据链路应记录为**曾经验证成功**，不能再笼统标记为测试失败或从未执行。

## 3. 可恢复的环境与启动信息

| 项目 | 已恢复的信息 | 证据限制 |
|---|---|---|
| 主控板 | YD-ESP32-S3 Classic V1.0 | 来自项目记录 |
| 温湿度传感器 | DHT11，历史配置 GPIO4 | 来自项目记录；不表示当前仍已接线 |
| Wi-Fi | ESP32-S3 曾成功连接无线网络 | SSID、加密方式、网络频段及 DHCP 详情未恢复；不得记录密码 |
| 设备到后端 | 本地源码显示使用 HTTP POST 上传 JSON；Flask 路由为 `POST /api/sensor/data` | ESP32 实际 `BACKEND_URL` 值及固件实际请求体尚未恢复；代码返回值不等于历史运行状态码 |
| Flask 后端启动命令 | 在 backend 目录执行 `python app.py` | 来自此前项目操作记录 |
| Flask 地址 | `http://127.0.0.1:5000` | 项目记录中的本机访问地址；未恢复 ESP32 实际访问的主机 IP |
| 前端启动命令 | `python -m http.server 8080 --directory frontend` | 来自此前项目操作记录 |
| 前端地址 | `http://localhost:8080` | 项目记录中的浏览器访问地址 |
| PlatformIO | 曾记录执行 `pio run` | 未恢复对应测试固件的精确 commit、upload 命令和完整串口输出 |

**网络地址说明：** `127.0.0.1` 和 `localhost` 是运行浏览器/后端所在计算机的回环地址，不是 ESP32 可直接访问的局域网服务器地址。ESP32 实际使用的电脑局域网 IP 尚未从历史证据中找回，因此本记录不猜测该 IP。

## 4. 历史测试过程（按已知信息重建）

以下步骤是依据已记录的启动命令和功能结果整理的过程摘要，不应冒充逐秒原始操作记录：

1. 在项目环境中准备 ESP32-S3 固件及 DHT11 传感器数据采集。
2. ESP32-S3 连接当时使用的 Wi-Fi 网络。
3. 启动 Flask 后端：进入 `backend` 目录并执行 `python app.py`。
4. 启动前端静态服务器：在项目目录执行 `python -m http.server 8080 --directory frontend`。
5. 浏览器访问 `http://localhost:8080`。
6. ESP32-S3 将实际温湿度数据通过 HTTP 发送到 Flask。
7. 检查后端接收情况和网页显示结果；同期项目记录报告实际温湿度、图表和设备状态均有显示。

## 5. 测试结论

| 子项 | 历史结论 | 依据 |
|---|---|---|
| ESP32 Wi-Fi 连接 | PASS（历史记录） | 2026-11-14 项目状态记录 |
| ESP32 HTTP 上传 | PASS（历史记录） | 同期记录明确写明 HTTP 上传已验证 |
| Flask 接收真实传感器数据 | PASS（历史记录） | 同期记录明确写明后端接收实际读数 |
| Web 显示真实温湿度/图表/设备状态 | PASS（历史记录） | 同期记录明确写明 Dashboard 显示实际数据 |
| 断线重连与长期稳定性 | 未确认 | 无对应日志 |
| 网页指令控制实体风扇 | 未验证 / 不属于本次通过项 | 风扇驱动尚未完成 |
| 当前 GitHub main 的可复现性 | 未确认 | 历史工作目录和当前远程 main 快照不一致 |

## 6. 2026-11-19 本地源码核查补充

以下是对本地工作目录 `E:\\PlatformIOProjects\\SmartFan2` 中源码的静态检查结果。它们证明代码结构和配置，不是历史运行日志，也不能单独证明当时请求成功。

| 核查项 | 源码中确认的事实 | 证据边界 |
|---|---|---|
| Wi-Fi 初始化 | `WifiManager::begin()` 设置 Station 模式并启用自动重连；`connect()` 调用 `WiFi.begin()`，连接成功后打印设备 IP | 静态代码事实；不替代历史串口输出 |
| 后端 URL 初始化 | 构造函数使用 `_backendURL(BACKEND_URL)` | `BACKEND_URL` 宏的具体定义值尚未从本次已提供的输出中确认 |
| HTTP 发送 | `_sendHTTPPost()` 设置 JSON Content-Type，调用 `http.POST(payload)` 并返回 HTTP 状态码 | 发送函数代码已核实；实际运行时 URL、请求体和状态码未恢复 |
| Flask 接口 | `POST /api/sensor/data`；必填字段为 `temperature`、`humidity`、`fan_gear`；可选读取 `wifi_signal`；成功分支返回 HTTP 200 和 `status: success` | 这是后端代码的接口契约，不证明历史请求体与该契约完全一致 |
| Flask 监听 | `app.run(host='0.0.0.0', port=5000, debug=True)` | 代码配置监听所有网卡；当时电脑的 LAN IPv4 和 ESP32 实际访问地址仍未恢复 |
| Web 前端 | `BACKEND_URL = 'http://localhost:5000'`；前端调用 `/api/status`、`/api/history`、`/api/fan/recommendation`、`/api/fan/control` | 本机浏览器访问地址；不能据此推断 ESP32 使用 localhost |
| 上传调度 | 本地 `main.cpp` 的循环以 `SENSOR_UPLOAD_INTERVAL_MS` 作为上传判断条件，注释标为每 5 秒；`wifi_manager.h` 另有 `SENSOR_DATA_INTERVAL 2000` | 两个符号不同；实际宏定义及运行时频率尚未完整核实，不将 2 秒和 5 秒混为一谈 |
| HTTP 超时 | `HTTP_TIMEOUT` 在头文件中定义为 5000 ms，发送函数调用 `http.setConnectTimeout(HTTP_TIMEOUT)` | 静态代码配置 |

**地址说明补充：** 后端绑定 `0.0.0.0:5000` 表示 Flask 监听本机网络接口；前端的 `localhost:5000` 是浏览器所在电脑的回环地址。ESP32 作为另一台网络设备，必须访问当时电脑在同一网络中可达的地址。由于实际 `BACKEND_URL` 定义值尚未确认，本记录不推测或补写具体 IP。

## 7. 证据完整性与缺口

目前已恢复的是同期项目状态记录、部分操作命令和 2026-11-19 本地源码静态检查结果，不是历史原始运行证据。以下材料尚未找到，故不得伪造或补写为事实：

- ESP32 完整串口日志（包括 Wi-Fi 连接成功行、IP 地址、HTTP 请求和响应）。
- 测试当时电脑的局域网 IPv4 地址。
- ESP32 请求的确切 API 路径、HTTP 方法、请求体和状态码。
- Flask 控制台完整日志。
- 测试当时的源码 commit SHA / 固件版本。
- 浏览器页面截图或录屏。
- 原始测试视频及其文件路径。

后续若从本地日志、截图、视频或历史备份中找回，应作为原始附件追加，并记录文件名、采集时间及 SHA-256；不要覆盖本历史回溯记录。

## 8. GitHub 归档状态

本记录用于修正文档对历史测试状态的错误描述。它目前写入独立文档更新分支 `docs/reconcile-2026-11-19`，并通过 PR #1 提交审阅；**尚未合并到 main**。

该记录证明项目曾有成功测试的同期记录，但不能代替原始日志，也不能证明当前 main 已包含当时全部代码。

## 9. 后续补证清单

- [ ] 从本地 `PROJECT_COMPLETE.md` 核对并归档完整原文。
- [ ] 检查 `backend/logs` 是否存在 Flask 历史日志。
- [ ] 检查项目 evidence / logs 目录是否有串口日志、截图或录屏。
- [x] 从本地 `backend/app.py` 与 `frontend/js/app.js` 核对 Flask API 路径、后端字段校验和前端请求路径（只作为代码事实，不等同于历史运行证据）。
- [x] 核对本地 `wifi_manager.cpp` 的 Wi-Fi 初始化与 HTTP POST 实现。
- [ ] 找回 `BACKEND_URL` 宏的实际定义值，并确认 ESP32 实际请求体（无需重新连接或烧录；只需已有源码中的非敏感配置片段）。
- [ ] 若原始证据找回，记录原文件 SHA-256 并链接到本实验。
- [ ] 如需重新验证，在记录新的测试日期、代码 commit 和原始日志后执行回归测试。

## 10. 资料来源与可信度说明

- 2026-11-14 的 `PROJECT_COMPLETE.md` 项目状态记录（此前对话中已确认其记录 ESP32 Wi-Fi、HTTP、Flask、Dashboard 成功；本次尚未取得完整原文文件）。
- 仓库 `experiment/records/2026-11-14/EXP-003-readme-status-reconciliation.md`，其中明确记录 Wi-Fi/HTTP、Flask 接收真实读数和 Web 展示为已验证。
- 此前对话中恢复的命令：`python app.py`、`python -m http.server 8080 --directory frontend`、`pio run`。

证据等级：**同期项目记录可支持历史成功结论；原始运行证据尚缺，复现参数不完整。**
