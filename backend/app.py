"""
SmartFan 2.0 Backend - Flask API Server
========================================

这是一个用于 SmartFan 2.0 的 Python Flask 后端服务。
提供传感器数据接收、AI 推荐、状态监控等 API 接口。

⚠️ 注意：这是框架代码，部分功能需要根据实际硬件和 AI 模型进行完善。

作者：SmartFan Team
版本：0.1.0
日期：2026-11-11
"""

import os
from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
from datetime import datetime, timedelta
import logging
import uuid
import threading
import time

# 初始化 Flask 应用
# 配置静态文件目录（frontend 目录相对于 backend/app.py）
app = Flask(
    __name__,
    static_folder='../frontend',
    static_url_path=''
)
CORS(app)  # 启用跨域支持

# 获取项目根目录路径
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# 配置日志
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# 数据存储（实际项目中应使用数据库）
sensor_data_history = []
system_status = {
    'status': 'offline',
    'last_seen': None,
    'wifi_signal': 0
}

# ==================== 指令队列数据结构 ====================

# 风扇指令队列（内存存储）
# 数据结构：{command_id: {command_id, mode, gear, created_at, expires_at, status, esp32_id, acked}}
fan_command_queue = {}

# 控制模式：'auto' 或 'manual'
control_mode = 'auto'

# 指令过期时间（分钟）
COMMAND_EXPIRY_MINUTES = 5

# 线程锁，保护共享数据
data_lock = threading.Lock()


# ==================== API 接口 ====================

