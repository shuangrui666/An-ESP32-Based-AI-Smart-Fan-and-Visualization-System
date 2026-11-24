/**
 * AI 接口模块
 * ==========
 * 
 * ⚠️ 注意：这是框架代码，AI 模型运行在后端服务器。
 * 
 * 功能：
 * - 与后端 AI 服务通信
 * - 获取风扇控制推荐
 * - 模型状态查询
 * - 离线降级策略
 * 
 * 架构说明：
 * - AI 模型（决策树）运行在 Python 后端
 * - ESP32 通过 HTTP API 获取推荐
 * - 支持离线本地规则降级
 * 
 * AI 模型要求：
 * - 输入：温度、湿度
 * - 输出：推荐风扇档位（0-5）
 * - 模型格式：scikit-learn 训练，pickle 保存
 * 
 * 作者：SmartFan Team
 * 版本：0.1.0
 * 日期：2026-11-11
 */

#include <Arduino.h>

// 本地规则引擎配置（当 AI 不可用时使用）
#define AI_FALLBACK_ENABLED true

// 简单的规则引擎阈值（摄氏度，百分比）
// 格式：{温度下限，温度上限，湿度下限，湿度上限，推荐档位}
struct Rule {
    float tempMin;
    float tempMax;
    float humMin;
    float humMax;
    int gear;
};

// 本地规则表（可根据实验数据调整）
static const Rule LOCAL_RULES[] = {
    {0, 22, 0, 100, 0},     // 温度<22°C：关闭
    {22, 25, 0, 60, 0},     // 22-25°C，湿度<60%：关闭
    {22, 25, 60, 100, 1},   // 22-25°C，湿度>=60%：1 档
    {25, 28, 0, 100, 1},    // 25-28°C：1 档
    {28, 30, 0, 100, 2},    // 28-30°C：2 档
    {30, 32, 0, 100, 3},    // 30-32°C：3 档
    {32, 35, 0, 100, 4},    // 32-35°C：4 档
    {35, 100, 0, 100, 5},   // >35°C：5 档
};

#define LOCAL_RULE_COUNT (sizeof(LOCAL_RULES) / sizeof(LOCAL_RULES[0]))

// AI 服务状态枚举
enum class AIServiceStatus {
    UNKNOWN,          // 未知
    AVAILABLE,        // 可用
    UNAVAILABLE,      // 不可用
    FALLBACK_ACTIVE,  // 降级到本地规则
    TRAINING_NEEDED   // 需要重新训练
};

/**
 * AI 接口类
 */
class AIInterface {
public:
    /**
     * 构造函数
     */
    AIInterface();
    
    /**
     * 初始化 AI 接口
     * @param wifiManager Wi-Fi 管理器指针
     * @return true 初始化成功
     */
    bool begin(void* wifiManager);
    
    /**
     * 获取 AI 服务状态
     * @return 服务状态
     */
    AIServiceStatus getStatus();
    
    /**
     * 获取状态描述
     * @return 状态描述字符串
     */
    const char* getStatusString();
    
    /**
     * 获取风扇推荐（优先使用 AI，降级到本地规则）
     * @param temperature 温度值
     * @param humidity 湿度值
     * @return 推荐档位（0-5）
     */
    int getRecommendation(float temperature, float humidity);
    
    /**
     * 获取 AI 推荐（需要网络连接）
     * @param temperature 温度值
     * @param humidity 湿度值
     * @return 推荐档位，失败返回 -1
     */
    int getAIRecommendation(float temperature, float humidity);
    
    /**
     * 获取本地规则推荐（离线模式）
     * @param temperature 温度值
     * @param humidity 湿度值
     * @return 推荐档位
     */
    int getLocalRecommendation(float temperature, float humidity);
    
    /**
     * 检查 AI 服务是否可用
     * @return true 可用
     */
    bool isAIAvailable();
    
    /**
     * 设置是否启用降级模式
     * @param enabled true 启用降级
     */
    void setFallbackEnabled(bool enabled);
    
    /**
     * 获取是否正在使用降级模式
     * @return true 正在使用降级
     */
    bool isUsingFallback();
    
    /**
     * 获取当前使用的推荐源
     * @return "AI" 或 "LOCAL"
     */
    const char* getRecommendationSource();
    
    /**
     * 记录推荐结果（用于后续模型优化）
     * @param temperature 温度
     * @param humidity 湿度
     * @param recommendedGear 推荐档位
     * @param actualGear 实际使用档位
     */
    void logRecommendation(float temperature, float humidity, 
                          int recommendedGear, int actualGear);

private:
    void* _wifiManager;
    AIServiceStatus _status;
    bool _fallbackEnabled;
    bool _usingFallback;
    String _currentSource;
    unsigned long _lastCheckTime;
    int _recommendationCount;
    int _fallbackCount;
    
    /**
     * 更新 AI 服务状态
     */
    void _updateStatus();
    
    /**
     * 检查本地规则匹配
     * @param temperature 温度
     * @param humidity 湿度
     * @return 匹配的档位
     */
    int _matchLocalRule(float temperature, float humidity);
};

// ==================== 实现 ====================

