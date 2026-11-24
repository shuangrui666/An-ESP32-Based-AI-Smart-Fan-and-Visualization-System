/**
 * 风扇控制模块 - 实现文件
 * ====================
 *
 * 功能：
 * - 风扇 PWM 调速控制
 * - 五档风速管理（20%、40%、60%、80%、100%）
 * - 软启动/软停止（非阻塞状态机）
 * - 状态机管理（IDLE, SOFT_START, RUNNING, SOFT_STOP）
 *
 * 非阻塞设计：
 * - 所有操作使用 millis() 计时
 * - softStart/softStop 不阻塞 CPU
 * - 每次调用 update() 根据经过时间计算并更新 PWM 占空比
 *
 * PWM 配置：
 * - 频率：5kHz（避免可听噪声）
 * - 分辨率：8-bit（0-255）
 * - 使用 Arduino-ESP32 LEDC API
 *
 * 作者：SmartFan Team
 * 版本：0.2.0
 * 日期：2026-11-20
 */

#include "fan_controller.h"

// LEDC 通道配置
#define FAN_PWM_CHANNEL 0  // 使用通道 0

/**
 * 构造函数
 */
FanController::FanController()
    : _pin(FAN_CONTROL_PIN)
    , _state(FanState::IDLE)
    , _currentGear(FAN_GEAR_OFF)
    , _targetGear(FAN_GEAR_OFF)
    , _currentDuty(FAN_DUTY_OFF)
    , _powerOn(false)
    , _initialized(false)
    , _stateStartTime(0)
    , _softStartDuration(FAN_SOFT_START_TIME_MS)
    , _softStopDuration(FAN_SOFT_STOP_TIME_MS)
{
    // 注意：构造函数可能在 Serial.begin() 之前被调用（全局对象初始化顺序问题）
    // 因此这里的 Serial.println 不会输出任何内容
    // 真正的初始化日志在 begin() 中输出
}

/**
 * 初始化风扇控制
 */
bool FanController::begin(int pin) {
    Serial.println("[FanController] begin() called");
    Serial.printf("[FanController] Pin parameter: %d\n", pin);
    _pin = pin;

    // 配置引脚为 PWM 输出
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);

    // 初始化 LEDC PWM
    // ESP32-S3 使用新的 ledcAttach API
    // ledcAttach(gpio, freq, resolution_bits)
    // 注意：ledcAttach 返回 bool，false 表示引脚配置失败
    if (!ledcAttach(_pin, FAN_PWM_FREQ, FAN_PWM_RESOLUTION)) {
        _initialized = false;
        Serial.println("[FanController] ERROR: ledcAttach() failed");
        Serial.printf("[FanController] Cannot attach PWM to GPIO%d\n", _pin);
        return false;
    }

    // 初始输出为 0
    // ledcWrite(gpio, duty) 返回 bool，false 表示写入失败
    if (!ledcWrite(_pin, FAN_DUTY_OFF)) {
        _initialized = false;
        Serial.println("[FanController] ERROR: ledcWrite() failed during init");
        Serial.printf("[FanController] Cannot write initial duty to GPIO%d\n", _pin);
        return false;
    }

    _initialized = true;
    _currentGear = FAN_GEAR_OFF;
    _targetGear = FAN_GEAR_OFF;
    _currentDuty = FAN_DUTY_OFF;
    _powerOn = false;
    _state = FanState::IDLE;

    Serial.println("[FanController] Initialized successfully");
    Serial.printf("[FanController] Control pin: GPIO%d\n", _pin);
    Serial.printf("[FanController] PWM: %dHz, %d-bit\n",
                  FAN_PWM_FREQ, FAN_PWM_RESOLUTION);

    return true;
}

/**
 * 主循环更新函数
 * 应在 loop() 中周期性调用
 */
void FanController::update() {
    if (!_initialized) return;

    // 根据当前状态处理
    switch (_state) {
        case FanState::IDLE:
            // IDLE 状态下不需要处理，风扇已关闭
            break;

        case FanState::SOFT_START:
            _handleSoftStart();
            break;

        case FanState::RUNNING:
            // RUNNING 状态下保持目标档位
            if (_currentDuty != getDutyForGear(_targetGear)) {
                _currentDuty = getDutyForGear(_targetGear);
                _updatePWM();
            }
            break;

        case FanState::SOFT_STOP:
            _handleSoftStop();
            break;
    }
}

