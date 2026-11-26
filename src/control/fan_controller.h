/**
 * 风扇控制模块 - 头文件
 * ====================
 *
 * 功能：
 * - 风扇 PWM 调速控制
 * - 五档风速管理（20%、40%、60%、80%、100%）
 * - 软启动/软停止（非阻塞状态机）
 * - 状态机管理（IDLE, SOFT_START, RUNNING, SOFT_STOP）
 *
 * 硬件要求（需确认）：
 * - MOSFET 或继电器模块
 * - 风扇（5V/12V）
 * - 独立电源（建议）
 *
 * 接线说明：
 * - MOSFET Gate → GPIO6（PWM 引脚，已确认）
 * - MOSFET Source → GND
 * - MOSFET Drain → 风扇负极
 * - 风扇正极 → 外部电源正极
 *
 * ⚠️ 重要警告：
 * - 实际硬件接线：ESP32-S3 GPIO6 → MOSFET 模块 TRIG/PWM
 * - 确保风扇电压与电源匹配
 * - 建议使用独立电源为风扇供电
 * - ESP32 GPIO 仅 3.3V，不可直接驱动 5V/12V 设备
 *
 * PWM 参数：
 * - 频率：5kHz（避免可听噪声）
 * - 分辨率：8-bit（0-255）
 *
 * 作者：SmartFan Team
 * 版本：0.2.0
 * 日期：2026-11-20
 */

#ifndef FAN_CONTROLLER_H
#define FAN_CONTROLLER_H

#include <Arduino.h>

// 风扇控制引脚配置
// 实际硬件接线：ESP32-S3 GPIO6 → MOSFET 模块 TRIG/PWM
#ifndef FAN_CONTROL_PIN
#define FAN_CONTROL_PIN 6
#endif

// PWM 配置
#define FAN_PWM_FREQ 5000           // 5kHz（避免可听噪声）
#define FAN_PWM_RESOLUTION 8        // 8-bit 分辨率（0-255）
#define FAN_PWM_MAX_DUTY 255        // 最大占空比

// 风速等级定义
#define FAN_GEAR_OFF 0
#define FAN_GEAR_1 1
#define FAN_GEAR_2 2
#define FAN_GEAR_3 3
#define FAN_GEAR_4 4
#define FAN_GEAR_5 5
#define FAN_MAX_GEAR 5

// 各档位占空比（需根据实际风扇调整）
#define FAN_DUTY_OFF 0
#define FAN_DUTY_GEAR_1 51      // 20%
#define FAN_DUTY_GEAR_2 102     // 40%
#define FAN_DUTY_GEAR_3 153     // 60%
#define FAN_DUTY_GEAR_4 204     // 80%
#define FAN_DUTY_GEAR_5 255     // 100%

// 软启动/软停止默认时间
#define FAN_SOFT_START_TIME_MS 3000     // 软启动 3 秒
#define FAN_SOFT_STOP_TIME_MS 2000      // 软停止 2 秒

/**
 * 风扇状态枚举
 */
enum class FanState {
    IDLE,           // 空闲状态（风扇关闭）
    SOFT_START,     // 软启动中
    RUNNING,        // 正常运行
    SOFT_STOP,      // 软停止中
    PWM_ERROR       // PWM 故障状态（连续失败达到阈值）
};

// PWM 失败阈值
#define MAX_PWM_ERRORS 3  // 连续失败 3 次进入故障状态

/**
 * 风扇控制器类
 * 
 * 非阻塞设计：
 * - 所有操作使用 millis() 计时
 * - softStart/softStop 不阻塞 CPU
 * - 每次调用 update() 更新状态和 PWM
 */
class FanController {
public:
    /**
     * @brief 构造函数
     */
    FanController();
    
    /**
     * @brief 初始化风扇控制
     * @param pin 控制引脚（PWM），默认 FAN_CONTROL_PIN
     * @return true 初始化成功，false 失败
     */
    bool begin(int pin = FAN_CONTROL_PIN);
    
