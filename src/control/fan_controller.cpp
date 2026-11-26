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
    , _pendingDuty(FAN_DUTY_OFF)
    , _retryPending(false)
    , _powerOn(false)
    , _initialized(false)
    , _stateStartTime(0)
    , _softStartDuration(FAN_SOFT_START_TIME_MS)
    , _softStopDuration(FAN_SOFT_STOP_TIME_MS)
    , _pwmErrorCount(0)
    , _startDuty(FAN_DUTY_OFF)
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
    _pendingDuty = FAN_DUTY_OFF;
    _retryPending = false;
    _powerOn = false;
    _state = FanState::IDLE;
    _startDuty = FAN_DUTY_OFF;

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
            // 如果 _retryPending 为 true，跳过重新计算，直接重试
            if (!_retryPending) {
                // 正常模式下保持目标档位
                int targetDuty = getDutyForGear(_targetGear);
                if (_currentDuty != targetDuty) {
                    // 问题 2 修复：先保存到 _pendingDuty，不得提前修改 _currentDuty
                    _pendingDuty = targetDuty;
                    // 设置重试标记，让 _updatePWM() 写入 _pendingDuty
                    _retryPending = true;
                }
            }
            // 尝试更新 PWM，失败时保持当前状态并重试
            if (!_updatePWM()) {
                _pwmErrorCount++;
                Serial.println("[FanController] RUNNING paused - PWM write failed, retrying...");
                // 达到阈值：进入故障状态
                if (_pwmErrorCount >= MAX_PWM_ERRORS) {
                    Serial.println("[FanController] PWM error threshold reached, entering PWM_ERROR state");
                    _transitionTo(FanState::PWM_ERROR);
                }
            } else {
                // 成功后才更新 _currentDuty
                _currentDuty = _pendingDuty;
                _pwmErrorCount = 0;  // 成功重置计数器
            }
            break;

        case FanState::SOFT_STOP:
            _handleSoftStop();
            break;

        case FanState::PWM_ERROR:
            // 故障状态下保持待写入占空比，持续重试
            Serial.printf("[FanController] PWM_ERROR state - retrying duty=%d\n", _pendingDuty);
            if (_updatePWM()) {
                _pwmErrorCount = 0;  // 成功重置计数器
                Serial.println("[FanController] PWM recovered from error state");
                // 恢复到 RUNNING 状态
                if (_targetGear > FAN_GEAR_OFF) {
                    _transitionTo(FanState::RUNNING);
                } else {
                    _transitionTo(FanState::IDLE);
                }
            }
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
    
    // 记录软启动起始占空比（从当前已成功的占空比开始）
    _startDuty = _currentDuty;
    _retryPending = false;  // 重置重试标记，新命令不继续旧任务

    // 如果当前处于 SOFT_STOP 状态，取消停止并重新启动
    if (_state == FanState::SOFT_STOP) {
        Serial.println("[FanController] Cancel soft stop, restart soft start");
    }

    _transitionTo(FanState::SOFT_START);

    Serial.printf("[FanController] Soft start to gear %d over %lu ms (startDuty=%d)\n",
                  targetGear, durationMs, _startDuty);
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
 * @return true 表示成功，false 表示失败（如未初始化或 PWM 写入失败）
 * @note 事务性写入：先保存旧值，成功后才更新，失败时恢复
 */
