/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "usart.h"           // ?? huart1 ??
#include "lora_protocol.h"
#include "bsp_nt35510_lcd.h"
#include "fonts.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

osMessageQueueId_t LoRaDataQueueHandle;  // ????????

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskLoRa */
osThreadId_t TaskLoRaHandle;
const osThreadAttr_t TaskLoRa_attributes = {
  .name = "TaskLoRa",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for TaskLCD */
osThreadId_t TaskLCDHandle;
const osThreadAttr_t TaskLCD_attributes = {
  .name = "TaskLCD",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
	
	// ????:?? 2 ??,?????? LoRa_Packet_t ??????
	LoRaDataQueueHandle = osMessageQueueNew(2, sizeof(LoRa_Packet_t), NULL);
	
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of TaskLoRa */
  TaskLoRaHandle = osThreadNew(StartTask02, NULL, &TaskLoRa_attributes);

  /* creation of TaskLCD */
  TaskLCDHandle = osThreadNew(StartTask03, NULL, &TaskLCD_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the TaskLoRa thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
  for(;;)
  {
// ????? LoRa ???? (is_data_ready ?????)
    if (is_data_ready) 
    {
      is_data_ready = 0; // ?????

      // 1. ?????? VOFA+
      HAL_UART_Transmit(&huart1, (uint8_t*)&received_data, sizeof(LoRa_Packet_t), 10);
      
      // 2. ???????,??? LCD ?? (????)
      osMessageQueuePut(LoRaDataQueueHandle, &received_data, 0U, 0U);
    }
    
    // ??????? 5ms ???,???????????? CPU
    osDelay(5);  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the TaskLCD thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
	LoRa_Packet_t lcd_data;
  char lcd_buf[64];
  
  // ??????????,??????
  LCD_SetColors(WHITE, BLACK);
  NT35510_DispStringLine_EN(LINE(1), (char*)"   RTOS LoRa Terminal ");
  
  for(;;)
  {
    // ??????,????????
    // osWaitForever ?????????,LCD ??????? CPU ??
    if (osMessageQueueGet(LoRaDataQueueHandle, &lcd_data, NULL, osWaitForever) == osOK)
    {
      // --- ?????? ---
      LCD_SetFont(&Font16x32);
      
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
        sprintf(lcd_buf, "Lat: %.4f       Lon: %.4f", received_data.latitude, received_data.longitude);
        LCD_SetColors(WHITE, BLACK);
        NT35510_DispStringLine_EN(LINE(12), (char*)lcd_buf);
        
        // 记得最后改回白色，以免影响其他显示
        LCD_SetColors(WHITE, BLACK);
				
				// ----------------------------------------
        // 3. 心跳指示 (Heartbeat)
        // ----------------------------------------
        // 定义一个静态变量，每次进入函数时它的值会被保留
        static uint8_t heartbeat_toggle = 0; 
        
        heartbeat_toggle = !heartbeat_toggle; // 每次取反：0->1, 1->0

        if (heartbeat_toggle) {
            LCD_SetColors(GREEN, BLACK); 
            // 在第0行末尾画一个圈或星号
            NT35510_DispStringLine_EN(LINE(5), (char*)"                      *"); 
        } else {
            LCD_SetColors(BLACK, BLACK); // 用黑色擦除它
            NT35510_DispStringLine_EN(LINE(5), (char*)"                      *"); 
        }
    }
  }  /* USER CODE END StartTask03 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

