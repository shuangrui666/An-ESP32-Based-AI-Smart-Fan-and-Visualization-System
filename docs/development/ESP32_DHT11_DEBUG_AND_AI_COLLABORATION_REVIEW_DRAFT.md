# ESP32-S3 + DHT11 调试与 AI 协作记录

> 本文档详细记录了 SmartFan2 项目中 DHT11 温湿度传感器集成过程中的问题诊断、排查和修复全过程。

---

## 📋 项目背景

**项目名称**：SmartFan2 - ESP32 智能风扇控制系统  
**硬件平台**：YD-ESP32-S3 Classic V1.0 (ESP32-S3-WROOM-1)  
**传感器**：DHT11 温湿度传感器  
**固件版本**：0.1.0  
**开发框架**：ESP32 Arduino 3.3.12 (PlatformIO)  
**调试日期**：2026-11-11  
**AI 参与模式**：实时协作调试  

---

## ⏱️ 调试时间线

### 阶段一：初始状态与问题发现

**时间点**：2026-11-11（具体时间未确认）

**初始代码状态（依据本次调试记录及用户报告整理，未能从本仓库独立读取本地源码核验）**：
- `src/main.cpp`：记录为每 2 秒读取并打印温湿度
- `src/sensors/dht_sensor.cpp`：记录为使用 Adafruit DHT 库封装 DHTSensor 类
- `src/sensors/dht_sensor.h`：传感器类头文件

说明：本次审查尝试从当前 GitHub 仓库读取上述源文件及 `platformio.ini`，对应路径未找到。因此，下文代码与配置片段属于调试记录中的摘录，不能视为本次已从 GitHub 源码逐行复核。

**主程序初始实现**（`main.cpp`）：
```cpp
// DHT11 传感器实例
static DHTSensor dhtSensor;

void systemInit() {
    // ... 其他初始化 ...
    
    // 初始化 DHT11 传感器
    Serial.println("[INIT] Initializing DHT11 sensor...");
    if (dhtSensor.begin(DHT_SENSOR_PIN)) {
        Serial.println("[INIT] DHT11 sensor initialized successfully");
    } else {
        Serial.println("[INIT] DHT11 sensor initialization FAILED");
    }
}

void printSystemStatus() {
    // ... 其他状态 ...
    
    float temperature, humidity;
    if (dhtSensor.readAll(temperature, humidity)) {
        Serial.printf("[DHT11] Temperature: %.1f °C\r\n", temperature);
        Serial.printf("[DHT11] Humidity: %.1f %%\r\n", humidity);
    } else {
        Serial.println("[DHT11] Temperature: Read Failed");
        Serial.println("[DHT11] Humidity: Read Failed");
    }
}
```

**问题现象**：
- 主程序持续输出：`[DHT11] Temperature: Read Failed, humidity=nan`
- 传感器初始化显示成功，但读取始终失败
- 温度、湿度值均为 NaN（非数字）

---

### 阶段二：首次代码检查与缓存逻辑修复

**AI 诊断步骤**：

1. **检查 `dht_sensor.cpp` 实现**
2. **发现缓存逻辑的潜在问题**

**原始代码问题**（修复前）：
```cpp
bool DHTSensor::_readData(float& temperature, float& humidity) {
    // DHT11 最小读取间隔 2 秒（保守设置）
    unsigned long currentTime = millis();
    if (currentTime - _lastReadTime < 2000) {
        // 使用缓存的数据（仅当缓存有效时）
        if (_cacheValid) {
            temperature = _cachedTemperature;
            humidity = _cachedHumidity;
            return true;
        }
        // 缓存无效时直接返回 false，未尝试读取
        return false;
    }

    // 读取逻辑...
}
```

**⚠️ 技术分析说明**：