/**
 * 启动软启动流程
 * 如果当前处于 SOFT_STOP 状态，会取消停止并重新启动
 */
void FanController::startSoftStart(int targetGear, unsigned long durationMs) {
    if (!_initialized) return;

    // 限制目标档位范围
    if (targetGear < FAN_GEAR_1) targetGear = FAN_GEAR_1;
    if (targetGear > FAN_MAX_GEAR) targetGear = FAN_MAX_GEAR;

    _targetGear = targetGear;
    _softStartDuration = durationMs;
    
    // 如果当前处于 SOFT_STOP 状态，取消停止并重新启动
    if (_state == FanState::SOFT_STOP) {
        Serial.println("[FanController] Cancel soft stop, restart soft start");
    }
    
    _transitionTo(FanState::SOFT_START);
    
    Serial.printf("[FanController] Soft start to gear %d over %lu ms\n", 
                  targetGear, durationMs);
}

/**
 * 启动软停止流程
 * 如果当前处于 SOFT_START 状态，会取消启动并开始停止
 */
void FanController::startSoftStop(unsigned long durationMs) {
    if (!_initialized) return;

    _softStopDuration = durationMs;
    
    // 如果当前处于 SOFT_START 状态，取消启动并开始停止
    if (_state == FanState::SOFT_START) {
        Serial.println("[FanController] Cancel soft start, start soft stop");
    }
    
    _transitionTo(FanState::SOFT_STOP);
    
    Serial.printf("[FanController] Soft stop over %lu ms\n", durationMs);
}

/**
 * 立即设置风扇档位
 * 会中断软启动/软停止流程
 * @return true 表示成功，false 表示失败（如未初始化）
 */
bool FanController::setGear(int gear) {
    if (!_initialized) {
        Serial.println("[FanController] ERROR: setGear() called but not initialized");
        return false;
    }

    // 限制档位范围
    if (gear < FAN_GEAR_OFF) gear = FAN_GEAR_OFF;
    if (gear > FAN_MAX_GEAR) gear = FAN_MAX_GEAR;

    _targetGear = gear;

    if (gear == FAN_GEAR_OFF) {
        _currentDuty = FAN_DUTY_OFF;
        _powerOn = false;
        _transitionTo(FanState::IDLE);
        Serial.println("[FanController] Gear: OFF");
    } else {
        _currentDuty = getDutyForGear(gear);
        _powerOn = true;
        _transitionTo(FanState::RUNNING);
        Serial.printf("[FanController] Gear: %d (%s)\n", gear, getGearName(gear));
    }

    _updatePWM();
    return true;
}

/**
 * 设置风扇开关状态
 */
void FanController::setPower(bool on) {
    if (!_initialized) return;

    if (on && !_powerOn) {
        // 开启：恢复到之前设定的档位
        if (_targetGear > FAN_GEAR_OFF) {
            setGear(_targetGear);
        } else {
            // 如果没有设定过档位，默认 3 档
            setGear(FAN_GEAR_3);
        }
    } else if (!on && _powerOn) {
        // 关闭：立即关闭
        setGear(FAN_GEAR_OFF);
        Serial.println("[FanController] Power OFF");
    }
}

/**
 * 获取当前风扇状态
 */
FanState FanController::getState() const {
    return _state;
}

/**
 * 获取当前状态描述字符串
 */
const char* FanController::getStateString() const {
    return getStateName(_state);
}

/**
 * 获取当前风速等级
 */
int FanController::getCurrentGear() const {
    // 在软启动/软停止过程中，返回当前实际档位（估算）
    if (_state == FanState::SOFT_START || _state == FanState::SOFT_STOP) {
        // 根据当前占空比反推档位
        for (int gear = FAN_MAX_GEAR; gear >= FAN_GEAR_1; gear--) {
            if (_currentDuty >= getDutyForGear(gear) - 5) {
                return gear;
            }
        }
        return FAN_GEAR_OFF;
    }
    return _currentGear;
}

/**
 * 获取当前占空比
 */
int FanController::getCurrentDutyCycle() const {
    return _currentDuty;
}

/**
 * 风扇是否开启
 */
bool FanController::isPowerOn() const {
    return _powerOn;
}

/**
 * 获取档位对应的占空比
 */
