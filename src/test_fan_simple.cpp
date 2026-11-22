/**
 * 风扇简单测试程序（独立循环模式）
 * =================================
 * 
 * 功能：
 * - GPIO6 持续输出 HIGH，风扇连续运行
 * - 不依赖串口输入，自动运行
 * - 使用 LEDC PWM API（与正式代码一致）
 * - 启动默认关闭、异常时关闭
 * 
 * 测试目的：
 * - 验证 MOSFET 驱动电路基本工作
 * - 验证 GPIO6 输出高电平
 * - 验证 LEDC PWM 初始化成功
 * 
 * 使用方法：
 * 1. 编译：pio run -e fan-simple-test
 * 2. 上传：pio run -e fan-simple-test -t upload
 * 3. 观察：GPIO6 持续输出 HIGH，风扇连续运行
 * 
 * 安全设计：
 * - 启动时立即将 GPIO6 设置为 LOW
 * - 初始化失败时保持关闭状态
 * - 异常时立即关闭输出
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

// PWM 配置（与正式代码保持一致）
#define PWM_FREQ 5000           // 5kHz 避免可听噪声
#define PWM_RESOLUTION 8        // 8-bit 分辨率 (0-255)

// 全局状态
static bool g_initialized = false;
static bool g_pwmEnabled = true;

/**
 * 初始化 LEDC PWM
 * 返回 true 表示成功，false 表示失败
 */
bool initPWM(int pin) {
    Serial.println("[Fan-Test] Initializing LEDC PWM...");
    Serial.printf("[Fan-Test] Pin: GPIO%d, Freq: %dHz, Resolution: %d-bit\n",
                  pin, PWM_FREQ, PWM_RESOLUTION);
    
    // 配置引脚为输出并立即设置为 LOW（最优先，确保安全）
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    
    // 初始化 LEDC PWM
    if (!ledcAttach(pin, PWM_FREQ, PWM_RESOLUTION)) {
        Serial.println("[Fan-Test] ERROR: ledcAttach() failed!");
        Serial.println("[Fan-Test] Cannot attach PWM. Keeping pin LOW.");
        return false;
    }
    
    // 初始输出为 0（关闭）
    if (!ledcWrite(pin, 0)) {
        Serial.println("[Fan-Test] ERROR: ledcWrite() failed!");
        Serial.println("[Fan-Test] Cannot write initial duty. Keeping pin LOW.");
        return false;
    }
    
    g_initialized = true;
    Serial.println("[Fan-Test] LEDC PWM initialized successfully");
    Serial.println("[Fan-Test] Fan will run continuously (GPIO6 HIGH)");
    Serial.println("");
    
    return true;
}

/**
 * 关闭风扇（安全关闭）
 */
void turnOffFan() {
    if (g_initialized && g_pwmEnabled) {
        ledcWrite(FAN_CONTROL_PIN, 0);
    } else {
        digitalWrite(FAN_CONTROL_PIN, LOW);
    }
    Serial.println("[Fan-Test] Fan OFF (safe shutdown)");
}

/**
 * Arduino setup
 */
void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(500);  // 给串口驱动初始化时间

    Serial.println("");
    Serial.println("[Fan-Test] ==================================");
    Serial.println("[Fan-Test] Fan Simple Test Program");
    Serial.println("[Fan-Test] ==================================");
    Serial.printf("[Fan-Test] Target Pin: GPIO%d\n", FAN_CONTROL_PIN);
    Serial.println("");
    
    // 初始化 PWM（失败时保持关闭）
    if (!initPWM(FAN_CONTROL_PIN)) {
        Serial.println("");
        Serial.println("[Fan-Test] ==================================");
        Serial.println("[Fan-Test] INITIALIZATION FAILED!");
        Serial.println("[Fan-Test] Pin kept LOW - Check hardware");
        Serial.println("[Fan-Test] ==================================");
        Serial.println("");
        // 初始化失败后进入无限循环，保持关闭状态
        while (true) {
            delay(1000);
        }
    }
    
    Serial.println("[Fan-Test] System ready. Fan will run continuously.");
    Serial.println("");
}

/**
 * Arduino loop
 * 持续输出 HIGH，不进行周期性关闭
 */
void loop() {
    if (!g_initialized) {
        digitalWrite(FAN_CONTROL_PIN, LOW);
        delay(100);
        return;
    }

    if (g_pwmEnabled) {
        ledcWrite(FAN_CONTROL_PIN, 255);
    } else {
        digitalWrite(FAN_CONTROL_PIN, HIGH);
    }

    Serial.println("[Fan-Test] Fan running continuously");
    delay(1000);
}