AIInterface::AIInterface()
    : _wifiManager(nullptr)
    , _status(AIServiceStatus::UNKNOWN)
    , _fallbackEnabled(AI_FALLBACK_ENABLED)
    , _usingFallback(false)
    , _currentSource("UNKNOWN")
    , _lastCheckTime(0)
    , _recommendationCount(0)
    , _fallbackCount(0)
{
}

bool AIInterface::begin(void* wifiManager) {
    _wifiManager = wifiManager;
    _status = AIServiceStatus::UNKNOWN;
    
    Serial.println("[AIInterface] Initialized");
    Serial.printf("[AIInterface] Fallback enabled: %s\n", _fallbackEnabled ? "true" : "false");
    
    return true;
}

void AIInterface::_updateStatus() {
    // TODO: 实际实现应检查后端 AI 服务状态
    // 这里简化处理，假设 AI 服务总是可用
    _status = AIServiceStatus::AVAILABLE;
}

AIServiceStatus AIInterface::getStatus() {
    unsigned long currentTime = millis();
    if (currentTime - _lastCheckTime > 60000) {  // 每分钟检查一次
        _updateStatus();
        _lastCheckTime = currentTime;
    }
    return _status;
}

const char* AIInterface::getStatusString() {
    switch (_status) {
        case AIServiceStatus::UNKNOWN: return "Unknown";
        case AIServiceStatus::AVAILABLE: return "Available";
        case AIServiceStatus::UNAVAILABLE: return "Unavailable";
        case AIServiceStatus::FALLBACK_ACTIVE: return "Fallback Active";
        case AIServiceStatus::TRAINING_NEEDED: return "Training Needed";
        default: return "Unknown";
    }
}

int AIInterface::getRecommendation(float temperature, float humidity) {
    _recommendationCount++;
    
    // 优先尝试 AI 推荐
    if (_fallbackEnabled && !isAIAvailable()) {
        _usingFallback = true;
        _fallbackCount++;
        int gear = getLocalRecommendation(temperature, humidity);
        _currentSource = "LOCAL";
        Serial.printf("[AIInterface] Using local rule: temp=%.1f, hum=%.1f -> gear %d\n", 
                     temperature, humidity, gear);
        return gear;
    }
    
    int gear = getAIRecommendation(temperature, humidity);
    
    if (gear >= 0) {
        _usingFallback = false;
        _currentSource = "AI";
        Serial.printf("[AIInterface] AI recommendation: temp=%.1f, hum=%.1f -> gear %d\n", 
                     temperature, humidity, gear);
    } else {
        // AI 失败，尝试降级
        if (_fallbackEnabled) {
            _usingFallback = true;
            _fallbackCount++;
            gear = getLocalRecommendation(temperature, humidity);
            _currentSource = "LOCAL";
            Serial.printf("[AIInterface] AI failed, using local rule: gear %d\n", gear);
        } else {
            Serial.println("[AIInterface] AI failed and fallback disabled");
            gear = 0;  // 安全默认值：关闭
        }
    }
    
    return gear;
}

int AIInterface::getAIRecommendation(float temperature, float humidity) {
    // TODO: 调用 WifiManager 的 getFanRecommendation 方法
    // 这里简化处理，返回 -1 表示需要实现
    // 
    // 示例代码（需要 WifiManager 实例）：
    // int recommendedGear = 0;
    // if (((WifiManager*)_wifiManager)->getFanRecommendation(temperature, humidity, recommendedGear)) {
    //     return recommendedGear;
    // }
    
    // 临时返回 -1
    return -1;
}

int AIInterface::getLocalRecommendation(float temperature, float humidity) {
    return _matchLocalRule(temperature, humidity);
}

bool AIInterface::isAIAvailable() {
    // TODO: 实际实现应检查网络连接和后端服务
    // 这里简化处理，假设不可用（需要实现）
    return false;
}

void AIInterface::setFallbackEnabled(bool enabled) {
    _fallbackEnabled = enabled;
    Serial.printf("[AIInterface] Fallback enabled: %s\n", _fallbackEnabled ? "true" : "false");
}

bool AIInterface::isUsingFallback() {
    return _usingFallback;
}

const char* AIInterface::getRecommendationSource() {
    return _currentSource.c_str();
}

int AIInterface::_matchLocalRule(float temperature, float humidity) {
    for (int i = 0; i < LOCAL_RULE_COUNT; i++) {
        const Rule& rule = LOCAL_RULES[i];
        
        if (temperature >= rule.tempMin && 
            temperature < rule.tempMax &&
            humidity >= rule.humMin && 
            humidity <= rule.humMax) {
            return rule.gear;
        }
    }
    
    // 默认规则：关闭
    return 0;
}

void AIInterface::logRecommendation(float temperature, float humidity, 
                                   int recommendedGear, int actualGear) {
    // TODO: 记录到日志或发送到后端用于模型优化
    // 示例：
    // String log = String("[AI] temp=") + temperature + 
    //              ", hum=" + humidity + 
    //              ", recommended=" + recommendedGear +
    //              ", actual=" + actualGear;
    // Serial.println(log);
    
    if (recommendedGear != actualGear) {
        Serial.printf("[AIInterface] Recommendation mismatch: recommended=%d, actual=%d\n", 
                     recommendedGear, actualGear);
    }
}

// ==================== 全局实例（可选）====================

// AIInterface aiInterface;
