#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
SmartFan 2.0 前端控制测试脚本
模拟浏览器发送控制请求，验证后端是否正确接收
"""

import requests
import json
import time

BASE_URL = "http://localhost:5000"

def test_get_status():
    """测试获取传感器状态"""
    print("\n【测试 1】GET /api/status")
    try:
        response = requests.get(f"{BASE_URL}/api/status", timeout=5)
        print(f"  状态码：{response.status_code}")
        print(f"  响应：{json.dumps(response.json(), indent=2, ensure_ascii=False)}")
        return True
    except Exception as e:
        print(f"  ❌ 错误：{e}")
        return False

def test_create_command():
    """测试创建控制指令"""
    print("\n【测试 2】POST /api/fan/command (创建 MANUAL 1 档指令)")
    try:
        payload = {
            "mode": "manual",
            "gear": 1,
            "esp32_id": "smartfan001"  # 必须指定设备 ID，与 ESP32 查询时使用相同的 ID
        }
        response = requests.post(
            f"{BASE_URL}/api/fan/command",
            json=payload,
            timeout=5
        )
        print(f"  状态码：{response.status_code}")
        print(f"  响应：{json.dumps(response.json(), indent=2, ensure_ascii=False)}")
        return response.json().get("command_id")
    except Exception as e:
        print(f"  ❌ 错误：{e}")
        return None

def test_get_command_status(command_id):
    """测试查询指令状态"""
    print(f"\n【测试 3】GET /api/fan/command/status?command_id={command_id}")
    try:
        response = requests.get(
            f"{BASE_URL}/api/fan/command/status?command_id={command_id}",
            timeout=5
        )
        print(f"  状态码：{response.status_code}")
        print(f"  响应：{json.dumps(response.json(), indent=2, ensure_ascii=False)}")
        return response.json()
    except Exception as e:
        print(f"  ❌ 错误：{e}")
        return None

def test_set_mode():
    """测试设置控制模式"""
    print("\n【测试 4】POST /api/fan/mode (设置 MANUAL 模式)")
    try:
        payload = {
            "mode": "manual",
            "esp32_id": "smartfan001"
        }
        response = requests.post(
            f"{BASE_URL}/api/fan/mode",
            json=payload,
            timeout=5
        )
        print(f"  状态码：{response.status_code}")
        print(f"  响应：{json.dumps(response.json(), indent=2, ensure_ascii=False)}")
        return response.status_code == 200
    except Exception as e:
        print(f"  ❌ 错误：{e}")
        return False

def test_get_pending_command():
    """测试 ESP32 获取待执行指令"""
    print("\n【测试 5】GET /api/fan/command?esp32_id=smartfan001 (ESP32 轮询)")
    try:
        response = requests.get(
            f"{BASE_URL}/api/fan/command?esp32_id=smartfan001",
            timeout=5
        )
        print(f"  状态码：{response.status_code}")
        data = response.json()
        print(f"  响应：{json.dumps(data, indent=2, ensure_ascii=False)}")
        return data.get("commands", [])
    except Exception as e:
        print(f"  ❌ 错误：{e}")
        return []

def test_ack_command(command_id):
    """测试 ESP32 确认指令"""
    print(f"\n【测试 6】POST /api/fan/command/ack (确认指令 {command_id})")
    try:
        payload = {
            "command_id": command_id,
            "success": True,
            "message": "ESP32 执行成功"
        }
        response = requests.post(
            f"{BASE_URL}/api/fan/command/ack",
            json=payload,
            timeout=5
        )
        print(f"  状态码：{response.status_code}")
        print(f"  响应：{json.dumps(response.json(), indent=2, ensure_ascii=False)}")
        return response.status_code == 200
    except Exception as e:
        print(f"  ❌ 错误：{e}")
        return False

def main():
    print("=" * 60)
    print("SmartFan 2.0 后端控制 API 测试")
    print("=" * 60)
    
    # 测试 1: 检查后端服务是否运行
    if not test_get_status():
        print("\n❌ 后端服务未运行，请先启动 Flask 后端")
        return
    
    # 测试 2: 创建控制指令
    command_id = test_create_command()
    if not command_id:
        print("\n❌ 创建指令失败")
        return
    
    # 测试 3: 查询指令状态
    test_get_command_status(command_id)
    
    # 测试 4: 设置模式
    test_set_mode()
    
    # 测试 5: ESP32 获取待执行指令
    commands = test_get_pending_command()
    
    # 测试 6: 确认指令（如果有待执行指令）
    if commands:
        test_ack_command(commands[0]["command_id"])
        # 再次查询状态
        time.sleep(0.5)
        test_get_command_status(commands[0]["command_id"])
    
    print("\n" + "=" * 60)
    print("测试完成！")
    print("=" * 60)
    print("\n请检查后端 Flask 日志，确认是否收到了上述所有请求")

if __name__ == "__main__":
    main()
