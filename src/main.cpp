/**
 * @file main.cpp
 * @brief SmartFan 2.0 - 主程序入口
 * @version 0.2.0 (第二阶段：PIR + 风扇自动控制)
 * @date 2026-11-20
 *
 * @description
 * 本阶段目标：PIR + 风扇 PWM 自动控制功能
 * - 整合 PIR 人体感应传感器（GPIO5）
 * - 整合风扇 PWM 控制（FAN_CONTROL_PIN，默认 GPIO6）
 * - 实现状态机：IDLE, SOFT_START, RUNNING, SOFT_STOP
 * - 60 秒无活动后自动软停止
 * - 保留 DHT11、Wi-Fi、后端通信功能
 *
 * @note 硬件待确认事项详见 docs/hardware.md
 */

#include <Arduino.h>
#include "sensors/dht_sensor.h"
#include "sensors/pir_sensor.h"
#include "control/fan_controller.h"
#include "communication/wifi_manager.h"

// ============================================================
// 全局配置
// ============================================================

// 引脚配置
#define PIR_SENSOR_PIN 5  // PIR 传感器输出连接到 GPIO5

// 程序版本信息
#define FIRMWARE_VERSION "0.2.0"
#define FIRMWARE_PHASE   "Phase 2: PIR + Fan Auto Control"

// 日志输出间隔（毫秒）
#define LOG_INTERVAL_MS 2000
#define SENSOR_UPLOAD_INTERVAL_MS 5000

// PIR 无活动超时时间（毫秒）- 60 秒
#define PIR_TIMEOUT_MS 60000

// ============================================================
// 温度控制阈值配置
// ============================================================

// 风扇启动温度阈值（℃）
#define TEMP_START_THRESHOLD 28.0

// 风扇关闭温度阈值（℃）- 形成 2℃回差
#define TEMP_STOP_THRESHOLD 26.0

// ============================================================
// 全局变量
// ============================================================

// 上次日志输出时间
static unsigned long lastLogTime = 0;

// 系统启动时间
static unsigned long systemStartTime = 0;

// DHT11 传感器实例
static DHTSensor dhtSensor;

// PIR 传感器实例（GPIO5，50ms 去抖）
static PIRSensor pirSensor(PIR_SENSOR_PIN, 50);

// 风扇控制器实例
static FanController fanController;

// Wi-Fi 管理实例
static WifiManager wifiManager;

// 上次传感器数据上传时间
static unsigned long lastSensorUploadTime = 0;

// 上次 PIR 活动时间
static unsigned long lastPirActivityTime = 0;

// 当前执行的远程命令 ID（空字符串表示无）
static String currentRemoteCommandId = "";

// 上次远程指令执行时间
static unsigned long lastRemoteCommandTime = 0;

// 当前控制模式（"auto" 或 "manual"）
static String currentControlMode = "auto";

// ============================================================
// 函数声明
// ============================================================

/**
 * @brief 初始化系统
 */
void systemInit();

/**
 * @brief 打印系统状态信息
 */
void printSystemStatus();

/**
 * @brief 获取系统运行时间（秒）
 * @return 运行时间（秒）
 */
unsigned long getSystemUptime();

/**
 * @brief 上传 DHT11 温湿度数据到后端
 */
void uploadSensorData();

/**
 * @brief 处理 PIR 检测和风扇控制状态机
 */
void handlePIRAndFanControl();

/**
 * @brief 执行远程指令
 * @param command 指令对象
 * @return true 执行成功
 */
bool executeCommand(const Command& command);

/**
 * @brief 处理远程指令轮询（Phase 3.2.2）
 */
void handleRemoteCommand();

// ============================================================
// 函数实现
// ============================================================

/**
 * @brief 系统初始化
 *
 * 初始化串口、打印启动信息、初始化各模块
 */
