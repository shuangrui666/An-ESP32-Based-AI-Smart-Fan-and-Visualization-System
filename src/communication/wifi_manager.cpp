/**
 * Wi-Fi 连接管理模块
 * ==================
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

#include "wifi_manager.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ==================== 实现 ====================

WifiManager::WifiManager()
    : _backendURL(BACKEND_URL)
    , _status(WifiStatus::DISCONNECTED)
    , _connectAttempts(0)
    , _lastReconnectTime(0)
    , _lastConnectionTime(0)
    , _lastCommandPollTime(0)
    , _processedCommandCount(0)
    , _lastProcessedCommandId("")
{
    // 初始化已处理命令 ID 数组（String 类型）
    for (int i = 0; i < MAX_PROCESSED_COMMANDS; i++) {
        _processedCommandIds[i] = "";
    }
}

bool WifiManager::begin(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;

    // 设置 Wi-Fi 模式为 Station 模式
    WiFi.mode(WIFI_STA);

    // 启用自动重连（ESP32 Arduino 核心库支持）
    WiFi.setAutoReconnect(true);

    Serial.println("[WifiManager] Initialized");
    Serial.printf("[WifiManager] SSID: %s\n", ssid);

    return true;
}

bool WifiManager::connect() {
    if (_status == WifiStatus::CONNECTED) {
        return true;
    }

    _updateStatus(WifiStatus::CONNECTING);
    _connectAttempts = 0;

    Serial.print("[WifiManager] Connecting to ");
    Serial.println(_ssid);

    // 开始连接
    WiFi.begin(_ssid.c_str(), _password.c_str());

    unsigned long startTime = millis();

    // 等待连接
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);

        _connectAttempts++;

        if (millis() - startTime > WIFI_CONNECT_TIMEOUT) {
            Serial.println("[WifiManager] Connection timeout");

            // 根据 WiFi.status() 判断失败原因
            uint8_t wifiStatus = WiFi.status();
            if (wifiStatus == WL_CONNECT_FAILED) {
                _updateStatus(WifiStatus::CONNECT_FAIL);
            } else if (wifiStatus == WL_IDLE_STATUS || wifiStatus == WL_DISCONNECTED) {
                _updateStatus(WifiStatus::CONNECT_FAIL);
            } else {
                // 其他情况视为认证失败或 AP 未找到
                _updateStatus(WifiStatus::AUTH_FAILED);
            }

            return false;
        }
    }

    _lastConnectionTime = millis();
    _updateStatus(WifiStatus::CONNECTED);

    Serial.println("[WifiManager] Connected!");
    Serial.printf("[WifiManager] IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("[WifiManager] RSSI: %d dBm\n", WiFi.RSSI());

    return true;
}

void WifiManager::disconnect() {
    WiFi.disconnect();
    _updateStatus(WifiStatus::DISCONNECTED);
    Serial.println("[WifiManager] Disconnected");
}

bool WifiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

WifiStatus WifiManager::getStatus() {
    uint8_t wifiStatus = WiFi.status();

    if (wifiStatus != WL_CONNECTED) {
        // WiFi 已断开，更新内部状态
        if (_status == WifiStatus::CONNECTED) {
            _updateStatus(WifiStatus::DISCONNECTED);
        }
        // 根据当前 WiFi 状态映射
        return _mapWiFiStatus(wifiStatus);
    }

    // WiFi 已连接，确保内部状态一致
    if (_status != WifiStatus::CONNECTED && _status != WifiStatus::CONNECTING) {
        _updateStatus(WifiStatus::CONNECTED);
    }

    return _status;
}

const char* WifiManager::getStatusString() {
    switch (_status) {
        case WifiStatus::DISCONNECTED: return "Disconnected";
        case WifiStatus::CONNECTING: return "Connecting...";
        case WifiStatus::CONNECTED: return "Connected";
        case WifiStatus::RECONNECTING: return "Reconnecting...";
        case WifiStatus::AUTH_FAILED: return "Auth Failed";
        case WifiStatus::NO_AP_FOUND: return "AP Not Found";
        case WifiStatus::CONNECT_FAIL: return "Connect Failed";
        default: return "Unknown";
    }
}

String WifiManager::getIPAddress() {
    if (isConnected()) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

int WifiManager::getRSSI() {
    return WiFi.RSSI();
}

String WifiManager::getBSSID() {
    if (isConnected()) {
        return WiFi.BSSIDstr();
    }
    return "";
}

void WifiManager::setBackendURL(const String& url) {
    _backendURL = url;
}

String WifiManager::getBackendURL() {
    return _backendURL;
}

void WifiManager::_updateStatus(WifiStatus newStatus) {
    if (_status != newStatus) {
        Serial.printf("[WifiManager] Status: %s -> %s\n",
                      getStatusString(),
                      WifiStatusToString(newStatus));
        _status = newStatus;
    }
}

/**
 * 将 WiFi.status() 返回值映射到 WifiStatus 枚举
 */
