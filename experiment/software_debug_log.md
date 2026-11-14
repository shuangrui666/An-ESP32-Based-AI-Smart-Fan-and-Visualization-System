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
