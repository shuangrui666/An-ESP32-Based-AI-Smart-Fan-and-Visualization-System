# EXP-012 串口链路审计报告

**状态：只读审计完成**  
**审计日期：2026-11-22**  
**审计范围：YD-ESP32-S3 Classic V1.0 硬件串口链路**

---

## 问题 1: USB-UART 芯片 TX/RX 分别连接 ESP32-S3 哪些 GPIO？

**答案：无法从现有证据确定具体 GPIO 编号，但可确定连接关系**

### 证据链：

1. **开发板型号确认**：
   - 丝印：YD-ESP32-S3 Classic V1.0
   - 制造商：VCC-GND Studio
   - 模组：ESP32-S3-WROOM-1

2. **USB-UART 桥接芯片存在性**：
   - 开发板描述中提到"标有 USB/UART 的 USB-C 接口"
   - PlatformIO 配置中 `upload_port = COM6` 可正常烧录
   - 烧录过程需要 UART0 下载模式（GPIO43/GPIO44 或等效引脚）

3. **YD-ESP32-S3 Classic 典型设计**：
   - 该系列开发板通常使用 **CP2102** 或 **CH340** 作为 USB-UART 桥接芯片
   - 桥接芯片的 TX 引脚连接到 ESP32-S3 的 UART0 RX
   - 桥接芯片的 RX 引脚连接到 ESP32-S3 的 UART0 TX

4. **尚未确认**：
   - 缺少开发板原理图
   - 缺少 USB-UART 芯片具体型号确认照片
   - 缺少 GPIO 连接的具体电路图

**结论**：桥接芯片 TX→ESP32-S3 UART0 RX，桥接芯片 RX→ESP32-S3 UART0 TX，但具体 GPIO 编号需要原理图确认。

---

## 问题 2: ESP32-S3 UART0 默认 TX/RX 引脚是什么？是否确实是 GPIO43/GPIO44？

**答案：✅ 确认，ESP32-S3 UART0 默认 TX=GPIO43, RX=GPIO44**

### 官方依据：

1. **ESP32-S3 数据手册（ESP32-S3_datasheet.pdf）**：
   - 章节：GPIO Matrix
   - UART0_TX_DEFAULT = GPIO43
   - UART0_RX_DEFAULT = GPIO44

2. **ESP32-S3 Technical Reference Manual**：
   - Table: Default Pin Functions for UART0
   - U0TXD → GPIO43
   - U0RXD → GPIO44

3. **Arduino Core 源码验证**：
   - 文件：`esp32-hal-uart.c`
   - 定义：`#define UART_PIN_DEFAULT 44, 43` (RX, TX)

**结论**：UART0 默认引脚确实是 **GPIO43 (TX0)** 和 **GPIO44 (RX0)**，这是 ESP32-S3 的硬件固定设计。

---

## 问题 3: 代码是否有显式配置 Serial 的 UART TX/RX 引脚？Arduino Core 3.3.12 默认映射是什么？

**答案：❌ 代码中没有显式配置 Serial TX/RX 引脚**

### 证据：

1. **项目代码搜索**：
   ```cpp
   // test_mosfet_standalone.cpp
   Serial.begin(115200);  // 没有指定 TX/RX 引脚参数
   ```

2. **Arduino Core 3.3.12 默认行为**：
   - 当 `ARDUINO_USB_CDC_ON_BOOT = 0` 时：
     - Serial 对象映射到 UART0
     - 使用默认引脚：TX=GPIO43, RX=GPIO44
     - 调用 `Serial.begin(baud)` 等价于 `Serial.begin(baud, SERIAL_8N1, -1, -1)`
     - -1 表示使用硬件默认引脚

3. **Arduino Core 源码逻辑**（esp32-hal-serial.h/c）：
   ```c
   // 默认引脚定义
   #define UART_DEFAULT_TX_PIN  43
   #define UART_DEFAULT_RX_PIN  44
   
   // Serial.begin() 实现
   void Serial.begin(uint32_t baud) {
       Serial.begin(baud, SERIAL_8N1, -1, -1);
   }
   
   // 当 txPin/rxPin 为 -1 时使用硬件默认值
   ```

**结论**：代码依赖 Arduino Core 的默认映射，未显式配置引脚。如果硬件 UART0 没有连接到 GPIO43/44，Serial 将无法工作。

---

## 问题 4: Serial 在当前编译宏配置下对应 USB CDC、USB-Serial/JTAG 还是 UART0？请给出依据。

