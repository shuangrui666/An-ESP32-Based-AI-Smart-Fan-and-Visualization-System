# 软件开发与调试日志

用于记录 ESP32 固件、Python 后端、Web 前端及通信接口的开发和调试。每个问题应能关联到代码版本和实验编号。

## 1. 调试问题记录模板

### BUG-编号：简短标题

- 日期 / 时间：
- 关联实验编号：
- 模块：firmware / backend / frontend / communication / environment
- 软件版本 / Git commit：
- 开发环境：
- 问题描述：
- 复现步骤：
- 预期结果：
- 实际结果：
- 完整报错 / 串口输出：
- 初步分析：
- 尝试过的解决方案：
- 最终修改：
- 修改文件：
- 验证步骤：
- 验证结果：
- 是否解决：yes / no / partial
- 相关截图 / 日志路径：
- 后续事项：



## 2.1 BUG-001：主程序 DHT11 未完成初始化

- 日期：2026-11-11
- 关联实验编号：[EXP-001](records/2026-11-11/EXP-001-esp32-dht11-validation.md)
- 模块：firmware / sensors
- 软件环境：VS Code + PlatformIO；具体版本未记录
- 问题描述：主程序持续输出 DHT11 读取失败或 NaN；独立测试程序在相同 GPIO4、DHT11 条件下可以正常读取。
- 复现现象（修复前）：
  - `[DHTSensor] Read failed, temperature=nan, humidity=nan`
  - `[DHT11] Temperature: Read Failed`
  - `[DHT11] Humidity: Read Failed`
- 对比发现：独立测试对象创建后调用了 `testDHT->begin(55)`；主程序创建 `DHT` 对象后缺少对应初始化调用。
- 原因定位：`src/sensors/dht_sensor.cpp` 的 `begin()` 函数中未调用 DHT 对象的 `begin(55)`。
- 最终修改：对象创建后增加 `_dhtInstance->begin(55);`。
- 其他修改：`platformio.ini` 主程序环境增加 `build_src_filter`，排除独立测试源文件，避免重复定义；过滤规则以当前仓库配置为准。
- 验证步骤：PlatformIO Build；上传主程序至 COM5；串口监视器 115200 波特率观察输出。
- 验证结果：用户报告 Build 和 Upload 均成功。主程序串口日志连续两轮显示 `[DHTSensor] Read OK: temp=25.0°C, hum=42.0%`，并输出温度 25.0 °C、湿度 42.0%。
- 是否解决：yes（本次读取问题已修复并通过当前验证；长期稳定性仍待测试）。
- 证据路径：`experiment/records/2026-11-11/EXP-001-esp32-dht11-validation.md`。
- 后续事项：继续观察长时间运行稳定性；在风扇驱动和供电安全核实前，不进行风扇控制测试。

## 2. 调试记录索引

| BUG 编号 | 日期 | 模块 | 问题摘要 | 状态 | 关联实验 |
|---|---|---|---|---|---|
| BUG-001 | 2026-11-11 | firmware / sensors | 主程序缺少 DHT11 begin(55) 初始化，修复后读取成功 | resolved (current test) | [EXP-001](records/2026-11-11/EXP-001-esp32-dht11-validation.md) |


## 2.2 FIND-002：风扇控制器尚未输出真实 PWM，且未接入主程序

