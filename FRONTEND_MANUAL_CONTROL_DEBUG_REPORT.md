# SmartFan 2.0 前端手动控制故障排查报告

**日期**: 2026-11-24  
**状态**: 后端 API 正常，前端请求未到达后端

---

## 一、已知事实

### 1.1 后端状态 ✅ 正常

- **Flask 进程**: PID 35252，运行在 http://0.0.0.0:5000
- **API 测试结果**:
  - GET `/api/status` - ✅ 正常
  - POST `/api/fan/command` - ✅ 正常（创建指令成功）
  - GET `/api/fan/command/status` - ✅ 正常（查询状态成功）
  - POST `/api/fan/mode` - ✅ 正常（切换模式成功）
  - GET `/api/fan/command?esp32_id=smartfan001` - ✅ 正常（ESP32 能获取指令）
  - POST `/api/fan/command/ack` - ✅ 正常（确认指令成功）

### 1.2 前端状态 ❓ 待验证

- **前端文件**: `E:\PlatformIOProjects\SmartFan2_GitHub\frontend\index.html`
- **JavaScript**: `E:\PlatformIOProjects\SmartFan2_GitHub\frontend\js\app.js`
- **问题**: 网页点击 MANUAL 和档位按钮后，**后端完全没有收到 POST 请求**

---

## 二、问题根因分析

### 2.1 已排除的原因

| 可能原因 | 验证结果 | 结论 |
|---------|---------|------|
| 后端 API 故障 | Python 测试脚本验证所有 API 正常 | ❌ 排除 |
| esp32_id 不一致 | 前端已传递 `esp32_id: 'smartfan001'` | ❌ 排除 |
| 指令队列逻辑错误 | 测试脚本验证指令创建、获取、ACK 流程正常 | ❌ 排除 |
| 前端函数未包含 esp32_id | 代码审查确认 `setManualGear()` 包含 esp32_id | ❌ 排除 |

### 2.2 待验证的原因

| 可能原因 | 验证方法 | 优先级 |
|---------|---------|-------|
| **浏览器缓存旧版 JavaScript** | 按 Ctrl+Shift+R 强制刷新 | ⭐⭐⭐ 高 |
| **JavaScript 运行时错误** | 打开浏览器 Console 查看错误 | ⭐⭐⭐ 高 |
| **currentControlMode 初始化失败** | Console 输入 `console.log(currentControlMode)` | ⭐⭐ 中 |
| **按钮 onclick 未绑定** | Console 输入 `document.querySelector('button[onclick*="MANUAL"]')` | ⭐⭐ 中 |
| **BACKEND_URL 配置错误** | Console 输入 `console.log(BACKEND_URL)` | ⭐ 低 |

---

## 三、浏览器实测步骤

### 步骤 1: 打开浏览器开发者工具

1. 使用 Chrome 或 Edge 浏览器
2. 按 `F12` 或 `Ctrl + Shift + I` 打开开发者工具
3. 切换到 **Console** 标签页
4. 切换到 **Network** 标签页，勾选 **"Preserve log"（保留日志）**

### 步骤 2: 强制刷新清除缓存

1. 访问 http://localhost:5000/
2. 按 `Ctrl + Shift + R` **强制刷新**（重要！）
3. 等待页面完全加载

### 步骤 3: 检查 Console 日志

页面加载后，Console 应该显示：
```
[App Init] 应用初始化...
[initUI] 初始化 UI...
[startSensorDataRefresh] 开始刷新传感器数据...
```

**如果没有看到这些日志，说明 JavaScript 未正确加载。**

### 步骤 4: 测试 MANUAL 模式切换

1. 点击页面上的 **MANUAL** 按钮
2. 观察 Console，应该显示：
   ```
   [setControlMode] 点击设置模式：manual
   [setControlMode] 发送 POST 请求到 /api/fan/mode
   ```
3. 观察 Network 标签页，应该看到：
   - **Method**: POST
   - **URL**: http://localhost:5000/api/fan/mode
   - **Request Payload**: `{"mode":"manual","esp32_id":"smartfan001"}`
   - **Status**: 200

