# SmartFan 2.0 项目 PIR 历史测试证据审计报告

- **审计日期**: 2026-11-20
- **审计类型**: 严格只读审查
- **审计目标**: 查找 PIR 模块烧录、串口监视器、人体感应测试的历史记录
- **审计范围**: Git 历史、源代码、实验记录、日志文件、证据目录
- **审计原则**: 只读审查，未修改任何文件，未执行烧录，未连接硬件

---

## 一、直接证据：无

| 证据类型 | 搜索结果 | 说明 |
|---|---|---|
| PIR 烧录记录 | 未找到 | Git 历史中无"upload"、"flash"、"program"等 PIR 相关提交；无 PlatformIO upload 成功日志 |
| 串口监视器输出 | 未找到 | 项目中无 `serial.log`、`monitor.txt`、`pir_test_log.txt` 等文件；`.vscode` 目录不存在 |
| 人体感应测试数据 | 未找到 | 无测试时间戳、无 HIGH/LOW 变化记录、无"detected"或"motion"关键词的实际输出 |
| PIR 测试照片/视频 | 未找到 | `experiment/evidence/EXP-005/` 中 10 张照片均非 PIR 连接测试场景 |

---

## 二、间接证据：存在

| 证据类型 | 文件路径 | Git 提交 | 内容摘要 |
|---|---|---|---|
| PIR 测试代码 | `src/test_pir_main.cpp` | `deaa23e` | GPIO5 配置为 INPUT，digitalRead 读取电平，代码明确标注"尚未编译、上传或进行硬件测试" |
| PlatformIO 测试环境 | `platformio.ini` | `15cf57d` | `[env:pir-read-test]` 环境，build_src_filter 仅编译 test_pir_main.cpp |
| PIR 测试实验记录 | `experiment/records/2026-11-20/EXP-007-pir-raw-signal-read-test-code.md` | `5eda5c8` | 明确写明"代码已创建；尚未编译、上传或进行硬件测试" |
| PIR 电气安全审查 | `experiment/records/2026-11-20/EXP-006-mosfet-pir-electrical-safety-review.md` | `072c8c5` | 指出 PIR OUT 电压未验证，禁止连接 ESP32-S3 |
| PIR 模块照片 | `experiment/evidence/EXP-005/IMG_20261114_193135.jpg` | `368868c` | HC-SR501 风格外观，但型号标记不可读；未连接 ESP32 |

---

## 三、无法确认的事项

| 事项 | 原因 |
|---|---|
| PIR 模块具体型号 | 照片中仅可见 HC-SR501 风格外观，无清晰可读的型号标记 |
| PIR OUT 高电平电压 | 未由 datasheet 确认，未由万用表测量，EXP-006 明确标注"未验证" |
| PIR 模块是否曾物理连接 ESP32 | 无照片、无测试日志、无烧录记录证明已连接 |
| PIR 模块供电电压 | 未记录是 3.3V 还是 5V 供电 |
| 是否曾进行人体感应测试 | 无测试时间戳、无串口输出、无"detected"记录 |

---

## 四、搜索范围说明

### Git 历史搜索

```
git log --all --oneline --grep="PIR"         — 找到 6 个提交（deaa23e, 15cf57d, d0a1f8d, 5eda5c8, 072c8c5, 368868c）
git log --all --oneline --grep="pir"         — 同上
git log --all --oneline --grep="人体感应"    — 无结果
git log --all --oneline --grep="upload"      — 无 PIR 相关结果
git log --all --oneline --grep="flash"       — 无 PIR 相关结果
git log --all --oneline --grep="烧录"        — 无 PIR 相关结果
git log --all --oneline --grep="serial"      — 无 PIR 相关结果
git log --all --oneline --grep="monitor"     — 无 PIR 相关结果
git log --all --oneline --grep="串口"        — 无 PIR 相关结果
```

### 文件搜索

```
**/test_pir*.cpp                             — 找到 test_pir_main.cpp
**/*pir*.md                                  — 找到 EXP-006、EXP-007
**/*log*.md                                  — 找到 software_debug_log.md、system_test_log.md、experiment_log.md，均无 PIR 测试记录
```

### 内容搜索

```
digitalRead / HIGH / LOW                     — 仅在 test_pir_main.cpp 和 fan_controller.cpp 中出现
serial.log / monitor.txt / pir_test_log.txt  — 无匹配文件
```

### 证据目录检查

```
experiment/evidence/EXP-005/                 — 10 张 JPG 照片，无 PIR 连接测试场景
experiment/hardware_connection_log.md        — 无 PIR 连接记录
```

---

## 五、证据缺口

| 缺失证据类型 | 重要性 | 建议补全方式 |
|---|---|---|
| PIR 模块 datasheet 或清晰型号照片 | 高 | 拍摄模块侧面/背面清晰照片，读取型号标记 |
| PIR OUT 电压测量记录 | 高 | 使用万用表测量 OUT 引脚高电平电压 |
| PIR 连接 ESP32 的照片 | 高 | 拍摄 PIR OUT 连接 GPIO5 的实物照片 |
| 串口监视器输出截图/日志 | 高 | 上传测试程序后，截取串口监视器完整输出 |
| 人体感应测试时间戳记录 | 中 | 记录测试时间、人体靠近/离开时刻、对应电平变化 |
| 烧录成功日志 | 中 | PlatformIO Upload 成功输出（"Upload OK"） |

---

## 六、结论

**基于现有证据，PIR 模块从未进行过实际硬件测试。**

### 关键依据

1. **EXP-007 明确声明**: "代码已创建；尚未编译、上传或进行硬件测试"
2. **EXP-006 安全限制**: "PIR OUT 电压未验证，禁止连接 ESP32-S3"
3. **Git 提交主题**: 均为"Add test code"、"Document test"、"Electrical safety review"，无"upload"、"flash"、"test result"等执行记录
4. **无测试日志文件**: 项目中不存在任何串口监视器输出文件或测试数据文件
5. **无物理连接证据**: EXP-005 照片仅展示 PIR 模块外观，未显示连接 ESP32

### 当前状态

PIR 测试代码和构建环境已创建并提交到 GitHub，处于"代码就绪、等待电气安全确认"阶段，未进行任何硬件测试。

---

**审计完成时间**: 2026-11-20  
**审计报告路径**: `experiment/audits/2026-11-20-PIR-test-evidence-audit.md`  
**原始记录保护**: 本次审计未修改 EXP-006、EXP-007 或任何其他原始实验记录
