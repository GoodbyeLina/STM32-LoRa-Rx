#ifndef __LORA_PROTOCOL_H
#define __LORA_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>  // offsetof

// 协议版本
#define LORA_PACKET_VER 0x02

// 必须强制 1 字节对齐
#pragma pack(1) 
typedef struct {
    // ========== 环境数据 (8 bytes) ==========
    float temperature;   // 温度 (°C)
    float humidity;      // 湿度 (%)

    // ========== 姿态数据 (24 bytes) ==========
    float acc_x;         // X轴加速度 (g)
    float acc_y;         // Y轴加速度 (g)
    float acc_z;         // Z轴加速度 (g)
    float gyro_x;        // X轴角速度 (°/s)
    float gyro_y;        // Y轴角速度 (°/s)
    float gyro_z;        // Z轴角速度 (°/s)

    // ========== 定位数据 (16 bytes) ==========
    float latitude;      // 纬度 (十进制度)
    float longitude;     // 经度 (十进制度)
    float altitude;      // 海拔高度 (m)
    float hdop;          // 水平精度因子

    // ========== RTK 状态 (4 bytes) ==========
    uint8_t fix_type;    // RTK 解状态: 0=无效, 4=固定解, 5=浮点解
    uint8_t satellites;  // 跟踪卫星数
    uint8_t reserved[2]; // 保留字节

    // ========== 校验 (6 bytes) ==========
    uint16_t crc16;      // CRC16
    uint32_t tail;       // 帧尾: 0x7F800000
} LoRa_Packet_t;
#pragma pack()

// 数据包大小常量
#define LORA_DATA_SIZE      offsetof(LoRa_Packet_t, crc16)  // CRC 校验范围 (52字节)
#define LORA_PACKET_SIZE    sizeof(LoRa_Packet_t)           // 总包大小 (58字节)

// --- 变量定义 ---
#define RX_BUFFER_SIZE 128       // 定义一个足够大的缓冲区
extern uint8_t rx_byte_u3;              // UART3 临时接收 1 字节的变量
extern uint8_t raw_buffer[RX_BUFFER_SIZE]; // 滑动窗口缓冲区
extern uint16_t buffer_index;        // 当前缓冲区索引
extern LoRa_Packet_t received_data;
extern uint8_t is_data_ready;       // 数据包就绪标志

#endif