bool FanController::setGear(int gear) {
    if (!_initialized) {
        Serial.println("[FanController] ERROR: setGear() called but not initialized");
        return false;
    }

    // 限制档位范围
    if (gear < FAN_GEAR_OFF) gear = FAN_GEAR_OFF;
    if (gear > FAN_MAX_GEAR) gear = FAN_MAX_GEAR;

    // 计算目标占空比
    int targetDuty = (gear == FAN_GEAR_OFF) ? FAN_DUTY_OFF : getDutyForGear(gear);

    // 事务性写入：先保存旧值，成功后才更新
    int oldGear = _currentGear;
    int oldDuty = _currentDuty;
    int oldPendingDuty = _pendingDuty;
    int oldTargetGear = _targetGear;
    bool oldPowerOn = _powerOn;

    // 更新内部状态
    _currentGear = gear;
    _currentDuty = targetDuty;
    _pendingDuty = targetDuty;  // 保存待写入占空比

    // 更新目标档位和电源状态
    _targetGear = gear;
    _powerOn = (gear > FAN_GEAR_OFF);

    // 尝试写入 PWM
    if (_updatePWM()) {
        // 成功：重置错误计数器，添加状态转换
        _pwmErrorCount = 0;
        
        // 问题 1 修复：成功后正确转换状态
        if (gear == FAN_GEAR_OFF) {
            _transitionTo(FanState::IDLE);
            Serial.printf("[FanController] Gear %d -> %d, duty: %d, state: IDLE\n", oldGear, gear, targetDuty);
        } else {
            _transitionTo(FanState::RUNNING);
            Serial.printf("[FanController] Gear %d -> %d, duty: %d, state: RUNNING\n", oldGear, gear, targetDuty);
        }
        return true;
    } else {
        // 失败：完整回滚所有状态
        _currentGear = oldGear;
        _currentDuty = oldDuty;
        _pendingDuty = oldPendingDuty;
        _targetGear = oldTargetGear;  // 问题 3 修复：回滚目标档位
        _powerOn = oldPowerOn;        // 问题 3 修复：回滚电源状态
        _pwmErrorCount++;
        Serial.printf("[FanController] PWM write failed, gear %d -> %d (count: %d)\n",
                      oldGear, gear, _pwmErrorCount);

        // 达到阈值：进入故障状态
        if (_pwmErrorCount >= MAX_PWM_ERRORS) {
            Serial.println("[FanController] PWM error threshold reached, entering PWM_ERROR state");
            _transitionTo(FanState::PWM_ERROR);
        }
        return false;
    }
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
        case FanState::PWM_ERROR: return "PWM_ERROR";
        default:                  return "UNKNOWN";
    }
}

/**
 * 更新 PWM 输出
 * 使用 LEDC API 输出 PWM 信号
 * @return true PWM 写入成功，false 失败（未初始化或 ledcWrite 失败）
 * @note 仅负责 PWM 写入，不负责更新错误计数器（由调用方控制）
 * @note 如果 _retryPending 为 true，写入 _pendingDuty；否则写入 _currentDuty
 * @note 写入成功后清除 _retryPending 标记
 */
bool FanController::_updatePWM() {
    if (!_initialized) {
        Serial.println("[FanController] ERROR: _updatePWM() called but not initialized");
        return false;
    }

    // 根据 _retryPending 标记决定写入哪个值
    int dutyToWrite = _retryPending ? _pendingDuty : _currentDuty;
    
    // 使用 LEDC API 写入 PWM 值
    // ESP32-S3: ledcWrite(gpio, duty)
    // 检查返回值以捕获 PWM 写入失败
    if (!ledcWrite(_pin, dutyToWrite)) {
        Serial.println("[FanController] ERROR: ledcWrite() failed");
        Serial.printf("[FanController] PWM write failed on GPIO%d, duty=%d (retryPending=%s)\n",
                      _pin, dutyToWrite, _retryPending ? "true" : "false");
        // 失败时保持 _retryPending 为 true，下次继续重试
        return false;
    }

    // 写入成功，清除 _retryPending 标记
    _retryPending = false;
    return true;
}

/**
 * 处理软启动状态
 * 非阻塞实现：每次调用根据经过时间计算并更新 PWM
 * @note 使用 _startDuty 作为起始占空比，目标 duty 作为结束占空比，按时间线性插值
 * @note 失败时保存占空比，下一轮优先重试
 */
