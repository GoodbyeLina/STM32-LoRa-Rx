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
    uint32_t tail; // 校验尾: 0x7F800000 (内存中为 00 00 80 7F)
} LoRa_Packet_t;
#pragma pack()

#define LORA_PACKET_SIZE sizeof(LoRa_Packet_t) // 44字节

#endif