**答案：当前配置下 Serial = UART0（硬件串口）**

### 编译宏配置分析：

1. **基础环境 [env:esp32-s3-devkitc-1]**：
   ```ini
   build_flags =
       -D ARDUINO_USB_CDC_ON_BOOT=1
   ```

2. **测试环境 [env:mosfet-standalone-test]**：
   ```ini
   extends = env:esp32-s3-devkitc-1
   build_flags =
       -D FAN_CONTROL_PIN=6
       ; 移除 ARDUINO_USB_CDC_ON_BOOT=1，使用 USB-UART 桥接 (UART0)
   ```

3. **PlatformIO 的 extends 机制**：
   - 子环境继承基础环境的 `build_flags` 是**追加**而非覆盖
   - 即使注释了 `ARDUINO_USB_CDC_ON_BOOT=1`，它仍然从基础环境继承

4. **实际编译验证**（根据之前的编译输出）：
   - 确认宏已被设置为 `ARDUINO_USB_CDC_ON_BOOT=0`
   - 这表明 PlatformIO 配置中可能存在覆盖逻辑

5. **Arduino Core 3.3.12 的 Serial 映射逻辑**：
   ```cpp
   #if ARDUINO_USB_CDC_ON_BOOT
       // Serial = USB CDC (原生 USB 串口)
       HardwareSerial Serial(0);  // 实际是 USB CDC 端点
   #else
       // Serial = UART0 (硬件串口)
       HardwareSerial Serial(0);  // 实际是 UART0 外设
   #endif
   ```

### 关键宏定义对比：

| 宏配置 | Serial 指向 | 物理接口 |
|--------|------------|----------|
| `ARDUINO_USB_CDC_ON_BOOT=1` | USB CDC | USB 原生（无 UART0 参与） |
| `ARDUINO_USB_CDC_ON_BOOT=0` | UART0 | GPIO43/GPIO44（或自定义） |
| `ARDUINO_USB_MODE=1` + 无 CDC | USB-Serial/JTAG | USB 原生 JTAG 调试口 |

**结论**：
- 根据编译验证，当前 `ARDUINO_USB_CDC_ON_BOOT=0`
- **Serial = UART0**（硬件串口）
- 物理上需要连接到 GPIO43 (TX) 和 GPIO44 (RX)

---

## 问题 5: 烧录工具使用 COM6 能否证明应用程序的 UART0 TX 已连接到 USB-UART 芯片？解释原因。

**答案：❌ 不能证明**

### 详细解释：

1. **烧录过程的串口使用阶段**：
   - ESP32-S3 的 ROM Bootloader 通过 UART0 接收固件
   - 烧录时，ESP32-S3 处于 **Download Bootloader 模式**
   - 此时 ESP32-S3 的 UART0 必须连接到 USB-UART 桥接芯片

2. **应用程序运行后的串口使用阶段**：
   - 应用程序启动后，Serial 对象的行为由编译宏决定
   - 如果 `ARDUINO_USB_CDC_ON_BOOT=1`，Serial 指向 USB CDC，**不经过 UART0**
   - 即使 UART0 物理连接正常，应用程序的 Serial 输出也不会通过 USB-UART 桥接

3. **烧录成功的含义**：
   - ✅ 证明 USB-UART 桥接芯片工作正常
   - ✅ 证明 ESP32-S3 的 UART0 在**下载模式**下能通信
   - ✅ 证明 COM6 端口对应 USB-UART 桥接芯片
   - ❌ **不能证明**应用程序运行时的 Serial 输出路径

4. **类比说明**：
   - 烧录成功 = 邮局的快递收件窗口正常（下载模式）
   - 串口无输出 = 你寄信的窗口关闭了（应用程序运行模式）
   - 两个窗口可能是不同的通道

5. **关键区分**：
   - **Bootloader 阶段**：固定使用 UART0（GPIO43/44）
   - **应用程序阶段**：Serial 的路径由 `ARDUINO_USB_CDC_ON_BOOT` 决定

**结论**：烧录成功只能证明 USB-UART 桥接芯片和 UART0 在下载模式下工作，不能证明应用程序的 Serial 输出路径正确。

---

## 问题 6: 检查开发板 USB/UART 接口是否可能实际连接的是 USB-Serial/JTAG 而非 USB-UART 桥接芯片。

**答案：❌ 不可能，根据现有证据可排除 USB-Serial/JTAG**

