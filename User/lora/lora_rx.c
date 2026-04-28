#include "lora_protocol.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include "bsp_nt35510_lcd.h"
#include "fonts.h"

uint8_t is_data_ready = 0;
uint8_t raw_buffer[128]; // 这里写下实际大小
uint16_t buffer_index = 0;
uint8_t rx_byte_u3 = 0;
LoRa_Packet_t received_data;

uint16_t Calculate_CRC16(uint8_t *ptr, uint16_t len);

/**
 * @brief 接收处理函数，在中断回调中被调用
 * @param byte 刚刚接收到的一个字�?
 */
void LoRa_Rx_Handler(uint8_t byte) {
    // 1. 入队：将新字节存入缓冲区
    raw_buffer[buffer_index++] = byte;

    // 2. 环形保护：防止数组越�?
    if (buffer_index >= RX_BUFFER_SIZE) {
        buffer_index = 0; 
    }

    // 3. 校验：只有接收字节数大于包长时才开始检�?
    if (buffer_index >= LORA_PACKET_SIZE) {
        // 4. 检查帧�?(0x7F800000 -> 00 00 80 7F)
        // 取当前索引向前推4个字节，转换�?uint32 检�?
        uint32_t *tail_ptr = (uint32_t*)&raw_buffer[buffer_index - 4];
        
        if (*tail_ptr == 0x7F800000) {
					
						// --- 帧尾匹配成功 ---
            uint8_t start_index = buffer_index - LORA_PACKET_SIZE;
            
            // 先临时拷贝到结构体中，以便提取其中的数据和收到的 CRC
            memcpy(&received_data, &raw_buffer[start_index], LORA_PACKET_SIZE);
            
            // 2. CRC 校验逻辑
            // 计算�?40 字节�?0�?float）的校验�?
            uint16_t cal_crc = Calculate_CRC16(&raw_buffer[start_index], LORA_DATA_SIZE);
            
            // 3. 比对：计算出�?CRC 是否等于结构体中收到�?CRC
            if (cal_crc == received_data.crc16) {
                // 校验通过，标记数据有�?
                is_data_ready = 1; 
                // 可选：在调试阶段打印成功信�?
                // printf("[LoRa] CRC Pass\r\n");
            } else {
                // 校验失败，说明中间字节有误，不设�?is_data_ready
                // 打印错误以便排查环境干扰
                printf("[LoRa] CRC Error! Cal:0x%04X, Recv:0x%04X\r\n", cal_crc, received_data.crc16);
            }
            
            // 4. 复位索引，准备接收下一�?
            buffer_index = 0;
        }
    }
}

/**
 * @brief 数据处理任务，在 main while(1) 中调�?
 */