缓存逻辑的设计逻辑：
- `_lastReadTime` 初始为 0 时，是否跳过 2000 ms 缓存判断取决于首次调用时的 `millis()`：若运行时间已达到 2000 ms，差值不小于 2000，会进入实际读取分支；若运行时间尚未达到 2000 ms，则会进入缓存分支。
- 若此时 `_cacheValid == false`，函数会返回 `false`，首次读取可能因此失败。故不能笼统地说首次调用一定跳过缓存判断。
- 仅凭当前记录中的代码片段，不能证明缓存逻辑在所有调用时序下都正确；应将其描述为曾进行的独立健壮性修改，而不是本次故障已被单独实验排除的原因。

**缓存逻辑相关修改（根据调试记录整理，未从当前仓库源代码/提交差异独立核验）**：
- 记录显示，修改增加了 `_initialized` 与 `_dhtInstance == nullptr` 的状态检查。
- 记录显示，修改完善了缓存有效性判断及缓存数据返回处理。
- 这些属于初始化状态检查和缓存健壮性改进；它们与后续补上 Adafruit `DHT::begin()` 调用是两个不同的修改点。

**真实故障原因**（见阶段四）：
- **本次调试中定位并修复的直接差异**：主程序封装创建 `DHT` 对象后，未调用 Adafruit 库的 `_dhtInstance->begin(55)`；独立测试程序则调用了 `testDHT->begin(55)`。
- 在所引用的 Adafruit 官方源码版本中，构造函数设置引脚和传感器类型，并计算 `_maxcycles`；`begin()` 负责设置引脚为 `INPUT_PULLUP`、初始化 `_lastreadtime`，并将参数写入 `pullTime`。因此遗漏 `begin()` 会使库对象缺少这些初始化设置。
- 现有证据支持将补上 `begin(55)` 记为本次读取恢复的直接修复点；但由于当前仓库未提供项目源代码和依赖锁定版本，本文件不能单独证明所有失败均由这一项造成。
- 缓存逻辑修改与 `begin(55)` 修复是两个独立修改点。

**修复后的代码**：
```cpp
bool DHTSensor::_readData(float& temperature, float& humidity) {
    if (!_initialized || _dhtInstance == nullptr) {
        _lastError = -1;
        Serial.println("[DHTSensor] Not initialized");
        return false;
    }

    // DHT11 最小读取间隔 2 秒（保守设置，避免时序问题）
    unsigned long currentTime = millis();
    if (currentTime - _lastReadTime < 2000) {
        // 使用缓存的数据（仅当缓存有效时）
        if (_cacheValid) {
            temperature = _cachedTemperature;
            humidity = _cachedHumidity;
            return true;
        }
        // 缓存无效，返回 false
        return false;
    }

    // 读取温湿度数据（Adafruit DHT 库 API）
    temperature = _dhtInstance->readTemperature();
    humidity = _dhtInstance->readHumidity();
    
    // ... 后续处理 ...
}
```

**修复文件**：
- ✅ `src/sensors/dht_sensor.cpp`

---

### 阶段三：创建独立 DHT11 测试程序

**目的**：
- 绕过项目封装层，直接调用 Adafruit DHT 库进行对照测试
- 在当前接线和测试条件下，观察 GPIO4 上的 DHT11 是否能返回有效数据
- 为比较独立测试程序与主程序的差异提供线索；单次对照测试不能完全隔离所有硬件、库和软件因素

**创建文件**：
- ✅ `src/test_dht11_main.cpp`

**关键代码**：
```cpp
// 创建 DHT 对象
testDHT = new DHT(TEST_DHT_PIN, TEST_DHT_TYPE, 6);

// 关键：调用 begin() 初始化传感器（设置读取前数据线的上拉等待时间为 55 微秒，即 pullTime）
testDHT->begin(55);

// 等待传感器稳定
delay(2000);

// 读取测试
bool readResult = testDHT->read(true);  // force=true 强制读取
float temp = testDHT->readTemperature();
float hum = testDHT->readHumidity();
```