void systemInit() {
    // 初始化串口监控
    Serial.begin(115200);

    // 等待串口缓冲就绪
    delay(100);

    // 打印启动信息
    Serial.println();
    Serial.println("========================================");
    Serial.println("   SmartFan 2.0 - ESP32 Smart Fan");
    Serial.println("========================================");
    Serial.printf("  Firmware Version: %s\r\n", FIRMWARE_VERSION);
    Serial.printf("  Development Phase: %s\r\n", FIRMWARE_PHASE);
    Serial.printf("  Chip Model: %s\r\n", ESP.getChipModel());
    Serial.printf("  Chip Revision: %d\r\n", ESP.getChipRevision());
    Serial.printf("  CPU Frequency: %lu MHz\r\n", ESP.getCpuFreqMHz());
    Serial.printf("  Heap Size: %lu KB\r\n", ESP.getHeapSize() / 1024);
    Serial.printf("  PSRAM Size: %lu KB\r\n", ESP.getPsramSize() / 1024);
    Serial.println("========================================");
    Serial.println();

    // 记录启动时间
    systemStartTime = millis();

    Serial.println("[INIT] System initialization started");

    // 初始化 DHT11 传感器
    Serial.println("[INIT] Initializing DHT11 sensor...");
    if (dhtSensor.begin(DHT_SENSOR_PIN)) {
        Serial.println("[INIT] DHT11 sensor initialized successfully");
    } else {
        Serial.println("[INIT] DHT11 sensor initialization FAILED");
    }

    // 初始化 PIR 传感器
    Serial.println("[INIT] Initializing PIR sensor...");
    if (pirSensor.begin()) {
        Serial.println("[INIT] PIR sensor initialized successfully");
        Serial.printf("[INIT] PIR GPIO: %d, Debounce: 50ms\n", PIR_SENSOR_PIN);
    } else {
        Serial.println("[INIT] PIR sensor initialization FAILED");
    }

    // 初始化风扇控制器
    Serial.println("[INIT] Initializing Fan Controller...");
    Serial.printf("[INIT] Fan PWM GPIO: %d\n", FAN_CONTROL_PIN);
    Serial.printf("[INIT] Calling fanController.begin()...\n");
    bool fanInitResult = fanController.begin();
    Serial.printf("[INIT] fanController.begin() returned: %s\n", fanInitResult ? "true" : "false");
    if (fanInitResult) {
        Serial.println("[INIT] Fan Controller initialized successfully");
    } else {
        Serial.println("[INIT] Fan Controller initialization FAILED");
    }

    // 初始化 Wi-Fi
    Serial.println("[INIT] Initializing Wi-Fi...");
    if (wifiManager.begin()) {
        Serial.println("[INIT] Wi-Fi initialized successfully");

        // 连接电脑移动热点
        Serial.println("[INIT] Connecting to Wi-Fi...");
        if (wifiManager.connect()) {
            Serial.println("[INIT] Wi-Fi connected successfully");
            Serial.printf("[WiFi] IP Address: %s\n",
                          wifiManager.getIPAddress().c_str());
            Serial.printf("[WiFi] Backend URL: %s\n",
                          wifiManager.getBackendURL().c_str());
        } else {
            Serial.println("[INIT] Wi-Fi connection FAILED");
        }
    } else {
        Serial.println("[INIT] Wi-Fi initialization FAILED");
    }

    // 初始化活动时间戳
    lastPirActivityTime = millis();

    Serial.println("[INIT] All systems initialized");
    Serial.println();
}

/**
 * @brief 获取系统运行时间
 *
 * @return 系统运行时间（秒）
 */
unsigned long getSystemUptime() {
    return (millis() - systemStartTime) / 1000;
}

/**
 * @brief 上传 DHT11 温湿度数据到后端
 */
void uploadSensorData() {
    if (!wifiManager.isConnected()) {
        Serial.println("[UPLOAD] Wi-Fi disconnected, skip upload");
        return;
    }

    float temperature;
    float humidity;

    if (!dhtSensor.readAll(temperature, humidity)) {
        Serial.println("[UPLOAD] DHT11 read failed, skip upload");
        return;
    }

    // 获取当前风扇档位（-1 表示未连接或关闭）
    int fanGear = fanController.isPowerOn() ? fanController.getCurrentGear() : -1;

    Serial.printf(
        "[UPLOAD] Sending: temp=%.1f C, hum=%.1f %%, fan=%d\r\n",
        temperature, humidity, fanGear
    );

    // 发送传感器数据和风扇状态
    if (wifiManager.sendSensorData(temperature, humidity, fanGear)) {
        Serial.println("[UPLOAD] Sensor data uploaded successfully");
    } else {
        Serial.println("[UPLOAD] Sensor data upload FAILED");
    }
}