WifiStatus WifiManager::_mapWiFiStatus(uint8_t status) {
    switch (status) {
        case WL_NO_SHIELD:
            return WifiStatus::NO_AP_FOUND;
        case WL_IDLE_STATUS:
            return WifiStatus::CONNECTING;
        case WL_CONNECT_FAILED:
            return WifiStatus::CONNECT_FAIL;
        case WL_CONNECTION_LOST:
            return WifiStatus::DISCONNECTED;
        case WL_SCAN_COMPLETED:
        case WL_DISCONNECTED:
            return WifiStatus::DISCONNECTED;
        default:
            return WifiStatus::UNKNOWN;
    }
}

/**
 * 辅助函数：将 WifiStatus 转换为字符串（供外部使用）
 */
const char* WifiStatusToString(WifiStatus status) {
    switch (status) {
        case WifiStatus::DISCONNECTED: return "Disconnected";
        case WifiStatus::CONNECTING: return "Connecting";
        case WifiStatus::CONNECTED: return "Connected";
        case WifiStatus::RECONNECTING: return "Reconnecting";
        case WifiStatus::AUTH_FAILED: return "Auth Failed";
        case WifiStatus::NO_AP_FOUND: return "AP Not Found";
        case WifiStatus::CONNECT_FAIL: return "Connect Failed";
        default: return "Unknown";
    }
}

int WifiManager::_sendHTTPPost(const String& url, const String& payload) {
    if (!isConnected()) {
        return -1;
    }

    HTTPClient http;
    http.setConnectTimeout(HTTP_TIMEOUT);  // 连接超时
    http.setTimeout(HTTP_TIMEOUT);          // 响应超时

    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.POST(payload);

    http.end();

    return httpCode;
}

int WifiManager::_sendHTTPGet(const String& url, String& response) {
    if (!isConnected()) {
        return -1;
    }

    HTTPClient http;
    http.setConnectTimeout(HTTP_TIMEOUT);  // 连接超时
    http.setTimeout(HTTP_TIMEOUT);          // 响应超时

    http.begin(url);

    int httpCode = http.GET();

    if (httpCode > 0) {
        response = http.getString();
    }

    http.end();

    return httpCode;
}

bool WifiManager::sendSensorData(float temperature, float humidity, int fanGear) {
    if (!isConnected()) {
        Serial.println("[WifiManager] Not connected, cannot send data");
        return false;
    }

    // 构建 JSON 数据
    String payload = String("{\"temperature\":") + temperature +
                     ",\"humidity\":" + humidity +
                     ",\"fan_gear\":" + fanGear + "}";

    String url = _backendURL + "/api/sensor/data";

    Serial.printf("[WifiManager] Sending data to %s\n", url.c_str());
    Serial.printf("[WifiManager] Payload: %s\n", payload.c_str());

    int httpCode = _sendHTTPPost(url, payload);

    if (httpCode == 200 || httpCode == 201) {
        Serial.println("[WifiManager] Data sent successfully");
        return true;
    } else {
        Serial.printf("[WifiManager] Send failed, HTTP code: %d\n", httpCode);
        return false;
    }
}