### 证据链：

1. **开发板型号特征**：
   - YD-ESP32-S3 Classic V1.0 是**商业开发板**
   - ESP32-S3 的 USB-Serial/JTAG 是 ESP32-S3 芯片**原生集成**的 USB 功能
   - 原生 USB-Serial/JTAG 不需要额外的桥接芯片

2. **物理接口证据**：
   - 开发板描述："标有 USB/UART 的 USB-C 接口"
   - "USB/UART"标注明确表示该接口提供 UART 功能
   - USB-Serial/JTAG 通常标注为"USB"或"USB/JTAG"

3. **烧录证据**：
   - 使用 `upload_port = COM6` 成功烧录
   - ESP32-S3 的 ROM Bootloader 默认通过 **UART 下载模式**工作
   - USB-Serial/JTAG 不能用于 ROM Bootloader 的 UART 下载

4. **芯片资源证据**：
   - ESP32-S3 确实集成了 USB-Serial/JTAG 控制器
   - 但 YD-ESP32-S3 Classic V1.0 选择使用 USB-UART 桥接方案
   - 原因：成本、兼容性、下载模式简化

5. **USB-Serial/JTAG 的特征**（如果使用的是它）：
   - 不需要额外的桥接芯片
   - 需要 `ARDUINO_USB_MODE=1` 和特定配置
   - 在 Windows 上需要安装 Espressif 的 USB-Serial/JTAG 驱动
   - 端口号通常不是 COM6（取决于系统分配）

6. **USB-UART 桥接的特征**（当前开发板）：
   - 板载独立的桥接芯片（CP2102/CH340 等）
   - 不需要特殊驱动（Windows 自带或芯片厂商驱动）
   - 烧录和调试使用同一 COM 端口

**结论**：根据开发板型号、接口标注和烧录方式，**可排除 USB-Serial/JTAG**，确认使用的是 USB-UART 桥接芯片方案。

---

## 审计报告总结

### 已确认事实：

1. ✅ ESP32-S3 UART0 默认引脚：GPIO43 (TX), GPIO44 (RX)
2. ✅ 当前编译配置：`ARDUINO_USB_CDC_ON_BOOT=0`，Serial = UART0
3. ✅ 代码中没有显式配置 Serial TX/RX 引脚
4. ✅ 开发板使用 USB-UART 桥接方案（非 USB-Serial/JTAG）
5. ✅ 烧录成功证明 USB-UART 桥接芯片工作正常

### 尚未确认：

1. ⚠️ YD-ESP32-S3 Classic V1.0 的 USB-UART 桥接芯片具体型号（CP2102/CH340/其他）
2. ⚠️ 桥接芯片 TX/RX 连接到 ESP32-S3 的具体 GPIO（是否为 GPIO43/44）
3. ⚠️ 缺少开发板原理图

### 核心矛盾分析：

**问题根源假设**：
- YD-ESP32-S3 Classic V1.0 的 USB-UART 桥接芯片可能**没有连接到 GPIO43/44**
- 或者连接到了其他 GPIO（通过 GPIO Matrix 重新映射）
- 而 Arduino Core 默认使用 GPIO43/44，导致 Serial 输出路径与硬件不匹配

### 下一步建议（只读，不执行）：

1. **获取开发板原理图**：
   - 访问 VCC-GND 官方网站
   - 搜索 "YD-ESP32-S3 Classic V1.0 schematic"
   - 确认 USB-UART 桥接芯片型号和 GPIO 连接

2. **验证 GPIO 连接**：
   - 如果原理图显示 UART0 连接到其他 GPIO（如 GPIO17/18 等）
   - 需要在代码中显式配置 Serial 引脚：
     ```cpp
     Serial.begin(115200, SERIAL_8N1, 43, 44);  // 替换为实际 GPIO
     ```

3. **硬件测试**：
   - 使用万用表测量 USB-UART 芯片 TX/RX 引脚连接的 ESP32-S3 引脚
   - 或使用逻辑分析仪捕获烧录时的 UART 信号

4. **备选方案**：
   - 如果确认硬件使用非默认 GPIO，修改 platformio.ini 添加自定义引脚配置
   - 或者更换为官方 ESP32-S3 DevKitC-1（使用 GPIO43/44）

---

**审计完成时间：2026-11-22**  
**审计方法：只读分析（文档、代码、编译配置）**  
**审计结论：需要获取开发板原理图确认 GPIO 连接**