/**
 * @brief 打印系统状态信息
 *
 * 输出当前系统运行状态，包括：
 * - 运行时间
 * - 空闲堆内存
 * - DHT11 温湿度
 * - PIR 状态
 * - 风扇状态
 * - WiFi 状态
 */
void printSystemStatus() {
    Serial.println("----------------------------------------");
    Serial.printf("[STATUS] Uptime: %lu seconds\r\n", getSystemUptime());
    Serial.printf("[STATUS] Free Heap: %lu bytes\r\n", ESP.getFreeHeap());
    
    // 读取并输出 DHT11 温湿度数据
    float temperature, humidity;
    if (dhtSensor.readAll(temperature, humidity)) {
        Serial.printf("[DHT11] Temperature: %.1f °C\r\n", temperature);
        Serial.printf("[DHT11] Humidity: %.1f %%\r\n", humidity);
    } else {
        Serial.println("[DHT11] Temperature: Read Failed");
        Serial.println("[DHT11] Humidity: Read Failed");
    }

    // PIR 状态
    Serial.printf("[PIR] Detected: %s, Raw: %s\r\n",
                  pirSensor.isDetected() ? "YES" : "NO",
                  pirSensor.getRawLevel() == HIGH ? "HIGH" : "LOW");

    // 风扇状态
    Serial.printf("[FAN] State: %s, Gear: %d (%s), Duty: %d%%\r\n",
                  fanController.getStateString(),
                  fanController.getCurrentGear(),
                  FanController::getGearName(fanController.getCurrentGear()),
                  fanController.getCurrentDutyCycle() * 100 / 255);

    // Wi-Fi 状态
    Serial.printf("[WiFi] Connected: %s, IP: %s\r\n",
                  wifiManager.isConnected() ? "YES" : "NO",
                  wifiManager.getIPAddress().c_str());

    // 无活动计时
    unsigned long idleTime = millis() - lastPirActivityTime;
    Serial.printf("[TIMEOUT] Idle time: %lu s / %u s (timeout)\r\n",
                  idleTime / 1000, (unsigned int)(PIR_TIMEOUT_MS / 1000));

    Serial.println("----------------------------------------");
}

/**
 * @brief 处理 PIR 检测和风扇控制状态机
 *
 * 控制逻辑（温度 + PIR 联合控制）：
 * 1. 检测到有人 + 温度≥28℃：启动风扇（PWM 255，100% 占空比）
 * 2. 检测到有人 + 温度<28℃：不启动风扇
 * 3. 风扇运行后暂时检测不到人：进入 60 秒延时计时
 * 4. 延时期间再次检测到人：重新计算延时
 * 5. 温度低于 26℃：关闭风扇（形成 2℃回差）
 * 6. 传感器读取失败：关闭风扇，输出错误日志
 *
 * 状态机流程：
 * - IDLE: 初始状态，风扇关闭，等待 PIR 触发且温度达标
 * - SOFT_START: PIR 触发且温度达标后，软启动到 100% 占空比
 * - RUNNING: 软启动完成，风扇全速运行
 * - SOFT_STOP: 60 秒无活动或温度低于 26℃后，软停止到关闭
 *
 * 关键逻辑：
 * - 每次 PIR 有效触发刷新 lastPirActivityTime
 * - 连续 60 秒无新触发后开始软停止
 * - 温度低于 26℃立即启动软停止
 * - 软启动/软停止过程中继续执行其他任务
 * - 软停止过程中再次触发且温度达标时，取消停止并重新启动
 */