void LoRa_Task_Process(void) {
    if (is_data_ready) {
        is_data_ready = 0; // 清除标志
#if 0
        // --- 打印到电�?(UART1) ---
        printf("\r\n=== LoRa Data Received ===\r\n");
        printf("Temp: %.1f C, Hum: %.1f %%\r\n", received_data.temperature, received_data.humidity);
        printf("Attitude: Roll=%.2f, Pitch=%.2f (Raw: %.2f, %.2f)\r\n", 
               received_data.acc_x, received_data.acc_y, received_data.acc_x, received_data.acc_y); // 这里根据你实际数据的含义打印
        printf("GPS: Lat=%.6f, Lon=%.6f\r\n", received_data.latitude, received_data.longitude);
        printf("==========================\r\n");
#endif
			
#if 1
			HAL_UART_Transmit(&huart1, (uint8_t*)&received_data, sizeof(LoRa_Packet_t), 100);
#endif
			
#if 1
// ----------------------------------------
        // 2. 屏幕数据刷新 (新增部分)
        // ----------------------------------------
        char lcd_buf[50]; // 定义一个字符缓冲区

        // --- 设置字体 ---
        LCD_SetFont(&Font16x32); // 使用大字体更清晰

        // --- 显示温度 (Temp) ---
        // 为了防止数字变化时原来的字没擦干净，我们在字符串后面多加几个空格覆�?
        sprintf(lcd_buf, "Temp: %.1f C     ", received_data.temperature); 
        LCD_SetColors(RED, BLACK); // 温度用红色显�?
        NT35510_DispStringLine_EN(LINE(3), (char*)lcd_buf);

        // --- 显示湿度 (Hum) ---
        sprintf(lcd_buf, "Hum:  %.1f %%     ", received_data.humidity);
        LCD_SetColors(BLUE, BLACK); // 湿度用蓝色显�?
        NT35510_DispStringLine_EN(LINE(4), (char*)lcd_buf);

        // --- 显示姿�?(Pitch/Roll) ---
        sprintf(lcd_buf, "acc_x: %.3f   gyro_x: %.3f", received_data.acc_x, received_data.gyro_x);
        LCD_SetColors(YELLOW, BLACK); // 姿态用黄色
        NT35510_DispStringLine_EN(LINE(6), (char*)lcd_buf);
				
        sprintf(lcd_buf, "acc_y: %.3f   gyro_y: %.3f", received_data.acc_y, received_data.gyro_y);
        LCD_SetColors(YELLOW, BLACK); // 姿态用黄色
        NT35510_DispStringLine_EN(LINE(8), (char*)lcd_buf);
				
        sprintf(lcd_buf, "acc_z: %.3f   gyro_z: %.3f", received_data.acc_z, received_data.gyro_z);
        LCD_SetColors(YELLOW, BLACK); // 姿态用黄色
        NT35510_DispStringLine_EN(LINE(10), (char*)lcd_buf);

        // --- 如果�?RSSI (信号强度) ---
        // 假设你有一个变量存RSSI，如果没有可以先显示经纬�?
        sprintf(lcd_buf, "Lat: %.4f  Lon: %.4f", received_data.latitude, received_data.longitude);
        LCD_SetColors(WHITE, BLACK);
        NT35510_DispStringLine_EN(LINE(12), (char*)lcd_buf);
        
        // 记得最后改回白色，以免影响其他显示
        LCD_SetColors(WHITE, BLACK);

        // --- RTK �o�Ȩd? (�l�Ш\��) ---
        sprintf(lcd_buf, "Fix: %d  Sats: %d  Alt: %.1f m", received_data.fix_type, received_data.satellites, received_data.altitude);
        LCD_SetColors(CYAN, BLACK);
        NT35510_DispStringLine_EN(LINE(14), (char*)lcd_buf);

        sprintf(lcd_buf, "HDOP: %.1f", received_data.hdop);
        LCD_SetColors(CYAN, BLACK);
        NT35510_DispStringLine_EN(LINE(16), (char*)lcd_buf);

        LCD_SetColors(WHITE, BLACK);

        // ----------------------------------------
        // 3. 心跳指示 (Heartbeat)
        // ----------------------------------------
        // 定义一个静态变量，每次进入函数时它的值会被保�?
        static uint8_t heartbeat_toggle = 0; 
        
        heartbeat_toggle = !heartbeat_toggle; // 每次取反�?->1, 1->0

        if (heartbeat_toggle) {
            LCD_SetColors(GREEN, BLACK); 
            // 在第0行末尾画一个圈或星�?
            NT35510_DispStringLine_EN(LINE(5), (char*)"                      *"); 
        } else {
            LCD_SetColors(BLACK, BLACK); // 用黑色擦除它
            NT35510_DispStringLine_EN(LINE(5), (char*)"                      *"); 
        }
#endif
			
    }
}

/**
  * @brief CRC16-MODBUS 校验算法
  * @param ptr: 数据首地址, len: 需要校验的长度
  * @retval 计算出的16位校验�?
  */
uint16_t Calculate_CRC16(uint8_t *ptr, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= ptr[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}