bool WifiManager::getFanRecommendation(float temperature, float humidity, int& recommendedGear) {
    if (!isConnected()) {
        return false;
    }

    String url = String(_backendURL) + "/api/fan/recommendation?temperature=" +
                 temperature + "&humidity=" + humidity;

    Serial.printf("[WifiManager] Getting recommendation from %s\n", url.c_str());

    String response;
    int httpCode = _sendHTTPGet(url, response);

    if (httpCode == 200) {
        // 解析 JSON 响应（简单解析，实际应使用 ArduinoJson）
        int gearPos = response.indexOf("\"gear\":");
        if (gearPos > 0) {
            String gearStr = response.substring(gearPos + 7);
            gearStr = gearStr.substring(0, gearStr.indexOf(","));
            recommendedGear = gearStr.toInt();
            Serial.printf("[WifiManager] Recommendation: gear %d\n", recommendedGear);
            return true;
        }
    }

    Serial.printf("[WifiManager] Get recommendation failed, HTTP code: %d\n", httpCode);
    return false;
}

bool WifiManager::checkBackendStatus() {
    if (!isConnected()) {
        return false;
    }

    String url = _backendURL + "/api/status";
    String response;

    int httpCode = _sendHTTPGet(url, response);

    return (httpCode == 200);
}

void WifiManager::handleReconnect() {
    if (isConnected()) {
        return;
    }

    unsigned long currentTime = millis();
    if (currentTime - _lastReconnectTime < WIFI_RECONNECT_INTERVAL) {
        return;
    }

    _lastReconnectTime = currentTime;

    if (_status != WifiStatus::RECONNECTING) {
        _updateStatus(WifiStatus::RECONNECTING);
        Serial.println("[WifiManager] Reconnecting...");
    }

    connect();
}

int WifiManager::getConnectAttempts() {
    return _connectAttempts;
}

// ==================== 指令轮询功能实现（Phase 3.2.2）====================

