/**
 * Wi-Fi 连接管理模块 - 头文件
 * ===========================
 *
 * ⚠️ 注意：这是框架代码，需要根据实际网络环境配置。
 *
 * 功能：
 * - Wi-Fi 连接管理
 * - 重连机制
 * - IP 地址获取
 * - 连接状态监控
 * - HTTP 客户端（发送传感器数据到后端）
 *
 * 硬件要求：
 * - ESP32-S3 内置 Wi-Fi
 *
 * 网络要求：
 * - 2.4GHz Wi-Fi 网络（ESP32 不支持 5GHz）
 * - 后端服务器可访问
 *
 * 配置说明：
 * - Wi-Fi SSID 和密码需在代码中配置
 * - 或使用 WiFiManager 进行配网
 *
 * 作者：SmartFan Team
 * 版本：0.1.1
 * 日期：2026-11-11
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"
#ifndef WIFI_SSID
#error "WIFI_SSID is not defined in secrets.h"
#endif

#ifndef WIFI_PASSWORD
#error "WIFI_PASSWORD is not defined in secrets.h"
#endif

#ifndef BACKEND_URL
#error "BACKEND_URL is not defined in secrets.h"
#endif


// Wi-Fi 配置
#define WIFI_CONNECT_TIMEOUT 10000    // 连接超时 10 秒
#define WIFI_RECONNECT_INTERVAL 5000  // 重连间隔 5 秒
#define WIFI_RSSI_THRESHOLD -70       // RSSI 阈值（信号质量）

// HTTP 配置
#define HTTP_TIMEOUT 5000             // HTTP 请求超时 5 秒
#define SENSOR_DATA_INTERVAL 2000     // 传感器数据上报间隔 2 秒

// 指令轮询配置（Phase 3.2.2）
#define COMMAND_POLL_INTERVAL 1000    // 指令轮询间隔 1 秒
#define MAX_PROCESSED_COMMANDS 10     // 已处理 command_id 缓存最大数量

/**
 * Wi-Fi 连接状态枚举
 */
enum class WifiStatus {
    DISCONNECTED,     // 未连接
    CONNECTING,       // 连接中
    CONNECTED,        // 已连接
    RECONNECTING,     // 重连中
    AUTH_FAILED,      // 认证失败
    NO_AP_FOUND,      // 找不到 AP
    CONNECT_FAIL,     // 连接失败
    UNKNOWN           // 未知状态
};

/**
 * 辅助函数：将 WifiStatus 转换为字符串（供内部使用）
 */
const char* WifiStatusToString(WifiStatus status);

/**
 * 指令数据结构（Phase 3.2.2 - 修复协议类型不匹配）
 * 
 * 后端返回格式：
 * - command_id: String（如 "1700000000_abc123"）
 * - mode: String（"auto" 或 "manual"）
 * - gear: int（0-5）
 * - expires_at: String（ISO 8601 格式，如 "2026-11-11T12:05:00"）
 * - remaining_ttl_ms: int（剩余 TTL，毫秒）
 */
struct Command {
    String command_id;        // 命令 ID（后端返回字符串格式）
    String mode;              // 模式："auto" 或 "manual"
    int gear;                 // 档位：0-5
    unsigned long expires_at; // 过期时间戳（毫秒，Unix 时间戳）
    int remaining_ttl_ms;     // 剩余 TTL（毫秒，后端计算）
    bool valid;               // 是否有效
    String status;            // 命令状态
    unsigned long receive_time; // 接收时间（millis()，用于去重检查）
    
    /**
     * 判断是否为手动模式
     */
    bool isManualMode() const {
        return mode == "manual";
    }
    
    /**
     * 判断是否为自动模式
     */
    bool isAutoMode() const {
        return mode == "auto";
    }
    
    /**
     * 判断指令是否已过期
     * 使用 millis() 无符号时间差判断，避免溢出问题
     */
    bool isExpired() const {
        if (!valid) return true;
        unsigned long now = millis();
        // 如果当前时间超过过期时间，则已过期
        return (now > expires_at);
    }
    
    /**
     * 获取剩余有效时间（毫秒）
     */
    unsigned long getRemainingTime() const {
        if (!valid) return 0;
        unsigned long now = millis();
        if (now >= expires_at) return 0;
        return expires_at - now;
    }
    
    /**
     * 打印指令信息（调试用）
     */
    void print() const {
        Serial.println(F("=== Command Info ==="));
        Serial.print(F("ID: "));
        Serial.println(command_id);
        Serial.print(F("Mode: "));
        Serial.println(mode);
        Serial.print(F("Gear: "));
        Serial.println(gear);
        Serial.print(F("Valid: "));
        Serial.println(valid ? F("true") : F("false"));
        Serial.print(F("Remaining TTL: "));
        Serial.print(remaining_ttl_ms);
        Serial.println(F(" ms"));
        Serial.println(F("===================="));
    }
};

/**
 * 指令状态枚举
 */
enum class CommandStatus {
    PENDING,          // 待执行
    EXECUTING,        // 执行中
    SUCCESS,          // 执行成功
    FAILED,           // 执行失败
    EXPIRED,          // 已过期
    CANCELLED         // 已取消
};