**测试配置**（`platformio.ini`）：
```ini
[env:dht11-test]
extends = env:esp32-s3-devkitc-1
board = esp32-s3-devkitc-1
build_src_filter = +<test_dht11_main.cpp>
upload_port = COM5
monitor_port = COM5
```

**用户执行操作**：
1. 编译独立测试程序：`pio run -e dht11-test -t upload`
2. 上传到 ESP32-S3（串口 COM5）
3. 打开串口监视器（115200 波特率）

**测试结果**（用户报告）：
```
========================================
    DHT11 独立硬件诊断测试程序
========================================
引脚配置：GPIO4
传感器类型：DHT11
========================================

[1/4] 配置 GPIO 引脚模式...
      ✓ 引脚配置完成 (INPUT_PULLUP)
[2/4] 创建 DHT 对象...
      ✓ DHT 对象创建完成
[3/4] 初始化 DHT 传感器...
      ✓ begin() 调用完成
[4/4] 初始化完成，等待 2 秒后开始读取...

--- 测试 #1 (耗时：2s) ---
[读取方式 1] 调用 read() 底层函数:
  read() 返回值：true (OK)
  温度：25.0 °C
  湿度：42.0 %
  >>> 本次读取：成功

[统计信息]
  成功次数：22
  失败次数：0
  成功率：100.0%
```

**结论**（仅限本次测试条件及用户报告的结果）：
- 独立测试程序在本次接线和运行条件下，通过 GPIO4 成功读取到温度 25.0 °C、湿度 42.0%。
- 该结果说明本次测试中的传感器数据读取链路能够工作；不能据此证明接线在所有条件下均无问题，也不能证明传感器长期稳定或测量精度。
- 独立测试与主程序的对照提示问题可能位于主程序封装/初始化路径；需结合代码差异判断，不能仅凭独立测试成功就排除其他因素。

---

### 阶段四：对比分析与根本问题定位

**对比分析**（AI 执行）：

| 对比项 | 独立测试程序 | 主程序封装层 | 结果 |
|--------|-------------|-------------|------|
| GPIO 引脚 | GPIO4 | GPIO4 | ✅ 一致 |
| 传感器类型 | DHT11 | DHT11 | ✅ 一致 |
| 对象创建 | `new DHT(pin, type, 6)` | `new DHT(_pin, _type, 6)` | ✅ 一致 |
| **初始化调用** | `testDHT->begin(55);` | **记录为未调用** | ❌ **存在差异** |
| 读取间隔 | 2 秒 | 2 秒 | ✅ 一致 |
| 读取函数 | 先 `read(true)` 强制读取，再调用温湿度接口 | 通过温湿度接口触发/复用读取 | ⚠️ 调用路径不完全相同 |

**根本问题发现**：

在 `src/sensors/dht_sensor.cpp` 的 `begin()` 函数中：

**修复前代码**：
```cpp
bool DHTSensor::begin(int pin) {
    _pin = pin;

    // 配置引脚模式
    pinMode(_pin, INPUT_PULLUP);

    // 创建 DHT 对象
    _dhtInstance = new DHT(_pin, _type, 6);
    
    // ❌ 问题：只创建了 DHT 对象，未调用 begin() 初始化传感器时序
    
    _initialized = true;
    _lastReadTime = 0;

    Serial.println("[DHTSensor] Initialized successfully");
    return true;
}
```

**问题分析**：
- Adafruit DHT 库的 `DHT` 类构造函数只设置引脚和类型参数
- 真正的传感器初始化需要在 `begin(uint8_t usec)` 方法中完成
- 独立测试程序显式调用了 `testDHT->begin(55)`，而主程序封装层遗漏了此调用