bool WifiManager::fetchPendingCommand(Command& command) {
    // ========== [FETCH] 开始阶段 ==========
    Serial.println("[WifiManager] [FETCH] ========================================");
    Serial.println("[WifiManager] [FETCH] Starting command fetch cycle");
    
    if (!isConnected()) {
        Serial.println("[WifiManager] [FETCH] ERROR: Not connected, cannot fetch command");
        command.valid = false;
        return false;
    }
    Serial.println("[WifiManager] [FETCH] Wi-Fi connection: OK");

    String url = String(_backendURL) + "/api/fan/command?esp32_id=" + String(DEVICE_ID);
    String response;

    // ========== [FETCH] 请求信息 ==========
    Serial.printf("[WifiManager] [FETCH] Request URL: %s\n", url.c_str());
    Serial.printf("[WifiManager] [FETCH] Backend URL: %s\n", _backendURL.c_str());
    Serial.printf("[WifiManager] [FETCH] Request method: GET\n");
    Serial.printf("[WifiManager] [FETCH] Timeout: %d ms\n", HTTP_TIMEOUT * 1000);

    int httpCode = _sendHTTPGet(url, response);

    // ========== [FETCH] 响应信息 ==========
    Serial.printf("[WifiManager] [FETCH] HTTP response code: %d\n", httpCode);
    Serial.printf("[WifiManager] [FETCH] Response length: %d bytes\n", response.length());
    
    if (response.length() > 0) {
        Serial.printf("[WifiManager] [FETCH] Raw response: \"%s\"\n", response.c_str());
    } else {
        Serial.println("[WifiManager] [FETCH] WARNING: Empty response body");
    }

    // ========== [FETCH] 响应有效性检查 ==========
    if (httpCode == 200) {
        // 检查响应是否为空
        if (response.length() == 0) {
            Serial.println("[WifiManager] [FETCH] INFO: Empty response, no command available");
            command.valid = false;
            return false;
        }

        // 检查是否包含 command 字段
        if (response.indexOf("\"command\"") < 0) {
            Serial.println("[WifiManager] [FETCH] ERROR: Response missing command field");
            Serial.printf("[WifiManager] [FETCH] Response content: %s\n", response.c_str());
            command.valid = false;
            return false;
        }

        // 检查 command 是否为 null
        if (response.indexOf("\"command\": null") >= 0 || response.indexOf("\"command\":null") >= 0) {
            Serial.println("[WifiManager] [FETCH] INFO: No pending command (command is null)");
            command.valid = false;
            return false;
        }

        Serial.println("[WifiManager] [FETCH] Response validation: PASSED");
        return _parseCommandJSON(response, command);
    } else {
        // ========== [FETCH] 错误诊断 ==========
        Serial.printf("[WifiManager] [FETCH] ERROR: Fetch command failed with HTTP code: %d\n", httpCode);
        
        if (httpCode < 0) {
            // HTTP 客户端错误
            Serial.println("[WifiManager] [FETCH] ERROR TYPE: HTTP client error (negative code)");
            if (httpCode == HTTPC_ERROR_CONNECTION_REFUSED) {
                Serial.println("[WifiManager] [FETCH] DETAIL: Connection refused by server");
            } else if (httpCode == HTTPC_ERROR_SEND_HEADER_FAILED) {
                Serial.println("[WifiManager] [FETCH] DETAIL: Failed to send HTTP header");
            } else if (httpCode == HTTPC_ERROR_READ_TIMEOUT) {
                Serial.println("[WifiManager] [FETCH] DETAIL: Response read timeout");
            } else if (httpCode == HTTPC_ERROR_CONNECTION_LOST) {
                Serial.println("[WifiManager] [FETCH] DETAIL: Connection lost during request");
            } else {
                Serial.printf("[WifiManager] [FETCH] DETAIL: Unknown HTTP client error code: %d\n", httpCode);
            }
        } else if (httpCode >= 400 && httpCode < 500) {
            // 客户端错误（4xx）
            Serial.println("[WifiManager] [FETCH] ERROR TYPE: Client error (4xx)");
            if (httpCode == 400) {
                Serial.println("[WifiManager] [FETCH] DETAIL: Bad Request - check request format");
            } else if (httpCode == 401) {
                Serial.println("[WifiManager] [FETCH] DETAIL: Unauthorized - authentication required");
            } else if (httpCode == 403) {
                Serial.println("[WifiManager] [FETCH] DETAIL: Forbidden - access denied");
            } else if (httpCode == 404) {
                Serial.println("[WifiManager] [FETCH] DETAIL: Not Found - endpoint may not exist");
            } else {
                Serial.printf("[WifiManager] [FETCH] DETAIL: Client error code: %d\n", httpCode);
            }
        } else if (httpCode >= 500) {
            // 服务器错误（5xx）
            Serial.println("[WifiManager] [FETCH] ERROR TYPE: Server error (5xx)");
            Serial.printf("[WifiManager] [FETCH] DETAIL: Backend server returned error code: %d\n", httpCode);
        }
        
        command.valid = false;
        return false;
    }
}

bool WifiManager::ackCommand(const String& command_id, bool success, const String& message) {
    if (!isConnected()) {
        Serial.println("[WifiManager] Not connected, cannot send ACK");
        return false;
    }

    String url = String(_backendURL) + "/api/fan/command/ack";

    // 构建 JSON 数据（command_id 为字符串）
    String payload = String("{\"command_id\":\"") + command_id +
                     "\",\"success\":" + (success ? "true" : "false");

    if (message.length() > 0) {
        payload += ",\"message\":\"" + message + "\"";
    }
    payload += "}";

    Serial.printf("[WifiManager] Sending ACK to %s\n", url.c_str());
    Serial.printf("[WifiManager] ACK payload: %s\n", payload.c_str());

    int httpCode = _sendHTTPPost(url, payload);

    if (httpCode == 200 || httpCode == 201) {
        Serial.println("[WifiManager] ACK sent successfully");
        return true;
    } else {
        Serial.printf("[WifiManager] ACK failed, HTTP code: %d\n", httpCode);
        return false;
    }
}