int FanController::getDutyForGear(int gear) {
    switch (gear) {
        case FAN_GEAR_1: return FAN_DUTY_GEAR_1;
        case FAN_GEAR_2: return FAN_DUTY_GEAR_2;
        case FAN_GEAR_3: return FAN_DUTY_GEAR_3;
        case FAN_GEAR_4: return FAN_DUTY_GEAR_4;
        case FAN_GEAR_5: return FAN_DUTY_GEAR_5;
        default: return FAN_DUTY_OFF;
    }
}

/**
 * 获取档位名称
 */
const char* FanController::getGearName(int gear) {
    switch (gear) {
        case FAN_GEAR_OFF: return "OFF";
        case FAN_GEAR_1: return "Gear 1 (20%)";
        case FAN_GEAR_2: return "Gear 2 (40%)";
        case FAN_GEAR_3: return "Gear 3 (60%)";
        case FAN_GEAR_4: return "Gear 4 (80%)";
        case FAN_GEAR_5: return "Gear 5 (100%)";
        default: return "Unknown";
    }
}

/**
 * 获取状态名称
 */
const char* FanController::getStateName(FanState state) {
    switch (state) {
        case FanState::IDLE:      return "IDLE";
        case FanState::SOFT_START: return "SOFT_START";
        case FanState::RUNNING:   return "RUNNING";
        case FanState::SOFT_STOP: return "SOFT_STOP";
        default:                  return "UNKNOWN";
    }
}

/**
 * 更新 PWM 输出
 * 使用 LEDC API 输出 PWM 信号
 */
void FanController::_updatePWM() {
    if (!_initialized) return;

    // 使用 LEDC API 写入 PWM 值
    // ESP32-S3: ledcWrite(gpio, duty)
    // 检查返回值以捕获 PWM 写入失败
    if (!ledcWrite(_pin, _currentDuty)) {
        Serial.println("[FanController] ERROR: ledcWrite() failed");
        Serial.printf("[FanController] PWM write failed on GPIO%d, duty=%d\n",
                      _pin, _currentDuty);
        return;
    }

    // 更新当前档位（用于状态显示）
    if (_currentDuty == FAN_DUTY_OFF) {
        _currentGear = FAN_GEAR_OFF;
        _powerOn = false;
    } else {
        _powerOn = true;
        // 根据占空比反推档位
        for (int gear = FAN_GEAR_1; gear <= FAN_MAX_GEAR; gear++) {
            if (_currentDuty == getDutyForGear(gear)) {
                _currentGear = gear;
                break;
            }
        }
    }
}

/**
 * 处理软启动状态
 * 非阻塞实现：每次调用根据经过时间计算并更新 PWM
 */
void FanController::_handleSoftStart() {
    unsigned long elapsedTime = millis() - _stateStartTime;
    
    if (elapsedTime >= _softStartDuration) {
        // 软启动完成，切换到 RUNNING 状态
        _currentDuty = getDutyForGear(_targetGear);
        _updatePWM();
        _transitionTo(FanState::RUNNING);
        Serial.printf("[FanController] Soft start completed, now at gear %d\n", _targetGear);
    } else {
        // 计算当前应达到的占空比（线性递增）
        int targetDuty = getDutyForGear(_targetGear);
        _currentDuty = (targetDuty * elapsedTime) / _softStartDuration;
        _updatePWM();
    }
}

/**
 * 处理软停止状态
 * 非阻塞实现：每次调用根据经过时间计算并更新 PWM
 */
void FanController::_handleSoftStop() {
    unsigned long elapsedTime = millis() - _stateStartTime;
    
    if (elapsedTime >= _softStopDuration) {
        // 软停止完成，切换到 IDLE 状态
        _currentDuty = FAN_DUTY_OFF;
        _updatePWM();
        _transitionTo(FanState::IDLE);
        Serial.println("[FanController] Soft stop completed, fan OFF");
    } else {
        // 计算当前应达到的占空比（线性递减）
        int currentDuty = getDutyForGear(_currentGear);
        int remainingPercent = (_softStopDuration - elapsedTime) * 100 / _softStopDuration;
        _currentDuty = (currentDuty * remainingPercent) / 100;
        _updatePWM();
    }
}

/**
 * 转换状态
 */
void FanController::_transitionTo(FanState newState) {
    if (_state != newState) {
        Serial.printf("[FanController] State change: %s -> %s\n", 
                      getStateName(_state), getStateName(newState));
        _state = newState;
        _stateStartTime = millis();
    }
}
