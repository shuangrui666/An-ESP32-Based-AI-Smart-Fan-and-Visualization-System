# SmartFan 2.0 — Web 前端说明

## 1. 项目简介

本目录存放 SmartFan 2.0 的 Web 前端页面，用于展示 ESP32-S3 采集的环境温湿度数据，并提供系统状态可视化界面。

前端使用 HTML、CSS 和 JavaScript 开发，数据图表使用 ECharts。

## 2. 当前已实现功能

- 温度数据展示。
- 湿度数据展示。
- 温湿度数据图表。
- 与 Flask 后端进行数据交互。
- 展示设备及风扇相关状态信息。
- 对尚未接入的风扇硬件，不伪造实际运行状态。

## 3. 功能实现状态

### 已实现的软件功能

- 展示设备及传感器状态信息。
- 与 Flask 后端进行数据交互。
- 提供风扇手动控制界面及档位选择。
- 提供自动/手动控制模式切换。
- 实现控制指令状态查询与前端反馈。

### 尚待完成的验证

- 完成网页、后端、ESP32 和实体风扇的完整端到端联调。
- 验证实体风扇启停及各档位控制效果。
- 验证真实硬件状态反馈与前端显示的一致性。
- 完成完整的自动温控联动实物测试。
- 完成基于训练完成的机器学习模型的智能控制。
## 4. 目录结构

```text
frontend/
├── index.html
├── css/
│   └── style.css
├── js/
│   └── app.js
└── README.md
```

## 5. 运行环境

- Windows
- Python 3
- Flask 后端
- 支持 JavaScript 的现代浏览器

## 6. 启动方法

### 6.1 启动后端

先在一个终端中启动 Flask：

```powershell
Set-Location "E:\PlatformIOProjects\SmartFan2\backend"
pip install -r requirements.txt
python app.py
```

后端本地地址：

`http://127.0.0.1:5000`

### 6.2 启动前端

另开一个终端：

```powershell
Set-Location "E:\PlatformIOProjects\SmartFan2"
python -m http.server 8080 --directory frontend
```

浏览器打开：

`http://localhost:8080`

注意：前端请求的后端地址应与当前实际运行的 Flask 服务地址一致。ESP32-S3 与电脑之间也需要处于能够互相通信的网络环境。

## 7. 技术说明

| 技术 | 用途 |
|---|---|
| HTML | 页面结构 |
| CSS | 页面布局和样式 |
| JavaScript | 数据获取与页面交互 |
| ECharts | 温湿度数据可视化 |
| Flask API | 获取后端传感器数据 |

## 8. 后续开发计划

1. 完善风扇控制界面。
2. 对接 Flask 风扇控制接口。
3. 实现 ESP32-S3 对控制命令的接收。
4. 实现真实风扇四种状态控制。
5. 将真实硬件状态反馈到前端。
6. 完成系统联调和演示测试。