**关于 `begin(55)` 的技术说明**（基于 Adafruit DHT 库官方源码）：
- `begin(uint8_t usec)` 的参数 `usec` 会写入成员变量 `pullTime`，表示读取前数据线的上拉等待时间，单位为微秒；官方声明的默认值为 55。
- `begin()` 不只设置 `pullTime`：官方实现还会将引脚设为 `INPUT_PULLUP`，并将 `_lastreadtime` 初始化为可立即进行首次读取的状态。
- 55 微秒是该库 `begin()` 参数的默认值，不应描述为 DHT11 专属标准，也不能据此声称 DHT22 需要 1000 微秒上拉等待。
- 官方源码中约 1 毫秒的脉冲读取超时上限由构造函数设置的 `_maxcycles` 及 `expectPulse()` 使用；它与 `pullTime` 是不同机制。
- 因此，主程序遗漏 `begin()` 会漏掉库要求的引脚及内部状态设置；本次调试记录将补上该调用作为直接修复点。

**官方源码参考**：
```cpp
// Adafruit DHT Library: DHT.cpp
void DHT::begin(uint8_t usec) {
    pinMode(_pin, INPUT_PULLUP);
    _lastreadtime = millis() - MIN_INTERVAL;
    pullTime = usec;  // 设置读取前数据线的上拉等待时间（pull-up time）
}
```

---

### 阶段五：修复主程序并验证

**修复文件**：
- ✅ `src/sensors/dht_sensor.cpp`

**修复后代码**：
```cpp
bool DHTSensor::begin(int pin) {
    _pin = pin;

    // 配置引脚模式
    pinMode(_pin, INPUT_PULLUP);

    // 创建 DHT 对象（使用 Adafruit DHT 库的构造函数）
    // DHT(uint8_t pin, uint8_t type, uint8_t count = 6)
    _dhtInstance = new DHT(_pin, _type, 6);

    // ✅ 关键：调用 begin() 初始化传感器（设置读取前数据线的上拉等待时间为 55 微秒，即 pullTime）
    _dhtInstance->begin(55);

    _initialized = true;
    _lastReadTime = 0;

    Serial.println("[DHTSensor] Initialized successfully");
    Serial.printf("[DHTSensor] Pin: %d, Type: DHT11\n", _pin);
    return true;
}
```

**Build 时的新问题**：

**错误信息**：
```
multiple definition of 'setup()'
multiple definition of 'loop()'
```

**原因**：
- `test_dht11_main.cpp` 与 `main.cpp` 同时被编译
- 两者都定义了 `setup()` 和 `loop()` 函数

**修复方案**（`platformio.ini`）：
```ini
[env:esp32-s3-devkitc-1]
; 添加 build_src_filter 排除测试文件
build_src_filter = +<main.cpp> +<sensors/> +<ai/> +<communication/> +<control/>
```

**Build 结果**（用户报告的终端构建记录）：
```
Building in release mode
Compiling .pio\build\esp32-s3-devkitc-1\src\main.cpp.o
Compiling .pio\build\esp32-s3-devkitc-1\src\sensors\dht_sensor.cpp.o
Linking .pio\build\esp32-s3-devkitc-1\firmware.elf
Building .pio\build\esp32-s3-devkitc-1\firmware.bin
========================= [SUCCESS] Took 64.97 seconds =========================
```

**编译状态**：
- ✅ 成功（无错误，仅有一些 format 警告）
- ✅ 固件位置：`.pio\build\esp32-s3-devkitc-1\firmware.bin`

**Upload 结果**（用户执行操作记录）：
- ✅ 用户执行命令：`pio run -e esp32-s3-devkitc-1 -t upload --upload-port COM5`
- ✅ 上传状态：SUCCESS（用户终端输出）

---

## 📝 修改文件清单

| 文件路径 | 修改类型 | 修改内容 | 修改原因 |
|---------|---------|---------|---------|
| `src/sensors/dht_sensor.cpp` | 修复 | 在 `begin()` 函数中添加 `_dhtInstance->begin(55);` 调用 | 修复传感器初始化缺失问题 |
| `platformio.ini` | 新增配置 | 添加 `build_src_filter` 过滤规则 | 避免测试文件与主程序冲突 |

