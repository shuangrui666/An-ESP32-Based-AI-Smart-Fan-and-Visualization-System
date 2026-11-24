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

// 后端 API 地址（根据实际情况修改）
const BACKEND_URL = 'http://localhost:5000';

// 数据刷新间隔（毫秒）
const REFRESH_INTERVAL = 2000;

// 图表最大数据点数
const MAX_DATA_POINTS = 100;

// ==================== 全局变量 ====================

let tempChart = null;
let humChart = null;
let refreshTimer = null;
let isConnected = false;

// ==================== 初始化 ====================

document.addEventListener('DOMContentLoaded', function() {
    console.log('SmartFan 2.0 Frontend initialized');
    
    // 初始化图表
    initCharts();
    
    // 开始刷新数据
    startDataRefresh();
    
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

// ==================== UI 更新 ====================

function updateDashboard(sensorData) {
    // 更新传感器值
    document.getElementById('temperatureValue').textContent = 
        sensorData.temperature.toFixed(1);
    document.getElementById('humidityValue').textContent = 
        sensorData.humidity.toFixed(1);
    document.getElementById('gearValue').textContent = sensorData.fan_gear;
    
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

async function setFanGear(gear) {
    try {
        const result = await controlFan(gear);
        
        if (result.status === 'success') {
            document.getElementById('controlStatus').textContent = 
                `已设置到${gear === 0 ? '关闭' : gear + '档'}`;
            
            // 更新当前档位显示
            document.getElementById('gearValue').textContent = gear;
            
            // 显示成功提示
            showNotification('风扇控制成功', 'success');
        } else {
            showNotification('控制失败：' + result.message, 'error');
        }
    } catch (error) {
        console.error('Failed to control fan:', error);
        showNotification('控制失败：' + error.message, 'error');
    }
}

// ==================== 连接检查 ====================

async function checkBackendConnection() {
    try {
        const response = await fetch(`${BACKEND_URL}/health`);
        if (response.ok) {
            console.log('Backend connection successful');
            updateConnectionStatus(true);
        } else {
            throw new Error('Backend health check failed');
        }
    } catch (error) {
        console.error('Backend connection failed:', error);
        updateConnectionStatus(false);
        
        // 显示错误提示
        showNotification('无法连接到后端服务，请确保后端正在运行', 'error');
    }
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
