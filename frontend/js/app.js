/**
 * SmartFan 2.0 Frontend - 主应用逻辑
 * ===================================
 * 
 * ⚠️ 注意：这是框架代码，需要后端服务正常运行才能使用。
 * 后端地址配置在 BACKEND_URL 变量中。
 * 
 * 作者：SmartFan Team
 * 版本：0.1.0
 * 日期：2026-11-11
 */

// ==================== 配置 ====================

// 后端 API 地址（使用相对路径，自动匹配当前域名和端口）
// 生产环境中，Flask 会提供静态文件和 API，因此使用相对路径
const BACKEND_URL = '';

// 数据刷新间隔（毫秒）
const REFRESH_INTERVAL = 2000;

// 图表最大数据点数
const MAX_DATA_POINTS = 100;

// ==================== 全局变量 ====================

let tempChart = null;
let humChart = null;
let refreshTimer = null;
let isConnected = false;

// 控制状态
let currentControlMode = 'auto';  // 'auto' 或 'manual'
let currentManualGear = 0;        // 0-5
let currentCommandId = null;      // 当前待查询的指令 ID
let commandStatusTimer = null;    // 指令状态轮询定时器

// ==================== 初始化 ====================

document.addEventListener('DOMContentLoaded', function() {
    console.log('SmartFan 2.0 Frontend initialized');

    // 初始化图表
    initCharts();

    // 初始化控制模式
    initControlMode();

    // 开始刷新数据
    startDataRefresh();

    // 开始指令状态轮询
    startCommandStatusRefresh();

    // 检查后端连接
    checkBackendConnection();
});

// ==================== 图表初始化 ====================

function initCharts() {
    // 温度图表
    const tempChartDiv = document.getElementById('tempChart');
    if (tempChartDiv) {
        tempChart = echarts.init(tempChartDiv);
        const tempOption = {
            title: {
                text: '温度变化',
                left: 10,
                top: 10,
                textStyle: {
                    fontSize: 14
                }
            },
            tooltip: {
                trigger: 'axis',
                formatter: '{b}<br/>{a}: {c}°C'
            },
            xAxis: {
                type: 'category',
                data: [],
                axisLabel: {
                    rotate: 45
                }
            },
            yAxis: {
                type: 'value',
                name: '温度 (°C)',
                min: 15,
                max: 40
            },
            series: [{
                name: '温度',
                type: 'line',
                smooth: true,
                data: [],
                itemStyle: {
                    color: '#f44336'
                },
                areaStyle: {
                    color: 'rgba(244, 67, 54, 0.1)'
                }
            }]
        };
        tempChart.setOption(tempOption);
    }
    
    // 湿度图表
    const humChartDiv = document.getElementById('humChart');
    if (humChartDiv) {
        humChart = echarts.init(humChartDiv);
        const humOption = {
            title: {
                text: '湿度变化',
                left: 10,
                top: 10,
                textStyle: {
                    fontSize: 14
                }
            },
            tooltip: {
                trigger: 'axis',
                formatter: '{b}<br/>{a}: {c}%'
            },
            xAxis: {
                type: 'category',
                data: [],
                axisLabel: {
                    rotate: 45
                }
            },
            yAxis: {
                type: 'value',
                name: '湿度 (%)',
                min: 0,
                max: 100
            },
            series: [{
                name: '湿度',
                type: 'line',
                smooth: true,
                data: [],
                itemStyle: {
                    color: '#2196F3'
                },
                areaStyle: {
                    color: 'rgba(33, 150, 243, 0.1)'
                }
            }]
        };
        humChart.setOption(humOption);
    }
    
    // 窗口大小改变时重绘图表
    window.addEventListener('resize', function() {
        if (tempChart) tempChart.resize();
        if (humChart) humChart.resize();
    });
}

// ==================== 数据刷新 ====================

function startDataRefresh() {
    // 立即执行一次
    refreshData();
    
    // 设置定时器
    refreshTimer = setInterval(refreshData, REFRESH_INTERVAL);
}

