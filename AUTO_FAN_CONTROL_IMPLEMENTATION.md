# SmartFan 2.0 自动风扇控制功能实现报告

## 项目信息

- **项目名称**: SmartFan 2.0 - ESP32 智能风扇控制系统
- **功能模块**: 自动风扇控制（温度 + PIR 联合控制）
- **实现日期**: 2026-11-23
- **固件版本**: 0.2.0

---

## 一、修改文件清单

### 主要修改文件

| 文件路径 | 修改内容 |
|---------|---------|
| `src/main.cpp` | 1. 添加温度阈值配置宏<br>2. 重写 `handlePIRAndFanControl()` 函数，实现温度+PIR 联合控制逻辑 |

### 修改详情

#### 1. 添加温度阈值配置宏（第 42-47 行）

```cpp
// ============================================================
// 温度控制阈值配置
// ============================================================

// 风扇启动温度阈值（℃）
#define TEMP_START_THRESHOLD 28.0

// 风扇关闭温度阈值（℃）- 形成 2℃回差
#define TEMP_STOP_THRESHOLD 26.0
```

#### 2. 重写 `handlePIRAndFanControl()` 函数（第 286-364 行）

完整实现了以下功能：
- ✅ 温度数据读取和错误处理
- ✅ 温度+PIR 联合判断逻辑
- ✅ 60 秒无人延时计时
- ✅ 温度低于 26℃时关闭风扇
- ✅ 传感器读取失败时关闭风扇并输出错误日志
- ✅ 详细的串口日志输出

---

## 二、完整控制逻辑

### 控制规则

| 序号 | 条件 | 动作 |
|-----|------|------|
| 1 | 检测到有人 + 温度≥28℃ | 启动风扇（PWM 255，100% 占空比） |
| 2 | 检测到有人 + 温度<28℃ | 不启动风扇 |
| 3 | 风扇运行后暂时检测不到人 | 进入 60 秒延时计时 |
| 4 | 延时期间再次检测到人 | 重新计算延时 |
| 5 | 温度低于 26℃ | 关闭风扇（形成 2℃回差） |
| 6 | 传感器读取失败 | 关闭风扇，输出错误日志 |

### 状态机流程

```
┌─────────────────────────────────────────────────────────────┐
│                        状态机流程图                          │
└─────────────────────────────────────────────────────────────┘

                    ┌──────────┐
                    │   IDLE   │ ← 初始状态，风扇关闭
                    └────┬─────┘
                         │ PIR 触发 + 温度≥28℃
                         ▼
                  ┌──────────────┐
                  │  SOFT_START  │ → 软启动到 100% 占空比
                  └──────┬───────┘
                         │ 软启动完成
                         ▼
                   ┌──────────┐
                   │ RUNNING  │ ← 风扇全速运行
                   └────┬─────┘
          ┌─────────────┼─────────────┐
          │             │             │
          │             │             │ 温度<26℃
          │             │             ▼
          │             │       ┌────────────┐
          │             │       │ SOFT_STOP  │ → 软停止到关闭
          │             │       └────────────┘
          │             │
  60 秒无活动           │ PIR 触发 + 温度≥28℃
          │             │
          └─────────────┘
              超时
```

### 关键逻辑说明

1. **温度回差设计**: 
   - 启动温度：28℃
   - 关闭温度：26℃
   - 回差：2℃（防止温度在阈值附近频繁启停）

2. **PIR 超时机制**:
   - 超时时间：60000ms（60 秒）
   - 每次 PIR 有效触发刷新 `lastPirActivityTime`
   - 连续 60 秒无新触发后启动软停止

3. **传感器错误处理**:
   - DHT11 读取失败时立即关闭风扇
   - 输出错误日志：`[AUTO_CONTROL] ERROR: DHT11 read failed, turning off fan`

4. **风扇启动策略**:
   - 直接设置到 FAN_GEAR_5（100% 占空比，PWM=255）
   - 软启动/软停止过程由 `fanController.update()` 非阻塞处理

---

## 三、编译结果

### 编译环境

- **PlatformIO 环境**: `esp32-s3-devkitc-1`
- **框架**: Arduino Core 3.3.12
- **芯片**: ESP32S3 240MHz, 320KB RAM, 8MB Flash

### 编译状态

```
✅ 编译成功
```

### 资源使用情况