---

## 🔍 关键代码修改前后对比

### 1. `dht_sensor.cpp` - `begin()` 函数

**修改前**：
```cpp
bool DHTSensor::begin(int pin) {
    _pin = pin;
    pinMode(_pin, INPUT_PULLUP);
    _dhtInstance = new DHT(_pin, _type, 6);
    // ❌ 缺少 begin(55) 调用
    _initialized = true;
    _lastReadTime = 0;
    Serial.println("[DHTSensor] Initialized successfully");
    return true;
}
```

**修改后**：
```cpp
bool DHTSensor::begin(int pin) {
    _pin = pin;
    pinMode(_pin, INPUT_PULLUP);
    _dhtInstance = new DHT(_pin, _type, 6);
    // ✅ 添加 begin(55) 初始化调用
    _dhtInstance->begin(55);
    _initialized = true;
    _lastReadTime = 0;
    Serial.println("[DHTSensor] Initialized successfully");
    Serial.printf("[DHTSensor] Pin: %d, Type: DHT11\n", _pin);
    return true;
}
```

### 2. `platformio.ini` - 新增 `build_src_filter`

**修改前**：
```ini
[env:esp32-s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
; 无 build_src_filter 配置
```

**修改后**：
```ini
[env:esp32-s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
; 新增：仅编译 main.cpp，排除测试文件
build_src_filter = +<main.cpp> +<sensors/> +<ai/> +<communication/> +<control/>
```

---

## 📊 Build 和 Upload 记录

### 独立测试程序（dht11-test）

**命令**：
```bash
pio run -e dht11-test -t upload --upload-port COM5
```

**编译结果**：
- ✅ 编译成功
- ✅ 上传成功（用户执行）

**串口输出**（用户报告）：
- 初始化：✓ 全部步骤成功
- 读取结果：22 次成功，0 次失败
- 成功率：100%
- 温度：25.0°C
- 湿度：42.0%

### 主程序（esp32-s3-devkitc-1）

**命令**：
```bash
pio run -e esp32-s3-devkitc-1 -t upload --upload-port COM5
```

**编译结果**（用户报告的终端构建记录）：
```
Build 耗时：64.97 秒
状态：SUCCESS
错误数：0
警告数：若干 format 警告（不影响功能）
固件：.pio\build\esp32-s3-devkitc-1\firmware.bin
```

**Upload 结果**（用户执行操作记录）：
- ✅ 用户上传成功（VS Code 终端显示 SUCCESS）

**串口输出**（用户提供）：
- ✅ 初始化成功：`[DHTSensor] Initialized successfully`
- ✅ 读取成功：`[DHTSensor] Read OK: temp=25.0°C, hum=42.0%`
- ✅ 主程序输出：`[DHT11] Temperature: 25.0 °C`
- ✅ 主程序输出：`[DHT11] Humidity: 42.0 %`
- ✅ 已观察到至少两轮有效输出

---

## 🧪 测试结果汇总

### 独立测试程序

| 测试项 | 结果 | 备注 |
|--------|------|------|
| 本次数据读取 | ✅ 成功 | 用户报告在本次测试条件下读到有效温湿度 |
| 使用引脚 | GPIO4 | 测试程序配置为 GPIO4；不代表已验证所有接线条件 |
| 传感器型号 | DHT11 | 用户确认所用模块为 DHT11 |
| 库与平台 | 本次组合可运行 | 仅说明本次测试组合成功，不代表所有版本/配置均兼容 |
| 读取成功率 | ✅ 100% | 22/22 次成功 |
| 温度读数 | ✅ 25.0°C | 数据合理 |
| 湿度读数 | ✅ 42.0% | 数据合理 |

### 主程序（修复前）

