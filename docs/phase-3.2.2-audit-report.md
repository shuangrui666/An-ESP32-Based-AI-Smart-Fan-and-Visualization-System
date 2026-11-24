# Phase 3.2.2 指令执行链路修复前核查报告

> **核查日期**: 2026-11-24  
> **核查类型**: 只读分析（不修改任何文件）  
> **状态**: 待确认修复方案

---

## 执行摘要

本次核查针对 SmartFan 2.0 项目的指令执行链路（Phase 3.2.2）进行全面的只读分析，重点检查前后端协议一致性、指令执行架构、ACK 机制、安全规则等关键问题。

**核心发现**:
- 前后端 JSON 协议存在 **4 处类型不匹配**
- 指令执行逻辑 **完全未实现**（仅有 TODO 注释）
- ACK 机制存在 **可靠性缺陷**
- 缺少过期指令和非法参数验证

---

## 1. 当前指令执行流程图

### 1.1 现有流程（当前实现）

```
┌─────────────┐
│  ESP32 loop │
└──────┬──────┘
       │ (每 1 秒)
       ▼
┌─────────────────────┐
│ handleCommandPolling│
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  fetchPendingCommand│──(HTTP GET /api/fan/command)──► 后端
└──────┬──────────────┘
       │ (返回 JSON)
       ▼
┌─────────────────────┐
│ _parseCommandJSON   │  ❌ 类型不匹配问题
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│ _isCommandProcessed │──(已处理)──► ACK 重复指令
└──────┬──────────────┘
       │ (未处理)
       ▼
┌─────────────────────┐
│ _recordProcessedCmd │  记录 command_id
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  ackCommand         │  ❌ 执行前发送 ACK
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  TODO: 执行指令     │  ❌ 未实现
└─────────────────────┘
```

### 1.2 问题点

1. **第 463-467 行**: 指令执行逻辑仅有 TODO 注释，未实际调用风扇控制
2. **第 471 行**: ACK 在记录后立即发送，而非执行完成后
3. **缺少过期检查**: 标注"简化处理"，未实现 expires_at 验证
4. **缺少参数验证**: 未检查 gear 是否在 0-5 范围内

---

## 2. 前后端 JSON 实际协议对照表

### 2.1 后端返回格式（app.py 第 321-331 行）

```json
{
  "command_id": "1700000000_abc123",  // 字符串格式
  "mode": "auto",                      // 字符串："auto" 或 "manual"
  "gear": 3,                           // 整数：0-5
  "status": "pending",                 // 字符串："pending" | "executing" | "completed"
  "expires_at": "2026-11-11T12:05:00" // ISO 8601 日期字符串
}
```

### 2.2 ESP32 端解析方式（wifi_manager.cpp 第 535-595 行）

| 字段 | 后端类型 | ESP32 解析方式 | 实际结果 | 问题 |
|------|----------|---------------|----------|------|
| `command_id` | 字符串 `"1700000000_abc123"` | `toInt()` | `0`（解析失败） | ❌ 严重 |
| `mode` | 字符串 `"auto"` | `toInt()` | `0` | ⚠️ 语义丢失 |
| `gear` | 整数 `3` | `toInt()` | `3` | ✅ 正确 |
| `status` | 字符串 `"pending"` | `substring` | `"pending"` | ✅ 正确 |
| `expires_at` | ISO 字符串 | `toInt()` | `0` | ❌ 严重 |

### 2.3 问题详解

#### 问题 1: `command_id` 类型不匹配

**后端**: 返回字符串格式，如 `"1700000000_abc123"`  
**ESP32**: 使用 `int` 类型存储，调用 `toInt()` 解析  
**结果**: 字符串 `"1700000000_abc123"` 调用 `toInt()` 返回 `0`

**影响**:
- 命令去重失效（所有命令 ID 都变为 0）
- ACK 发送错误的 command_id

#### 问题 2: `mode` 类型不匹配

**后端**: 返回字符串 `"auto"` 或 `"manual"`  
**ESP32**: 使用 `int` 类型存储，调用 `toInt()` 解析  
**结果**: `"auto".toInt()` 返回 `0`，`"manual".toInt()` 返回 `0`

**影响**:
- 无法区分 AUTO/MANUAL 模式
- 所有指令都被视为 mode=0

#### 问题 3: `expires_at` 类型不匹配