void FanController::_handleSoftStart() {
    // 处理 durationMs == 0 的特殊情况：直接执行目标 duty
    if (_softStartDuration == 0) {
        int targetDuty = getDutyForGear(_targetGear);
        _pendingDuty = targetDuty;  // 先保存到 _pendingDuty
        if (!_updatePWM()) {
            _retryPending = true;
            _pwmErrorCount++;
            Serial.println("[FanController] Soft start (instant) PWM failed, retrying...");
            if (_pwmErrorCount >= MAX_PWM_ERRORS) {
                Serial.println("[FanController] PWM error threshold reached, entering PWM_ERROR state");
                _transitionTo(FanState::PWM_ERROR);
            }
            return;
        }
        _currentDuty = _pendingDuty;  // 成功后才更新 _currentDuty
        _pwmErrorCount = 0;
        _transitionTo(FanState::RUNNING);
        Serial.printf("[FanController] Soft start completed (instant), now at gear %d\n", _targetGear);
        return;
    }

    unsigned long elapsedTime = millis() - _stateStartTime;

    // 如果 _retryPending 为 true，优先重试，不重新计算占空比
    if (_retryPending) {
        if (!_updatePWM()) {
            _pwmErrorCount++;
            Serial.println("[FanController] Soft start retry failed, continuing retry...");
            if (_pwmErrorCount >= MAX_PWM_ERRORS) {
                Serial.println("[FanController] PWM error threshold reached, entering PWM_ERROR state");
                _transitionTo(FanState::PWM_ERROR);
            }
            return;  // 继续重试，不更新任何状态
        }
        _pwmErrorCount = 0;  // 重试成功，重置计数器
        _retryPending = false;  // 清除重试标记
        // 重试成功后，继续软启动流程
    }

    if (elapsedTime >= _softStartDuration) {
        // 软启动完成，切换到 RUNNING 状态
        int targetDuty = getDutyForGear(_targetGear);
        _pendingDuty = targetDuty;  // 先保存到 _pendingDuty
        if (!_updatePWM()) {
            // PWM 写入失败，设置 _retryPending 并重试
            _retryPending = true;
            _pwmErrorCount++;
            Serial.println("[FanController] Soft start completed but PWM failed, retrying...");
            if (_pwmErrorCount >= MAX_PWM_ERRORS) {
                Serial.println("[FanController] PWM error threshold reached, entering PWM_ERROR state");
                _transitionTo(FanState::PWM_ERROR);
            }
            return;  // 不切换状态，下次 update() 继续尝试
        }
        _currentDuty = _pendingDuty;  // 成功后才更新 _currentDuty
        _pwmErrorCount = 0;  // 成功重置计数器
        _transitionTo(FanState::RUNNING);
        Serial.printf("[FanController] Soft start completed, now at gear %d\n", _targetGear);
    } else {
        // 计算当前应达到的占空比（线性插值：从 _startDuty 到目标 duty）
        int targetDuty = getDutyForGear(_targetGear);
        // 使用长整型避免溢出：(_startDuty * (duration - elapsed) + targetDuty * elapsed) / duration
        unsigned long calculatedDuty = ((unsigned long)_startDuty * (_softStartDuration - elapsedTime) + 
                                        (unsigned long)targetDuty * elapsedTime) / _softStartDuration;
        
        // 限制占空比范围
        if (calculatedDuty > 255) calculatedDuty = 255;
        
        // 只有成功后才更新 _currentDuty，这里先保存到 _pendingDuty
        _pendingDuty = (int)calculatedDuty;
        
        if (!_updatePWM()) {
            // PWM 写入失败，设置 _retryPending 并重试
            _retryPending = true;
            _pwmErrorCount++;
            Serial.println("[FanController] Soft start PWM failed, retrying...");
            if (_pwmErrorCount >= MAX_PWM_ERRORS) {
                Serial.println("[FanController] PWM error threshold reached, entering PWM_ERROR state");
                _transitionTo(FanState::PWM_ERROR);
            }
            return;  // 不更新 _currentDuty，下次重试
        }
        // 成功后才更新 _currentDuty
        _currentDuty = _pendingDuty;
        _pwmErrorCount = 0;  // 成功重置计数器
    }
}