| 测试项 | 结果 | 备注 |
|--------|------|------|
| 传感器初始化 | ⚠️ 显示成功 | 但实际未调用 begin(55) |
| 温度读取 | ❌ 失败 | 输出 "Read Failed" |
| 湿度读取 | ❌ 失败 | 输出 "nan" |
| 成功率 | ❌ 0% | 持续失败 |

### 主程序（修复后）

| 测试项 | 结果 | 备注 |
|--------|------|------|
| 编译状态 | ✅ 成功 | 无错误 |
| 上传状态 | ✅ 成功 | 用户执行上传，VS Code 终端显示 SUCCESS |
| 初始化 | ✅ 成功 | `[DHTSensor] Initialized successfully` |
| 温度读取 | ✅ 成功 | 输出 `25.0 °C`（用户串口输出） |
| 湿度读取 | ✅ 成功 | 输出 `42.0 %`（用户串口输出） |
| 串口输出 | ✅ 正常 | 已观察到至少两轮有效输出 |

---

## 🤖 AI 实际参与的工作

### 1. 问题诊断
- 根据调试过程记录，AI 协助检查 `dht_sensor.cpp` / `dht_sensor.h`、分析缓存与初始化路径，并协助比较独立测试程序和主程序的差异。
- 本次读取失败的直接修复点记录为补上 `begin(55)`；缓存相关修改另列为健壮性改进。

### 2. 代码修复建议
- ✅ 提供缓存逻辑修复方案
- ✅ 提供 `begin()` 函数修复方案
- ✅ 提供 `platformio.ini` 配置修复方案

### 3. 文档整理
- 独立测试程序由谁具体编写、各 AI 工具分别贡献了哪些代码，当前证据不足以逐项归属；因此不将其笼统记为某一个 AI 独立创建。
- 本文档由 AI 协作整理，测试结果由用户运行并报告。

### 4. Build/Upload 指导
- ✅ 提供编译命令
- ✅ 提供上传命令
- ✅ 提供串口监视器配置

---

## 👤 用户实际执行的工作

### 1. 硬件操作
- ✅ 连接 DHT11 传感器到 ESP32-S3（GPIO4）
- ✅ 执行独立测试程序上传
- ✅ 执行主程序上传（修复前）

### 2. 测试验证
- ✅ 运行独立测试程序，报告测试结果（22 次成功，0 次失败）
- ✅ 观察并报告主程序故障现象（"Read Failed", "nan"）
- ✅ 重新上传修复后的主程序固件（VS Code 终端执行）
- ✅ 验证主程序 DHT11 读取正常（串口输出温度 25.0°C，湿度 42.0%）
- ✅ 报告修复后的串口输出（至少两轮有效输出）

---

## ⚠️ 尚未核实的事项

1. **长期稳定性测试**
   - 状态：未进行
   - 建议：连续运行 24 小时，观察是否有偶发读取失败

2. **不同环境条件下的测试**
   - 状态：未进行
   - 建议：在高温、高湿环境下测试传感器准确性

3. **内存泄漏检查**
   - 状态：未进行
   - 建议：长时间运行后检查堆内存使用情况

---

## 💡 经验总结

### 1. Adafruit DHT 库使用注意事项

**关键 API**：
```cpp
// 构造函数只设置参数，不初始化传感器
DHT* dht = new DHT(pin, type, count);

// 必须调用 begin() 进行初始化（设置读取前数据线的上拉等待时间为 55 微秒）
dht->begin(55);  // 参数设置 pullTime（读取前数据线的上拉等待时间）
```

**常见错误**：
- ❌ 只创建对象，不调用 `begin()`
- ❌ 读取间隔过短：本项目封装按 2 秒间隔读取；所引用的 Adafruit DHT 库源码定义 `MIN_INTERVAL` 为 2000 ms，非强制读取在间隔不足时会复用上次读取结果。不要将此库行为误写成仅适用于 DHT11 的传感器通用规格。

### 2. 调试策略

