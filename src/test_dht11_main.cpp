/**
 * DHT11 独立测试程序
 * ==================
 * 
 * 用途：诊断 GPIO4 上的 DHT11 传感器硬件故障
 * 
 * 测试功能：
 * - 直接调用 Adafruit DHT 库（绕过封装层）
 * - 初始化后等待 2 秒再开始读取
 * - 每隔 2 秒读取一次温湿度
 * - 输出详细诊断信息（初始化状态、读取结果、错误代码）
 * 
 * 硬件接线：
 * - DHT11 VCC → ESP32-S3 3.3V
 * - DHT11 DATA → ESP32-S3 GPIO4
 * - DHT11 GND → ESP32-S3 GND
 * 
 * 使用方法：
 * 1. 编译上传此文件
 * 2. 打开串口监视器（115200 波特率）
 * 3. 观察输出判断故障原因
 */

#include <Arduino.h>
#include <DHT.h>

// 测试配置
#define TEST_DHT_PIN     4      // GPIO4
#define TEST_DHT_TYPE    DHT11  // DHT11 传感器
#define TEST_INTERVAL    2000   // 读取间隔 2 秒

// 全局对象
static DHT* testDHT = nullptr;

// 计数器
static int successCount = 0;
static int failCount = 0;

void setup() {
    // 初始化串口
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    Serial.println("\n");
    Serial.println("========================================");
    Serial.println("    DHT11 独立硬件诊断测试程序");
    Serial.println("========================================");
    Serial.printf("引脚配置：GPIO%d\n", TEST_DHT_PIN);
    Serial.printf("传感器类型：DHT11\n");
    Serial.println("========================================");
    Serial.println("\n");

    // 配置引脚模式
    Serial.println("[1/4] 配置 GPIO 引脚模式...");
    pinMode(TEST_DHT_PIN, INPUT_PULLUP);
    Serial.println("      ✓ 引脚配置完成 (INPUT_PULLUP)");
    delay(100);

    // 创建 DHT 对象
    Serial.println("[2/4] 创建 DHT 对象...");
    testDHT = new DHT(TEST_DHT_PIN, TEST_DHT_TYPE, 6);
    Serial.println("      ✓ DHT 对象创建完成");
    delay(100);

    // 初始化传感器
    Serial.println("[3/4] 初始化 DHT 传感器...");
    testDHT->begin(55);  // 55us 延迟（DHT11 标准）
    Serial.println("      ✓ begin() 调用完成");
    delay(100);

    Serial.println("[4/4] 初始化完成，等待 2 秒后开始读取...");
    Serial.println("      (DHT11 需要稳定时间)");
    
    // 关键：等待至少 2 秒让传感器稳定
    delay(2000);

    Serial.println("\n========================================");
    Serial.println("    开始循环读取测试");
    Serial.println("========================================\n");
}

void loop() {
    static unsigned long lastReadTime = 0;
    static int testNumber = 0;

    unsigned long currentTime = millis();

    // 每隔 2 秒读取一次
    if (currentTime - lastReadTime >= TEST_INTERVAL) {
        lastReadTime = currentTime;
        testNumber++;

        Serial.printf("\n--- 测试 #%d (耗时：%lus) ---\n", 
                      testNumber, currentTime / 1000);

        // 方式 1: 使用 read() 底层函数（最详细）
        Serial.println("[读取方式 1] 调用 read() 底层函数:");
        bool readResult = testDHT->read(true);  // force=true 强制重新读取
        Serial.printf("  read() 返回值: %s\n", 
                      readResult ? "true (OK)" : "false (Failed)");

        if (readResult) {
            // 读取成功，获取数据
            float temp = testDHT->readTemperature();
            float hum = testDHT->readHumidity();
            
            Serial.printf("  温度: %.1f °C\n", temp);
            Serial.printf("  湿度: %.1f %%\n", hum);
            
            // 验证数据合理性
            bool tempValid = !isnan(temp) && temp >= -40 && temp <= 80;
            bool humValid = !isnan(hum) && hum >= 0 && hum <= 100;
            
            Serial.printf("  数据验证: 温度%s, 湿度%s\n",
                          tempValid ? "✓ 有效" : "✗ 无效",
                          humValid ? "✓ 有效" : "✗ 无效");
            
            if (tempValid && humValid) {
                successCount++;
                Serial.println("  >>> 本次读取：成功");
            } else {
                failCount++;
                Serial.println("  >>> 本次读取：数据异常");
            }
        } else {
            // 读取失败
            float temp = testDHT->readTemperature();
            float hum = testDHT->readHumidity();
            
            Serial.printf("  温度原始值: %f (isnan: %s)\n", 
                          temp, isnan(temp) ? "true" : "false");
            Serial.printf("  湿度原始值: %f (isnan: %s)\n", 
                          hum, isnan(hum) ? "true" : "false");
            
            failCount++;
            Serial.println("  >>> 本次读取：失败 (传感器无响应)");
        }

        // 方式 2: 直接调用 readTemperature() 和 readHumidity()
        Serial.println("\n[读取方式 2] 直接调用 readTemperature()/readHumidity():");
        float temp2 = testDHT->readTemperature();
        float hum2 = testDHT->readHumidity();
        Serial.printf("  温度：%.1f °C (isnan: %s)\n", 
                      temp2, isnan(temp2) ? "true" : "false");
        Serial.printf("  湿度：%.1f %% (isnan: %s)\n", 
                      hum2, isnan(hum2) ? "true" : "false");

        // 统计信息
        Serial.println("\n[统计信息]");
        Serial.printf("  成功次数：%d\n", successCount);
        Serial.printf("  失败次数：%d\n", failCount);
        if (testNumber > 0) {
            float successRate = (successCount * 100.0) / testNumber;
            Serial.printf("  成功率：%.1f%%\n", successRate);
        }

        // 诊断建议
        Serial.println("\n[故障诊断]");
        if (failCount > 0 && successCount == 0) {
            Serial.println("  ⚠️  所有读取均失败，可能原因:");
            Serial.println("      1. 检查接线是否牢固");
            Serial.println("      2. 检查 DHT11 是否需要 10k 上拉电阻");
            Serial.println("      3. 检查电源是否稳定 (3.3V)");
            Serial.println("      4. 确认传感器是否损坏");
            Serial.println("      5. 确认 GPIO4 是否被其他设备占用");
        } else if (failCount > successCount) {
            Serial.println("  ⚠️  失败率超过 50%，可能原因:");
            Serial.println("      1. 接线松动或接触不良");
            Serial.println("      2. 上拉电阻不足（建议 10k）");
            Serial.println("      3. 电源干扰");
        } else if (successCount > 0) {
            Serial.println("  ✓ 传感器工作正常");
        }

        Serial.println("----------------------------------------");
    }

    delay(100);
}
