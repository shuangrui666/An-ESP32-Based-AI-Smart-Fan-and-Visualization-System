/**
 * MOSFET 独立测试程序
 * ====================
 * 
 * 功能：
 * - 串口命令控制：ON / OFF / STATUS
 * - LEDC PWM 初始化（GPIO6, 5kHz, 8-bit）
 * - 最低档位测试（20% 占空比 = 51）
 * - 5 秒超时自动关闭保护
 * - 初始化失败禁止启动
 * 
 * 安全设计：
 * - 启动时默认关闭风扇
 * - 初始化失败时禁止任何启动操作
 * - 5 秒自动超时保护（防止忘记关闭）
 * 
 * 使用方法：
 * 1. 编译上传：pio run -e mosfet-standalone-test -t upload
 * 2. 打开串口监控：pio device monitor
 * 3. 发送命令：ON / OFF / STATUS
 * 
 * 作者：SmartFan Team
 * 版本：1.0.0
 * 日期：2026-11-22
 */

#include <Arduino.h>

// 引脚配置（通过 platformio.ini 的 build_flags 定义）
#ifndef FAN_CONTROL_PIN
    #define FAN_CONTROL_PIN 6
#endif

// PWM 配置
#define PWM_FREQ 5000           // 5kHz 避免可听噪声
#define PWM_RESOLUTION 8        // 8-bit 分辨率 (0-255)
#define PWM_MIN_DUTY 51         // 最低档位 20% = 255 * 0.2 = 51

// 超时保护配置
#define AUTO_SHUTDOWN_TIMEOUT_MS 5000  // 5 秒自动关闭

// 全局状态
static bool g_initialized = false;
static bool g_fanOn = false;
static unsigned long g_lastActivityTime = 0;
static bool g_timeoutProtectionEnabled = true;

/**
 * 初始化 LEDC PWM
 * 返回 true 表示成功，false 表示失败
 */
bool initPWM(int pin) {
    Serial.println("[MOSFET-Test] Initializing LEDC PWM...");
    Serial.printf("[MOSFET-Test] Pin: GPIO%d, Freq: %dHz, Resolution: %d-bit\n",
                  pin, PWM_FREQ, PWM_RESOLUTION);
    
    // 注意：pinMode 和 digitalWrite 已在 setup() 中执行，此处不再重复
    
    // 初始化 LEDC PWM
    if (!ledcAttach(pin, PWM_FREQ, PWM_RESOLUTION)) {
        Serial.println("[MOSFET-Test] ERROR: ledcAttach() failed!");
        Serial.println("[MOSFET-Test] Cannot attach PWM. MOSFET will NOT start.");
        return false;
    }
    
    // 初始输出为 0（关闭）
    if (!ledcWrite(pin, 0)) {
        Serial.println("[MOSFET-Test] ERROR: ledcWrite() failed!");
        Serial.println("[MOSFET-Test] Cannot write initial duty. MOSFET will NOT start.");
        return false;
    }
    
    g_initialized = true;
    Serial.println("[MOSFET-Test] LEDC PWM initialized successfully");
    Serial.println("[MOSFET-Test] MOSFET is OFF by default");
    Serial.println("[MOSFET-Test] Send 'ON' to start at minimum duty (20%)");
    Serial.println("[MOSFET-Test] Send 'OFF' to stop immediately");
    Serial.println("[MOSFET-Test] Send 'STATUS' to check current state");
    Serial.println("[MOSFET-Test] Auto-shutdown after 5 seconds if no activity");
    Serial.println("");
    
    return true;
}

/**
 * 打开风扇（最低档位）
 */
void turnOnFan() {
    if (!g_initialized) {
        Serial.println("[MOSFET-Test] ERROR: PWM not initialized. Cannot turn on.");
        return;
    }
    
    ledcWrite(FAN_CONTROL_PIN, PWM_MIN_DUTY);
    g_fanOn = true;
    g_lastActivityTime = millis();
    
    Serial.println("[MOSFET-Test] Fan ON at minimum duty (20% = 51)");
    Serial.printf("[MOSFET-Test] Auto-shutdown in %d seconds\n", 
                  AUTO_SHUTDOWN_TIMEOUT_MS / 1000);
}

/**
 * 关闭风扇
 */
void turnOffFan() {
    if (!g_initialized) {
        Serial.println("[MOSFET-Test] ERROR: PWM not initialized.");
        return;
    }
    
    ledcWrite(FAN_CONTROL_PIN, 0);
    g_fanOn = false;
    g_lastActivityTime = millis();
    
    Serial.println("[MOSFET-Test] Fan OFF");
}

/**
 * 显示当前状态
 */