- 日期：2026-11-14
- 关联实验编号：[EXP-002](records/2026-11-14/EXP-002-firmware-fan-control-audit.md)
- 模块：firmware / control / integration
- 软件环境：Windows + VS Code / PowerShell + PlatformIO；本次未重新记录工具版本
- 检查文件：`src/control/fan_controller.cpp`、`src/main.cpp`、`src/sensors/dht_sensor.h`、`src/sensors/dht_sensor.cpp`
- 问题描述：源码包含风扇档位和 PWM 参数，但实际 PWM 初始化和写入被注释；主程序未发现风扇控制器实例、初始化或档位调用。
- 实际检查结果：
  - `FAN_CONTROL_PIN` 的临时默认值为 GPIO21，注释明确要求根据实际情况修改；不能据此直接接线。
  - `DHT_SENSOR_PIN` 定义为 GPIO4，与此前 DHT11 接线记录一致。
  - `begin()` 只配置普通 OUTPUT 并拉低；`ledcSetup()`、`ledcAttachPin()` 被注释。
  - `_updatePWM()` 中 `ledcWrite()` 被注释；实际以 `digitalWrite(HIGH/LOW)` 代替，因此所有非零占空比都会变成持续高电平。
  - 控制器源码定义 OFF 和 GEAR_1 至 GEAR_5，与项目目标 OFF/1/2/3 档不一致。
  - `main.cpp` 只初始化 DHT11 和 Wi-Fi；主循环每 2 秒打印状态、每 5 秒上传传感器数据。上传调用使用 `sendSensorData(temperature, humidity, -1)`。本次检查没有发现接收风扇命令的逻辑。
  - `src/control/fan_controller.h` 不存在；本次没有擅自创建该文件。
- 初步分析：风扇控制器尚处于未完成/未集成状态；软件档位计算不能代表真实 PWM 输出或真实转速。
- 最终修改：本次只进行只读审查和记录，没有修改固件源代码。
- 验证结果：代码现状已通过终端输出确认；真实风扇、驱动电路、PWM 波形和档位转速均未测试。
- 是否解决：no（这是现状审查记录，不是修复）。
- 后续事项：先审查 Wi-Fi 通信模块与 Flask API，再确认风扇内部电路和启动电流，确定驱动器及 GPIO；之后才实现并验证真实 PWM 和双向控制。

## 3. 调试规范

1. 尽量保留完整错误信息，不只记录“程序运行失败”。
2. 记录运行环境、依赖版本、板卡型号和代码 commit。
3. 一次尽量只改变一个关键因素，便于判断原因。
4. 不删除失败方案；记录尝试及失败原因。
5. 修复后执行可重复的验证步骤。
6. 不将未验证的修复写成“已解决”。
7. 日志和截图应去除密码、令牌及个人隐私。


## 2.3 DOC-003：README 项目状态校正与文档修改过程

- 日期：2026-11-14
- 关联记录：[EXP-003](records/2026-11-14/EXP-003-readme-status-reconciliation.md)
- 模块：documentation / project-status
- 修改目标：将 README 的项目状态与截至当日已验证的软件功能、尚未完成的实体风扇控制保持一致。
- 修改过程：
  1. 先备份本地 README。
  2. 首次整段字符串替换因 Markdown 空格/换行不一致而未匹配；保护性检查触发，未写入文件。
  3. 改用逐行定位更新功能状态表，避免依赖整段换行格式。
  4. 读取并核对顶部状态、功能表和风扇参数。
  5. 只替换 README 第11节，保留第12节 Evidence-Based Development。
  6. 检查并修正英文单词间缺失的空格。
- 修改结论：README 已反映 DHT11、Wi-Fi/HTTP、Flask 数据接收和 Web 实时数据显示的已验证状态；风扇控制、真实三档调速和训练好的 ML 模型仍未实现。
- 注意：本次是文档修订，不是硬件实验；没有测量风扇启动电流、确定电机类型、完成驱动接线或验证实体 PWM。
- 协作改进：后续应优先复用用户此前提供的型号、参数、硬件清单和电路资料，不反复要求用户重复提供；将时间优先投入风扇实体控制。
- 状态：completed（文档记录）；实体风扇控制问题仍 open。


## 2.4 FIX-010：ESP32-S3 LEDC API 兼容性、PIR 引脚定义与格式字符串修复