void handlePIRAndFanControl() {
    // MANUAL 模式：跳过 AUTO 控制逻辑，由远程指令直接控制风扇
    if (currentControlMode == "manual") {
        // 在 MANUAL 模式下仍然需要调用 fanController.update() 以维持软启动/软停止状态机
        fanController.update();
        return;
    }
    
    // AUTO 模式：继续执行温度+PIR 联合控制
    float temperature;
    float humidity;

    // 读取温度数据
    if (!dhtSensor.readAll(temperature, humidity)) {
        // 传感器读取失败：关闭风扇，输出错误日志
        Serial.println("[AUTO_CONTROL] ERROR: DHT11 read failed, turning off fan");
        FanState currentState = fanController.getState();
        if (currentState == FanState::RUNNING || currentState == FanState::SOFT_START) {
            Serial.println("[AUTO_CONTROL] State: RUNNING/SOFT_START -> SOFT_STOP (sensor error)");
            fanController.startSoftStop();
        }
        fanController.update();
        return;
    }

    // 检查 PIR 是否有新的有效触发
    bool pirDetected = pirSensor.checkTrigger();
    if (pirDetected) {
        Serial.println("[PIR] New trigger detected!");
        lastPirActivityTime = millis();

        // 温度判断：只有温度≥28℃才启动风扇
        if (temperature >= TEMP_START_THRESHOLD) {
            Serial.printf("[AUTO_CONTROL] Temperature %.1f°C >= %d°C, PIR detected, starting fan\n",
                          temperature, (int)TEMP_START_THRESHOLD);

            // 根据当前状态决定动作
            FanState currentState = fanController.getState();

            if (currentState == FanState::IDLE) {
                // IDLE 状态：启动软启动到 100% 占空比（255）
                Serial.println("[AUTO_CONTROL] State: IDLE -> SOFT_START");
                // 直接设置 PWM 为 255（100% 占空比）
                fanController.setGear(FAN_GEAR_5);  // 100% 占空比
            } else if (currentState == FanState::SOFT_STOP) {
                // SOFT_STOP 状态：取消停止，重新启动
                Serial.println("[AUTO_CONTROL] State: SOFT_STOP -> RUNNING (restart)");
                fanController.setGear(FAN_GEAR_5);  // 100% 占空比
            }
            // RUNNING 和 SOFT_START 状态：不需要动作，继续运行
        } else {
            Serial.printf("[AUTO_CONTROL] Temperature %.1f°C < %d°C, PIR detected but fan NOT started\n",
                          temperature, (int)TEMP_START_THRESHOLD);
        }
    }

    // 检查是否超时（60 秒无活动）
    unsigned long idleTime = millis() - lastPirActivityTime;
    if (idleTime >= PIR_TIMEOUT_MS) {
        // 超时检查：只有在 RUNNING 状态才启动软停止
        FanState currentState = fanController.getState();
        if (currentState == FanState::RUNNING) {
            Serial.printf("[AUTO_CONTROL] No PIR activity for %lu ms, starting soft stop\n", idleTime);
            Serial.println("[AUTO_CONTROL] State: RUNNING -> SOFT_STOP");
            fanController.startSoftStop();
        }
    }

    // 温度判断：温度低于 26℃时关闭风扇（无论 PIR 状态）
    if (temperature < TEMP_STOP_THRESHOLD) {
        FanState currentState = fanController.getState();
        if (currentState == FanState::RUNNING || currentState == FanState::SOFT_START) {
            Serial.printf("[AUTO_CONTROL] Temperature %.1f°C < %d°C, turning off fan\n",
                          temperature, (int)TEMP_STOP_THRESHOLD);
            Serial.println("[AUTO_CONTROL] State: RUNNING/SOFT_START -> SOFT_STOP (temp low)");
            fanController.startSoftStop();
        }
    }
    
    // 更新风扇控制器状态（必须在 loop 中周期性调用）
    fanController.update();
}

/**
 * @brief 执行远程指令
 * 
 * 根据指令的 mode 和 gear 设置风扇控制模式：
 * - mode = "manual": 设置手动模式，执行指定档位
 * - mode = "auto": 设置自动模式，恢复温度+PIR 控制
 * 
 * @param command 指令对象
 * @return true 执行成功，false 执行失败
 */