**后端**: 返回 ISO 8601 格式 `"2026-11-11T12:05:00"`  
**ESP32**: 使用 `int` 存储，调用 `toInt()` 解析  
**结果**: ISO 字符串调用 `toInt()` 返回 `0`

**影响**:
- 过期检查失效
- 无法判断指令是否过期

#### 问题 4: `gear` 解析正确但无验证

**后端**: 返回整数 `0-5`  
**ESP32**: 使用 `toInt()` 解析，结果正确  
**问题**: 未验证 gear 是否在有效范围内

---

## 3. 所有需要修改的文件及修改原因

### 3.1 核心修改文件

| 文件 | 修改原因 | 优先级 |
|------|----------|--------|
| `src/communication/wifi_manager.h` | 修改 Command 结构体字段类型 | 高 |
| `src/communication/wifi_manager.cpp` | 修复 JSON 解析、实现指令执行、ACK 重试 | 高 |
| `src/main.cpp` | 集成远程指令执行逻辑、模式切换 | 高 |

### 3.2 详细修改清单

#### 3.2.1 `wifi_manager.h`

**需要修改**:

```cpp
// 当前定义（第 15-25 行）
struct Command {
    int command_id;      // ❌ 应为 String
    int mode;            // ❌ 应改为枚举或 String
    int gear;
    String status;
    int expires_at;      // ❌ 应为 String 或 Unix 时间戳
    bool valid;
};

// 修改为:
struct Command {
    String command_id;   // ✅ 匹配后端字符串格式
    int mode;            // 0=AUTO, 1=MANUAL（解析后转换）
    int gear;
    String status;
    String expires_at;   // ✅ 保持 ISO 字符串，或解析为 Unix 时间戳
    bool valid;
    bool isExpired;      // ✅ 新增：过期标记
};
```

**修改原因**:
- `command_id`: 后端返回字符串，ESP32 端必须用 `String` 存储
- `expires_at`: 需要正确解析 ISO 日期字符串
- `isExpired`: 新增字段用于过期检查

---

#### 3.2.2 `wifi_manager.cpp`

**需要修改的功能模块**:

| 行号 | 功能 | 当前状态 | 修改内容 |
|------|------|----------|----------|
| 453-456 | 过期检查 | ❌ 简化处理未实现 | 实现 ISO 日期解析和过期比较 |
| 463-467 | 指令执行 | ❌ 仅有 TODO 注释 | 调用风扇控制接口执行指令 |
| 471 | ACK 发送时机 | ❌ 执行前发送 | 改为执行后发送 |
| 535-595 | JSON 解析 | ❌ 类型不匹配 | 修复 command_id、mode、expires_at 解析 |
| ackCommand | ACK 失败重试 | ❌ 无重试机制 | 添加重试逻辑（最多 3 次） |
| _isCommandProcessed | 去重 | ⚠️ int 数组 | 改为 String 数组 |

**修改原因**:
- 修复协议解析错误
- 实现完整的指令执行链路
- 提高 ACK 可靠性

---

#### 3.2.3 `main.cpp`

**需要修改**:

```cpp
// 当前定义（第 82-83 行）
static int currentRemoteCommandId = -1;  // ❌ 应为 String

// 修改为:
static String currentRemoteCommandId = "";  // ✅ 匹配 command_id 类型

// 新增：远程指令模式标记
static bool remoteManualMode = false;  // true=远程 MANUAL 模式，false=本地 AUTO 模式
```

**修改原因**:
- `currentRemoteCommandId` 类型必须与后端返回的字符串格式匹配
- 需要标记当前是否处于远程 MANUAL 模式，防止与本地 AUTO 模式冲突

---

## 4. 推荐的统一控制架构

### 4.1 架构设计原则

1. **单一控制源**: 同一时间只有一个控制源生效（本地 AUTO 或 远程 MANUAL）
2. **模式互斥**: AUTO 和 MANUAL 模式互斥，不能同时生效
3. **安全优先**: 远程指令不能覆盖温度保护等安全逻辑
4. **状态同步**: 前端显示的模式必须与 ESP32 实际模式一致

### 4.2 推荐架构

```
┌─────────────────────────────────────────────────────────┐
│                    控制仲裁层                            │
│  (在 main.cpp 中实现，通过 remoteManualMode 标记)         │
└──────────────┬──────────────────────┬───────────────────┘
               │                      │
    ┌──────────▼─────────┐  ┌─────────▼──────────┐
    │  本地 AUTO 控制     │  │  远程 MANUAL 控制   │
    │  (PIR+ 温度逻辑)    │  │  (远程指令执行)     │
    └──────────┬─────────┘  └─────────┬──────────┘
               │                      │
               └──────────┬───────────┘
                          │
                          ▼
                 ┌─────────────────┐
                 │  FanController  │
                 │  (PWM 控制)      │
                 └─────────────────┘
```

