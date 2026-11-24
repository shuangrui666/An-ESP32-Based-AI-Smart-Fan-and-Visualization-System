# Phase 3.2.2 补充架构核查报告

**报告日期**: 2026-11-24  
**核查类型**: 只读检查（不修改任何文件）  
**核查范围**: FanController 接口、main.cpp 控制权、后端协议、前端状态同步

---

## 1. FanController 实际接口清单

### 1.1 类定义与配置

| 项目 | 实际值 | 说明 |
|------|--------|------|
| 类名 | `FanController` | 定义于 `src/control/fan_controller.h` |
| 对象实例 | `static FanController fanController;` | 定义于 `src/main.cpp` 第 67 行 |
| PWM GPIO | `FAN_CONTROL_PIN` (默认 GPIO6) | 与某些文档提到的 GPIO21 **不一致** |
| PWM 频率 | 5000Hz | `FAN_PWM_FREQ` |
| PWM 分辨率 | 8-bit | `FAN_PWM_RESOLUTION` (0-255) |

### 1.2 档位定义与 PWM 映射

| 档位常量 | 数值 | 占空比 | PWM 值 |
|----------|------|--------|-------|
| `FAN_GEAR_OFF` | 0 | 0% | 0 |
| `FAN_GEAR_1` | 1 | 20% | 51 |
| `FAN_GEAR_2` | 2 | 40% | 102 |
| `FAN_GEAR_3` | 3 | 60% | 153 |
| `FAN_GEAR_4` | 4 | 80% | 204 |
| `FAN_GEAR_5` | 5 | 100% | 255 |

### 1.3 状态机枚举

| 状态 | 枚举值 | 说明 |
|------|--------|------|
| `FanState::IDLE` | 0 | 初始状态，风扇关闭 |
| `FanState::SOFT_START` | 1 | 软启动中，占空比线性递增 |
| `FanState::RUNNING` | 2 | 全速运行 |
| `FanState::SOFT_STOP` | 3 | 软停止中，占空比线性递减 |

### 1.4 核心方法

| 方法 | 功能 | 阻塞性 |
|------|------|--------|
| `begin()` | 初始化 LEDC PWM 通道 | 否 |
| `update()` | 周期性调用，处理状态机 | 否 |
| `setGear(int gear)` | 设置档位，中断软启动/软停止 | 否 |
| `startSoftStop()` | 启动软停止 | 否 |
| `getState()` | 返回当前状态 | 否 |
| `getCurrentGear()` | 返回当前档位 | 否 |
| `getCurrentDutyCycle()` | 返回当前 PWM 值 (0-255) | 否 |
| `isPowerOn()` | 返回风扇是否开启 | 否 |

---

## 2. main.cpp 控制权分析

### 2.1 GPIO6 控制权归属

**结论**: `FanController` 拥有 GPIO6 的**唯一控制权**

- `FanController::begin()` 调用 `ledcAttach(FAN_CONTROL_PIN, 5000, 8)` 初始化 PWM
- 无其他模块访问 GPIO6
- `main.cpp` 仅通过 `fanController.setGear()` 间接控制

### 2.2 AUTO 控制逻辑（`handlePIRAndFanControl()`）

**控制流程**（第 285-393 行）:

```
1. 读取 DHT11 温湿度
   └─ 失败 → 关闭风扇，返回

2. 检查 PIR 触发 (checkTrigger())
   └─ 触发 → 更新 lastPirActivityTime
      └─ 温度 ≥ 28℃ → 根据状态执行:
          ├─ IDLE → setGear(FAN_GEAR_5) → SOFT_START
          ├─ SOFT_STOP → setGear(FAN_GEAR_5) → RUNNING
          └─ RUNNING/SOFT_START → 无动作

3. 检查超时 (idleTime ≥ 60 秒)
   └─ 超时且状态=RUNNING → startSoftStop() → SOFT_STOP

4. 检查温度 < 26℃
   └─ 温度低且状态=RUNNING/SOFT_START → startSoftStop() → SOFT_STOP

5. 调用 fanController.update() 更新状态机
```

