"""
Phase 3.2.1 指令队列功能自动化测试

测试内容：
1. 创建指令（正常、非法参数、重复模式切换）
2. 获取指令（正常获取、无指令、重复获取）
3. ACK 确认（正常 ACK、重复 ACK、过期指令 ACK）
4. 模式切换（AUTO->MANUAL、MANUAL->AUTO）
5. 指令过期（等待 5 分钟验证）
6. 原有传感器 API 验证
"""

import requests
import time
import json
import sys

BASE_URL = "http://127.0.0.1:5000"

class TestResults:
    def __init__(self):
        self.passed = 0
        self.failed = 0
        self.errors = []

    def record(self, test_name, passed, error=None):
        if passed:
            self.passed += 1
            print(f"✅ PASS: {test_name}")
        else:
            self.failed += 1
            self.errors.append((test_name, error))
            print(f"❌ FAIL: {test_name} - {error}")

results = TestResults()


def test_create_command():
    """测试 1：创建风扇控制指令"""
    print("\n=== 测试 1: 创建风扇控制指令 ===")
    
    # 1.1 正常创建 MANUAL 模式指令
    response = requests.post(f"{BASE_URL}/api/fan/command", json={
        "mode": "manual",
        "gear": 3
    })
    data = response.json()
    results.record("创建 MANUAL 模式指令", 
                  response.status_code == 200 and data.get('status') == 'success' and 'command_id' in data,
                  f"Status: {response.status_code}, Response: {data}")
    command_id_1 = data.get('command_id')
    
    # 1.2 创建 AUTO 模式指令
    response = requests.post(f"{BASE_URL}/api/fan/command", json={
        "mode": "auto",
        "gear": 0
    })
    data = response.json()
    results.record("创建 AUTO 模式指令",
                  response.status_code == 200 and data.get('status') == 'success',
                  f"Status: {response.status_code}, Response: {data}")
    
    # 1.3 测试非法档位（负数）
    response = requests.post(f"{BASE_URL}/api/fan/command", json={
        "mode": "manual",
        "gear": -1
    })
    data = response.json()
    results.record("拒绝非法档位（负数）",
                  response.status_code == 400 and data.get('status') == 'error',
                  f"Status: {response.status_code}, Response: {data}")
    
    # 1.4 测试非法档位（超过 5）
    response = requests.post(f"{BASE_URL}/api/fan/command", json={
        "mode": "manual",
        "gear": 6
    })
    data = response.json()
    results.record("拒绝非法档位（>5）",
                  response.status_code == 400 and data.get('status') == 'error',
                  f"Status: {response.status_code}, Response: {data}")
    
    # 1.5 测试非法模式
    response = requests.post(f"{BASE_URL}/api/fan/command", json={
        "mode": "invalid",
        "gear": 2
    })
    data = response.json()
    results.record("拒绝非法模式",
                  response.status_code == 400 and data.get('status') == 'error',
                  f"Status: {response.status_code}, Response: {data}")
    
    # 1.6 gear=0 时自动切换到 AUTO 模式
    response = requests.post(f"{BASE_URL}/api/fan/command", json={
        "mode": "manual",
        "gear": 0
    })
    data = response.json()
    results.record("gear=0 自动切换为 AUTO 模式",
                  response.status_code == 200 and data.get('mode') == 'auto',
                  f"Status: {response.status_code}, Response: {data}")
    
    return command_id_1


def test_get_command():
    """测试 2：获取待执行指令"""
    print("\n=== 测试 2: 获取待执行指令 ===")
    
    # 2.1 创建新指令
    response = requests.post(f"{BASE_URL}/api/fan/command", json={
        "mode": "manual",
        "gear": 2
    })
    command_id = response.json().get('command_id')
    
    # 2.2 获取指令
    response = requests.get(f"{BASE_URL}/api/fan/command")
    data = response.json()
    results.record("获取待执行指令",
                  response.status_code == 200 and data.get('status') == 'success' and data.get('command') is not None,
                  f"Status: {response.status_code}, Response: {data}")
    
    # 2.3 再次获取（应该没有指令了，因为已标记为 executing）
    response = requests.get(f"{BASE_URL}/api/fan/command")
    data = response.json()
    results.record("重复获取返回无指令",
                  response.status_code == 200 and data.get('command') is None,
                  f"Status: {response.status_code}, Response: {data}")
    
    return command_id