    /**
     * @brief 主循环更新函数（应在 loop() 中周期性调用）
     * @note 非阻塞，根据当前状态更新 PWM 输出
     */
    void update();
    
    /**
     * @brief 启动软启动流程
     * @param targetGear 目标档位（1-5）
     * @param durationMs 软启动时间（毫秒），默认 3 秒
     * @note 如果当前处于 SOFT_STOP 状态，会取消停止并重新启动
     */
    void startSoftStart(int targetGear = FAN_GEAR_3, unsigned long durationMs = FAN_SOFT_START_TIME_MS);
    
    /**
     * @brief 启动软停止流程
     * @param durationMs 软停止时间（毫秒），默认 2 秒
     * @note 如果当前处于 SOFT_START 状态，会取消启动并开始停止
     */
    void startSoftStop(unsigned long durationMs = FAN_SOFT_STOP_TIME_MS);
    
    /**
     * @brief 立即设置风扇档位
     * @param gear 档位（0-5，0=关闭）
     * @return true 表示成功，false 表示失败（如未初始化）
     * @note 会中断软启动/软停止流程
     */
    bool setGear(int gear);
    
    /**
     * @brief 设置风扇开关状态
     * @param on true 开启（恢复到之前档位），false 关闭
     */
    void setPower(bool on);
    
    /**
     * @brief 获取当前风扇状态
     * @return 当前状态枚举
     */
    FanState getState() const;
    
    /**
     * @brief 获取当前状态描述字符串
     * @return 状态描述
     */
    const char* getStateString() const;
    
    /**
     * @brief 获取当前风速等级
     * @return 当前等级（0-5）
     */
    int getCurrentGear() const;
    
    /**
     * @brief 获取当前占空比
     * @return 当前占空比（0-255）
     */
    int getCurrentDutyCycle() const;
    
    /**
     * @brief 风扇是否开启
     * @return true 开启，false 关闭
     */
    bool isPowerOn() const;
    
    /**
     * @brief 获取档位对应的占空比
     * @param gear 档位
     * @return 占空比
     */
    static int getDutyForGear(int gear);
    
    /**
     * @brief 获取档位名称
     * @param gear 档位
     * @return 档位名称字符串
     */
    static const char* getGearName(int gear);
    
    /**
     * @brief 获取状态名称
     * @param state 状态
     * @return 状态名称字符串
     */
    static const char* getStateName(FanState state);
    
    /**
     * @brief 重置 PWM 错误状态（从故障状态恢复）
     * @note 调用 setGear() 成功时会自动重置错误计数器
     */
    void resetPwmError();

private:
    int _pin;
    FanState _state;
    int _currentGear;
    int _targetGear;
    int _currentDuty;
    int _pendingDuty;        // 待写入占空比（PWM 失败时保存）
    bool _retryPending;      // 待重试标记（true 表示 _pendingDuty 尚未成功写入）
    bool _powerOn;
    bool _initialized;
    unsigned long _stateStartTime;
    unsigned long _softStartDuration;
    unsigned long _softStopDuration;
    uint8_t _pwmErrorCount;  // PWM 连续失败计数器
    int _startDuty;          // 软启动起始占空比（用于线性插值计算）

    /**
     * @brief 更新 PWM 输出
     * @return true PWM 写入成功，false 失败（未初始化或 ledcWrite 失败）
     * @note 使用 LEDC API 输出 PWM 信号
     * @note 仅负责 PWM 写入，不负责更新内部状态（由调用方控制）
     */
    bool _updatePWM();
    
    /**
     * @brief 处理软启动状态
     */
    void _handleSoftStart();
    
    /**
     * @brief 处理软停止状态
     */
    void _handleSoftStop();
    
    /**
     * @brief 转换状态
     * @param newState 新状态
     */
    void _transitionTo(FanState newState);
};

#endif // FAN_CONTROLLER_H
