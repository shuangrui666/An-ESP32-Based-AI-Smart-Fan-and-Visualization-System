// SmartFan 2.0 前端诊断脚本
// 在浏览器 Console 中粘贴并运行此脚本

console.log('='.repeat(60));
console.log('SmartFan 2.0 前端诊断工具');
console.log('='.repeat(60));

// 1. 检查全局变量
console.log('\n【1】检查全局变量');
console.log('BACKEND_URL:', typeof BACKEND_URL !== 'undefined' ? BACKEND_URL : '❌ 未定义');
console.log('currentControlMode:', typeof currentControlMode !== 'undefined' ? currentControlMode : '❌ 未定义');
console.log('currentManualGear:', typeof currentManualGear !== 'undefined' ? currentManualGear : '❌ 未定义');
console.log('currentCommandId:', typeof currentCommandId !== 'undefined' ? currentCommandId : '❌ 未定义');

// 2. 检查函数是否存在
console.log('\n【2】检查函数是否存在');
console.log('setControlMode:', typeof setControlMode);
console.log('setManualGear:', typeof setManualGear);
console.log('fetchCommandStatus:', typeof fetchCommandStatus);
console.log('startCommandStatusRefresh:', typeof startCommandStatusRefresh);

// 3. 测试后端连接
console.log('\n【3】测试后端连接');
async function testBackendConnection() {
    try {
        const response = await fetch('/api/status');
        console.log('GET /api/status 状态:', response.status);
        const data = await response.json();
        console.log('响应数据:', data);
    } catch (error) {
        console.error('❌ 后端连接失败:', error);
    }
}
testBackendConnection();

// 4. 测试 MANUAL 模式切换
console.log('\n【4】测试 MANUAL 模式切换（不会真正执行，仅检查函数）');
async function testSetControlMode() {
    if (typeof setControlMode !== 'function') {
        console.log('❌ setControlMode 函数不存在');
        return;
    }
    
    console.log('准备测试 setControlMode("manual")...');
    console.log('提示：点击页面上的 MANUAL 按钮，观察 Console 日志');
}
testSetControlMode();

// 5. 测试档位控制
console.log('\n【5】测试档位控制（不会真正执行，仅检查函数）');
async function testSetManualGear() {
    if (typeof setManualGear !== 'function') {
        console.log('❌ setManualGear 函数不存在');
        return;
    }
    
    console.log('准备测试 setManualGear(1)...');
    console.log('提示：点击页面上的 "1" 档位按钮，观察 Console 日志');
}
testSetManualGear();

// 6. 手动发送控制请求
console.log('\n【6】手动发送控制请求（测试用）');
async function testManualCommand() {
    try {
        console.log('发送测试指令...');
        const response = await fetch('/api/fan/mode', {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({mode: 'manual', esp32_id: 'smartfan001'})
        });
        console.log('POST /api/fan/mode 状态:', response.status);
        const result = await response.json();
        console.log('响应数据:', result);
    } catch (error) {
        console.error('❌ 发送指令失败:', error);
    }
}

// 7. 检查按钮事件绑定
console.log('\n【7】检查按钮事件绑定');
function checkButtonEvents() {
    const buttons = document.querySelectorAll('button');
    console.log(`找到 ${buttons.length} 个按钮`);
    
    buttons.forEach((btn, index) => {
        const text = btn.textContent.trim();
        const onclick = btn.getAttribute('onclick');
        if (text.includes('MANUAL') || text.includes('AUTO') || /^[0-5]$/.test(text)) {
            console.log(`按钮 [${index}]: "${text}"`);
            console.log(`  onclick 属性：${onclick || '❌ 无 onclick 属性'}`);
            console.log(`  onmousedown: ${btn.onmousedown ? '✓ 已绑定' : '❌ 未绑定'}`);
        }
    });
}
checkButtonEvents();

// 8. 检查页面 DOM 结构
console.log('\n【8】检查页面 DOM 结构');
function checkDOM() {
    const elements = {
        '模式切换区域': document.getElementById('modeControls'),
        '档位控制区域': document.getElementById('manualControls'),
        '传感器数据显示': document.getElementById('sensorData'),
        '指令状态显示': document.getElementById('commandStatus')
    };
    
    for (const [name, element] of Object.entries(elements)) {
        console.log(`${name}: ${element ? '✓ 存在' : '❌ 不存在'}`);
    }
}
checkDOM();

console.log('\n' + '='.repeat(60));
console.log('诊断完成！');
console.log('='.repeat(60));
console.log('\n下一步操作:');
console.log('1. 查看上面的诊断结果');
console.log('2. 点击页面上的 MANUAL 按钮');
console.log('3. 点击页面上的 "1" 档位按钮');
console.log('4. 观察 Console 中是否有相应的日志输出');
console.log('5. 如果没有任何日志，说明 JavaScript 函数未正确加载或执行');
console.log('\n提示：按 F12 打开开发者工具，切换到 Network 标签页');
console.log('     查看是否有 POST /api/fan/mode 或 POST /api/fan/command 请求');