| 资源类型 | 使用量 | 总量 | 百分比 |
|---------|--------|------|--------|
| **RAM** | 47,040 bytes | 327,680 bytes | 14.4% |
| **Flash** | 1,022,024 bytes | 3,342,336 bytes | 30.6% |

### 编译日志

- **编译时间**: 79.12 秒
- **构建产物**:
  - `.pio/build/esp32-s3-devkitc-1/firmware.elf`
  - `.pio/build/esp32-s3-devkitc-1/firmware.bin`
  - `.pio/build/esp32-s3-devkitc-1/bootloader.bin`
  - `.pio/build/esp32-s3-devkitc-1/partitions.bin`

---

## 四、硬件测试步骤

### 测试准备

1. **硬件连接检查**
   ```
   - DHT11: GPIO4（数据引脚）
   - HC-SR501 PIR: GPIO5（输出引脚）
   - MOSFET 风扇：GPIO6（控制引脚）
   - USB 数据线：连接 ESP32-S3 到电脑
   ```

2. **串口监控工具准备**
   - 使用 PlatformIO 串口监视器：`pio device monitor`
   - 或使用 Arduino IDE 串口监视器
   - 波特率：115200

### 测试场景

#### 场景 1：正常启动流程（温度≥28℃ + 有人）

**步骤**:
1. 上传固件到 ESP32-S3
2. 打开串口监视器
3. 等待系统初始化完成（看到 `[INIT] All systems initialized`）
4. 用热源（如热水杯）加热 DHT11，使温度≥28℃
5. 在 PIR 传感器前挥手触发

**预期日志**:
```
[PIR] New trigger detected!
[AUTO_CONTROL] Temperature 28.5°C >= 28°C, PIR detected, starting fan
[AUTO_CONTROL] State: IDLE -> SOFT_START
[FAN] State: SOFT_START, Gear: 5 (Gear_5), Duty: 100%
```

#### 场景 2：温度不达标不启动（温度<28℃ + 有人）

**步骤**:
1. 确保环境温度<28℃（如 25℃）
2. 在 PIR 传感器前挥手触发

**预期日志**:
```
[PIR] New trigger detected!
[AUTO_CONTROL] Temperature 25.3°C < 28°C, PIR detected but fan NOT started
```

#### 场景 3：60 秒超时自动停止

**步骤**:
1. 触发风扇启动（场景 1）
2. 停止 PIR 触发，等待 60 秒
3. 观察串口日志

**预期日志**（60 秒后）:
```
[AUTO_CONTROL] No PIR activity for 60000 ms, starting soft stop
[AUTO_CONTROL] State: RUNNING -> SOFT_STOP
[FAN] State: SOFT_STOP, Gear: 5 (Gear_5), Duty: 85%
...（逐渐降低到 0%）
```

#### 场景 4：温度低于 26℃强制关闭

**步骤**:
1. 触发风扇启动（场景 1）
2. 用冷风或冰块降低 DHT11 温度至<26℃
3. 观察串口日志

**预期日志**:
```
[AUTO_CONTROL] Temperature 25.8°C < 26°C, turning off fan
[AUTO_CONTROL] State: RUNNING -> SOFT_STOP (temp low)
```

#### 场景 5：传感器错误处理

**步骤**:
1. 风扇运行时拔掉 DHT11 或断开连接
2. 观察串口日志

**预期日志**:
```
[AUTO_CONTROL] ERROR: DHT11 read failed, turning off fan
[AUTO_CONTROL] State: RUNNING -> SOFT_STOP (sensor error)
```

#### 场景 6：延时期间重新触发

**步骤**:
1. 触发风扇启动（场景 1）
2. 停止 PIR 触发，等待 30 秒
3. 再次触发 PIR
4. 再等待 30 秒，观察是否超时

**预期结果**:
- 第 30 秒重新触发时，`lastPirActivityTime` 刷新
- 从第二次触发开始重新计时 60 秒
- 总运行时间应>90 秒

**预期日志**:
```
[PIR] New trigger detected!
[AUTO_CONTROL] Temperature 28.2°C >= 28°C, PIR detected, starting fan
...（继续运行，不会在 60 秒时停止）
```

### 测试检查清单

