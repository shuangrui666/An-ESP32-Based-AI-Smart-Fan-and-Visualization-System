/**
 * PIR 人体感应传感器驱动 - 实现文件
 * ===============================
 *
 * 功能：
 * - 读取 PIR 传感器数字输出
 * - 信号去抖（50ms 稳定时间）
 * - 上升沿检测（只触发一次）
 * - 稳定 LOW 后重新允许触发
 *
 * 非阻塞设计：
 * - 使用 millis() 计时，不占用 CPU
 * - 可在 loop() 中频繁调用
 *
 * 作者：SmartFan Team
 * 版本：0.1.0
 * 日期：2026-11-20
 */

#include "pir_sensor.h"

/**
 * 构造函数
 */
PIRSensor::PIRSensor(int pin, unsigned long debounceMs)
    : _pin(pin)
    , _debounceMs(debounceMs)
    , _initialized(false)
    , _lastRawState(LOW)
    , _debouncedState(LOW)
    , _lastChangeTime(0)
    , _hasTriggered(false)
    , _lastTriggerTime(0)
{
}

/**
 * 初始化 PIR 传感器
 */
bool PIRSensor::begin() {
    // 配置引脚为输入模式
    pinMode(_pin, INPUT);
    
    // 读取初始状态
    _lastRawState = digitalRead(_pin);
    _debouncedState = _lastRawState;
    _lastChangeTime = millis();
    
    _initialized = true;
    
    Serial.printf("[PIRSensor] Initialized on GPIO%d, debounce=%lu ms\n", 
                  _pin, _debounceMs);
    
    return true;
}

/**
 * 读取当前 PIR 状态（去抖后）
 */
bool PIRSensor::isDetected() {
    if (!_initialized) {
        return false;
    }
    
    // 读取原始电平
    bool rawState = digitalRead(_pin) == HIGH;
    
    // 检查状态是否变化
    if (rawState != _lastRawState) {
        // 状态变化，记录时间
        _lastChangeTime = millis();
        _lastRawState = rawState;
    } else {
        // 状态稳定超过去抖时间
        unsigned long currentTime = millis();
        if (currentTime - _lastChangeTime >= _debounceMs) {
            // 稳定时间已到，更新去抖状态
            if (_debouncedState != rawState) {
                _debouncedState = rawState;
                
                // 检测上升沿（从 LOW 到 HIGH）
                if (_debouncedState == HIGH && !_hasTriggered) {
                    _hasTriggered = true;
                    _lastTriggerTime = currentTime;
                    Serial.printf("[PIRSensor] Trigger detected at %lu ms\n", _lastTriggerTime);
                }
            }
        }
    }
    
    return _debouncedState == HIGH;
}

/**
 * 检查是否有新的有效触发
 */
bool PIRSensor::checkTrigger() {
    if (_hasTriggered) {
        _hasTriggered = false;  // 清除触发标志
        return true;
    }
    return false;
}

/**
 * 获取当前原始电平
 */
int PIRSensor::getRawLevel() {
    if (!_initialized) {
        return LOW;
    }
    return digitalRead(_pin);
}

/**
 * 获取上次触发时间
 */
unsigned long PIRSensor::getLastTriggerTime() {
    return _lastTriggerTime;
}

/**
 * 手动重置触发标志
 */
void PIRSensor::resetTriggerFlag() {
    _hasTriggered = false;
}
