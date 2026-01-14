# STM32F407 LoRa 接收终端与可视化显示 (LoRa Rx Station with LCD)

本项目是一个基于 **STM32F407** 的 LoRa 数据接收终端。除了将接收到的传感器数据通过串口（UART）转发给上位机（VOFA+）进行波形分析外，新增了 **板载 LCD 实时显示功能**，使其成为一个独立的可视化手持终端。

---

## 📅 更新日志 (Update)

* **[最新] v2.0 - LCD 可视化界面**: 集成 NT35510 屏幕驱动，实现温湿度、姿态、GPS 数据的实时刷新，添加心跳指示与色彩分级显示。
* **v1.0 - 基础透传**: 完成 LoRa 射频接收，支持结构体数据解析与 UART 转发。

---

## 🛠 硬件环境 (Hardware)

* **MCU**: STM32F407ZGT6 (野火/Wildfire 开发板)
* **LoRa 模块**: SX1278 / RA-02 (SPI 接口)
* **显示屏**: 4.3寸/5寸电容/电阻屏 (驱动 IC: **NT35510**)
* **接口方式**: FSMC (16-bit 并口 8080 时序)

### 🔌 引脚定义 (Pinout)

#### 1. LoRa (SPI)

| Pin | Function | STM32 Pin |
| --- | --- | --- |
| NSS | SPI CS | PA4 |
| SCK | SPI CLK | PA5 |
| MISO | SPI MISO | PA6 |
| MOSI | SPI MOSI | PA7 |
| DIO0 | IRQ | PB0 |
| RST | Reset | PC0 |

#### 2. LCD (FSMC Bank 3)

* **Chip Select (CS)**: `NE3` (PG10) - 对应基地址 `0x68000000`
* **Register Select (RS/DC)**: `A0` (PF0) - 对应地址偏移 `0x02`
* **Data Bus**: `D0` - `D15` (16位宽)
* **Backlight (BL)**: `PB15` (高电平点亮)

---

## ⚙️ 软件配置 (Software Configuration)

### STM32CubeMX FSMC 设置 (关键!)

为了驱动 NT35510，FSMC 必须严格按照以下参数配置，否则会导致白屏或花屏：

* **Bank**: `NOR Flash/PSRAM/SRAM/ROM/LCD 3`
* **Memory Type**: `LCD Interface`
* **LCD Register Select**: `A0`
* **Data Width**: `16 bits`
* **Timing (时序)**:
* Address Setup Time: `4` Cycles
* Data Setup Time: `10` Cycles (若有噪点可增至 15)
* **Write Operation**: `Enable` (必须开启!)



### 目录结构

* `User/lora/`: LoRa 底层驱动与协议处理 (`lora_rx.c`)
* `User/LCD/`: 屏幕驱动 (`bsp_nt35510_lcd.c`) 与字库 (`fonts.c`)

---

## 📺 界面展示 (UI Layout)

LCD 采用色彩编码 (Color-coded) 布局，以区分不同类型的数据，并在右上角包含心跳指示以监测系统状态。

**屏幕刷新逻辑**：`LoRa_Task_Process()` 接收到数据包后触发局部刷新。

```text
+--------------------------------------------------+
|                    * <-- (绿色闪烁心跳/Heartbeat) | LINE 0
|   STM32 LoRa Ready                               | LINE 1
| ----------------------                           | LINE 2
| Temp: 25.6 C           <-- (红色 Red)             | LINE 3
| Hum:  60.2 %           <-- (蓝色 Blue)            | LINE 4
|                                                  |
| acc_x: 0.123  gyro_x: 0.001                      | LINE 6
|                                                  |
| acc_y: -0.05  gyro_y: 0.002    (黄色 Yellow)      | LINE 8
|                                                  |
| acc_z: 9.801  gyro_z: 0.000                      | LINE 10
|                                                  |
| Lat: 22.5432  Lon: 113.9432    (白色 White)       | LINE 12
+--------------------------------------------------+

```

---

## 🚀 如何使用 (Usage)

1. **编译与下载**: 使用 Keil MDK 或 STM32CubeIDE 编译工程并下载至 F407。
2. **启动**:
* 系统上电后，屏幕显示标题栏，背光点亮。
* **右上角绿色 `*` 开始闪烁**，表示系统正在运行且等待数据。


3. **数据接收**:
* 当 LoRa 收到有效数据包时，屏幕上的数值会实时更新。
* 同时，数据会通过串口 1 (`UART1`) 透传输出，可连接电脑使用 **VOFA+** 查看波形。



---

## ⚠️ 注意事项

1. **背光**: 如果屏幕内容有显示但极暗，请检查 `PB15` 是否为高电平 (3.3V)。
2. **编译报错**: 如果提示找不到头文件，请确保 `User/LCD` 已加入编译路径 (Include Paths)，且代码中已去掉 `./lcd/` 相对路径前缀。
3. **显示残影**: 采用了 `sprintf` 后补空格的方式清除旧字符，无需全屏刷新，避免闪烁。