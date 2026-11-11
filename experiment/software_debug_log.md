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

## 3. 调试规范

1. 尽量保留完整错误信息，不只记录“程序运行失败”。
2. 记录运行环境、依赖版本、板卡型号和代码 commit。
3. 一次尽量只改变一个关键因素，便于判断原因。
4. 不删除失败方案；记录尝试及失败原因。
5. 修复后执行可重复的验证步骤。
6. 不将未验证的修复写成“已解决”。
7. 日志和截图应去除密码、令牌及个人隐私。