function stopDataRefresh() {
    if (refreshTimer) {
        clearInterval(refreshTimer);
        refreshTimer = null;
    }
}

async function refreshData() {
    try {
        // 获取系统状态
        const status = await fetchSystemStatus();
        updateSystemInfo(status);
        
        // 获取历史数据
        const history = await fetchSensorHistory();
        if (history && history.data && history.data.length > 0) {
            updateDashboard(history.data[history.data.length - 1]);
            updateCharts(history.data);
        }
        
        updateConnectionStatus(true);
    } catch (error) {
        console.error('Failed to refresh data:', error);
        updateConnectionStatus(false);
    }
}

// ==================== API 调用 ====================

async function fetchSystemStatus() {
    const response = await fetch(`${BACKEND_URL}/api/status`);
    if (!response.ok) {
        throw new Error('Failed to fetch system status');
    }
    return await response.json();
}

async function fetchSensorHistory(limit = 50) {
    const response = await fetch(`${BACKEND_URL}/api/history?limit=${limit}`);
    if (!response.ok) {
        throw new Error('Failed to fetch sensor history');
    }
    return await response.json();
}

async function fetchRecommendation(temperature, humidity) {
    const response = await fetch(
        `${BACKEND_URL}/api/fan/recommendation?temperature=${temperature}&humidity=${humidity}`
    );
    if (!response.ok) {
        throw new Error('Failed to fetch recommendation');
    }
    return await response.json();
}

async function controlFan(gear) {
    const response = await fetch(`${BACKEND_URL}/api/fan/control`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            action: 'set_gear',
            gear: gear
        })
    });
    if (!response.ok) {
        throw new Error('Failed to control fan');
    }
    return await response.json();
}

// ==================== 控制模式 API ====================

async function fetchControlMode() {
    const response = await fetch(`${BACKEND_URL}/api/fan/mode`);
    if (!response.ok) {
        throw new Error('Failed to fetch control mode');
    }
    return await response.json();
}

async function setControlMode(mode) {
    console.log(`[setControlMode] 点击切换到 ${mode} 模式`);
    try {
        console.log(`[setControlMode] 发送 POST 请求到 ${BACKEND_URL}/api/fan/mode`, { mode: mode });
        const response = await fetch(`${BACKEND_URL}/api/fan/mode`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ mode: mode })
        });
        
        console.log(`[setControlMode] 收到响应状态: ${response.status}`);
        
        if (!response.ok) {
            throw new Error('Failed to set control mode');
        }
        
        const result = await response.json();
        console.log(`[setControlMode] 响应数据:`, result);
        
        if (result.status === 'success') {
            currentControlMode = mode;
            updateModeUI(mode);
            showNotification(`控制模式已切换到 ${mode === 'auto' ? 'AUTO 自动' : 'MANUAL 手动'}`, 'success');

            // 注意：不在这里自动调用 setManualGear
            // 避免用户切换模式时重复发送指令
            // 用户需要手动点击档位按钮来设置风扇档位
        } else {
            throw new Error(result.message || '设置模式失败');
        }
    } catch (error) {
        console.error('[setControlMode] 错误:', error);
        showNotification(`设置控制模式失败：${error.message}`, 'error');
        throw error;
    }
}

