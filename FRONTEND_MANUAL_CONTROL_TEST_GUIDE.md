# SmartFan 2.0 前端手动控制测试指南

## 当前状态

- **Flask 后端进程**: PID 35252，运行在 http://0.0.0.0:5000
- **前端文件位置**: `E:\PlatformIOProjects\SmartFan2_GitHub\frontend\index.html`
- **JavaScript 文件位置**: `E:\PlatformIOProjects\SmartFan2_GitHub\frontend\js\app.js`

## 已知问题

前端按钮点击后，后端没有收到 POST `/api/fan/mode` 或 POST `/api/fan/command` 请求。

## 测试步骤

### 第一步：打开浏览器开发者工具

1. 按 `Ctrl + Shift + I` 打开开发者工具
2. 切换到 **Console** 标签页
3. 切换到 **Network** 标签页，勾选 "Preserve log"（保留日志）

### 第二步：强制刷新清除缓存

1. 访问 http://localhost:5000/
2. 按 `Ctrl + Shift + R` 强制刷新（清除浏览器缓存）
3. 等待页面完全加载

### 第三步：检查 Console 日志

页面加载后，Console 应该显示类似以下日志：
```
[App Init] 应用初始化...
[initUI] 初始化 UI...
[startSensorDataRefresh] 开始刷新传感器数据...
```

**如果看到 JavaScript 错误，请截图记录。**

### 第四步：测试 MANUAL 模式切换

1. 点击 **MANUAL** 按钮
2. 观察 Console 日志，应该显示：
   ```
   [setControlMode] 点击设置模式：manual
   [setControlMode] 发送 POST 请求到 /api/fan/mode
   ```
3. 观察 Network 标签页，应该看到：
   - **Method**: POST
   - **URL**: http://localhost:5000/api/fan/mode
   - **Request Payload**: `{"mode":"manual","esp32_id":"smartfan001"}`
   - **Status**: 200 或 201

**如果 Network 标签页没有显示 POST 请求，说明 JavaScript 函数没有执行。**

### 第五步：测试档位控制

1. 确保已经切换到 MANUAL 模式
2. 点击 **1** 档位按钮
3. 观察 Console 日志，应该显示：
   ```
   [setManualGear] 点击设置 1 档，currentControlMode=manual
   [setManualGear] 发送 POST 请求到 /api/fan/command
   ```
4. 观察 Network 标签页，应该看到：
   - **Method**: POST
   - **URL**: http://localhost:5000/api/fan/command
   - **Request Payload**: `{"mode":"manual","gear":1,"esp32_id":"smartfan001"}`
   - **Status**: 200 或 201

### 第六步：检查后端日志

在另一个窗口查看 Flask 后端输出（如果有的话），应该看到：
```
POST /api/fan/mode
POST /api/fan/command
```

## 可能的问题及解决方案

### 问题 1: Console 显示 "currentControlMode=auto"

**原因**: 页面初始化时 `currentControlMode` 默认为 'auto'，这是正常的。

**解决方法**: 先点击 MANUAL 按钮切换模式。

### 问题 2: 点击按钮后 Console 没有任何日志

**原因**: JavaScript 文件没有加载，或者函数定义有问题。

**解决方法**:
1. 检查 Console 是否有 JavaScript 错误
2. 检查 Network 标签页，确认 app.js 是否成功加载（状态码 200）
3. 在 Console 中输入 `typeof setControlMode`，应该返回 "function"
4. 在 Console 中输入 `typeof setManualGear`，应该返回 "function"

### 问题 3: Network 标签页显示请求失败（红色）

**原因**: 后端服务未运行，或者 CORS 配置问题。

**解决方法**:
1. 确认 Flask 后端正在运行（PID 35252）
2. 检查后端是否有错误日志

### 问题 4: 请求成功但风扇没有反应

**原因**: ESP32 可能没有正确轮询指令队列，或者硬件有问题。

**解决方法**:
1. 检查 ESP32 串口日志
2. 确认 ESP32 能够成功 GET `/api/fan/command?esp32_id=smartfan001`

## 需要反馈的信息

请提供以下信息：

1. **Console 日志截图**（页面加载和点击按钮后的完整日志）
2. **Network 标签页截图**（显示所有请求的列表）
3. **后端 Flask 输出**（如果有日志文件的话）
4. **测试结论**（哪个步骤成功，哪个步骤失败）

## 快速诊断命令

在浏览器 Console 中输入以下命令进行快速诊断：

```javascript
// 检查函数是否存在
console.log('setControlMode:', typeof setControlMode);
console.log('setManualGear:', typeof setManualGear);

// 检查当前模式
console.log('currentControlMode:', currentControlMode);
console.log('currentManualGear:', currentManualGear);

// 检查 BACKEND_URL
console.log('BACKEND_URL:', BACKEND_URL);

// 手动测试发送请求
fetch('/api/fan/mode', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({mode: 'manual', esp32_id: 'smartfan001'})
}).then(r => console.log('手动测试结果:', r.status, r.json()));
```
