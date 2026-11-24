/**
 * PIR 人体感应传感器驱动 - 头文件
 * ===============================
 *
 * 功能：
 * - 读取 PIR 传感器数字输出
 * - 信号去抖（防止噪声误触发）
 * - 上升沿检测（只触发一次）
 * - 稳定 LOW 后重新允许触发
 *
 * 硬件要求：
 * - PIR 传感器模块（HC-SR501 风格）
 * - PIR VCC → ESP32 5V 或 3.3V（根据模块规格）
 * - PIR GND → ESP32 GND
 * - PIR OUT → 配置的数字输入引脚
 *
 * ⚠️ 安全警告：
 * - 确认 PIR OUT 电压不超过 3.3V 后再连接 ESP32 GPIO
 * - HC-SR501 模块输出可能为 3.3V 或 5V，需查阅 datasheet 或万用表测量
 *
 * 作者：SmartFan Team
 * 版本：0.1.0
 * 日期：2026-11-20
 */

#ifndef PIR_SENSOR_H
#define PIR_SENSOR_H

#include <Arduino.h>

class PIRSensor {
public:
    /**
     * @brief 构造函数
     * @param pin PIR OUT 连接的 GPIO 引脚
     * @param debounceMs 去抖时间（毫秒），默认 50ms
     */
    PIRSensor(int pin, unsigned long debounceMs = 50);

    /**
     * @brief 初始化 PIR 传感器
     * @return true 初始化成功，false 失败
     */
    bool begin();

    /**
     * @brief 读取当前 PIR 状态
     * @return true 检测到人体（HIGH），false 未检测到（LOW）
     *
     * @note 此方法返回去抖后的稳定状态，不是原始电平
     */
    bool isDetected();

    /**
     * @brief 检查是否有新的有效触发（上升沿）
     * @return true 本周期检测到新的上升沿触发
     *
     * @note 调用后会清除触发标志，每次上升沿只返回一次 true
     */
    bool checkTrigger();

    /**
     * @brief 获取当前原始电平（未经过去抖）
     * @return HIGH 或 LOW
     */
    int getRawLevel();

    /**
     * @brief 获取上次触发时间
     * @return 上次有效触发的 millis() 时间戳
     */
    unsigned long getLastTriggerTime();

    /**
     * @brief 手动重置触发标志
     * @note 可用于外部逻辑刷新计时
     */
    void resetTriggerFlag();

private:
    int _pin;
    unsigned long _debounceMs;
    bool _initialized;
    bool _lastRawState;
    bool _debouncedState;
    unsigned long _lastChangeTime;
    bool _hasTriggered;
    unsigned long _lastTriggerTime;
};

#endif // PIR_SENSOR_H
