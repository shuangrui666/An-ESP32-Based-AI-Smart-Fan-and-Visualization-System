# SmartFan 2.0 Backend

Python Flask 后端服务，提供传感器数据接收、AI 推荐、状态监控等 API 接口。

## 📁 文件结构

```
backend/
├── app.py              # Flask 主应用
├── requirements.txt    # Python 依赖
├── README.md          # 本文件
├── models/            # AI 模型文件（待创建）
│   └── .gitkeep
└── logs/              # 日志文件（待创建）
    └── .gitkeep
```

## 🚀 快速开始

### 1. 创建虚拟环境（推荐）

```bash
# Windows
python -m venv venv
venv\Scripts\activate

# Linux/Mac
python3 -m venv venv
source venv/bin/activate
```

### 2. 安装依赖

```bash
pip install -r requirements.txt
```

### 3. 启动服务

```bash
python app.py
```

服务将在 `http://localhost:5000` 启动。

## 📡 API 接口

### 健康检查

```
GET /health
```

### 接收传感器数据

```
POST /api/sensor/data
Content-Type: application/json

{
    "temperature": 25.5,
    "humidity": 60.0,
    "fan_gear": 1,
    "wifi_signal": -45
}
```

### 获取风扇推荐

```
GET /api/fan/recommendation?temperature=28.5&humidity=65.0
```

### 获取系统状态

```
GET /api/status
```

### 获取历史数据

```
GET /api/history?limit=100
```

## ⚠️ 当前实现状态与限制

1. **传感器数据接口：** 已实现传感器数据接收及相关状态查询功能。
2. **风扇控制接口：** 已实现控制命令队列、命令获取及指令状态查询机制。
3. **自动控制与推荐：** 当前 AI 推荐仍采用规则逻辑，尚未替换为训练完成的机器学习模型。
4. **数据存储：** 当前存储方案仍需结合实际部署环境进一步完善。
5. **硬件验证：** 软件控制接口已经实现，实体风扇的完整端到端运行效果仍以实际硬件联调证据为准。
## 🔧 开发说明

### 添加新的 API 接口

在 `app.py` 中添加新的路由：

```python
@app.route('/api/new-endpoint', methods=['GET'])
def new_endpoint():
    return jsonify({'status': 'success'})
```

### 使用 AI 模型

1. 训练模型并保存到 `models/` 目录
2. 在 `app.py` 中实现 `load_ai_model()` 函数
3. 替换 `simple_rule_based_recommendation()` 为模型推理

## 📝 日志

日志输出到控制台，格式如下：

```
2026-11-11 12:00:00 - __main__ - INFO - Sensor data received: temp=25.5°C, hum=60.0%, gear=1
```

## 🐛 故障排除

### 端口被占用

如果 5000 端口被占用，修改 `app.py` 中的端口：

```python
app.run(host='0.0.0.0', port=8080, debug=True)
```

### CORS 错误

已启用 `flask-cors`，如有特殊需求可配置：

```python
CORS(app, resources={r"/api/*": {"origins": "http://localhost:3000"}})
```

## 📄 许可证

MIT License
