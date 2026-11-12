/**
 * DHT11 温湿度传感器驱动
 * ======================
 *
 * 功能：
 * - 初始化 DHT11 传感器
 * - 读取温度数据
 * - 读取湿度数据
 * - 传感器状态检测
 *
 * 硬件要求：
 * - DHT11 传感器模块
 * - 10k 上拉电阻（DATA 引脚到 VCC）
 *
 * 接线说明：
 * - DHT11 VCC → ESP32 3.3V
 * - DHT11 GND → ESP32 GND
 * - DHT11 DATA → GPIO4
 *
 * 作者：SmartFan Team
 * 版本：1.0.0
 * 日期：2026-11-11
 */

#include <Arduino.h>
#include "dht_sensor.h"
#include <DHT.h>

// DHT 传感器实例（静态）
static DHT* _dhtInstance = nullptr;

/**
 * DHT 传感器类
 */
DHTSensor::DHTSensor()
    : _pin(DHT_SENSOR_PIN)
    , _type(DHT_SENSOR_TYPE)
    , _initialized(false)
    , _lastError(0)
    , _lastReadTime(0)
    , _cachedTemperature(NAN)
    , _cachedHumidity(NAN)
    , _cacheValid(false)
{
}

bool DHTSensor::begin(int pin) {
    _pin = pin;

    // 配置引脚模式
    pinMode(_pin, INPUT_PULLUP);

    // 创建 DHT 对象（使用 Adafruit DHT 库的构造函数）
    // DHT(uint8_t pin, uint8_t type, uint8_t count = 6)
    _dhtInstance = new DHT(_pin, _type, 6);

    // 关键：调用 begin() 初始化传感器（DHT11 需要 55us 延迟）
    _dhtInstance->begin(55);

    _initialized = true;
    _lastReadTime = 0;

    Serial.println("[DHTSensor] Initialized successfully");
    Serial.printf("[DHTSensor] Pin: %d, Type: DHT11\n", _pin);
    return true;
}

bool DHTSensor::_readData(float& temperature, float& humidity) {
    if (!_initialized || _dhtInstance == nullptr) {
        _lastError = -1;
        Serial.println("[DHTSensor] Not initialized");
        return false;
    }

    // DHT11 最小读取间隔 2 秒（保守设置，避免时序问题）
    unsigned long currentTime = millis();
    if (currentTime - _lastReadTime < 2000) {
        // 使用缓存的数据（仅当缓存有效时）
        if (_cacheValid) {
            temperature = _cachedTemperature;
            humidity = _cachedHumidity;
            return true;
        }
        // 缓存无效，返回 false
        return false;
    }

    // 读取温湿度数据（Adafruit DHT 库 API）
    temperature = _dhtInstance->readTemperature();
    humidity = _dhtInstance->readHumidity();

    // 检查读取是否失败
    if (isnan(temperature) || isnan(humidity)) {
        _lastError = 1; // DHT 库读取失败
        Serial.printf("[DHTSensor] Read failed, temperature=%f, humidity=%f\n", temperature, humidity);
        Serial.println("[DHTSensor] Sensor read timeout or checksum error");

        // 不更新缓存，保持上次有效数据
        temperature = NAN;
        humidity = NAN;
        return false;
    }

    // 成功读取，更新缓存
    _lastReadTime = currentTime;
    _lastError = 0;
    _cachedTemperature = temperature;
    _cachedHumidity = humidity;
    _cacheValid = true;

    Serial.printf("[DHTSensor] Read OK: temp=%.1f°C, hum=%.1f%%\n", temperature, humidity);
    return true;
}

float DHTSensor::readTemperature(char unit) {
    float temp, hum;
    if (!_readData(temp, hum)) {
        return NAN;
    }

    if (unit == 'F') {
        return temp * 9.0 / 5.0 + 32.0;
    }
    return temp;
}

float DHTSensor::readHumidity() {
    float temp, hum;
    if (!_readData(temp, hum)) {
        return NAN;
    }
    return hum;
}

bool DHTSensor::readAll(float& temperature, float& humidity) {
    return _readData(temperature, humidity);
}

bool DHTSensor::isAvailable() {
    if (!_initialized) {
        return false;
    }

    // 尝试读取一次来验证传感器是否响应
    float temp, hum;
    bool result = _readData(temp, hum);

    // 如果是缓存数据（间隔小于 2 秒且缓存有效），视为可用
    if (_cacheValid && millis() - _lastReadTime < 2000) {
        return true;
    }

    return result;
}

int DHTSensor::getLastError() {
    return _lastError;
}