### 2.3 loop() 调用顺序

```cpp
void loop() {
    // 1. PIR + 温度自动控制
    handlePIRAndFanControl();
    
    // 2. 远程指令轮询
    wifiManager.handleCommandPolling();
    
    // 3. 状态打印 (每 2 秒)
    // 4. 数据上传 (每 5 秒)
    // 5. 延时 10ms
}
```

**关键发现**: AUTO 控制**优先于**远程指令处理，两者**不互斥**

### 2.4 AUTO/MANUAL 互斥实现

**现状**: **未实现互斥**

- `main.cpp` 第 82 行定义 `static int currentRemoteCommandId = -1;`
- 该变量**仅存在于 main.cpp**，`wifi_manager.cpp` 无法访问
- `handlePIRAndFanControl()` 不检查远程命令状态
- **AUTO 控制会覆盖 MANUAL 指令**

---

## 3. 后端协议验证

### 3.1 Command 结构体定义（wifi_manager.h 第 38-45 行）

```cpp
struct Command {
    int command_id;           // ❌ 后端返回字符串，此处期望 int
    int mode;                 // ❌ 后端返回"auto"/"manual"，此处期望 int
    int gear;
    unsigned long expires_at; // ❌ 后端返回 ISO 字符串，此处期望时间戳
    bool valid;
    String status;
};
```

### 3.2 后端真实返回格式（app.py）

| 字段 | 后端返回类型 | 示例值 | ESP32 解析方式 | 结果 |
|------|--------------|--------|---------------|------|
| `command_id` | 字符串 | `"1700000000_abc123"` | `toInt()` | **解析失败，返回 0** |
| `mode` | 字符串 | `"auto"` 或 `"manual"` | `toInt()` | **解析失败，返回 0** |
| `gear` | 整数 | `3` | `toInt()` | ✅ 正常 |
| `expires_at` | ISO 字符串 | `"2026-11-11T12:05:00"` | `toInt()` | **解析失败，返回 0** |

### 3.3 指令轮询逻辑（wifi_manager.cpp 第 423-489 行）

**关键发现**:

1. **指令执行逻辑未实现**（第 468-484 行仅有 TODO 注释）
   ```cpp
   // TODO: 执行指令逻辑
   // 根据 command.mode 和 command.gear 控制风扇
   // 记录当前执行的命令 ID，避免 AUTO 控制覆盖
   ```

2. **ACK 时机错误**（第 393-421 行）
   - 当前：接收时**立即发送 ACK**
   - 需求：**执行后发送 ACK**
   - 影响：后端认为指令已执行，实际未执行

3. **去重机制**
   - `_isCommandProcessed()` + `_recordProcessedCommand()`
   - 使用 `processedCommands[]` 数组记录已处理命令
   - **但因 command_id 解析失败，去重失效**

### 3.4 后端指令队列状态

| 状态 | 说明 |
|------|------|
| `pending` | 待执行 |
| `executing` | 执行中 |
| `completed` | 已完成 |
| `expired` | 已过期 |

---

## 4. 前端状态同步分析

### 4.1 前端 UI 组件（index.html）

| 组件 | 功能 |
|------|------|
| 传感器数据卡片 | 显示温度、湿度、风扇状态、运行档位 |
| AI 推荐卡片 | 显示 AI 推荐档位 |
| 手动控制按钮 | 关闭、1-3 档（实际支持 0-5 档） |
| 实时图表 | ECharts 显示历史数据 |
| 系统信息 | WiFi 信号、最后通信、数据点数 |

### 4.2 控制逻辑（js/app.js）

**关键发现**: `setFanGear()` 函数返回**模拟提示**

```javascript
// 返回： "控制未执行：风扇硬件尚未接入，当前仅支持温湿度监测"
```

**影响**: 前端控制功能**未真正实现**，仅显示占位提示

### 4.3 状态同步机制

| 功能 | 实现状态 |
|------|----------|
| 模式读取 | ✅ `fetchSystemStatus()` 获取 `control_mode` |
| 状态显示 | ✅ 实时显示温度、湿度、风扇档位 |
| 执行确认 | ❌ 前端控制返回"硬件未接入"提示 |
| 远程命令状态 | ❌ 未显示 ESP32 执行的远程命令 |