**如果 Network 标签页没有显示 POST 请求，说明 JavaScript 函数没有执行。**

### 步骤 5: 测试档位控制

1. 确保已经切换到 MANUAL 模式（按钮应该高亮）
2. 点击 **1** 档位按钮
3. 观察 Console，应该显示：
   ```
   [setManualGear] 点击设置 1 档，currentControlMode=manual
   [setManualGear] 发送 POST 请求到 /api/fan/command
   ```
4. 观察 Network 标签页，应该看到：
   - **Method**: POST
   - **URL**: http://localhost:5000/api/fan/command
   - **Request Payload**: `{"mode":"manual","gear":1,"esp32_id":"smartfan001"}`
   - **Status**: 200

### 步骤 6: 快速诊断命令

在浏览器 Console 中输入以下命令：

```javascript
// 检查全局变量
console.log('BACKEND_URL:', BACKEND_URL);
console.log('currentControlMode:', currentControlMode);
console.log('currentManualGear:', currentManualGear);

// 检查函数是否存在
console.log('setControlMode:', typeof setControlMode);
console.log('setManualGear:', typeof setManualGear);

// 手动测试发送请求（会真正执行）
fetch('/api/fan/mode', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({mode: 'manual', esp32_id: 'smartfan001'})
}).then(r => console.log('测试结果:', r.status));
```

---

## 四、诊断页面

已创建诊断页面：http://localhost:5000/debug.html

**功能**:
- 检查 JavaScript 是否加载
- 检查函数是否存在
- 检查后端连接
- 测试按钮事件绑定

**使用方法**:
1. 访问 http://localhost:5000/debug.html
2. 按 `Ctrl + Shift + R` 强制刷新
3. 查看页面显示的诊断结果
4. 点击测试按钮，查看结果

---

## 五、需要用户反馈的信息

请提供以下信息：

### 5.1 Console 日志截图

- 页面加载后的完整 Console 日志
- 点击 MANUAL 按钮后的 Console 日志
- 点击 1 档按钮后的 Console 日志

### 5.2 Network 标签页截图

- 显示所有请求的列表（包括 GET 和 POST）
- 如果没有 POST 请求，截图证明

### 5.3 诊断页面结果

- 访问 http://localhost:5000/debug.html 的截图
- 诊断页面显示的所有状态

### 5.4 快速诊断命令输出

在 Console 中运行以下命令并复制输出：
```javascript
console.log('BACKEND_URL:', BACKEND_URL);
console.log('currentControlMode:', currentControlMode);
console.log('setControlMode:', typeof setControlMode);
console.log('setManualGear:', typeof setManualGear);
```

---

## 六、可能的修复方案

### 方案 1: 浏览器缓存问题（最可能）

**症状**: Network 标签页显示 app.js 状态码为 304（未修改）

**修复**: 
1. 按 `Ctrl + Shift + R` 强制刷新
2. 或者在开发者工具中右键刷新按钮，选择 "清空缓存并硬性重新加载"

### 方案 2: JavaScript 加载失败

**症状**: Console 显示 `app.js` 加载失败（404 或 500）

**修复**:
1. 检查 Flask 静态文件配置
2. 确认 `frontend/js/app.js` 文件存在

### 方案 3: currentControlMode 初始化错误

**症状**: Console 显示 `currentControlMode: undefined`

**修复**: 检查 `app.js` 第 32 行的初始化代码

### 方案 4: 按钮事件未绑定

**症状**: 点击按钮没有任何反应，Console 无日志

**修复**: 检查 `index.html` 按钮的 `onclick` 属性

---

## 七、下一步行动

1. **用户操作**: 按照上述步骤进行浏览器实测
2. **收集信息**: 将 Console 日志、Network 截图、诊断页面结果反馈
3. **定位根因**: 根据反馈确定具体问题
4. **修复验证**: 修复后进行实际验证

---

## 八、附录：后端测试脚本

已创建 Python 测试脚本：`test_frontend_control.py`

**运行方式**:
```bash
python test_frontend_control.py
```

**测试结果**: 所有后端 API 正常 ✅