### 4.3 模式切换逻辑

```cpp
// 伪代码：模式切换逻辑

void handleCommandPolling() {
    Command cmd;
    if (!fetchPendingCommand(cmd)) return;
    if (!cmd.valid) return;
    
    // 解析 mode 字段
    if (cmd.mode_str == "manual") {
        // 远程 MANUAL 模式
        remoteManualMode = true;
        executeRemoteCommand(cmd);  // 执行远程指令
    } else if (cmd.mode_str == "auto") {
        // 切换回本地 AUTO 模式
        remoteManualMode = false;
        cancelRemoteCommand();      // 取消远程指令
    }
    
    // 记录已处理命令
    _recordProcessedCommand(cmd.command_id);
    
    // 执行后发送 ACK
    ackCommand(cmd.command_id, true, "Executed");
}

void handlePIRAndFanControl() {
    // 只有在 AUTO 模式下才执行本地控制逻辑
    if (remoteManualMode) {
        return;  // 远程 MANUAL 模式生效，跳过本地 AUTO 控制
    }
    
    // 本地 PIR+ 温度控制逻辑
    // ...
}
```

### 4.4 安全规则

| 规则 | 实现位置 | 说明 |
|------|----------|------|
| 温度低于 26℃关闭风扇 | main.cpp | 无论 AUTO/MANUAL 都生效 |
| 非法档位拒绝执行 | wifi_manager.cpp | gear 不在 0-5 范围时拒绝 |
| 过期指令不执行 | wifi_manager.cpp | expires_at 过期时跳过 |
| 断网时保持 AUTO | main.cpp | 远程指令失效时自动切换回 AUTO |

---

## 5. ACK 重试和去重方案

### 5.1 ACK 重试机制

**当前问题**: ACK 发送失败无重试，可能导致后端未收到确认

**推荐方案**:

```cpp
bool WifiManager::ackCommandWithRetry(int command_id, bool success, 
                                       const String& message, int maxRetries) {
    for (int i = 0; i < maxRetries; i++) {
        if (ackCommand(command_id, success, message)) {
            return true;
        }
        
        Serial.printf("[WifiManager] ACK retry %d/%d\n", i + 1, maxRetries);
        delay(500);  // 重试间隔 500ms
    }
    
    Serial.printf("[WifiManager] ACK failed after %d retries\n", maxRetries);
    return false;
}
```

**参数**:
- `maxRetries`: 最大重试次数，建议 3 次
- 重试间隔：500ms

### 5.2 命令去重机制

**当前实现**: 使用 `int` 数组存储已处理命令 ID（第 49-53 行）

```cpp
int _processedCommandIds[MAX_PROCESSED_COMMANDS];  // ❌ 应为 String
```

**问题**:
- `command_id` 后端返回字符串，ESP32 端解析为 `int` 后全部变为 `0`
- 去重失效，同一命令会被重复处理

**修复方案**:

```cpp
// wifi_manager.h
static const int MAX_PROCESSED_COMMANDS = 20;  // 增加到 20
String _processedCommandIds[MAX_PROCESSED_COMMANDS];  // ✅ String 数组

// wifi_manager.cpp
bool WifiManager::_isCommandProcessed(const String& command_id) {
    for (int i = 0; i < _processedCommandCount; i++) {
        if (_processedCommandIds[i] == command_id) {
            return true;
        }
    }
    return false;
}
```

### 5.3 去重 + 重试流程图

```
┌─────────────────────┐
│  收到远程指令       │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│ _isCommandProcessed │──(已处理)──► ACK 重复指令（1 次）
└──────┬──────────────┘         返回
       │ (未处理)
       ▼
┌─────────────────────┐
│ 执行指令            │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│ ackCommandWithRetry │──(成功)──► 记录 command_id
│ (最多 3 次重试)       │
└─────────────────────┘
```

---

## 6. 过期及非法指令处理方案

### 6.1 过期指令处理

**当前问题**: expires_at 解析失败（toInt() 返回 0），过期检查未实现

**后端格式**: ISO 8601 字符串 `"2026-11-11T12:05:00"`

**ESP32 端处理方案**:

```cpp
/**
 * @brief 解析 ISO 8601 日期字符串为 Unix 时间戳
 * @param isoDate 格式："2026-11-11T12:05:00"
 * @return Unix 时间戳（秒），解析失败返回 0
 */
unsigned long parseISO8601(const String& isoDate) {
    // 简化解析：仅提取年月日时分秒
    // 格式：YYYY-MM-DDTHH:MM:SS
    
    if (isoDate.length() < 19) return 0;
    
    int year = isoDate.substring(0, 4).toInt();
    int month = isoDate.substring(5, 7).toInt();
    int day = isoDate.substring(8, 10).toInt();
    int hour = isoDate.substring(11, 13).toInt();
    int minute = isoDate.substring(14, 16).toInt();
    int second = isoDate.substring(17, 19).toInt();
    
    // 使用 ESP32 的 mktime 计算 Unix 时间戳
    struct tm timeInfo = {0};
    timeInfo.tm_year = year - 1900;
    timeInfo.tm_mon = month - 1;
    timeInfo.tm_mday = day;
    timeInfo.tm_hour = hour;
    timeInfo.tm_min = minute;
    timeInfo.tm_sec = second;
    
    return mktime(&timeInfo);
}

/**
 * @brief 检查指令是否过期
 */
bool WifiManager::isCommandExpired(const String& expires_at) {
    if (expires_at.length() == 0) return false;  // 无过期时间，视为永不过期
    
    unsigned long expiresTime = parseISO8601(expires_at);
    if (expiresTime == 0) return true;  // 解析失败，视为过期
    
    // 获取当前时间（需要 NTP 同步）
    configTime(8 * 3600, 0, "ntp.aliyun.com");  // 东八区
    struct tm timeInfo;
    if (!getLocalTime(&timeInfo)) {
        Serial.println("[WifiManager] Failed to get local time");
        return false;  // 时间获取失败，保守处理：不过期
    }
    
    unsigned long currentTime = mktime(&timeInfo);
    return (currentTime > expiresTime);
}
```

**使用位置**: 在 `handleCommandPolling()` 中，解析后立即检查

```cpp
// 解析指令
Command command;
if (!_parseCommandJSON(response, command)) return false;

// 检查过期
if (isCommandExpired(command.expires_at)) {
    Serial.println("[WifiManager] Command expired, skipping");
    return false;  // 不执行，不发送 ACK
}
```

### 6.2 非法指令处理

**验证规则**:

| 字段 | 合法范围 | 非法处理 |
|------|----------|----------|
| `gear` | 0-5 | 拒绝执行，返回 ACK(success=false) |
| `mode` | "auto"/"manual" | 拒绝执行，返回 ACK(success=false) |
| `command_id` | 非空字符串 | 拒绝执行 |
| `expires_at` | ISO 8601 格式 | 解析失败视为不过期 |

**实现代码**:

```cpp
bool WifiManager::validateCommand(const Command& command) {
    // 检查 command_id
    if (command.command_id.length() == 0) {
        Serial.println("[WifiManager] Invalid command: empty command_id");
        return false;
    }
    
    // 检查 mode
    if (command.mode != 0 && command.mode != 1) {
        Serial.printf("[WifiManager] Invalid command: mode=%d\n", command.mode);
        return false;
    }
    
    // 检查 gear
    if (command.gear < 0 || command.gear > 5) {
        Serial.printf("[WifiManager] Invalid command: gear=%d\n", command.gear);
        return false;
    }
    
    return true;
}
```

**使用位置**: 在解析成功后、执行前进行验证

```cpp
// 解析指令
Command command;
if (!_parseCommandJSON(response, command)) return false;

// 验证指令
if (!validateCommand(command)) {
    Serial.println("[WifiManager] Invalid command, sending error ACK");
    ackCommand(command.command_id, false, "Invalid command parameters");
    return false;
}

// 检查过期
if (isCommandExpired(command.expires_at)) {
    Serial.println("[WifiManager] Command expired, skipping");
    return false;
}

// 执行指令
executeCommand(command);
```

---

## 7. AUTO/MANUAL 安全规则实现及风险

### 7.1 当前实现状态

**main.cpp**:
- 第 364-393 行：PIR+ 温度自动控制逻辑
- 第 427 行：`wifiManager.handleCommandPolling()` 调用
- 第 82 行：`currentRemoteCommandId` 变量（类型错误）