**隔离法**：
- 当封装层出现问题时，创建独立测试程序直接调用底层库
- 如果独立测试成功而主程序失败，说明两条测试路径存在值得检查的差异；应进一步比较初始化、引脚、调用顺序、库版本和编译配置，不能直接断定问题必然只在封装层。
- 如果独立测试失败，也不能仅凭这一结果断定是硬件或底层库问题，仍需逐项排查供电、接线、时序和程序配置。

**对比法**：
- 对比成功代码与失败代码的差异
- 关注初始化、配置、调用顺序等关键步骤

### 3. 项目结构建议

**测试文件管理**：
- 使用 `platformio.ini` 的 `build_src_filter` 控制编译范围
- 测试文件与主程序分离，避免冲突
- 为不同测试环境创建独立的 `env` 配置

---

## 📌 后续建议

### 短期（已完成）

1. **验证修复效果** ✅
   - 已上传修复后的固件
   - 已观察串口输出，确认读取成功
   - 已记录温度和湿度数据（25.0°C, 42.0%）

### 短期（后续建议）

1. **持续观察**
   - 连续运行数小时，统计读取成功率
   - 检查是否有偶发读取失败

2. **长期运行测试**
   - 连续运行 24 小时
   - 检查内存使用情况

### 中期（1 周）

1. **完善错误处理**
   - 添加读取失败重试机制
   - 添加传感器故障检测
   - 添加数据合理性验证

2. **性能优化**
   - 调整读取间隔（根据实际需求）
   - 优化缓存策略
   - 减少串口输出（降低 CPU 负载）

### 长期（项目完成前）

1. **集成测试**
   - 与风扇控制逻辑集成
   - 与 WiFi 通信模块集成
   - 与 AI 决策模块集成

2. **文档完善**
   - 更新硬件接线图
   - 补充 API 使用文档
   - 编写故障排查指南

---

## 📚 参考资料

1. [Adafruit DHT Library GitHub](https://github.com/adafruit/DHT-sensor-library)
2. [Adafruit DHT Library API Source Code](https://github.com/adafruit/DHT-sensor-library/blob/master/DHT.cpp) - `begin(uint8_t usec)` 设置 `pullTime` 参数
3. [ESP32-S3 技术参考手册](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)
4. [DHT11 数据手册](https://cdn-learn.adafruit.com/assets/100337)
5. [PlatformIO ESP32 文档](https://docs.platformio.org/en/latest/platforms/espressif32.html)

---

## 📄 文档信息

**文档版本**：1.1.0（技术事实审查修订版）  
**创建日期**：2026-11-11  
**创建人员**：AI 协作助手  
**最后更新**：2026-11-12  
**修订说明**：完成技术事实审查，修正 DHT22 错误表述、缓存逻辑矛盾、测试结论确定性、Build 结果证据分类  
**维护状态**：持续更新中  

**相关文档**：
- `docs/hardware.md` - 硬件信息
- `docs/wiring.md` - 接线说明
- `docs/troubleshooting.md` - 故障排查
- `docs/experiment_log.md` - 实验记录

---

**附录 A：完整命令参考**

```bash
# 独立测试程序编译上传
pio run -e dht11-test -t upload --upload-port COM5

# 主程序编译上传
pio run -e esp32-s3-devkitc-1 -t upload --upload-port COM5

# 打开串口监视器
pio device monitor -p COM5 --baud 115200

# 仅编译（不上传）
pio run -e esp32-s3-devkitc-1

# 清理构建缓存
pio run -e esp32-s3-devkitc-1 -t clean
```

**附录 B：串口输出示例**

**修复前（主程序）**：
```
[DHT11] Temperature: Read Failed
[DHT11] Humidity: Read Failed
```

**修复后（实际输出）**：
```
[DHTSensor] Initialized successfully
[DHTSensor] Pin: 4, Type: DHT11
[DHTSensor] Read OK: temp=25.0°C, hum=42.0%
[DHT11] Temperature: 25.0 °C
[DHT11] Humidity: 42.0 %
```