bool executeCommand(const Command& command) {
    Serial.println("[CMD] Executing command...");
    command.print();
    
    // 检查指令是否过期
    if (command.isExpired()) {
        Serial.println("[CMD] ERROR: Command expired, skip execution");
        return false;
    }
    
    // 检查档位有效性
    if (command.gear < 0 || command.gear > 5) {
        Serial.printf("[CMD] ERROR: Invalid gear %d, must be 0-5\n", command.gear);
        return false;
    }
    
    // 根据模式设置控制方式
    if (command.isManualMode()) {
        Serial.printf("[CMD] MANUAL mode: Setting gear %d\n", command.gear);
        currentControlMode = "manual";

        // 手动模式：直接设置档位
        bool success = fanController.setGear(command.gear);  // gear 已经是 int 类型（0-5）
        if (!success) {
            Serial.println("[CMD] ERROR: Failed to set gear, FanController not initialized");
            return false;
        }

        if (command.gear == 0) {
            Serial.println("[CMD] Fan turned OFF (gear 0)");
        } else {
            Serial.printf("[CMD] Fan set to gear %d (%s)\n", command.gear, FanController::getGearName(command.gear));
        }

    } else if (command.isAutoMode()) {
        Serial.println("[CMD] AUTO mode: Restoring automatic control");
        currentControlMode = "auto";
        // 自动模式：不直接设置风扇，由 handlePIRAndFanControl() 控制
        // 这里只记录模式切换，风扇状态由温度和 PIR 决定
        Serial.println("[CMD] Auto control restored, fan will be controlled by temperature and PIR");
    } else {
        Serial.printf("[CMD] ERROR: Unknown mode '%s'\n", command.mode.c_str());
        return false;
    }
    
    // 记录执行的命令 ID 和时间
    currentRemoteCommandId = command.command_id;
    lastRemoteCommandTime = millis();
    
    Serial.printf("[CMD] Command executed successfully: mode=%s, gear=%d\n", 
                  command.mode.c_str(), command.gear);
    return true;
}

/**
 * @brief 处理远程指令轮询（Phase 3.2.2）
 *
 * 从后端获取待执行指令，执行后发送 ACK
 */
void handleRemoteCommand() {
    if (!wifiManager.isConnected()) {
        return;  // Wi-Fi 未连接，跳过
    }

    Command command;

    // 使用 handleCommandPolling() 代替 fetchPendingCommand()
    // 这样可以确保去重检查和过期检查生效
    if (wifiManager.handleCommandPolling(command)) {
        Serial.println("[CMD] Received new valid command from backend");

        // 执行指令
        bool success = executeCommand(command);

        // 发送 ACK
        if (success) {
            String message = String("Command ") + currentRemoteCommandId +
                           " executed: mode=" + currentControlMode +
                           ", gear=" + String(fanController.getCurrentGear());
            wifiManager.ackCommand(command.command_id, true, message);
        } else {
            String message = String("Command ") + command.command_id + " execution failed";
            wifiManager.ackCommand(command.command_id, false, message);
        }
    }
}

// ============================================================
// Arduino 回调函数
// ============================================================

/**
 * @brief Arduino 初始化回调
 *
 * 系统启动时调用一次
 */
void setup() {
    systemInit();
}

/**
 * @brief Arduino 主循环
 *
 * 周期性：
 * - 处理 PIR 检测和风扇控制状态机
 * - 打印系统状态
 * - 上传传感器数据
 * - 处理远程指令轮询（Phase 3.2.2）
 */
void loop() {
    unsigned long currentTime = millis();

    // 处理 PIR 检测和风扇控制（每次循环都调用）
    handlePIRAndFanControl();

    // 处理远程指令轮询（Phase 3.2.2：ESP32 指令轮询与 ACK）
    // 非阻塞调用，由 handleRemoteCommand() 内部使用 millis() 控制轮询间隔
    handleRemoteCommand();

    // 每 2 秒打印一次系统状态
    if (currentTime - lastLogTime >= LOG_INTERVAL_MS) {
        lastLogTime = currentTime;
        printSystemStatus();
    }

    // 每 5 秒上传一次传感器数据
    if (currentTime - lastSensorUploadTime >= SENSOR_UPLOAD_INTERVAL_MS) {
        lastSensorUploadTime = currentTime;
        uploadSensorData();
    }

    // 小延时避免 CPU 占用过高
    delay(10);
}