/**
 * Wi-Fi 管理类
 */
class WifiManager {
public:
    // 类型别名，方便外部使用
    using Status = WifiStatus;
    
    /**
     * 构造函数
     */
    WifiManager();

    /**
     * 初始化 Wi-Fi
     * @param ssid Wi-Fi 名称
     * @param password Wi-Fi 密码
     * @return true 初始化成功
     */
    bool begin(const char* ssid = WIFI_SSID, const char* password = WIFI_PASSWORD);

    /**
     * 连接 Wi-Fi
     * @return true 连接成功
     */
    bool connect();

    /**
     * 断开 Wi-Fi
     */
    void disconnect();

    /**
     * 检查 Wi-Fi 连接状态
     * @return true 已连接
     */
    bool isConnected();

    /**
     * 获取当前连接状态
     * @return 连接状态枚举
     */
    WifiStatus getStatus();

    /**
     * 获取状态描述
     * @return 状态描述字符串
     */
    const char* getStatusString();

    /**
     * 获取 IP 地址
     * @return IP 地址字符串
     */
    String getIPAddress();

    /**
     * 获取信号强度
     * @return RSSI (dBm)
     */
    int getRSSI();

    /**
     * 获取 BSSID
     * @return BSSID 字符串
     */
    String getBSSID();

    /**
     * 设置后端服务器地址
     * @param url 服务器 URL
     */
    void setBackendURL(const String& url);

    /**
     * 获取后端服务器地址
     * @return 服务器 URL
     */
    String getBackendURL();

    /**
     * 发送传感器数据到后端
     * @param temperature 温度值
     * @param humidity 湿度值
     * @param fanGear 当前风扇档位
     * @return true 发送成功
     */
    bool sendSensorData(float temperature, float humidity, int fanGear);

    /**
     * 从后端获取风扇推荐
     * @param temperature 温度值
     * @param humidity 湿度值
     * @param recommendedGear 输出参数：推荐档位
     * @return true 获取成功
     */
    bool getFanRecommendation(float temperature, float humidity, int& recommendedGear);

    /**
     * 检查后端服务是否可用
     * @return true 后端可用
     */
    bool checkBackendStatus();

    /**
     * 处理 Wi-Fi 重连（应在 loop 中调用）
     */
    void handleReconnect();

    /**
     * 获取连接尝试次数
     * @return 尝试次数
     */
    int getConnectAttempts();

    /**
     * 从后端获取待执行指令（Phase 3.2.2）
     * @param command 输出参数：指令对象
     * @return true 获取到有效指令
     */
    bool fetchPendingCommand(Command& command);

    /**
     * 向后端发送指令执行确认（Phase 3.2.2）
     * @param command_id 指令 ID（字符串格式）
     * @param success 执行是否成功
     * @param message 执行结果消息
     * @return true 发送成功
     */
    bool ackCommand(const String& command_id, bool success, const String& message = "");

    /**
     * 处理指令轮询（Phase 3.2.2，应在 loop 中调用）
     * @param command 输出参数：获取到的有效指令对象
     * @return true 获取到新指令（已通过去重和过期检查）
     */
    bool handleCommandPolling(Command& command);

private:
    String _ssid;
    String _password;
    String _backendURL;
    WifiStatus _status;
    int _connectAttempts;
    unsigned long _lastReconnectTime;
    unsigned long _lastConnectionTime;

    // 指令轮询相关（Phase 3.2.2）
    unsigned long _lastCommandPollTime;              // 上次指令轮询时间
    String _processedCommandIds[MAX_PROCESSED_COMMANDS];  // 已处理的 command_id 列表（String 类型）
    int _processedCommandCount;                      // 已处理命令数量
    String _lastProcessedCommandId;                  // 上次处理的命令 ID（用于重复 ACK 处理）

    /**
     * 更新状态
     */
    void _updateStatus(WifiStatus newStatus);

    /**
     * 执行 HTTP POST 请求
     * @param url 请求 URL
     * @param payload JSON 数据
     * @return HTTP 状态码
     */
    int _sendHTTPPost(const String& url, const String& payload);

    /**
     * 执行 HTTP GET 请求
     * @param url 请求 URL
     * @param response 输出参数：响应内容
     * @return HTTP 状态码
     */
    int _sendHTTPGet(const String& url, String& response);

    /**
     * 根据 WiFi.status() 返回值映射到 WifiStatus
     */
    WifiStatus _mapWiFiStatus(uint8_t status);

    /**
     * 检查 command_id 是否已处理过
     * @param command_id 指令 ID（字符串格式）
     * @return true 已处理
     */
    bool _isCommandProcessed(const String& command_id);

    /**
     * 记录已处理的 command_id
     * @param command_id 指令 ID（字符串格式）
     */
    void _recordProcessedCommand(const String& command_id);

    /**
     * 解析 JSON 响应获取指令
     * @param json JSON 响应字符串
     * @param command 输出参数：指令对象
     * @return true 解析成功
     */
    bool _parseCommandJSON(const String& json, Command& command);
};

#endif // WIFI_MANAGER_H