- [ ] 系统启动后串口正常输出
- [ ] DHT11 温度读数准确（可用温度计对比）
- [ ] PIR 传感器能正确检测人体
- [ ] 温度≥28℃ + 有人时风扇启动
- [ ] 温度<28℃ + 有人时风扇不启动
- [ ] 60 秒无人后风扇自动软停止
- [ ] 温度<26℃时风扇强制关闭
- [ ] DHT11 断开时风扇安全关闭
- [ ] 延时期间重新触发能重置计时
- [ ] 风扇 PWM 输出正常（可听到风速变化）

---

## 五、串口日志说明

### 日志级别

| 前缀 | 说明 |
|------|------|
| `[INIT]` | 系统初始化信息 |
| `[DHT11]` | 温度传感器数据 |
| `[PIR]` | 人体感应检测 |
| `[FAN]` | 风扇状态信息 |
| `[AUTO_CONTROL]` | 自动控制逻辑 |
| `[TIMEOUT]` | 超时计时信息 |
| `[STATUS]` | 系统状态快照 |
| `[UPLOAD]` | 数据上传信息 |
| `[WiFi]` | Wi-Fi 连接状态 |

### 关键日志示例

```
========================================
   SmartFan 2.0 - ESP32 Smart Fan
========================================
  Firmware Version: 0.2.0
  Development Phase: Phase 2: PIR + Fan Auto Control
  Chip Model: ESP32-S3-WROOM-1
  Chip Revision: 0
  CPU Frequency: 240 MHz
  Heap Size: 320 KB
  PSRAM Size: 0 KB
========================================

[INIT] System initialization started
[INIT] Initializing DHT11 sensor...
[INIT] DHT11 sensor initialized successfully
[INIT] Initializing PIR sensor...
[INIT] PIR sensor initialized successfully
[INIT] PIR GPIO: 5, Debounce: 50ms
[INIT] Initializing Fan Controller...
[INIT] Fan Controller initialized successfully
[INIT] Fan PWM GPIO: 6
[INIT] All systems initialized

----------------------------------------
[STATUS] Uptime: 2 seconds
[STATUS] Free Heap: 275432 bytes
[DHT11] Temperature: 28.5 °C
[DHT11] Humidity: 65.2 %
[PIR] Detected: YES, Raw: HIGH
[FAN] State: RUNNING, Gear: 5 (Gear_5), Duty: 100%
[WiFi] Connected: YES, IP: 192.168.1.100
[TIMEOUT] Idle time: 2 s / 60 s (timeout)
----------------------------------------
```

---

## 六、配置参数汇总

| 参数名 | 值 | 说明 |
|--------|-----|------|
| `PIR_SENSOR_PIN` | 5 | PIR 传感器 GPIO |
| `FAN_CONTROL_PIN` | 6 | 风扇 PWM GPIO |
| `DHT_SENSOR_PIN` | 4 | DHT11 数据 GPIO |
| `TEMP_START_THRESHOLD` | 28.0℃ | 风扇启动温度 |
| `TEMP_STOP_THRESHOLD` | 26.0℃ | 风扇关闭温度 |
| `PIR_TIMEOUT_MS` | 60000 | 无人超时时间（60 秒） |
| `LOG_INTERVAL_MS` | 2000 | 状态日志间隔（2 秒） |
| `SENSOR_UPLOAD_INTERVAL_MS` | 5000 | 数据上传间隔（5 秒） |
| `FAN_GEAR_5` | 255 | 100% PWM 占空比 |

---

## 七、下一步工作

### 可选优化

1. **温度阈值动态配置**
   - 通过 Wi-Fi 后端远程配置温度阈值
   - 添加 Web 界面调整参数

2. **风速分级控制**
   - 根据温度高低自动调节风速
   - 如：28-30℃用 50%，30-35℃用 75%，35℃+ 用 100%

3. **历史数据记录**
   - 记录温度变化和风扇启停事件
   - 用于后续数据分析和优化

4. **节能模式**
   - 添加定时开关机功能
   - 根据时间段调整控制策略

---

## 八、问题反馈

如遇到任何问题，请检查：

1. **串口日志** - 最直接的调试信息源
2. **硬件接线** - 确认 GPIO 引脚连接正确
3. **传感器供电** - DHT11 和 PIR 需要稳定 5V 供电
4. **Wi-Fi 配置** - 检查 `.env` 文件中的 Wi-Fi 凭据

---

**报告生成时间**: 2026-11-23  
**编译验证状态**: ✅ PASSED  
**功能实现状态**: ✅ COMPLETE