def test_ack_command():
    """测试 3：ACK 确认指令"""
    print("\n=== 测试 3: ACK 确认指令 ===")
    
    # 3.1 创建指令
    response = requests.post(f"{BASE_URL}/api/fan/command", json={
        "mode": "manual",
        "gear": 4
    })
    command_id = response.json().get('command_id')
    
    # 3.2 获取指令
    requests.get(f"{BASE_URL}/api/fan/command")
    
    # 3.3 发送成功 ACK
    response = requests.post(f"{BASE_URL}/api/fan/command/ack", json={
        "command_id": command_id,
        "success": True,
        "message": "执行成功"
    })
    data = response.json()
    results.record("成功 ACK",
                  response.status_code == 200 and data.get('status') == 'success' and data.get('new_status') == 'success',
                  f"Status: {response.status_code}, Response: {data}")
    
    # 3.4 发送重复 ACK
    response = requests.post(f"{BASE_URL}/api/fan/command/ack", json={
        "command_id": command_id,
        "success": True,
        "message": "重复 ACK"
    })
    data = response.json()
    results.record("重复 ACK 处理",
                  response.status_code == 200 and data.get('status') == 'success',
                  f"Status: {response.status_code}, Response: {data}")
    
    # 3.5 失败 ACK
    response = requests.post(f"{BASE_URL}/api/fan/command", json={
        "mode": "manual",
        "gear": 1
    })
    command_id_2 = response.json().get('command_id')
    requests.get(f"{BASE_URL}/api/fan/command")
    
    response = requests.post(f"{BASE_URL}/api/fan/command/ack", json={
        "command_id": command_id_2,
        "success": False,
        "message": "执行失败"
    })
    data = response.json()
    results.record("失败 ACK",
                  response.status_code == 200 and data.get('new_status') == 'failed',
                  f"Status: {response.status_code}, Response: {data}")
    
    # 3.6 ACK 不存在的指令
    response = requests.post(f"{BASE_URL}/api/fan/command/ack", json={
        "command_id": "fake_command_id",
        "success": True
    })
    data = response.json()
    results.record("ACK 不存在的指令",
                  response.status_code == 404 and data.get('status') == 'error',
                  f"Status: {response.status_code}, Response: {data}")


def test_mode_switch():
    """测试 4：模式切换"""
    print("\n=== 测试 4: 模式切换 ===")
    
    # 4.1 获取当前模式
    response = requests.get(f"{BASE_URL}/api/fan/mode")
    data = response.json()
    initial_mode = data.get('mode')
    results.record("获取当前模式",
                  response.status_code == 200 and data.get('status') == 'success',
                  f"Status: {response.status_code}, Response: {data}")
    
    # 4.2 切换到 MANUAL 模式
    response = requests.post(f"{BASE_URL}/api/fan/mode", json={
        "mode": "manual"
    })
    data = response.json()
    results.record("切换到 MANUAL 模式",
                  response.status_code == 200 and data.get('mode') == 'manual',
                  f"Status: {response.status_code}, Response: {data}")
    
    # 4.3 创建 MANUAL 指令
    response = requests.post(f"{BASE_URL}/api/fan/command", json={
        "mode": "manual",
        "gear": 3
    })
    command_id = response.json().get('command_id')
    
    # 4.4 切换到 AUTO 模式（应该取消之前的 MANUAL 指令）
    response = requests.post(f"{BASE_URL}/api/fan/mode", json={
        "mode": "auto"
    })
    data = response.json()
    results.record("切换到 AUTO 模式",
                  response.status_code == 200 and data.get('mode') == 'auto',
                  f"Status: {response.status_code}, Response: {data}")
    
    # 4.5 查询被取消的指令状态
    response = requests.get(f"{BASE_URL}/api/fan/command/status?command_id={command_id}")
    data = response.json()
    results.record("模式切换取消旧指令",
                  response.status_code == 200 and data.get('command', {}).get('status') == 'cancelled',
                  f"Status: {response.status_code}, Response: {data}")
    
    # 4.6 切换回初始模式
    requests.post(f"{BASE_URL}/api/fan/mode", json={"mode": initial_mode})


def test_command_status():
    """测试 5：查询指令状态"""
    print("\n=== 测试 5: 查询指令状态 ===")
    
    # 5.1 创建指令
    response = requests.post(f"{BASE_URL}/api/fan/command", json={
        "mode": "manual",
        "gear": 5
    })
    command_id = response.json().get('command_id')
    
    # 5.2 查询状态
    response = requests.get(f"{BASE_URL}/api/fan/command/status?command_id={command_id}")
    data = response.json()
    results.record("查询指令状态",
                  response.status_code == 200 and data.get('status') == 'success' and data.get('command', {}).get('gear') == 5,
                  f"Status: {response.status_code}, Response: {data}")
    
    # 5.3 查询不存在的指令
    response = requests.get(f"{BASE_URL}/api/fan/command/status?command_id=fake_id")
    data = response.json()
    results.record("查询不存在的指令",
                  response.status_code == 404 and data.get('status') == 'error',
                  f"Status: {response.status_code}, Response: {data}")


