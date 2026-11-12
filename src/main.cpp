/**
 * @file main.cpp
 * @brief SmartFan 2.0 - 主程序入口
 * @version 0.1.0 (第一阶段：基础测试)
 * @date 2026-11-11
 * 
 * @description
 * 本阶段目标：ESP32 基础测试程序
 * - 实现 Serial 串口初始化
 * - 每隔 2 秒打印 ESP32 运行状态信息
 * - 读取 DHT11 温湿度数据
 * - 不连接风扇
 * - 不使用未经确认的 GPIO 引脚
 * 
 * @note 硬件待确认事项详见 docs/hardware.md
 */

#include <Arduino.h>
#include "sensors/dht_sensor.h"

// ============================================================
// 全局配置
// ============================================================

// 程序版本信息
#define FIRMWARE_VERSION "0.1.0"
#define FIRMWARE_PHASE   "Phase 1: Basic Test"

// 日志输出间隔（毫秒）
#define LOG_INTERVAL_MS 2000

// ============================================================
// 全局变量
// ============================================================

// 上次日志输出时间
static unsigned long lastLogTime = 0;

// 系统启动时间
static unsigned long systemStartTime = 0;

// DHT11 传感器实例
static DHTSensor dhtSensor;

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

// ============================================================
// 函数实现
// ============================================================

/**
 * @brief 系统初始化
 *
 * 初始化串口、打印启动信息、初始化 DHT11 传感器
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
    Serial.printf("  CPU Frequency: %d MHz\r\n", ESP.getCpuFreqMHz());
    Serial.printf("  Heap Size: %d KB\r\n", ESP.getHeapSize() / 1024);
    Serial.printf("  PSRAM Size: %d KB\r\n", ESP.getPsramSize() / 1024);
    Serial.println("========================================");
    Serial.println();

    // 记录启动时间
    systemStartTime = millis();

    Serial.println("[INIT] System initialization completed");
    
    // 初始化 DHT11 传感器
    Serial.println("[INIT] Initializing DHT11 sensor...");
    if (dhtSensor.begin(DHT_SENSOR_PIN)) {
        Serial.println("[INIT] DHT11 sensor initialized successfully");
    } else {
        Serial.println("[INIT] DHT11 sensor initialization FAILED");
    }
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
 * @brief 打印系统状态信息
 *
 * 输出当前系统运行状态，包括：
 * - 运行时间
 * - 空闲堆内存
 * - DHT11 温湿度（第二阶段启用）
 * - WiFi 状态（后续阶段启用）
 */
void printSystemStatus() {
    Serial.println("----------------------------------------");
    Serial.printf("[STATUS] Uptime: %lu seconds\r\n", getSystemUptime());
    Serial.printf("[STATUS] Free Heap: %d bytes\r\n", ESP.getFreeHeap());
    Serial.printf("[STATUS] Chip Temp: %.1f °C\r\n", temperatureRead());
    
    // 读取并输出 DHT11 温湿度数据
    float temperature, humidity;
    if (dhtSensor.readAll(temperature, humidity)) {
        Serial.printf("[DHT11] Temperature: %.1f °C\r\n", temperature);
        Serial.printf("[DHT11] Humidity: %.1f %%\r\n", humidity);
    } else {
        Serial.println("[DHT11] Temperature: Read Failed");
        Serial.println("[DHT11] Humidity: Read Failed");
    }
    
    Serial.println("----------------------------------------");
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
 * 周期性打印系统状态
 */
void loop() {
    unsigned long currentTime = millis();
    
    // 定期检查是否到达日志输出时间
    if (currentTime - lastLogTime >= LOG_INTERVAL_MS) {
        // 更新下次输出时间
        lastLogTime = currentTime;
        
        // 打印系统状态
        printSystemStatus();
    }
    
    // 主循环中保持低功耗（后续阶段可替换为任务处理）
    delay(100);
}