- 记录日期：2026-11-22
- 实际构建日期：待确认（用户未提供本次编译的具体日期）
- 关联实验：[EXP-010](records/2026-11-22/EXP-010-esp32s3-fan-controller-build-fix.md)
- 模块：firmware / control / build
- 开发环境：Windows + VS Code + PlatformIO；具体版本未提供
- 目标环境：`esp32-s3-devkitc-1`
- 问题来源：用户转述 VS Code 中 Qwen 的修复总结；本次记录未取得完整修复前后 diff。
- 修复前问题：
  1. `fan_controller.cpp` 使用旧版 LEDC API（`ledcSetup`、`ledcAttachPin`、按 channel 调用 `ledcWrite`），与当前 ESP32-S3 Arduino 核心 API 不兼容。
  2. `main.cpp` 使用的 `PIR_SENSOR_PIN` 未定义。
  3. 存在 5 处格式字符串与参数类型不匹配警告。
- 据用户提供的修复摘要：
  1. 使用 `ledcAttach(gpio, freq, resolution)` 替换旧版通道配置方式。
  2. 使用 `ledcWrite(gpio, duty)` 按 GPIO 写入 PWM 占空比。
  3. 在 `main.cpp` 中添加 `PIR_SENSOR_PIN` 定义。
  4. 修复 5 处格式字符串与参数类型不匹配问题，涉及 `%d → %lu` 和 `%lu → %u`。
- 验证结果（用户提供）：编译成功、无警告；RAM 47,040 / 327,680 bytes（14.4%）；Flash 1,020,840 / 3,342,336 bytes（30.5%）；生成 `.pio/build/esp32-s3-devkitc-1/firmware.bin`。
- 当前结论：本次代码通过目标环境编译验证。由于尚未取得修复代码的 commit SHA、完整 diff、烧录日志和实际 PWM 测量，不能据此确认 PWM 输出、MOSFET 驱动或风扇动作正常。
- 是否解决：编译层面 yes（按用户提供报告）；硬件层面 pending_verification。
- 后续事项：
  1. 补存完整构建终端日志和本次修复的 Git commit。
  2. 只读核对 PWM GPIO、频率、分辨率、占空比、输出极性及 PIR 输入调用链。
  3. 在电气安全条件满足后进行烧录、PWM、MOSFET 和风扇实测。
  4. 单独验证最后一次 PIR 触发后 60 秒关闭及再次触发刷新计时的行为。

## 2.5 DEF-001 / DEF-003：LEDC 失败处理与 HTTP 超时返修

- 日期：2026-11-22
- 关联实验：[EXP-011](records/2026-11-22/DEF-001_REPAIR_REPORT.md)
- 模块：firmware / fan-control / communication
- 信息来源：用户提供的 Qwen 返修摘要；尚未取得完整源码 diff 和本轮构建原始日志。
- DEF-001 修复摘要：
  1. 删除使用 `ledcRead()` 返回值小于零判断初始化失败的错误逻辑。
  2. 据报告，`fan_controller.cpp` 现在检查 `ledcAttach()` 返回值；失败时清除初始化状态、记录错误并返回失败。
  3. 据报告，PWM 写入检查 `ledcWrite()` 返回值；控制函数在未初始化时提前返回。
  4. `main.cpp` 在 `fanController.begin()` 失败时输出错误日志。
- DEF-003 修复摘要：
  1. 据报告，`_sendHTTPPost()` 与 `_sendHTTPGet()` 均设置 `http.setConnectTimeout(HTTP_TIMEOUT)`。
  2. 两条路径均增加 `http.setTimeout(HTTP_TIMEOUT)`，当前配置值据报告为 5 秒。
  3. 该配置涉及连接与响应读取等待，不应表述为整个同步 HTTP 请求的绝对总耗时上限。
- 编译验证：据用户提供报告，三个 PlatformIO 环境均编译成功；RAM 47,040 bytes（14.4%），Flash 1,021,176 bytes（30.6%）。
- 验证边界：本轮没有烧录、没有硬件上电、没有测量 PWM、没有验证 MOSFET / 风扇动作；HTTP 超时行为也尚未通过断网或服务端无响应测试。源码 diff、commit SHA 和完整构建日志待补。
- 当前状态：编译层面通过（据报告）；源码级与运行时验证 pending。
- 后续事项：先核对本轮完整 diff 和构建日志，再完成 MOSFET 接线安全审查；满足电气安全条件后，分项验证 PWM、风扇控制和 HTTP 超时行为。

