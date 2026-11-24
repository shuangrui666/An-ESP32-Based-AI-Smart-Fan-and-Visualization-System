# SmartFan 2.0 Phase 3.2.1 交付报告

**交付日期**: 2026-11-23  
**阶段名称**: Flask 后端指令队列开发  
**状态**: ✅ 完成

---

## A. 修改文件清单

### 新建文件
| 文件路径 | 说明 |
|---------|------|
| `backend/app.py` | Flask 后端主程序（新增指令队列功能） |
| `backend/test_command_queue.py` | 指令队列自动化测试脚本 |
| `backend/test_results.json` | 测试结果文件 |

### 修改文件
| 文件路径 | 修改内容 |
|---------|---------|
| `backend/app.py` | 新增 6 个 API 路由、指令队列数据结构、辅助函数 |
| `backend/test_command_queue.py` | 完整的 22 项功能测试 |

---

## B. 修改内容

### 1. 新增数据结构（backend/app.py）

```python
# 风扇指令队列（内存存储）
# 数据结构：{command_id: {command_id, mode, gear, created_at, expires_at, status, esp32_id, acked}}
fan_command_queue = {}

# 控制模式：'auto' 或 'manual'
control_mode = 'auto'

# 指令过期时间（分钟）
COMMAND_EXPIRY_MINUTES = 5

# 线程锁，保护共享数据
data_lock = threading.Lock()
```

### 2. 新增 API 路由

#### POST /api/fan/command - 创建风扇控制指令
```json
// Request
{
  "mode": "manual",  // 或 "auto"
  "gear": 3          // 0-5
}

// Response
{
  "status": "success",
  "command_id": "1700736000_a1b2c3d4",
  "mode": "manual",
  "gear": 3,
  "expires_in": 300
}
```

#### GET /api/fan/command - ESP32 获取待执行指令
```json
// Response (有指令)
{
  "status": "success",
  "command": {
    "command_id": "1700736000_a1b2c3d4",
    "mode": "manual",
    "gear": 3
  }
}

// Response (无指令)
{
  "status": "success",
  "command": null
}
```

#### POST /api/fan/command/ack - ESP32 确认指令执行结果
```json
// Request
{
  "command_id": "1700736000_a1b2c3d4",
  "success": true,
  "message": "执行成功"
}

// Response
{
  "status": "success",
  "new_status": "success"
}
```

#### POST /api/fan/mode - 设置控制模式
```json
// Request
{
  "mode": "manual"  // 或 "auto"
}

// Response
{
  "status": "success",
  "mode": "manual",
  "message": "Previous manual commands cancelled"
}
```

#### GET /api/fan/command/status - 前端查询指令状态
```json
// Response
{
  "status": "success",
  "command": {
    "command_id": "1700736000_a1b2c3d4",
    "mode": "manual",
    "gear": 3,
    "status": "pending",
    "created_at": "2026-11-23T12:00:00",
    "expires_at": "2026-11-23T12:05:00"
  }
}
```

#### GET /api/fan/mode - 获取当前控制模式
```json
// Response
{
  "status": "success",
  "mode": "auto"
}
```

### 3. 指令状态机

| 状态 | 说明 | 触发条件 |
|-----|------|---------|
| `pending` | 待执行 | 指令创建后 |
| `executing` | 执行中 | ESP32 获取指令后 |
| `success` | 成功 | ESP32 ACK 成功 |
| `failed` | 失败 | ESP32 ACK 失败 |
| `expired` | 过期 | 超过 5 分钟未执行 |
| `cancelled` | 已取消 | 模式切换时取消旧指令 |

### 4. 核心功能实现

- ✅ **唯一 command_id 生成**: `{timestamp}_{random_8hex}`
- ✅ **档位验证**: 0-5 范围，超出返回 400
- ✅ **模式自动切换**: gear=0 时自动切换到 AUTO 模式
- ✅ **指令过期处理**: 5 分钟有效期，自动清理过期指令
- ✅ **重复 ACK 处理**: 非 pending/executing 状态的 ACK 返回成功但不改变状态
- ✅ **模式切换取消旧指令**: 切换模式时取消所有 pending 状态的旧指令
- ✅ **线程安全**: 使用 threading.Lock 保护共享数据

---

## C. API 接口示例

### 完整交互流程示例

```bash
# 1. 创建指令
curl -X POST http://127.0.0.1:5000/api/fan/command \
  -H "Content-Type: application/json" \
  -d '{"mode": "manual", "gear": 3}'
# 返回：{"status": "success", "command_id": "1700736000_a1b2c3d4", ...}

# 2. ESP32 获取指令
curl http://127.0.0.1:5000/api/fan/command
# 返回：{"status": "success", "command": {...}}

# 3. ESP32 执行完成后 ACK
curl -X POST http://127.0.0.1:5000/api/fan/command/ack \
  -H "Content-Type: application/json" \
  -d '{"command_id": "1700736000_a1b2c3d4", "success": true, "message": "OK"}'
# 返回：{"status": "success", "new_status": "success"}

# 4. 前端查询指令状态
curl "http://127.0.0.1:5000/api/fan/command/status?command_id=1700736000_a1b2c3d4"
# 返回：{"status": "success", "command": {...}}

# 5. 切换控制模式
curl -X POST http://127.0.0.1:5000/api/fan/mode \
  -H "Content-Type: application/json" \
  -d '{"mode": "auto"}'
# 返回：{"status": "success", "mode": "auto"}
```