bool WifiManager::handleCommandPolling(Command& command) {
    // ========== [POLL] 轮询开始 ==========
    Serial.println("[WifiManager] [POLL] ========================================");
    Serial.println("[WifiManager] [POLL] Starting command polling cycle");

    unsigned long currentTime = millis();
    unsigned long timeSinceLastPoll = currentTime - _lastCommandPollTime;

    // 检查是否到达轮询间隔
    if (currentTime - _lastCommandPollTime < COMMAND_POLL_INTERVAL) {
        Serial.printf("[WifiManager] [POLL] SKIP: Too early, %lu ms since last poll (interval: %d ms)\n",
                      timeSinceLastPoll, COMMAND_POLL_INTERVAL);
        command.valid = false;
        return false;
    }
    Serial.printf("[WifiManager] [POLL] OK: Poll interval reached (%lu ms >= %d ms)\n",
                  timeSinceLastPoll, COMMAND_POLL_INTERVAL);
    Serial.printf("[WifiManager] [POLL] Last poll time: %lu ms ago\n", timeSinceLastPoll);

    _lastCommandPollTime = currentTime;

    // 检查 Wi-Fi 连接
    if (!isConnected()) {
        Serial.println("[WifiManager] [POLL] SKIP: Wi-Fi not connected");
        command.valid = false;
        return false;
    }
    Serial.println("[WifiManager] [POLL] Wi-Fi connection: OK");

    // 获取待执行指令
    Serial.println("[WifiManager] [POLL] --- Fetching command ---");
    if (!fetchPendingCommand(command)) {
        // 网络请求失败，但不视为错误（可能是没有指令）
        Serial.println("[WifiManager] [POLL] Result: No command fetched (fetchPendingCommand returned false)");
        Serial.println("[WifiManager] [POLL] Polling cycle completed");
        command.valid = false;
        return false;
    }

    // 检查指令是否有效
    if (!command.valid) {
        Serial.println("[WifiManager] [POLL] Result: No valid command (command.valid = false)");
        Serial.println("[WifiManager] [POLL] Polling cycle completed");
        return false;
    }
    Serial.println("[WifiManager] [POLL] Command validity: OK");

    // 检查命令是否已处理过（去重）
    Serial.println("[WifiManager] [POLL] --- Duplicate check ---");
    if (_isCommandProcessed(command.command_id)) {
        Serial.printf("[WifiManager] [POLL] DUPLICATE: Command %s already processed\n",
                      command.command_id.c_str());
        Serial.println("[WifiManager] [POLL] Polling cycle completed (duplicate discarded)");
        return false;
    }
    Serial.printf("[WifiManager] [POLL] DUPLICATE CHECK: PASSED (command %s is new)\n",
                  command.command_id.c_str());

    // 检查指令是否过期
    Serial.println("[WifiManager] [POLL] --- Expiration check ---");
    if (command.isExpired()) {
        Serial.printf("[WifiManager] [POLL] EXPIRED: Command %s has expired\n",
                      command.command_id.c_str());
        Serial.printf("[WifiManager] [POLL]   - expires_at: %lu\n", command.expires_at);
        Serial.printf("[WifiManager] [POLL]   - receive_time: %lu\n", command.receive_time);
        Serial.printf("[WifiManager] [POLL]   - remaining_ttl_ms: %d\n", command.remaining_ttl_ms);
        Serial.println("[WifiManager] [POLL] Polling cycle completed (expired command discarded)");
        return false;
    }
    Serial.println("[WifiManager] [POLL] EXPIRATION CHECK: PASSED (command is valid)");

    Serial.printf("[WifiManager] [POLL] RECEIVED: New valid command\n");
    Serial.printf("[WifiManager] [POLL]   - id: %s\n", command.command_id.c_str());
    Serial.printf("[WifiManager] [POLL]   - mode: %s\n", command.mode.c_str());
    Serial.printf("[WifiManager] [POLL]   - gear: %d\n", command.gear);

    // 记录已处理的命令 ID（用于去重）
    Serial.println("[WifiManager] [POLL] --- Recording command ---");
    _recordProcessedCommand(command.command_id);
    _lastProcessedCommandId = command.command_id;

    Serial.println("[WifiManager] [POLL] ========================================");
    Serial.println("[WifiManager] [POLL] Polling cycle completed successfully");
    Serial.printf("[WifiManager] [POLL] Result: true (new command ready for execution)\n");

    // 返回 true 表示获取到新指令，Command 对象已通过引用参数返回
    // 由 main.cpp 中的 handleRemoteCommand() 负责执行和发送 ACK
    return true;
}