async function setManualGear(gear) {
    console.log(`[setManualGear] 点击设置 ${gear}档, currentControlMode=${currentControlMode}`);
    
    if (gear < 0 || gear > 5) {
        showNotification('档位必须在 0-5 之间', 'error');
        return;
    }

    try {
        // 先确保在 MANUAL 模式
        if (currentControlMode !== 'manual') {
            console.log('[setManualGear] 不在 MANUAL 模式，自动切换到 MANUAL 模式');
            await setControlMode('manual');
        }

        console.log(`[setManualGear] 发送 POST 请求到 ${BACKEND_URL}/api/fan/command`, {
            mode: 'manual',
            gear: gear,
            esp32_id: 'smartfan001'
        });
        
        const response = await fetch(`${BACKEND_URL}/api/fan/command`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                mode: 'manual',
                gear: gear,
                esp32_id: 'smartfan001'  // 明确指定设备 ID，与 ESP32 查询时使用相同的 ID
            })
        });
        
        console.log(`[setManualGear] 收到响应状态: ${response.status}`);

        if (!response.ok) {
            throw new Error('Failed to set manual gear');
        }

        const result = await response.json();
        console.log(`[setManualGear] 响应数据:`, result);
        
        if (result.status === 'success') {
            currentManualGear = gear;
            // 保存后端返回的 command_id，用于后续状态查询
            currentCommandId = result.command_id;
            console.log(`[setManualGear] 保存 command_id: ${currentCommandId}`);
            updateManualGearUI(gear);
            showNotification(`手动档位已设置为 ${gear} 档`, 'success');
        } else {
            throw new Error(result.message || '设置档位失败');
        }
    } catch (error) {
        console.error('Failed to set manual gear:', error);
        showNotification(`设置档位失败：${error.message}`, 'error');
        throw error;
    }
}

async function fetchCommandStatus() {
    // 没有待查询的指令时，不发送请求
    if (!currentCommandId) {
        console.log('[fetchCommandStatus] 没有待查询的指令 ID，跳过');
        return null;
    }
    console.log(`[fetchCommandStatus] 查询 command_id: ${currentCommandId}`);
    const response = await fetch(`${BACKEND_URL}/api/fan/command/status?command_id=${currentCommandId}`);
    console.log(`[fetchCommandStatus] 收到响应状态: ${response.status}`);
    if (!response.ok) {
        throw new Error('Failed to fetch command status');
    }
    return await response.json();
}

async function refreshCommandStatus() {
    console.log('[refreshCommandStatus] 开始轮询指令状态');
    try {
        const status = await fetchCommandStatus();
        console.log('[refreshCommandStatus] 轮询结果:', status);
        // 没有待查询指令时，不更新 UI，也不显示错误
        if (!status) {
            return;
        }
        updateCommandStatusUI(status);
    } catch (error) {
        console.error('[refreshCommandStatus] 错误:', error);
        showNotification('刷新指令状态失败', 'error');
    }
}

// ==================== UI 更新 ====================

// 控制模式相关 UI 更新函数
function initControlMode() {
    // 从后端获取当前控制模式
    fetchControlMode()
        .then(response => {
            if (response.status === 'success') {
                currentControlMode = response.mode || 'auto';
                currentManualGear = response.manual_gear || 0;
                updateModeUI(currentControlMode);
                updateManualGearUI(currentManualGear);
                document.getElementById('currentMode').textContent = 
                    currentControlMode === 'auto' ? 'AUTO 自动' : 'MANUAL 手动';
            }
        })
        .catch(error => {
            console.error('Failed to init control mode:', error);
            // 默认使用 AUTO 模式
            currentControlMode = 'auto';
            updateModeUI(currentControlMode);
        });
}

function updateModeUI(mode) {
    const autoBtn = document.getElementById('autoModeBtn');
    const manualBtn = document.getElementById('manualModeBtn');
    const currentModeDisplay = document.getElementById('currentMode');
    
    if (mode === 'auto') {
        autoBtn.classList.add('active');
        manualBtn.classList.remove('active');
        currentModeDisplay.textContent = 'AUTO 自动';
    } else {
        autoBtn.classList.remove('active');
        manualBtn.classList.add('active');
        currentModeDisplay.textContent = 'MANUAL 手动';
    }
}

function updateManualGearUI(gear) {
    const manualGearStatus = document.getElementById('manualGearStatus');
    if (manualGearStatus) {
        manualGearStatus.textContent = `${gear} 档`;
    }
}

// 指令状态相关 UI 更新函数
function startCommandStatusRefresh() {
    // 立即执行一次
    refreshCommandStatus();
    
    // 设置定时器，每 3 秒刷新一次
    commandStatusTimer = setInterval(refreshCommandStatus, 3000);
}

