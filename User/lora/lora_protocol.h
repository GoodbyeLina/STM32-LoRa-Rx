#ifndef __LORA_PROTOCOL_H
#define __LORA_PROTOCOL_H

#include <stdint.h>


// 必须强制 1 字节对齐
#pragma pack(1) 
typedef struct {
    float temperature;
    float humidity;
    float acc_x;
    float acc_y;
    float acc_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float latitude;
    float longitude;
		uint16_t crc16;    // 接收到的校验位
    uint32_t tail; // 校验尾: 0x7F800000 (内存中为 00 00 80 7F)
} LoRa_Packet_t;
#pragma pack()

#define LORA_PACKET_SIZE sizeof(LoRa_Packet_t) // 44字节
// --- 变量定义 ---
#define RX_BUFFER_SIZE 128       // 定义一个足够大的缓冲区
extern uint8_t rx_byte_u3;              // UART3 临时接收 1 字节的变量
extern uint8_t raw_buffer[RX_BUFFER_SIZE]; // 滑动窗口缓冲区
extern uint16_t buffer_index;        // 当前缓冲区索引
extern LoRa_Packet_t received_data;
extern uint8_t is_data_ready;       // 数据包就绪标志

#endif