---

## D. 自动化测试结果

```
============================================================
SmartFan 2.0 Phase 3.2.1 指令队列功能自动化测试
============================================================

✅ 后端服务正在运行：http://127.0.0.1:5000

=== 测试 1: 创建风扇控制指令 ===
✅ PASS: 创建 MANUAL 模式指令
✅ PASS: 创建 AUTO 模式指令
✅ PASS: 拒绝非法档位（负数）
✅ PASS: 拒绝非法档位（>5）
✅ PASS: 拒绝非法模式
✅ PASS: gear=0 自动切换为 AUTO 模式

=== 测试 2: 获取待执行指令 ===
✅ PASS: 获取待执行指令
✅ PASS: 重复获取返回无指令

=== 测试 3: ACK 确认指令 ===
✅ PASS: 成功 ACK
✅ PASS: 重复 ACK 处理
✅ PASS: 失败 ACK
✅ PASS: ACK 不存在的指令

=== 测试 4: 模式切换 ===
✅ PASS: 获取当前模式
✅ PASS: 切换到 MANUAL 模式
✅ PASS: 切换到 AUTO 模式
✅ PASS: 模式切换取消旧指令

=== 测试 5: 查询指令状态 ===
✅ PASS: 查询指令状态
✅ PASS: 查询不存在的指令

=== 测试 6: 原有 API 验证 ===
⚠️  SKIP: DHT 传感器 API - 路由未实现（/api/sensors/dht）
⚠️  SKIP: PIR 传感器 API - 路由未实现（/api/sensors/pir）
✅ PASS: 系统状态 API（包含 control_mode）
✅ PASS: 风扇推荐 API
✅ PASS: 历史数据 API
✅ PASS: 健康检查 API

============================================================
测试结果汇总
============================================================
✅ 通过：22
❌ 失败：0
总计：22
```

**测试结果文件**: `backend/test_results.json`
```json
{
  "passed": 22,
  "failed": 0,
  "total": 22,
  "errors": []
}
```

---

## E. 未解决问题

| 问题 | 说明 | 影响 | 建议 |
|-----|------|------|------|
| DHT/PIR传感器API未实现 | `/api/sensors/dht` 和 `/api/sensors/pir` 路由不存在 | Phase 3.2.1 不要求添加新路由，不影响核心功能 | 后续阶段根据需要添加 |
| 内存存储 | 指令队列使用内存存储，重启后数据丢失 | 适合开发测试，生产环境需持久化 | Phase 3.3 可考虑添加 SQLite 支持 |
| 无指令持久化 | 指令过期后仅从内存删除，无历史记录 | 无法审计历史指令 | 可考虑添加指令历史记录功能 |

---

## F. Git 工作区状态

```
On branch main
Your branch is behind 'origin/main' by 18 commits, and can be fast-forward.

Changes not staged for commit:
  modified:   README.md
  modified:   platformio.ini
  modified:   src/main.cpp
  modified:   src/sensors/dht_sensor.cpp
  modified:   src/sensors/dht_sensor.h
  modified:   src/test_dht11_main.cpp

Untracked files:
  .env.example
  .gitignore
  AUTO_FAN_CONTROL_IMPLEMENTATION.md
  DEF-001_REPAIR_REPORT.md
  DEPLOYMENT_REPORT.md
  EXP-012_CHANGES.diff
  EXP-012_COMPLETION_REPORT.md
  PROJECT_COMPLETE.md
  Procfile
  README.before-merge-20261114.md
  backend/                    ← Phase 3.2.1 新增
  docs/course_project_05/
  evidence/EXP-005/
  experiment/audits/
  experiment/evidence/EXP-012/
  frontend/
  render.yaml
  src/ai/
  src/communication/
  src/control/
  src/sensors/pir_sensor.cpp
  src/sensors/pir_sensor.h
  src/test_fan_simple.cpp
  src/test_mosfet_standalone.cpp
```

**约束遵守情况**:
- ✅ 未修改 ESP32 源码
- ✅ 未修改 frontend/index.html
- ✅ 未修改 frontend/js/app.js
- ✅ 未改变现有传感器 API 行为
- ✅ 未删除已有功能
- ✅ 未使用 `git add .`
- ✅ 未提交 GitHub
- ✅ 未输出真实 Wi-Fi 密码

---

## 下一阶段准备

Phase 3.2.1 已完成，等待下一阶段指令：
- Phase 3.2.2: 前端指令队列 UI 开发
- Phase 3.2.3: ESP32 指令轮询与 ACK 实现
- Phase 3.3: 数据库持久化

**暂停等待用户指令。**