function updateCommandStatusUI(status) {
    if (!status) return;

    // 后端返回的结构是 {status: 'success', command: {...}}
    // 需要从 command 对象中读取实际数据
    const command = status.command;
    
    const commandId = document.getElementById('currentCommandId');
    const commandMode = document.getElementById('commandMode');
    const commandGear = document.getElementById('commandGear');
    const commandStatus = document.getElementById('commandStatus');
    const commandAck = document.getElementById('commandAck');

    if (commandId) {
        commandId.textContent = command ? (command.command_id || '无') : '无';
    }

    if (commandMode) {
        commandMode.textContent = command && command.mode ? 
            (command.mode === 'auto' ? 'AUTO' : 'MANUAL') : '--';
    }

    if (commandGear) {
        commandGear.textContent = command && command.gear !== undefined ? command.gear : '--';
    }

    if (commandStatus) {
        const statusText = command ? (command.status || 'unknown') : 'unknown';
        let statusClass = 'unknown';

        switch (statusText) {
            case 'pending':
                statusClass = 'pending';
                break;
            case 'executing':
                statusClass = 'executing';
                break;
            case 'success':
                statusClass = 'success';
                break;
            case 'failed':
                statusClass = 'failed';
                break;
            case 'expired':
                statusClass = 'failed';
                break;
            case 'cancelled':
                statusClass = 'failed';
                break;
        }

        commandStatus.textContent = statusText;
        commandStatus.className = `command-value status-${statusClass}`;
        
        // 指令到达终态时，清除 currentCommandId，停止轮询
        if (command && ['success', 'failed', 'expired', 'cancelled'].includes(statusText)) {
            // 保留当前指令 ID 一段时间供用户查看，但不需要额外逻辑
        }
    }

    if (commandAck) {
        commandAck.textContent = command && command.acked !== undefined ?
            (command.acked ? '✓ 已确认' : '✗ 未确认') : '--';
    }
}

// 原有 UI 更新函数
function updateDashboard(sensorData) {
    console.log('[updateDashboard] Received sensor data:', sensorData);
    
    // 更新传感器值
    document.getElementById('temperatureValue').textContent =
        Number(sensorData.temperature).toFixed(1);

    document.getElementById('humidityValue').textContent =
        Number(sensorData.humidity).toFixed(1);

    const fanStatus = document.getElementById('fanStatus');
    const fanStatusDetail = document.getElementById('fanStatusDetail');
    const gearValue = document.getElementById('gearValue');

    // 根据后端返回的风扇档位判断风扇状态
    // fan_gear >= 0 表示风扇已连接且正在运行
    // fan_gear == -1 表示风扇已连接但关闭
    const actualGear = Number(sensorData.fan_gear);
    console.log('[updateDashboard] Parsed fan_gear:', actualGear);

    if (fanStatus && fanStatusDetail) {
        if (actualGear >= 0) {
            // 风扇正在运行
            fanStatus.textContent = '已连接';
            fanStatusDetail.textContent = `运行在 ${actualGear} 档`;
        } else if (actualGear === -1) {
            // 风扇已连接但关闭
            fanStatus.textContent = '已连接';
            fanStatusDetail.textContent = '风扇已关闭';
        } else {
            // 无效值，可能是未连接
            fanStatus.textContent = '未连接';
            fanStatusDetail.textContent = '风扇硬件尚未接入';
        }
    }

    // 显示实际档位（0-5 档有效，-1 表示关闭）
    if (Number.isInteger(actualGear) && actualGear >= -1 && actualGear <= 5) {
        if (actualGear === -1) {
            gearValue.textContent = '关闭';
        } else {
            gearValue.textContent = actualGear;
        }
    } else {
        gearValue.textContent = '未知';
    }

    // 获取推荐
    getRecommendation(sensorData.temperature, sensorData.humidity);
}
async function getRecommendation(temperature, humidity) {
    try {
        const recommendation = await fetchRecommendation(temperature, humidity);
        
        if (recommendation.status === 'success') {
            document.getElementById('recommendGear').textContent = 
                recommendation.gear === 0 ? '关闭' : recommendation.gear + '档';
            document.getElementById('recommendReason').textContent = 
                recommendation.reason;
            document.getElementById('recommendMethod').textContent = 
                `推荐方法：${recommendation.method === 'rule_based' ? '规则引擎（临时）' : 'AI 模型'}`;
        }
    } catch (error) {
        console.error('Failed to get recommendation:', error);
        document.getElementById('recommendReason').textContent = '获取推荐失败';
    }
}

