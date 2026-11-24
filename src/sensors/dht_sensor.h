/**
 * DHT11 温湿度传感器驱动 - 头文件
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

#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include <Arduino.h>

// DHT 传感器引脚配置
#define DHT_SENSOR_PIN 4

// DHT 传感器类型
#define DHT_SENSOR_TYPE DHT11

/**
 * DHT 传感器类
 */
class DHTSensor {
public:
    /**
     * 构造函数
     */
    DHTSensor();

    /**
     * 初始化传感器
     * @param pin 数据引脚（默认 DHT_SENSOR_PIN）
     * @return true 初始化成功，false 失败
     */
    bool begin(int pin = DHT_SENSOR_PIN);

    /**
     * 读取温度
     * @param unit 温度单位，'C' 或 'F'
     * @return 温度值，失败返回 NAN
     */
    float readTemperature(char unit = 'C');

    /**
     * 读取湿度
     * @return 湿度值（0-100%），失败返回 NAN
     */
    float readHumidity();

    /**
     * 检查传感器是否可用
     * @return true 传感器正常，false 故障
     */
    bool isAvailable();

    /**
     * 获取最后一次错误代码
     * @return 错误代码（0 表示无错误）
     */
    int getLastError();

    /**
     * 读取所有数据（一次性读取温度和湿度，提高效率）
     * @param temperature 输出参数：温度值
     * @param humidity 输出参数：湿度值
     * @return true 读取成功，false 失败
     */
    bool readAll(float& temperature, float& humidity);

private:
    int _pin;
    int _type;
    bool _initialized;
    int _lastError;
    unsigned long _lastReadTime;
    float _cachedTemperature;
    float _cachedHumidity;
    bool _cacheValid;

    /**
     * 读取原始数据
     * @return true 读取成功
     */
    bool _readData(float& temperature, float& humidity);
};

#endif // DHT_SENSOR_H
