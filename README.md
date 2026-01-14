# STM32F407 LoRa Environmental Monitor - Receiver Node

本项目是环境监测系统的**接收端**（Rx）。基于 STM32F407 开发，通过 LoRa 无线模块接收远端传感器数据，并通过串口将解析后的数据实时推送到 PC 端上位机（VOFA+）进行波形显示。

🔗 **配套发射端仓库 (Tx)**: [点击此处访问发送端工程](这里替换成你发送端仓库的URL)

---

## 🚀 功能特点
* **高可靠传输**: 采用基于帧尾校验的滑动窗口解析算法，有效解决无线传输中的丢包与乱码问题。
* **实时监控**: 适配 VOFA+ 上位机 "JustFloat" 协议，实时绘制 10 路传感器曲线。
* **数据对齐**: 强制 1 字节对齐（`#pragma pack(1)`），确保 F103(Tx) 与 F407(Rx) 之间的数据解包无缝衔接。

## 🔌 硬件连接 (Hardware Setup)

| STM32F407 引脚 | 设备 (Device) | 功能 (Function) |
| :--- | :--- | :--- |
| **PB10 (USART3_TX)** | LoRa 模块 RXD | 发送配置指令 |
| **PB11 (USART3_RX)** | LoRa 模块 TXD | **接收无线数据包** |
| **PA9 (USART1_TX)** | USB-TTL RXD  | **发送到 PC (VOFA+)** |
| **PA10 (USART1_RX)** | USB-TTL TXD  | 串口通信调试 |
| **GND** | 公共地        | 必须共地 |

> **注意**: 请确保 LoRa 模块的 **M0** 和 **M1** 引脚均接地 (GND)，以进入正常透传模式。

## 📊 数据协议 (Protocol)

数据包总长度为 **44 Bytes**，格式如下：

| 偏移 (Offset) | 类型 (Type) | 含义 (Data) | 单位 (Unit) |
| :--- | :--- | :--- | :--- |
| 0 | float | Temperature | ℃ |
| 4 | float | Humidity | % |
| 8-16 | float | Accel X, Y, Z | g |
| 20-28 | float | Gyro X, Y, Z | deg/s |
| 32 | float | Latitude | deg |
| 36 | float | Longitude | deg |
| 40 | uint32_t | **Tail (0x7F800000)** | 帧尾 |

## 🛠 如何使用

1. **硬件准备**: 按照上表连接 STM32F407、LoRa 模块和 USB-TTL。
2. **烧录固件**: 使用 Keil MDK 打开工程，编译并下载至开发板。
3. **打开 VOFA+**: 
   - 选择 **数据驱动 (Driver)** 为 `JustFloat`。
   - 绑定对应的串口，波特率设置为 `115200` (或你代码中定义的速率)。
   - 在控件中添加波形图，即可看到实时传感器曲线。

---
© 2026 GoodbyeLina. Licensed under the MIT License.
