#include "lora_protocol.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

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

        // --- 打印到电脑 (UART1) ---
        printf("\r\n=== LoRa Data Received ===\r\n");
        printf("Temp: %.1f C, Hum: %.1f %%\r\n", received_data.temperature, received_data.humidity);
        printf("Attitude: Roll=%.2f, Pitch=%.2f (Raw: %.2f, %.2f)\r\n", 
               received_data.acc_x, received_data.acc_y, received_data.acc_x, received_data.acc_y); // 这里根据你实际数据的含义打印
        printf("GPS: Lat=%.6f, Lon=%.6f\r\n", received_data.latitude, received_data.longitude);
        printf("==========================\r\n");
    }
}