void showStatus() {
    Serial.println("[MOSFET-Test] === STATUS ===");
    Serial.printf("[MOSFET-Test] Initialized: %s\n", g_initialized ? "YES" : "NO");
    Serial.printf("[MOSFET-Test] Fan State: %s\n", g_fanOn ? "ON" : "OFF");
    Serial.printf("[MOSFET-Test] Control Pin: GPIO%d\n", FAN_CONTROL_PIN);
    Serial.printf("[MOSFET-Test] PWM Freq: %dHz\n", PWM_FREQ);
    Serial.printf("[MOSFET-Test] PWM Resolution: %d-bit\n", PWM_RESOLUTION);
    
    if (g_fanOn && g_timeoutProtectionEnabled) {
        unsigned long timeSinceLastActivity = millis() - g_lastActivityTime;
        unsigned long remainingTime = AUTO_SHUTDOWN_TIMEOUT_MS - timeSinceLastActivity;
        if (remainingTime > 0) {
            Serial.printf("[MOSFET-Test] Auto-shutdown in %lu seconds\n", 
                          remainingTime / 1000);
        } else {
            Serial.println("[MOSFET-Test] Timeout expired, will shutdown soon");
        }
    }
    Serial.println("");
}

/**
 * 检查超时保护
 */
void checkTimeoutProtection() {
    if (!g_initialized || !g_fanOn || !g_timeoutProtectionEnabled) {
        return;
    }
    
    unsigned long timeSinceLastActivity = millis() - g_lastActivityTime;
    
    if (timeSinceLastActivity >= AUTO_SHUTDOWN_TIMEOUT_MS) {
        Serial.println("[MOSFET-Test] TIMEOUT PROTECTION TRIGGERED");
        Serial.println("[MOSFET-Test] Auto-shutting down fan");
        turnOffFan();
    }
}

/**
 * 解析串口命令
 */
void processSerialCommand() {
    if (!Serial.available()) {
        return;
    }
    
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toUpperCase();
    
    Serial.printf("[MOSFET-Test] Received command: %s\n", command.c_str());
    
    if (command == "ON") {
        turnOnFan();
    } else if (command == "OFF") {
        turnOffFan();
    } else if (command == "STATUS") {
        showStatus();
    } else {
        Serial.println("[MOSFET-Test] Unknown command. Available commands: ON, OFF, STATUS");
        showStatus();
    }
}

/**
 * Arduino setup
 */
void setup() {
    // 1. 首先将 GPIO6 配置为输出并设置为 LOW（最优先，确保安全）
    pinMode(FAN_CONTROL_PIN, OUTPUT);
    digitalWrite(FAN_CONTROL_PIN, LOW);

    // 2. 初始化串口
    Serial.begin(115200);
    delay(500);  // 给串口驱动初始化时间

    // 3. 立即输出调试日志，验证串口工作
    Serial.println("[DEBUG] Serial.begin() called");

    Serial.println("");
    Serial.println("[MOSFET-Test] ==================================");
    Serial.println("[MOSFET-Test] MOSFET Standalone Test Program");
    Serial.println("[MOSFET-Test] ==================================");
    Serial.printf("[MOSFET-Test] Target Pin: GPIO%d\n", FAN_CONTROL_PIN);
    Serial.println("");
    
    // 4. 初始化 PWM（GPIO6 已在上面安全关闭）
    if (!initPWM(FAN_CONTROL_PIN)) {
        Serial.println("");
        Serial.println("[MOSFET-Test] ==================================");
        Serial.println("[MOSFET-Test] INITIALIZATION FAILED!");
        Serial.println("[MOSFET-Test] MOSFET CANNOT START - Check hardware");
        Serial.println("[MOSFET-Test] ==================================");
        Serial.println("[MOSFET-Test] Send 'STATUS' to check details");
        Serial.println("");
        // 初始化失败后进入无限循环，禁止任何启动操作
        while (true) {
            delay(1000);
            // 仍然可以响应 STATUS 命令，但禁止 ON 操作
            if (Serial.available()) {
                String command = Serial.readStringUntil('\n');
                command.trim();
                command.toUpperCase();
                if (command == "STATUS") {
                    showStatus();
                }
            }
        }
    }
    
    Serial.println("[MOSFET-Test] System ready. Waiting for commands...");
    Serial.println("[MOSFET-Test] Send: ON / OFF / STATUS");
    Serial.println("");
    
    g_lastActivityTime = millis();
}

/**
 * Arduino loop
 */
void loop() {
    // 处理串口命令
    processSerialCommand();
    
    // 检查超时保护
    checkTimeoutProtection();
    
    delay(10);  // 小幅延迟，降低 CPU 占用
}