@app.route('/api/sensor/data', methods=['POST'])
def receive_sensor_data():
    """
    接收 ESP32 上传的传感器数据
    
    Request Body (JSON):
    {
        "temperature": 25.5,
        "humidity": 60.0,
        "fan_gear": 1,
        "timestamp": "2026-11-11T12:00:00"
    }
    
    Response:
    {
        "status": "success",
        "message": "Data received"
    }
    """
    try:
        data = request.get_json()
        
        # 验证必填字段
        required_fields = ['temperature', 'humidity', 'fan_gear']
        for field in required_fields:
            if field not in data:
                return jsonify({
                    'status': 'error',
                    'message': f'Missing field: {field}'
                }), 400
        
        # 记录数据
        record = {
            'temperature': data['temperature'],
            'humidity': data['humidity'],
            'fan_gear': data['fan_gear'],
            'timestamp': datetime.now().isoformat(),
            'wifi_signal': data.get('wifi_signal', 0)
        }
        sensor_data_history.append(record)
        
        # 限制历史记录数量（最多 1000 条）
        if len(sensor_data_history) > 1000:
            sensor_data_history.pop(0)
        
        # 更新系统状态
        system_status['status'] = 'online'
        system_status['last_seen'] = record['timestamp']
        system_status['wifi_signal'] = record['wifi_signal']
        
        logger.info(f"Sensor data received: temp={record['temperature']}°C, "
                   f"hum={record['humidity']}%, gear={record['fan_gear']}")
        
        return jsonify({
            'status': 'success',
            'message': 'Data received',
            'record_id': len(sensor_data_history)
        }), 200
        
    except Exception as e:
        logger.error(f"Error receiving sensor data: {str(e)}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@app.route('/api/fan/recommendation', methods=['GET'])
def get_fan_recommendation():
    """
    获取风扇档位推荐
    
    Query Parameters:
    - temperature: 温度值 (℃)
    - humidity: 湿度值 (%)
    
    Response:
    {
        "gear": 2,
        "reason": "温度较高，建议开启 2 档",
        "temperature": 28.5,
        "humidity": 65.0
    }
    
    ⚠️ 注意：当前使用简单规则，后期替换为 AI 模型
    """
    try:
        temperature = float(request.args.get('temperature', 0))
        humidity = float(request.args.get('humidity', 0))
        
        # TODO: 替换为 AI 模型推理
        # 当前使用简单规则
        gear, reason = simple_rule_based_recommendation(temperature, humidity)
        
        logger.info(f"Recommendation: temp={temperature}°C, hum={humidity}%, "
                   f"gear={gear}, reason={reason}")
        
        return jsonify({
            'status': 'success',
            'gear': gear,
            'reason': reason,
            'temperature': temperature,
            'humidity': humidity,
            'method': 'rule_based'  # TODO: 改为 'ai_model'
        }), 200
        
    except Exception as e:
        logger.error(f"Error getting recommendation: {str(e)}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@app.route('/api/status', methods=['GET'])
def get_system_status():
    """
    获取系统状态

    Response:
    {
        "status": "online",
        "last_seen": "2026-11-11T12:00:00",
        "wifi_signal": -45,
        "data_points": 100,
        "control_mode": "auto"
    }
    """
    try:
        with data_lock:
            current_mode = control_mode
        
        return jsonify({
            'status': 'success',
            'system': system_status,
            'data_points': len(sensor_data_history),
            'control_mode': current_mode
        }), 200

    except Exception as e:
        logger.error(f"Error getting system status: {str(e)}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@app.route('/api/history', methods=['GET'])
def get_sensor_history():
    """
    获取传感器历史数据
    
    Query Parameters:
    - limit: 返回记录数（默认 100，最大 1000）
    
    Response:
    {
        "status": "success",
        "data": [...],
        "count": 100
    }
    """
    try:
        limit = min(int(request.args.get('limit', 100)), 1000)
        
        # 返回最新的 limit 条记录
        history = sensor_data_history[-limit:]
        
        return jsonify({
            'status': 'success',
            'data': history,
            'count': len(history)
        }), 200
        
    except Exception as e:
        logger.error(f"Error getting history: {str(e)}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@app.route('/api/fan/control', methods=['POST'])
def control_fan():
    """
    ⚠️ 预留接口：手动控制风扇

    实际项目中，此接口用于 Web 界面手动控制风扇档位

    Request Body (JSON):
    {
        "action": "set_gear",  // 或 "turn_on", "turn_off"
        "gear": 2
    }

    Response:
    {
        "status": "success",
        "message": "Fan gear set to 2"
    }
    """
    try:
        data = request.get_json()
        action = data.get('action', '')

        # TODO: 实现实际的风扇控制逻辑
        # 可以通过 MQTT 或其他协议发送到 ESP32

        return jsonify({
            'status': 'success',
            'message': f'Fan control command sent: {action}',
            'note': '此功能需要 ESP32 端配合实现'
        }), 200

    except Exception as e:
        logger.error(f"Error controlling fan: {str(e)}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


# ==================== 指令队列 API ====================

@app.route('/api/fan/command', methods=['POST'])
def create_fan_command():
    """
    创建风扇控制指令

    Request Body (JSON):
    {
        "mode": "manual",  // 或 "auto"
        "gear": 2,         // 0-5，0 表示关闭
        "esp32_id": "device_001"  // 可选，指定目标设备
    }

    Response:
    {
        "status": "success",
        "command_id": "1700000000_abc123",
        "message": "Command created",
        "expires_at": "2026-11-11T12:05:00"
    }
    """
    try:
        # 详细请求日志
        logger.info(f"=== POST /api/fan/command ===")
        logger.info(f"Request headers: {dict(request.headers)}")
        logger.info(f"Request JSON body: {request.get_json()}")
        
        data = request.get_json()
        
        # 验证必填字段
        if 'mode' not in data:
            return jsonify({
                'status': 'error',
                'message': 'Missing field: mode'
            }), 400
        
        mode = data.get('mode', 'manual')
        gear = data.get('gear', 0)
        esp32_id = data.get('esp32_id', 'default')
        
        # 验证模式
        if mode not in ['auto', 'manual']:
            return jsonify({
                'status': 'error',
                'message': 'Invalid mode. Must be "auto" or "manual"'
            }), 400
        
        # 验证档位（0-5）
        try:
            gear = int(gear)
            if gear < 0 or gear > 5:
                return jsonify({
                    'status': 'error',
                    'message': 'Invalid gear. Must be between 0 and 5'
                }), 400
        except (ValueError, TypeError):
            return jsonify({
                'status': 'error',
                'message': 'Invalid gear. Must be an integer between 0 and 5'
            }), 400

        # 注意：手动模式下 gear=0 保持 manual 模式（关闭风扇）
        # 不再自动改为 auto 模式
        
        # 生成唯一 command_id
        command_id = f"{int(datetime.now().timestamp())}_{uuid.uuid4().hex[:8]}"

        # 计算过期时间
        created_at = datetime.now()
        expires_at = created_at + timedelta(minutes=COMMAND_EXPIRY_MINUTES)

        # 创建指令
        command = {
            'command_id': command_id,
            'mode': mode,
            'gear': gear,
            'esp32_id': esp32_id,
            'created_at': created_at.isoformat(),
            'expires_at': expires_at.isoformat(),
            'ttl_ms': COMMAND_EXPIRY_MINUTES * 60 * 1000,  # 总 TTL（毫秒）
            'status': 'pending',  # pending, executing, success, failed, expired, cancelled
            'acked': False,
            'ack_time': None
        }
        
        with data_lock:
            # 如果是 MANUAL 模式，取消所有其他 pending/executing 指令
            if mode == 'manual':
                for cid, cmd in fan_command_queue.items():
                    if cmd['status'] in ['pending', 'executing']:
                        cmd['status'] = 'cancelled'
                        cmd['cancel_reason'] = 'Replaced by new manual command'
            
            # 保存指令
            fan_command_queue[command_id] = command
        
        logger.info(f"Command created: id={command_id}, mode={mode}, gear={gear}")
        
        return jsonify({
            'status': 'success',
            'command_id': command_id,
            'message': 'Command created',
            'expires_at': expires_at.isoformat(),
            'mode': mode,
            'gear': gear
        }), 200
        
    except Exception as e:
        logger.error(f"Error creating command: {str(e)}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@app.route('/api/fan/command', methods=['GET'])
def get_pending_command():
    """
    ESP32 获取待执行指令
    
    Query Parameters:
    - esp32_id: 设备 ID（可选）
    
    Response:
    {
        "status": "success",
        "command": {
            "command_id": "1700000000_abc123",
            "mode": "manual",
            "gear": 2,
            "expires_at": "2026-11-11T12:05:00"
        }
    }
    或
    {
        "status": "success",
        "command": null,
        "message": "No pending commands"
    }
    """
    try:
        esp32_id = request.args.get('esp32_id', 'default')
        
        with data_lock:
            # 清理过期指令
            _cleanup_expired_commands()

            # 查找待执行指令（根据 esp32_id 筛选）
            pending_command = None
            for command in fan_command_queue.values():
                if command['status'] == 'pending' and command['esp32_id'] == esp32_id:
                    pending_command = command
                    break
            
            if pending_command:
                # 标记为执行中
                pending_command['status'] = 'executing'
                
                # 计算剩余 TTL（毫秒）
                expires_at = datetime.fromisoformat(pending_command['expires_at'])
                remaining_ttl_ms = int((expires_at - datetime.now()).total_seconds() * 1000)
                if remaining_ttl_ms < 0:
                    remaining_ttl_ms = 0
                
                logger.info(f"Command fetched: id={pending_command['command_id']}, "
                           f"mode={pending_command['mode']}, gear={pending_command['gear']}, "
                           f"remaining_ttl_ms={remaining_ttl_ms}")

                return jsonify({
                    'status': 'success',
                    'command': {
                        'command_id': pending_command['command_id'],
                        'mode': pending_command['mode'],
                        'gear': pending_command['gear'],
                        'expires_at': pending_command['expires_at'],
                        'remaining_ttl_ms': remaining_ttl_ms
                    }
                }), 200
            else:
                return jsonify({
                    'status': 'success',
                    'command': None,
                    'message': 'No pending commands'
                }), 200
        
    except Exception as e:
        logger.error(f"Error getting pending command: {str(e)}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@app.route('/api/fan/command/ack', methods=['POST'])
def ack_command():
    """
    ESP32 确认指令执行结果

    Request Body (JSON):
    {
        "command_id": "1700000000_abc123",
        "success": true,  // 或 false
        "message": "执行成功"  // 可选
    }

    Response:
    {
        "status": "success",
        "command_id": "1700000000_abc123",
        "new_status": "success"
    }
    """
    try:
        # 详细请求日志
        logger.info(f"=== POST /api/fan/command/ack ===")
        logger.info(f"Request headers: {dict(request.headers)}")
        logger.info(f"Request JSON body: {request.get_json()}")
        
        data = request.get_json()
        
        # 验证必填字段
        if 'command_id' not in data:
            return jsonify({
                'status': 'error',
                'message': 'Missing field: command_id'
            }), 400
        
        if 'success' not in data:
            return jsonify({
                'status': 'error',
                'message': 'Missing field: success'
            }), 400
        
        command_id = data['command_id']
        success = data['success']
        message = data.get('message', '')
        
        with data_lock:
            if command_id not in fan_command_queue:
                return jsonify({
                    'status': 'error',
                    'message': f'Command not found: {command_id}'
                }), 404
            
            command = fan_command_queue[command_id]
            
            # 检查指令是否已过期
            if command['status'] == 'expired':
                return jsonify({
                    'status': 'error',
                    'message': 'Command already expired',
                    'command_id': command_id,
                    'new_status': 'expired'
                }), 400
            
            # 检查指令是否已处理（重复 ACK）
            if command['status'] in ['success', 'failed', 'cancelled']:
                logger.warning(f"Duplicate ACK for command {command_id}, status={command['status']}")
                return jsonify({
                    'status': 'success',
                    'command_id': command_id,
                    'new_status': command['status'],
                    'message': 'Command already processed (duplicate ACK ignored)'
                }), 200
            
            # 更新状态
            command['acked'] = True
            command['ack_time'] = datetime.now().isoformat()
            command['status'] = 'success' if success else 'failed'
            command['ack_message'] = message
            
            logger.info(f"Command ACK received: id={command_id}, "
                       f"success={success}, new_status={command['status']}")
            
            return jsonify({
                'status': 'success',
                'command_id': command_id,
                'new_status': command['status']
            }), 200
        
    except Exception as e:
        logger.error(f"Error processing ACK: {str(e)}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@app.route('/api/fan/mode', methods=['POST'])
def set_control_mode():
    """
    设置控制模式（AUTO/MANUAL 切换）
    
    Request Body (JSON):
    {
        "mode": "manual"  // 或 "auto"
    }
    
    Response:
    {
        "status": "success",
        "mode": "manual",
        "message": "Control mode switched to manual"
    }
    """
    try:
        data = request.get_json()
        
        if 'mode' not in data:
            return jsonify({
                'status': 'error',
                'message': 'Missing field: mode'
            }), 400
        
        mode = data['mode']
        
        if mode not in ['auto', 'manual']:
            return jsonify({
                'status': 'error',
                'message': 'Invalid mode. Must be "auto" or "manual"'
            }), 400
        
        global control_mode

        # 详细请求日志
        logger.info(f"=== POST /api/fan/mode ===")
        logger.info(f"Request headers: {dict(request.headers)}")
        logger.info(f"Request JSON body: {request.get_json()}")

        with data_lock:
            old_mode = control_mode
            control_mode = mode
            
            # 如果切换到 MANUAL 模式，取消所有 pending/executing 指令
            if mode == 'manual' and old_mode == 'auto':
                cancelled_count = 0
                for command in fan_command_queue.values():
                    if command['status'] in ['pending', 'executing']:
                        command['status'] = 'cancelled'
                        command['cancel_reason'] = 'Switched to manual mode'
                        cancelled_count += 1
                if cancelled_count > 0:
                    logger.info(f"Cancelled {cancelled_count} commands due to mode switch to manual")
            
            # 如果切换到 AUTO 模式，取消所有 MANUAL 模式的 pending 指令
            if mode == 'auto' and old_mode == 'manual':
                cancelled_count = 0
                for command in fan_command_queue.values():
                    if command['status'] == 'pending' and command['mode'] == 'manual':
                        command['status'] = 'cancelled'
                        command['cancel_reason'] = 'Switched to auto mode'
                        cancelled_count += 1
                if cancelled_count > 0:
                    logger.info(f"Cancelled {cancelled_count} manual commands due to mode switch to auto")
        
        logger.info(f"Control mode switched: {old_mode} -> {mode}")
        
        return jsonify({
            'status': 'success',
            'mode': mode,
            'message': f'Control mode switched to {mode}'
        }), 200
        
    except Exception as e:
        logger.error(f"Error setting control mode: {str(e)}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@app.route('/api/fan/command/status', methods=['GET'])
def get_command_status():
    """
    获取指令状态（前端查询）

    Query Parameters:
    - command_id: 指令 ID（必填）

    Response:
    {
        "status": "success",
        "command": {
            "command_id": "1700000000_abc123",
            "mode": "manual",
            "gear": 2,
            "status": "pending",
            "created_at": "2026-11-11T12:00:00",
            "expires_at": "2026-11-11T12:05:00"
        }
    }
    """
    try:
        # 详细请求日志
        logger.info(f"=== GET /api/fan/command/status ===")
        logger.info(f"Request query params: {request.args}")
        
        command_id = request.args.get('command_id')
        
        if not command_id:
            return jsonify({
                'status': 'error',
                'message': 'Missing parameter: command_id'
            }), 400
        
        with data_lock:
            # 清理过期指令
            _cleanup_expired_commands()
            
            if command_id not in fan_command_queue:
                return jsonify({
                    'status': 'error',
                    'message': f'Command not found: {command_id}'
                }), 404
            
            command = fan_command_queue[command_id]
            
            return jsonify({
                'status': 'success',
                'command': {
                    'command_id': command['command_id'],
                    'mode': command['mode'],
                    'gear': command['gear'],
                    'status': command['status'],
                    'created_at': command['created_at'],
                    'expires_at': command['expires_at'],
                    'acked': command.get('acked', False),
                    'ack_time': command.get('ack_time'),
                    'ack_message': command.get('ack_message')
                }
            }), 200
        
    except Exception as e:
        logger.error(f"Error getting command status: {str(e)}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@app.route('/api/fan/mode', methods=['GET'])
def get_control_mode():
    """
    获取当前控制模式
    
    Response:
    {
        "status": "success",
        "mode": "auto"  // 或 "manual"
    }
    """
    try:
        with data_lock:
            current_mode = control_mode
        
        return jsonify({
            'status': 'success',
            'mode': current_mode
        }), 200
        
    except Exception as e:
        logger.error(f"Error getting control mode: {str(e)}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


# ==================== 辅助函数 ====================

def _cleanup_expired_commands():
    """
    清理过期指令（内部函数，需持有 data_lock）
    """
    now = datetime.now()
    expired_count = 0
    
    for command_id, command in fan_command_queue.items():
        if command['status'] in ['pending', 'executing']:
            expires_at = datetime.fromisoformat(command['expires_at'])
            if now > expires_at:
                command['status'] = 'expired'
                expired_count += 1
                logger.info(f"Command expired: id={command_id}")
    
    if expired_count > 0:
        logger.info(f"Cleaned up {expired_count} expired commands")


# ==================== 辅助函数 ====================

def simple_rule_based_recommendation(temperature, humidity):
    """
    基于简单规则的档位推荐（临时方案）
    
    TODO: 替换为 AI 决策树模型
    
    规则说明：
    - 温度 < 25℃: 1 档或关闭
    - 25℃ <= 温度 < 28℃: 2 档
    - 温度 >= 28℃: 3 档
    - 湿度 > 70%: 适当提高档位
    """
    if temperature < 22:
        return 0, "温度适宜，建议关闭风扇"
    elif temperature < 25:
        return 1, "温度略高，建议开启 1 档"
    elif temperature < 28:
        if humidity > 70:
            return 2, "温度适中但湿度较高，建议 2 档"
        return 2, "温度较高，建议开启 2 档"
    else:
        if humidity > 70:
            return 3, "温度高且湿度大，建议 3 档全速运行"
        return 3, "温度较高，建议开启 3 档"


def load_ai_model():
    """
    ⚠️ 预留函数：加载 AI 模型
    
    TODO: 实现决策树模型加载
    使用 scikit-learn 训练并保存模型后，在此处加载
    
    示例：
    import joblib
    model = joblib.load('models/decision_tree.pkl')
    """
    logger.warning("AI model not loaded yet. Using rule-based recommendation.")
    return None


# ==================== 健康检查 ====================

@app.route('/health', methods=['GET'])
def health_check():
    """健康检查接口"""
    return jsonify({
        'status': 'healthy',
        'service': 'SmartFan Backend',
        'version': '0.1.0'
    }), 200


@app.route('/')
def index():
    """提供前端页面"""
    return send_from_directory(app.static_folder, 'index.html')


# ==================== 主程序 ====================

if __name__ == '__main__':
    logger.info("Starting SmartFan 2.0 Backend Server...")
    logger.info("Server running on http://0.0.0.0:5000")
    logger.info("API documentation: http://localhost:5000/")

    # 开发模式：启用调试但禁用 reloader 以避免进程隔离
    # 禁用 reloader 后，debug 模式的调试功能仍然可用，但不会自动重载代码
    # 生产环境应使用 gunicorn 或其他 WSGI 服务器
    app.run(
        host='0.0.0.0',
        port=5000,
        debug=True,
        use_reloader=False  # 禁用 reloader 避免多进程导致队列隔离
    )