---

## 5. 8 项架构问题与建议

### 问题 1: command_id 类型不匹配

**现象**: 后端返回字符串 `"1700000000_abc123"`，ESP32 期望 `int`  
**影响**: `toInt()` 解析失败，command_id 始终为 0，去重机制失效  
**建议**: 
- 方案 A: 后端改为返回整数时间戳（去掉 UUID 部分）
- 方案 B: ESP32 改为 `String` 类型，完整解析 JSON

### 问题 2: mode 类型不匹配

**现象**: 后端返回 `"auto"`/`"manual"`，ESP32 期望 `int`  
**影响**: mode 始终为 0，无法区分自动/手动模式  
**建议**: 
- 方案 A: 后端返回整数（0=auto, 1=manual）
- 方案 B: ESP32 改为 `String` 类型，增加模式判断逻辑

### 问题 3: expires_at 类型不匹配

**现象**: 后端返回 ISO 字符串，ESP32 期望 `unsigned long` 时间戳  
**影响**: 过期检查失效  
**建议**: 后端同时返回 `expires_at_timestamp`（Unix 时间戳）

### 问题 4: 指令执行逻辑未实现

**现象**: `wifi_manager.cpp` 第 468-484 行仅有 TODO 注释  
**影响**: 指令仅被记录，未真正执行  
**建议**: 补充指令执行逻辑，调用 `fanController.setGear()`

### 问题 5: ACK 时机错误

**现象**: 接收时立即发送 ACK，而非执行后  
**影响**: 后端认为指令已执行，实际未执行  
**建议**: 将 `ackCommand()` 调用移至指令执行完成后

### 问题 6: AUTO/MANUAL 未互斥

**现象**: `currentRemoteCommandId` 未共享，AUTO 控制会覆盖 MANUAL  
**影响**: 手动指令被自动逻辑覆盖  
**建议**: 
- 方案 A: 将 `currentRemoteCommandId` 提升为全局变量（.h 文件声明）
- 方案 B: `handlePIRAndFanControl()` 检查远程命令状态，有命令时暂停 AUTO

### 问题 7: currentRemoteCommandId 未共享

**现象**: 仅存在于 `main.cpp`，`wifi_manager.cpp` 无法访问  
**影响**: 无法追踪远程命令状态  
**建议**: 在 `wifi_manager.h` 中添加 `getCurrentCommandId()` 方法

### 问题 8: FR-014 文档不存在

**现象**: `docs/FR-014_MANUAL_COMFORT_CONTROL.md` 文件不存在  
**影响**: 无法验证 FR-014 需求内容  
**建议**: 
- 方案 A: 创建 FR-014 需求文档
- 方案 B: 确认 FR-014 是否已合并到其他文档

---

## 6. 优先级排序

| 优先级 | 问题 | 紧急程度 | 影响范围 |
|--------|------|----------|----------|
| P0 | 问题 4: 指令执行逻辑未实现 | 🔴 高 | 核心功能缺失 |
| P0 | 问题 5: ACK 时机错误 | 🔴 高 | 状态同步错误 |
| P1 | 问题 1/2/3: 类型不匹配 | 🟡 中 | 协议解析失败 |
| P1 | 问题 6/7: AUTO/MANUAL 未互斥 | 🟡 中 | 控制冲突 |
| P2 | 问题 8: FR-014 文档缺失 | 🟢 低 | 文档完整性 |

---

## 7. 待确认事项

1. **command_id 格式**: 是否修改后端返回格式，还是修改 ESP32 解析逻辑？
2. **FR-014 需求**: 是否需要创建需求文档，还是确认需求已实现？
3. **AUTO/MANUAL 互斥策略**: 采用方案 A（全局变量）还是方案 B（检查远程命令状态）？
4. **前端控制功能**: 是否需要实现真正的前端控制逻辑，还是保持"硬件未接入"提示？

---

**报告结束**

**下一步**: 等待用户确认后再开始修改代码
