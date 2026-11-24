# DEF-002: 指令轮询架构修复报告

## 问题描述

在 Phase 3.2.2 代码核查中发现 `handleRemoteCommand()` 函数存在架构问题：

### 原代码问题

```cpp
// src/main.cpp - handleRemoteCommand()
void handleRemoteCommand() {
    Command command;
    
    // 直接使用 fetchPendingCommand()，绕过了去重和过期检查
    if (wifiManager.fetchPendingCommand(command)) {
        // 执行指令...
    }
}
```

**问题：**
1. `handleRemoteCommand()` 直接调用 `fetchPendingCommand()`，绕过了 `handleCommandPolling()` 中的：
   - 去重检查（`_isCommandProcessed()`）
   - 过期检查（`command.isExpired()`）
2. 可能导致：
   - 重复处理同一个命令
   - 执行已过期的命令

### 架构设计不一致

原本的设计意图是：
- `handleCommandPolling()`：负责完整的指令轮询流程（包括去重、过期检查）
- `fetchPendingCommand()`：仅负责 HTTP 请求和 JSON 解析
- `handleRemoteCommand()`：应该调用 `handleCommandPolling()` 获取有效指令

但实际实现中，`handleRemoteCommand()` 绕过了中间层，直接调用底层函数。

## 修复方案

### 修改 1: `wifi_manager.h`

将 `handleCommandPolling()` 函数签名改为通过引用参数返回 Command 对象：

```cpp
// 原签名
bool handleCommandPolling();

// 新签名
bool handleCommandPolling(Command& command);
```

**说明：**
- 通过引用参数返回 Command 对象，让调用者能够获取完整的指令数据
- 在返回 `false` 的所有路径中设置 `command.valid = false`

### 修改 2: `wifi_manager.cpp`

在 `handleCommandPolling()` 实现中：
1. 移除内部的 `Command command;` 局部变量声明
2. 直接使用传入的引用参数 `command`
3. 在所有返回 `false` 的路径中设置 `command.valid = false`
4. 在返回 `true` 前，Command 对象已通过引用填充完整数据

### 修改 3: `main.cpp`

修改 `handleRemoteCommand()` 函数，调用新的 `handleCommandPolling(Command& command)` 接口：

```cpp
void handleRemoteCommand() {
    Command command;
    
    // 使用 handleCommandPolling() 代替 fetchPendingCommand()
    // 这样可以确保去重检查和过期检查生效
    if (wifiManager.handleCommandPolling(command)) {
        Serial.println("[CMD] Received new valid command from backend");
        
        // 执行指令
        bool success = executeCommand(command);
        
        // 发送 ACK
        if (success) {
            String message = String("Command ") + currentRemoteCommandId + 
                           " executed: mode=" + currentControlMode + 
                           ", gear=" + String(fanController.getCurrentGear());
            wifiManager.ackCommand(command.command_id, true, message);
        } else {
            String message = String("Command ") + command.command_id + " execution failed";
            wifiManager.ackCommand(command.command_id, false, message);
        }
    }
}
```

## 修复后的流程

```
loop()
  └─> handleRemoteCommand()
        └─> handleCommandPolling(Command& command)  ← 新增：获取有效 Command 对象
              ├─> 检查轮询间隔
              ├─> 检查 Wi-Fi 连接
              ├─> fetchPendingCommand(command)      ← 获取原始指令
              ├─> 检查 command.valid
              ├─> _isCommandProcessed()             ← 去重检查
              └─> command.isExpired()               ← 过期检查
              
        ← 返回有效 Command 对象
        └─> executeCommand(command)                 ← 执行指令
        └─> ackCommand()                            ← 发送 ACK
```

## 验证结果

### 编译测试

```bash
platformio run
```

**结果：** ✅ 所有 5 个环境编译成功
- esp32-s3-devkitc-1: SUCCESS
- dht11-test: SUCCESS
- pir-read-test: SUCCESS
- mosfet-standalone-test: SUCCESS
- fan-simple-test: SUCCESS

### 代码审查

修复后的代码符合以下原则：
1. ✅ 单一职责：`handleCommandPolling()` 负责完整的轮询逻辑
2. ✅ 去重机制：通过 `_processedCommandIds` 数组防止重复处理
3. ✅ 过期检查：通过 `isExpired()` 方法确保指令有效性
4. ✅ 清晰的调用链：`handleRemoteCommand()` → `handleCommandPolling()` → `fetchPendingCommand()`

## 影响范围

### 修改的文件

1. `src/communication/wifi_manager.h` - 函数签名修改
2. `src/communication/wifi_manager.cpp` - 实现修改
3. `src/main.cpp` - 调用方式修改

### 向后兼容性

- 此修改不影响外部 API
- 后端和前端无需修改
- 仅影响 ESP32 固件内部实现

## 后续建议

1. **硬件测试**：上传固件到实际设备，验证：
   - 去重机制是否生效（重复命令不会被执行两次）
   - 过期机制是否生效（过期命令不会被执行）
   - ACK 机制是否正常工作

2. **日志监控**：观察串口日志中的 `[POLL]` 标记，确认：
   - 轮询间隔正常（1 秒）
   - 去重检查正常（DUPLICATE 标记）
   - 过期检查正常（EXPIRED 标记）

3. **压力测试**：快速发送多个命令，验证：
   - 命令队列管理正常
   - 内存使用稳定

## 相关文档

- [Phase 3.2.2 完成报告](EXP-012_COMPLETION_REPORT.md)
- [AUTO_FAN_CONTROL_IMPLEMENTATION.md](AUTO_FAN_CONTROL_IMPLEMENTATION.md)
- [wifi_manager.h](src/communication/wifi_manager.h)
- [wifi_manager.cpp](src/communication/wifi_manager.cpp)

---

**修复日期：** 2026-11-24  
**修复版本：** Phase 3.2.2  
**修复状态：** ✅ 已完成