bool WifiManager::_isCommandProcessed(const String& command_id) {
    Serial.printf("[WifiManager] [DEDUP] Checking if command %s was processed\n", 
                  command_id.c_str());
    Serial.printf("[WifiManager] [DEDUP] Processed command array size: %d\n", 
                  _processedCommandCount);
    
    if (_processedCommandCount == 0) {
        Serial.println("[WifiManager] [DEDUP] Array is empty, command is NEW");
        return false;
    }
    
    for (int i = 0; i < _processedCommandCount; i++) {
        Serial.printf("[WifiManager] [DEDUP]   [%d] %s\n", i, _processedCommandIds[i].c_str());
        if (_processedCommandIds[i] == command_id) {
            Serial.printf("[WifiManager] [DEDUP] MATCH found at index %d, command is DUPLICATE\n", i);
            return true;
        }
    }
    
    Serial.println("[WifiManager] [DEDUP] No match found, command is NEW");
    return false;
}

void WifiManager::_recordProcessedCommand(const String& command_id) {
    // 如果数组已满，移除最旧的记录（队列式管理）
    if (_processedCommandCount >= MAX_PROCESSED_COMMANDS) {
        // 移动数组，移除第一个元素
        for (int i = 0; i < MAX_PROCESSED_COMMANDS - 1; i++) {
            _processedCommandIds[i] = _processedCommandIds[i + 1];
        }
        _processedCommandIds[MAX_PROCESSED_COMMANDS - 1] = command_id;
    } else {
        _processedCommandIds[_processedCommandCount++] = command_id;
    }

    Serial.printf("[WifiManager] Recorded command %s, total processed: %d\n",
                  command_id.c_str(), _processedCommandCount);
}

