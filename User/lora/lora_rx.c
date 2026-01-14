#include "lora_protocol.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include "bsp_nt35510_lcd.h"
#include "fonts.h"

// --- 变量定义 ---
#define RX_BUFFER_SIZE 128       // 定义一个足够大的缓冲区
uint8_t rx_byte_u3;              // UART3 临时接收 1 字节的变量
uint8_t raw_buffer[RX_BUFFER_SIZE]; // 滑动窗口缓冲区
uint8_t buffer_index = 0;        // 当前缓冲区索引

LoRa_Packet_t received_data;     // 解析成功后的数据包
uint8_t is_data_ready = 0;       // 数据包就绪标志

/**
 * @brief 接收处理函数，在中断回调中被调用
 * @param byte 刚刚接收到的一个字节
 */
void LoRa_Rx_Handler(uint8_t byte) {
    // 1. 入队：将新字节存入缓冲区
    raw_buffer[buffer_index++] = byte;

    // 2. 环形保护：防止数组越界
    if (buffer_index >= RX_BUFFER_SIZE) {
        buffer_index = 0; 
    }

    // 3. 校验：只有接收字节数大于包长时才开始检查
    if (buffer_index >= LORA_PACKET_SIZE) {
        // 4. 检查帧尾 (0x7F800000 -> 00 00 80 7F)
        // 取当前索引向前推4个字节，转换成 uint32 检查
        uint32_t *tail_ptr = (uint32_t*)&raw_buffer[buffer_index - 4];
        
        if (*tail_ptr == 0x7F800000) {
            // --- 帧尾匹配成功 ---
            
            // 计算包头的起始位置
            uint8_t start_index = buffer_index - LORA_PACKET_SIZE;
            
            // 内存拷贝：将缓冲区的数据复制到结构体中
            memcpy(&received_data, &raw_buffer[start_index], LORA_PACKET_SIZE);
            
            is_data_ready = 1; // 设置标志位，通知 main 函数处理
            
            // 5. 复位索引：为了简单起见，接收成功后清空缓冲区索引
            // 这样可以避免处理残留数据，准备接收下一包
            buffer_index = 0;
        }
    }
}

/**
 * @brief 数据处理任务，在 main while(1) 中调用
 */
void LoRa_Task_Process(void) {
    if (is_data_ready) {
        is_data_ready = 0; // 清除标志
#if 0
        // --- 打印到电脑 (UART1) ---
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
        // 为了防止数字变化时原来的字没擦干净，我们在字符串后面多加几个空格覆盖
        sprintf(lcd_buf, "Temp: %.1f C     ", received_data.temperature); 
        LCD_SetColors(RED, BLACK); // 温度用红色显示
        NT35510_DispStringLine_EN(LINE(3), (char*)lcd_buf);

        // --- 显示湿度 (Hum) ---
        sprintf(lcd_buf, "Hum:  %.1f %%     ", received_data.humidity);
        LCD_SetColors(BLUE, BLACK); // 湿度用蓝色显示
        NT35510_DispStringLine_EN(LINE(4), (char*)lcd_buf);

        // --- 显示姿态 (Pitch/Roll) ---
        sprintf(lcd_buf, "acc_x: %.3f   gyro_x: %.3f", received_data.acc_x, received_data.gyro_x);
        LCD_SetColors(YELLOW, BLACK); // 姿态用黄色
        NT35510_DispStringLine_EN(LINE(6), (char*)lcd_buf);
				
        sprintf(lcd_buf, "acc_y: %.3f   gyro_y: %.3f", received_data.acc_y, received_data.gyro_y);
        LCD_SetColors(YELLOW, BLACK); // 姿态用黄色
        NT35510_DispStringLine_EN(LINE(8), (char*)lcd_buf);
				
        sprintf(lcd_buf, "acc_z: %.3f   gyro_z: %.3f", received_data.acc_z, received_data.gyro_z);
        LCD_SetColors(YELLOW, BLACK); // 姿态用黄色
        NT35510_DispStringLine_EN(LINE(10), (char*)lcd_buf);

        // --- 如果有 RSSI (信号强度) ---
        // 假设你有一个变量存RSSI，如果没有可以先显示经纬度
        sprintf(lcd_buf, "Lat: %.4f       Lat: %.4f", received_data.latitude, received_data.longitude);
        LCD_SetColors(WHITE, BLACK);
        NT35510_DispStringLine_EN(LINE(12), (char*)lcd_buf);
        
        // 记得最后改回白色，以免影响其他显示
        LCD_SetColors(WHITE, BLACK);
#endif
			
    }
}
