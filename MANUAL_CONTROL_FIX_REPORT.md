# MANUAL 手动控制修复报告

**修复日期**: 2026-11-24  
**问题**: 网页 MANUAL 模式无法控制风扇  
**状态**: ✅ 已修复并验证

---

## 问题根因

1. **前端问题**: `setManualGear()` 创建指令时没有传递 `esp32_id`，使用后端默认值 `default`
2. **后端问题**: `get_pending_command()` 虽然接收 `esp32_id` 参数，但没有根据它筛选指令
3. **结果**: 前端创建的指令（esp32_id=default）和 ESP32 查询的指令（esp32_id=smartfan001）不匹配

---

## 修复内容

### 1. 前端修改 (`frontend/js/app.js`)

**函数**: `setManualGear(gear)` (第 287-325 行)

**修改前**:
```javascript
body: JSON.stringify({
    mode: 'manual',
    gear: gear
})
```

**修改后**:
```javascript
body: JSON.stringify({
    mode: 'manual',
    gear: gear,
    esp32_id: 'smartfan001'  // 明确指定设备 ID，与 ESP32 查询时使用相同的 ID
})
```

---

### 2. 后端修改 (`backend/app.py`)

**函数**: `get_pending_command()` (第 403-460 行)

**修改前**:
```python
# 查找待执行指令
for command in fan_command_queue.values():
    if command['status'] == 'pending':
        pending_command = command
        break
```

**修改后**:
```python
# 查找待执行指令（根据 esp32_id 筛选）
for command in fan_command_queue.values():
    if command['status'] == 'pending' and command['esp32_id'] == esp32_id:
        pending_command = command
        break
```

---

## 验证结果

### ✅ 后端 API 测试

**1. POST 创建指令**
```bash
curl -X POST http://localhost:5000/api/fan/command \
  -H "Content-Type: application/json" \
  -d '{"mode":"manual","gear":2,"esp32_id":"smartfan001"}'
```

**响应**:
```json
{
  "command_id": "1795496872_25c07adb",
  "expires_at": "2026-11-24T13:12:52.221369",
  "gear": 2,
  "message": "Command created",
  "mode": "manual",
  "status": "success"
}
```
✅ 指令创建成功

---

**2. GET 查询指令**
```bash
curl "http://localhost:5000/api/fan/command?esp32_id=smartfan001"
```

**第一次响应** (指令存在):
```json
{
  "command": {
    "command_id": "1795496872_25c07adb",
    "mode": "manual",
    "gear": 2,
    ...
  },
  "message": "Command fetched",
  "status": "success"
}
```

**第二次响应** (指令已被消费):
```json
{
  "command": null,
  "message": "No pending commands",
  "status": "success"
}
```
✅ GET 查询正常，第一次返回指令，第二次返回空（正常行为）

---

## 指令链路说明

```
┌─────────────┐
│  网页前端   │
│ 点击档位按钮 │
└──────┬──────┘
       │
       │ POST /api/fan/command
       │ {mode: 'manual', gear: 2, esp32_id: 'smartfan001'}
       ▼
┌─────────────────┐
│   Flask 后端     │
│ 存入 fan_command_queue │
│ esp32_id='smartfan001' │
└──────┬──────────┘
       │
       │ 每 2 秒轮询
       │ GET /api/fan/command?esp32_id=smartfan001
       ▼
┌─────────────────┐
│   ESP32 设备     │
│ 筛选 esp32_id='smartfan001' │
│ 状态改为 executing │
└──────┬──────────┘
       │
       │ 控制风扇
       ▼
┌─────────────────┐
│   MOSFET 风扇    │
│ 根据档位调整转速 │
└──────┬──────────┘
       │
       │ POST /api/fan/command/status
       │ {command_id: '...', acked: true, status: 'success'}
       ▼
┌─────────────────┐
│   Flask 后端     │
│ 更新指令状态     │
└─────────────────┘
```

---

## 待用户实机验证

以下项目需要用户在真实硬件环境下测试：

- [ ] 网页切换到 MANUAL 模式，显示成功
- [ ] 网页点击档位按钮（0-5），POST 创建成功
- [ ] ESP32 实机日志显示获取到指令（command_id 与网页创建的一致）
- [ ] ESP32 控制风扇转速变化
- [ ] 网页显示指令状态从 pending → executing → success

---

## 未修改内容

以下内容和功能保持不变：

- ✅ GPIO 配置（GPIO17 - PWM 风扇控制）
- ✅ PWM 配置（频率 5000Hz，分辨率 8 位）
- ✅ 硬件接线（JZ-MOS 模块）
- ✅ 独立风扇测试程序（`src/test_fan_simple.cpp`）
- ✅ PIR 传感器代码
- ✅ 温度传感器代码

---

## 技术细节

### 指令状态流转

```
pending (待执行)
  ↓ [ESP32 GET 获取]
executing (执行中)
  ↓ [ESP32 POST 确认]
success (成功) / failed (失败)
```

### 指令过期机制

- TTL: 5 分钟（300000ms）
- 过期后自动失效，ESP32 不会获取到过期指令

### 设备 ID 隔离

- 不同 `esp32_id` 的指令相互隔离
- 支持多设备同时运行而不互相干扰
- 默认设备 ID: `default`
- SmartFan 设备 ID: `smartfan001`

---

## 总结

修复完成，前后端代码已修改并验证。核心改进：

1. **前端明确传递设备 ID**: 确保创建的指令与 ESP32 查询的指令属于同一设备
2. **后端按设备 ID 筛选**: 确保 ESP32 只能获取到属于自己的指令
3. **指令链路完整**: 从网页点击到 ESP32 执行，整个链路现在使用相同的设备 ID

请用户在实际硬件环境下测试网页 MANUAL 控制功能。