function updateSystemInfo(statusData) {
    if (!statusData || !statusData.system) return;
    
    const system = statusData.system;
    
    // WiFi 信号
    const signalElement = document.getElementById('wifiSignal');
    if (system.wifi_signal && system.wifi_signal < 0) {
        signalElement.textContent = `${system.wifi_signal} dBm`;
    } else {
        signalElement.textContent = '--';
    }
    
    // 最后通信时间
    if (system.last_seen) {
        const lastSeen = new Date(system.last_seen);
        document.getElementById('lastSeen').textContent = 
            lastSeen.toLocaleTimeString('zh-CN');
    }
    
    // 数据点数
    document.getElementById('dataPoints').textContent = 
        statusData.data_points || 0;
    
    // 更新连接状态
    const statusDot = document.getElementById('statusDot');
    const statusText = document.getElementById('statusText');
    
    if (system.status === 'online') {
        statusDot.classList.add('online');
        statusDot.classList.remove('offline');
        statusText.textContent = '在线';
    } else {
        statusDot.classList.remove('online');
        statusDot.classList.add('offline');
        statusText.textContent = '离线';
    }
}

function updateConnectionStatus(connected) {
    isConnected = connected;
    
    const statusDot = document.getElementById('statusDot');
    const statusText = document.getElementById('statusText');
    
    if (connected) {
        statusDot.classList.add('online');
        statusDot.classList.remove('offline');
        statusText.textContent = '在线';
    } else {
        statusDot.classList.remove('online');
        statusDot.classList.add('offline');
        statusText.textContent = '连接后端失败';
    }
}

function updateCharts(historyData) {
    if (!tempChart || !humChart) return;
    
    // 提取时间和数据
    const timestamps = historyData.map(d => {
        const date = new Date(d.timestamp);
        return date.toLocaleTimeString('zh-CN', { 
            hour: '2-digit', 
            minute: '2-digit', 
            second: '2-digit' 
        });
    });
    
    const temperatures = historyData.map(d => d.temperature);
    const humidities = historyData.map(d => d.humidity);
    
    // 更新温度图表
    tempChart.setOption({
        xAxis: {
            data: timestamps
        },
        series: [{
            data: temperatures
        }]
    });
    
    // 更新湿度图表
    humChart.setOption({
        xAxis: {
            data: timestamps
        },
        series: [{
            data: humidities
        }]
    });
}

// ==================== 风扇控制 ====================

function setFanGear(gear) {
    const controlStatus = document.getElementById('controlStatus');

    if (controlStatus) {
        controlStatus.textContent = '暂不可控制：风扇硬件尚未接入';
    }

    showNotification(
        '控制未执行：风扇硬件尚未接入，当前仅支持温湿度监测',
        'info'
    );

    console.warn(
        `Fan control request rejected: gear=${gear}, hardware not connected.`
    );
}

// ==================== 通知 ====================

function showNotification(message, type = 'info') {
    // 创建通知元素
    const notification = document.createElement('div');
    notification.className = `${type}-message`;
    notification.textContent = message;
    
    // 添加到页面
    document.body.appendChild(notification);
    
    // 3 秒后移除
    setTimeout(() => {
        notification.remove();
    }, 3000);
}

// ==================== 清理 ====================

// 页面卸载时清理定时器
window.addEventListener('beforeunload', function() {
    stopDataRefresh();
    if (tempChart) tempChart.dispose();
    if (humChart) humChart.dispose();
});