/**
 * 处理软停止状态
 * 非阻塞实现：每次调用根据经过时间计算并更新 PWM
 * @note 失败时保存占空比，下一轮优先重试
 */
void FanController::_handleSoftStop() {
    unsigned long elapsedTime = millis() - _stateStartTime;

    // 如果 _retryPending 为 true，跳过重新计算，直接重试
    if (_retryPending) {
        if (!_updatePWM()) {
            _pwmErrorCount++;
            Serial.println("[FanController] Soft stop retry failed, continuing retry...");
            if (_pwmErrorCount >= MAX_PWM_ERRORS) {
                Serial.println("[FanController] PWM error threshold reached, entering PWM_ERROR state");
                _transitionTo(FanState::PWM_ERROR);
            }
            return;
        }
        _pwmErrorCount = 0;  // 成功重置计数器
        _retryPending = false;  // 清除重试标记
        // 重试成功后，继续软停止流程
    }

    if (elapsedTime >= _softStopDuration) {
        // 软停止完成，切换到 IDLE 状态
        _pendingDuty = FAN_DUTY_OFF;  // 先保存到 _pendingDuty
        if (!_updatePWM()) {
            // PWM 写入失败，设置 _retryPending 并重试
            _retryPending = true;
            _pwmErrorCount++;
            Serial.println("[FanController] Soft stop completed but PWM failed, retrying...");
            if (_pwmErrorCount >= MAX_PWM_ERRORS) {
                Serial.println("[FanController] PWM error threshold reached, entering PWM_ERROR state");
                _transitionTo(FanState::PWM_ERROR);
            }
            return;  // 不切换状态，下次 update() 继续尝试
        }
        _currentDuty = _pendingDuty;  // 成功后才更新 _currentDuty
        _pwmErrorCount = 0;  // 成功重置计数器
        _transitionTo(FanState::IDLE);
        Serial.println("[FanController] Soft stop completed, fan OFF");
    } else {
        // 计算当前应达到的占空比（线性递减）
        // 问题 4 修复：从 _currentDuty（最近一次成功写入的占空比）开始，而不是从 getDutyForGear(_currentGear) 推算
        int startDuty = _currentDuty;
        int remainingPercent = (_softStopDuration - elapsedTime) * 100 / _softStopDuration;
        int newDuty = (startDuty * remainingPercent) / 100;

        // 只有成功后才更新 _currentDuty，这里先保存到 _pendingDuty
        _pendingDuty = newDuty;

        if (!_updatePWM()) {
            // PWM 写入失败，设置 _retryPending 并重试
            _retryPending = true;
            _pwmErrorCount++;
            Serial.println("[FanController] Soft stop PWM failed, retrying...");
            if (_pwmErrorCount >= MAX_PWM_ERRORS) {
                Serial.println("[FanController] PWM error threshold reached, entering PWM_ERROR state");
                _transitionTo(FanState::PWM_ERROR);
            }
            return;  // 不更新 _currentDuty，下次重试
        }
        // 成功后才更新 _currentDuty
        _currentDuty = _pendingDuty;
        _pwmErrorCount = 0;  // 成功重置计数器
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

/**
 * 重置 PWM 错误状态（从故障状态恢复）
 * @note 调用 setGear() 成功时会自动重置错误计数器
 */
void FanController::resetPwmError() {
    _pwmErrorCount = 0;
    if (_state == FanState::PWM_ERROR) {
        Serial.println("[FanController] Reset PWM error state");
        // 根据目标档位恢复到相应状态
        if (_targetGear > FAN_GEAR_OFF) {
            _transitionTo(FanState::RUNNING);
        } else {
            _transitionTo(FanState::IDLE);
        }
    }
}