**问题**:
1. 没有模式切换逻辑
2. 远程指令执行后未阻止本地 AUTO 控制
3. `currentRemoteCommandId` 类型与后端不匹配

### 7.2 安全规则实现方案

#### 规则 1: 模式互斥

```cpp
// main.cpp 全局变量
static bool remoteManualMode = false;  // false=本地 AUTO, true=远程 MANUAL

// handlePIRAndFanControl() 开头
void handlePIRAndFanControl() {
    // 远程 MANUAL 模式生效时，跳过本地 AUTO 控制
    if (remoteManualMode) {
        return;
    }
    
    // 本地 PIR+ 温度控制逻辑
    // ...
}

// handleCommandPolling() 中设置模式
void handleCommandPolling() {
    Command cmd;
    if (!fetchPendingCommand(cmd)) return;
    
    if (cmd.mode == 1) {  // MANUAL 模式
        remoteManualMode = true;
        executeRemoteCommand(cmd);
    } else if (cmd.mode == 0) {  // AUTO 模式
        remoteManualMode = false;
    }
}
```

#### 规则 2: 温度保护优先

```cpp
// 无论 AUTO/MANUAL，温度低于 26℃都关闭风扇
void handleTemperatureProtection() {
    float temperature;
    if (!dhtSensor.readTemperature(temperature)) return;
    
    if (temperature < TEMP_STOP_THRESHOLD) {
        Serial.printf("[PROTECTION] Temperature %.1f°C < %d°C, force stop fan\n",
                      temperature, (int)TEMP_STOP_THRESHOLD);
        fanController.startSoftStop();
    }
}
```

#### 规则 3: 断网自动切换回 AUTO

```cpp
void loop() {
    // 检查 Wi-Fi 连接
    if (!wifiManager.isConnected()) {
        if (remoteManualMode) {
            Serial.println("[MODE] Wi-Fi disconnected, switching to AUTO mode");
            remoteManualMode = false;
        }
        return;
    }
    
    // 正常处理
    handlePIRAndFanControl();
    wifiManager.handleCommandPolling();
    // ...
}
```

### 7.3 风险评估

| 风险 | 当前状态 | 修复后 |
|------|----------|--------|
| AUTO/MANUAL同时生效 | ❌ 无互斥逻辑 | ✅ 模式互斥 |
| 远程指令覆盖温度保护 | ❌ 无保护 | ✅ 温度保护优先 |
| 断网后远程模式卡死 | ❌ 无检测 | ✅ 断网切换 AUTO |
| 非法参数导致风扇异常 | ❌ 无验证 | ✅ 参数验证 |
| 过期指令被执行 | ❌ 无检查 | ✅ 过期检查 |

---

## 8. 修复优先级

### 8.1 高优先级（必须修复）

1. **JSON 解析类型不匹配** - `command_id`、`mode`、`expires_at`
2. **指令执行逻辑实现** - 调用风扇控制接口
3. **ACK 发送时机** - 改为执行后发送
4. **模式互斥逻辑** - 防止 AUTO/MANUAL 冲突

### 8.2 中优先级（建议修复）

5. **ACK 重试机制** - 提高可靠性
6. **过期指令检查** - 避免执行过期命令
7. **参数验证** - 拒绝非法 gear 值

### 8.3 低优先级（可选）

8. **去重数组扩容** - 从 10 增加到 20
9. **时间同步** - NTP 客户端集成

---

## 9. 待确认事项

1. **是否保留当前 JSON 解析方式**（不使用 ArduinoJson）？
   - 当前：手动字符串解析
   - 建议：继续使用手动解析，减少依赖

2. **是否需要 NTP 时间同步**？
   - 过期检查需要系统时间
   - 建议：集成 NTP 客户端

3. **远程 MANUAL 模式如何退出**？
   - 方案 A: 后端发送 mode="auto" 切换回 AUTO
   - 方案 B: 断网自动切换回 AUTO
   - 建议：两者都支持

---

## 10. 下一步行动

等待用户确认后，按以下顺序执行修复：

1. ✅ 修改 `wifi_manager.h` - Command 结构体类型
2. ✅ 修改 `wifi_manager.cpp` - JSON 解析、指令执行、ACK 重试
3. ✅ 修改 `main.cpp` - 模式切换、温度保护
4. ✅ 编译验证
5. ⏸️ 停止，等待用户确认

---

**报告结束**

> **状态**: 等待用户确认修复方案  
> **约束**: 本次核查未修改任何文件