bool WifiManager::_parseCommandJSON(const String& json, Command& command) {
    // ========== [PARSE] 开始阶段 ==========
    Serial.println("[WifiManager] [PARSE] ========================================");
    Serial.println("[WifiManager] [PARSE] Starting JSON command parsing");
    Serial.printf("[WifiManager] [PARSE] JSON input: %s\n", json.c_str());

    // 初始化命令对象
    command.valid = false;
    command.command_id = "";
    command.mode = "";
    command.gear = 0;
    command.status = "";
    command.expires_at = 0;
    command.remaining_ttl_ms = 0;
    command.receive_time = millis();

    // 简单 JSON 解析（不使用 ArduinoJson，减少依赖）

    // ========== [PARSE] 解析 command_id ==========
    Serial.println("[WifiManager] [PARSE] --- Parsing command_id ---");
    int idPos = json.indexOf("\"command_id\":");
    if (idPos < 0) {
        Serial.println("[WifiManager] [PARSE] ERROR: command_id field not found");
        return false;
    }
    Serial.printf("[WifiManager] [PARSE] command_id field found at position: %d\n", idPos);
    String idStr = json.substring(idPos + 13);
    idStr.trim();
    // 去掉开头的引号
    if (idStr.startsWith("\"")) {
        idStr = idStr.substring(1);
    }
    // 找到结束引号或逗号或右括号
    int endPos = idStr.indexOf("\"");
    if (endPos < 0) {
        endPos = idStr.indexOf(",");
    }
    if (endPos < 0) {
        endPos = idStr.indexOf("}");
    }
    if (endPos > 0) {
        idStr = idStr.substring(0, endPos);
    }
    idStr.trim();
    command.command_id = idStr;
    Serial.printf("[WifiManager] [PARSE] command_id extracted: \"%s\"\n", command.command_id.c_str());

    // ========== [PARSE] 解析 mode ==========
    Serial.println("[WifiManager] [PARSE] --- Parsing mode ---");
    int modePos = json.indexOf("\"mode\":");
    if (modePos > 0) {
        Serial.printf("[WifiManager] [PARSE] mode field found at position: %d\n", modePos);
        String modeStr = json.substring(modePos + 7);
        modeStr.trim();
        // 去掉开头的引号
        if (modeStr.startsWith("\"")) {
            modeStr = modeStr.substring(1);
        }
        // 找到结束引号
        int modeEnd = modeStr.indexOf("\"");
        if (modeEnd > 0) {
            modeStr = modeStr.substring(0, modeEnd);
        }
        modeStr.trim();
        command.mode = modeStr;
        Serial.printf("[WifiManager] [PARSE] mode extracted: \"%s\"\n", command.mode.c_str());
    } else {
        Serial.println("[WifiManager] [PARSE] WARNING: mode field not found, using empty string");
    }

    // ========== [PARSE] 解析 gear ==========
    Serial.println("[WifiManager] [PARSE] --- Parsing gear ---");
    int gearPos = json.indexOf("\"gear\":");
    if (gearPos > 0) {
        Serial.printf("[WifiManager] [PARSE] gear field found at position: %d\n", gearPos);
        String gearStr = json.substring(gearPos + 7);
        gearStr = gearStr.substring(0, gearStr.indexOf(","));
        gearStr.trim();
        int closeParen = gearStr.indexOf("}");
        if (closeParen > 0) {
            gearStr = gearStr.substring(0, closeParen);
        }
        gearStr.trim();
        command.gear = gearStr.toInt();
        Serial.printf("[WifiManager] [PARSE] gear extracted: %d (raw: \"%s\")\n", command.gear, gearStr.c_str());
    } else {
        Serial.println("[WifiManager] [PARSE] WARNING: gear field not found, using 0");
    }

    // ========== [PARSE] 解析 status ==========
    Serial.println("[WifiManager] [PARSE] --- Parsing status ---");
    int statusPos = json.indexOf("\"status\":");
    if (statusPos > 0) {
        Serial.printf("[WifiManager] [PARSE] status field found at position: %d\n", statusPos);
        String statusStr = json.substring(statusPos + 9);
        int quoteEnd = statusStr.indexOf("\"");
        if (quoteEnd > 0) {
            command.status = statusStr.substring(0, quoteEnd);
            Serial.printf("[WifiManager] [PARSE] status extracted: \"%s\"\n", command.status.c_str());
        } else {
            Serial.println("[WifiManager] [PARSE] WARNING: status field value not properly quoted");
        }
    } else {
        Serial.println("[WifiManager] [PARSE] WARNING: status field not found");
    }

    // ========== [PARSE] 解析 remaining_ttl_ms（优先使用）==========
    Serial.println("[WifiManager] [PARSE] --- Parsing remaining_ttl_ms (priority) ---");
    int ttlPos = json.indexOf("\"remaining_ttl_ms\":");
    if (ttlPos > 0) {
        Serial.printf("[WifiManager] [PARSE] remaining_ttl_ms field found at position: %d\n", ttlPos);
        String ttlStr = json.substring(ttlPos + 19);
        ttlStr = ttlStr.substring(0, ttlStr.indexOf(","));
        ttlStr.trim();
        int closeParen2 = ttlStr.indexOf("}");
        if (closeParen2 > 0) {
            ttlStr = ttlStr.substring(0, closeParen2);
        }
        ttlStr.trim();
        command.remaining_ttl_ms = ttlStr.toInt();
        Serial.printf("[WifiManager] [PARSE] remaining_ttl_ms extracted: %d (raw: \"%s\")\n",
                      command.remaining_ttl_ms, ttlStr.c_str());
    } else {
        Serial.println("[WifiManager] [PARSE] INFO: remaining_ttl_ms field not present, using default 30s");
        command.remaining_ttl_ms = 30000; // 默认 30 秒
    }

    // ========== [PARSE] 解析 expires_at（仅用于日志，实际过期时间用 remaining_ttl_ms 计算）==========
    Serial.println("[WifiManager] [PARSE] --- Parsing expires_at (for logging only) ---");
    int expiresPos = json.indexOf("\"expires_at\":");
    if (expiresPos > 0) {
        Serial.printf("[WifiManager] [PARSE] expires_at field found at position: %d\n", expiresPos);
        String expiresStr = json.substring(expiresPos + 13);
        expiresStr.trim();
        // 去掉开头的引号
        if (expiresStr.startsWith("\"")) {
            expiresStr = expiresStr.substring(1);
        }
        // 找到结束引号
        int expiresEnd = expiresStr.indexOf("\"");
        if (expiresEnd > 0) {
            expiresStr = expiresStr.substring(0, expiresEnd);
        }
        expiresStr.trim();
        Serial.printf("[WifiManager] [PARSE] expires_at (backend): \"%s\"\n", expiresStr.c_str());

        // 尝试直接解析为数字（毫秒时间戳）
        if (expiresStr.length() > 0) {
            unsigned long backendExpiresAt = expiresStr.toInt();
            Serial.printf("[WifiManager] [PARSE] expires_at (backend numeric): %lu\n", backendExpiresAt);
        }
    } else {
        Serial.println("[WifiManager] [PARSE] WARNING: expires_at field not found");
    }

    // ========== [PARSE] 使用 remaining_ttl_ms 计算本地过期时间 ==========
    Serial.println("[WifiManager] [PARSE] --- Calculating local expires_at from remaining_ttl_ms ---");
    command.expires_at = millis() + command.remaining_ttl_ms;
    Serial.printf("[WifiManager] [PARSE] Local expires_at calculated: millis() + %d = %lu\n",
                  command.remaining_ttl_ms, command.expires_at);

    // ========== [PARSE] 解析完成 ==========
    command.valid = true;
    Serial.println("[WifiManager] [PARSE] ========================================");
    Serial.println("[WifiManager] [PARSE] JSON parsing completed successfully");
    Serial.printf("[WifiManager] [PARSE] Final command object:\n");
    Serial.printf("[WifiManager] [PARSE]   - command_id: %s\n", command.command_id.c_str());
    Serial.printf("[WifiManager] [PARSE]   - mode: %s\n", command.mode.c_str());
    Serial.printf("[WifiManager] [PARSE]   - gear: %d\n", command.gear);
    Serial.printf("[WifiManager] [PARSE]   - status: %s\n", command.status.c_str());
    Serial.printf("[WifiManager] [PARSE]   - expires_at: %lu\n", command.expires_at);
    Serial.printf("[WifiManager] [PARSE]   - remaining_ttl_ms: %d\n", command.remaining_ttl_ms);
    Serial.printf("[WifiManager] [PARSE]   - receive_time: %lu\n", command.receive_time);

    return true;
}

// ==================== 全局实例（可选）====================

// WifiManager wifiManager;