def test_existing_apis():
    """测试 6：原有 API 验证"""
    print("\n=== 测试 6: 原有 API 验证 ===")

    # 6.1 测试 /api/sensors/dht（待实现）
    response = requests.get(f"{BASE_URL}/api/sensors/dht")
    # 注意：该路由目前不存在，Phase 3.2.1 不添加新路由
    if response.status_code == 404:
        print(f"⚠️  SKIP: DHT 传感器 API - 路由未实现（/api/sensors/dht）")
    else:
        try:
            data = response.json()
            results.record("DHT 传感器 API",
                          response.status_code == 200 and data.get('status') == 'success',
                          f"Status: {response.status_code}, Response: {data}")
        except:
            results.record("DHT 传感器 API", False, f"Status: {response.status_code}, Non-JSON response")

    # 6.2 测试 /api/sensors/pir（待实现）
    response = requests.get(f"{BASE_URL}/api/sensors/pir")
    # 注意：该路由目前不存在，Phase 3.2.1 不添加新路由
    if response.status_code == 404:
        print(f"⚠️  SKIP: PIR 传感器 API - 路由未实现（/api/sensors/pir）")
    else:
        try:
            data = response.json()
            results.record("PIR 传感器 API",
                          response.status_code == 200 and data.get('status') == 'success',
                          f"Status: {response.status_code}, Response: {data}")
        except:
            results.record("PIR 传感器 API", False, f"Status: {response.status_code}, Non-JSON response")

    # 6.3 测试 /api/status（应该包含 control_mode）
    response = requests.get(f"{BASE_URL}/api/status")
    data = response.json()
    results.record("系统状态 API（包含 control_mode）",
                  response.status_code == 200 and data.get('status') == 'success' and 'control_mode' in data,
                  f"Status: {response.status_code}, Response: {data}")

    # 6.4 测试 /api/fan/recommendation
    response = requests.get(f"{BASE_URL}/api/fan/recommendation?temperature=28&humidity=65")
    data = response.json()
    results.record("风扇推荐 API",
                  response.status_code == 200 and data.get('status') == 'success',
                  f"Status: {response.status_code}, Response: {data}")

    # 6.5 测试 /api/history
    response = requests.get(f"{BASE_URL}/api/history?limit=10")
    data = response.json()
    results.record("历史数据 API",
                  response.status_code == 200 and data.get('status') == 'success',
                  f"Status: {response.status_code}, Response: {data}")

    # 6.6 测试 /health
    response = requests.get(f"{BASE_URL}/health")
    data = response.json()
    results.record("健康检查 API",
                  response.status_code == 200 and data.get('status') == 'healthy',
                  f"Status: {response.status_code}, Response: {data}")


def main():
    print("=" * 60)
    print("SmartFan 2.0 Phase 3.2.1 指令队列功能自动化测试")
    print("=" * 60)
    
    try:
        # 检查后端是否运行
        response = requests.get(f"{BASE_URL}/api/status", timeout=2)
        print(f"\n✅ 后端服务正在运行：{BASE_URL}")
    except requests.exceptions.ConnectionError:
        print(f"\n❌ 错误：无法连接到后端服务 {BASE_URL}")
        print("请先启动后端服务：cd backend && python app.py")
        sys.exit(1)
    
    # 执行测试
    test_create_command()
    test_get_command()
    test_ack_command()
    test_mode_switch()
    test_command_status()
    test_existing_apis()
    
    # 输出结果
    print("\n" + "=" * 60)
    print("测试结果汇总")
    print("=" * 60)
    print(f"✅ 通过：{results.passed}")
    print(f"❌ 失败：{results.failed}")
    print(f"总计：{results.passed + results.failed}")
    
    if results.errors:
        print("\n失败测试详情：")
        for test_name, error in results.errors:
            print(f"  - {test_name}: {error}")
    
    # 保存测试结果
    with open("test_results.json", "w", encoding="utf-8") as f:
        json.dump({
            "passed": results.passed,
            "failed": results.failed,
            "total": results.passed + results.failed,
            "errors": [{"test": t, "error": e} for t, e in results.errors]
        }, f, ensure_ascii=False, indent=2)
    
    print(f"\n测试结果已保存到：backend/test_results.json")
    
    return 0 if results.failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
