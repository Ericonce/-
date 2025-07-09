/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "data.h"
#include <stdlib.h>
#include <rtthread.h>
#include <rthw.h>
/* #include "perform_fft.h" */



/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define RVE 1.333f
#define ADC_SCALE (3.3f / 4095.0f)
#define BUFFER_SIZE 1080*4

#define EVENT_FLAG_ADC_1_half   (1 << 0)         //ADC采集完成信号
#define EVENT_FLAG_ADC_2_half   (1 << 1)
#define EVENT_FLAG_ADC_1_entire   (1 << 2)
#define EVENT_FLAG_ADC_2_entire   (1 << 3)

#define EVENT_FLAG_bufferA  (1 << 4)        //事件处理完
#define EVENT_FLAG_bufferB  (1 << 5)
#define EVENT_FLAG_bufferA_foc  (1 << 6)
#define EVENT_FLAG_bufferB_foc  (1 << 7)

#define EVENT_FLAG_SENT          (1 << 8)    // UART发送前半信号
#define EVENT_FLAG_SENT_FOC      (1 << 9)  

#define UART_RX_LEN 64
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
float a = 3.14f;
float voltage_adc1,voltage_adc2;
uint16_t adc_buffer[6200];     //数据采集数组
volatile uint8_t DMA_half = 0;//volatile初始化标志位，防止被更改
volatile uint8_t DMA_finish = 0;
volatile int uart_tx_busy = 0;//串口
char motor_flag = 0;
uint8_t uart_rx_buf[UART_RX_LEN]; // DMA接收缓冲区

static rt_thread_t uart_tid = RT_NULL;//线程句柄
static rt_thread_t event_tid = RT_NULL;
static rt_event_t global_event;



char UART_1_HALF_COMPLETE_ELE;  //串口发送标志位
char UART_1_ENTIRE_COMPLETE_ELE;
char UART_2_HALF_COMPLETE_ELE;
char UART_2_ENTIRE_COMPLETE_ELE;

char UART_1_HALF_COMPLETE_SOUND;  //串口发送标志位
char UART_1_ENTIRE_COMPLETE_SOUND;
char UART_2_HALF_COMPLETE_SOUND;
char UART_2_ENTIRE_COMPLETE_SOUND;

char UART_1_HALF_COMPLETE_SHOCK;  //串口发送标志位
char UART_1_ENTIRE_COMPLETE_SHOCK;
char UART_2_HALF_COMPLETE_SHOCK;
char UART_2_ENTIRE_COMPLETE_SHOCK;

float *ele_buffer = RT_NULL;//定义动态数组指针
float *ele_buffer_foc = RT_NULL;
char *buffer_A = RT_NULL;
char *buffer_B = RT_NULL;
char *buffer_A_foc = RT_NULL;
char *buffer_B_foc = RT_NULL;
char *buffer_A_sound = RT_NULL;
char *buffer_B_sound = RT_NULL;
char *buffer_A_foc_sound = RT_NULL;
char *buffer_B_foc_sound = RT_NULL;
char *buffer_A_shock = RT_NULL;
char *buffer_B_shock = RT_NULL;
char *buffer_A_foc_shock = RT_NULL;
char *buffer_B_foc_shock = RT_NULL;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

const char* motor_change(char flag)//电机数据通道切换
{
  switch (flag)
{
  case 0: // dcm1
  return "dcm_1";//退出并返回flag
  case 1: // dcm2
  return "dcm_2";
  case 2: // dcm3
  return "dcm_3";
  case 3: // dcm4
  return "dcm_4";
  case 4: // foc1
  return "foc_1";
  case 5: // foc2
  return "foc_2";
  case 6: // foc3
  return "foc_3";
  case 7: // foc4
  return "foc_4";
  default: 
  return "unknown"; 
}
}

//数据发送线程
static void uart_thread_entry(void *parameter)
{
    rt_uint32_t recved;
while (1)
{
    if (rt_event_recv(global_event,
                          EVENT_FLAG_bufferA | EVENT_FLAG_bufferA_foc | 
                          EVENT_FLAG_bufferB | EVENT_FLAG_bufferB_foc,
                          RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                          RT_WAITING_FOREVER,
                          &recved) == RT_EOK)//等待接受事件标志+自动清除标志
        {
            if (recved & EVENT_FLAG_bufferA)
            {
                  /* 发送电流 */
                  if(buffer_A) rt_free(buffer_A);
                  buffer_A = (char *)rt_malloc(1080 * 4 * sizeof(char));
									if (!buffer_A) {
									rt_kprintf("Malloc failed: buffer_A\n");
									continue;
													}
                  const char* motor = motor_change(motor_flag); 
									int offset = 0;//sniprintf格式化数组
                  offset+= snprintf(buffer_A + offset,BUFFER_SIZE-offset,"%s\n%s\n%s\n","FF FF",motor,"1");
                  for (int i = 0; i < 3070; i+=3)
                  {
                    offset+= snprintf(buffer_A + offset,BUFFER_SIZE-offset,"%.5f\n",ele_buffer[i]-0.06f);
                  }
                    offset+= snprintf(buffer_A + offset,BUFFER_SIZE-offset,"%s\n","FF FE");
									while (uart_tx_busy) 
									rt_thread_mdelay(1);
									uart_tx_busy = 1;
                  HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buffer_A, offset);
                  UART_1_HALF_COMPLETE_ELE = 1;
                  rt_thread_mdelay(3000);

                  /* 发送声音 */
                  if(buffer_A_sound) rt_free(buffer_A_sound);
                  buffer_A_sound = (char *)rt_malloc(1080 * 4 * sizeof(char));
									if (!buffer_A_sound) {
									rt_kprintf("Malloc failed: buffer_A_sound\n");
									continue;
													}
                  offset = 0; 
									motor = motor_change(motor_flag); 
                  offset+= snprintf(buffer_A_sound + offset,BUFFER_SIZE-offset,"%s\n%s\n%s\n","FF FF",motor,"2");
                  for (int i = 1; i < 3071; i+=3)
                  {
                  offset+= snprintf(buffer_A_sound + offset,BUFFER_SIZE-offset,"%.5f\n",ele_buffer[i]-0.16f);
                  }
                  offset+= snprintf(buffer_A_sound + offset,BUFFER_SIZE-offset,"%s\n","FF FE");
									while (uart_tx_busy) 
									rt_thread_mdelay(1);
									uart_tx_busy = 1;									
                  HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buffer_A_sound, offset);
                  UART_1_HALF_COMPLETE_SOUND = 1;
                  rt_thread_mdelay(3000);

                  /* 发送震动 */
                  if(buffer_A_shock) rt_free(buffer_A_shock);
                  buffer_A_shock = (char *)rt_malloc(1080 * 4 * sizeof(char)); 
									if (!buffer_A_sound) {
									rt_kprintf("Malloc failed: buffer_A_sound\n");
									continue;
													}											
                  offset = 0;
									motor = motor_change(motor_flag); 
                  offset+= snprintf(buffer_A_shock + offset,BUFFER_SIZE-offset,"%s\n%s\n%s\n","FF FF",motor,"3");
                  for (int i = 2; i < 3072; i+=3)
                  {
                  offset+= snprintf(buffer_A_shock + offset,BUFFER_SIZE-offset,"%.5f\n",ele_buffer[i]);
                  }
                  offset+= snprintf(buffer_A_shock + offset,BUFFER_SIZE-offset,"%s\n","FF FE");
									while (uart_tx_busy) 
									rt_thread_mdelay(1);
									uart_tx_busy = 1;									
                  HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buffer_A_shock, offset);
                  UART_1_HALF_COMPLETE_SHOCK = 1;
                  rt_thread_mdelay(3000);
            }

            if (recved & EVENT_FLAG_bufferA_foc)
            {
                 /* 发送foc电流 */
                 if(buffer_A_foc) rt_free(buffer_A_foc);
                 buffer_A_foc = (char *)rt_malloc(1080 * 4 * sizeof(char));
								 if (!buffer_A_foc) {
								 rt_kprintf("Malloc failed: buffer_A_foc\n");
								 continue;
													}											
                 int offset = 0;//sniprintf格式化数组
								 const char* motor = motor_change(motor_flag); 
                 offset+= snprintf(buffer_A_foc + offset,BUFFER_SIZE-offset,"%s\n%s\n%s\n","FF FF",motor,"1");
                 for (int i = 0; i < 3070; i+=3)
                 {
                 offset+= snprintf(buffer_A_foc + offset,BUFFER_SIZE-offset,"%.5f\n",ele_buffer_foc[i]);
                 }
                 offset+= snprintf(buffer_A_foc + offset,BUFFER_SIZE-offset,"%s\n","FF FE");
								 while (uart_tx_busy) 
								 rt_thread_mdelay(1);
								 uart_tx_busy = 1;							 
                 HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buffer_A_foc, offset);
                 UART_2_HALF_COMPLETE_ELE = 1;
                 rt_thread_mdelay(3000);

                 /* 发送foc声音 */
                 if(buffer_A_foc_sound) rt_free(buffer_A_foc_sound);
                 buffer_A_foc_sound = (char *)rt_malloc(1080 * 4 * sizeof(char));
								 if (!buffer_A_foc_sound) {
								 rt_kprintf("Malloc failed: buffer_A_foc_sound\n");
								 continue;
													}											 
                 offset = 0;//sniprintf格式化数组
								 motor = motor_change(motor_flag);
                 offset+= snprintf(buffer_A_foc_sound + offset,BUFFER_SIZE-offset,"%s\n%s\n%s\n","FF FF",motor,"2");
                 for (int i = 1; i < 3071; i+=3)
                 {
                 offset+= snprintf(buffer_A_foc_sound + offset,BUFFER_SIZE-offset,"%.5f\n",ele_buffer_foc[i]);
                 }
                 offset+= snprintf(buffer_A_foc_sound + offset,BUFFER_SIZE-offset,"%s\n","FF FE");
								 while (uart_tx_busy) 
								 rt_thread_mdelay(1);
								 uart_tx_busy = 1;								 
                 HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buffer_A_foc_sound, offset);
                 UART_2_HALF_COMPLETE_SOUND = 1;
                 rt_thread_mdelay(3000);

                 /* 发送foc震动 */
                 if(buffer_A_foc_shock) rt_free(buffer_A_foc_shock);
                 buffer_A_foc_shock = (char *)rt_malloc(1080 * 4 * sizeof(char));
								 if (!buffer_A_foc_shock) {
								 rt_kprintf("Malloc failed: buffer_A_foc_shock\n");
								 continue;
													}															 
                 offset = 0;//sniprintf格式化数组
								 motor = motor_change(motor_flag);
                 offset+= snprintf(buffer_A_foc_shock + offset,BUFFER_SIZE-offset,"%s\n%s\n%s\n","FF FF",motor,"3");
                 for (int i = 2; i < 3072; i+=3)
                 {
                 offset+= snprintf(buffer_A_foc_shock + offset,BUFFER_SIZE-offset,"%.5f\n",ele_buffer_foc[i]);
                 }
                 offset+= snprintf(buffer_A_foc_shock + offset,BUFFER_SIZE-offset,"%s\n","FF FE");
								 while (uart_tx_busy) 
								 rt_thread_mdelay(1);
								 uart_tx_busy = 1;												 
                 HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buffer_A_foc_shock, offset);
                 UART_2_HALF_COMPLETE_SHOCK = 1;
                 rt_thread_mdelay(3000);
            }
            if (recved & EVENT_FLAG_ADC_1_entire)
            {
                  /* 发送电流 */
                  if(buffer_B) rt_free(buffer_B);                 
                  buffer_B = (char *)rt_malloc(1080 * 4 * sizeof(char));
								  if (!buffer_B) {
								  rt_kprintf("Malloc failed: buffer_B\n");
								  continue;
													}															 									
                  int offset = 0;//sniprintf格式化数组
									const char* motor = motor_change(motor_flag);
                  offset+= snprintf(buffer_B + offset,BUFFER_SIZE-offset,"%s\n%s\n%s\n","FF FF",motor,"1");
                  for (int i = 3072; i < 6142; i+=3)
                  {
                    offset+= snprintf(buffer_B + offset,BUFFER_SIZE-offset,"%.5f\n",ele_buffer[i]-0.06f);
                  }
                    offset+= snprintf(buffer_B + offset,BUFFER_SIZE-offset,"%s\n","FF FE");
								  while (uart_tx_busy) 
								  rt_thread_mdelay(1);
								  uart_tx_busy = 1;													
                  HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buffer_B, offset);
                  UART_1_ENTIRE_COMPLETE_ELE = 1;
                  rt_thread_mdelay(3000);

                  /* 发送声音 */
                  if(buffer_B_sound) rt_free(buffer_B_sound);
                  buffer_B_sound = (char *)rt_malloc(1080 * 4 * sizeof(char));
								  if (!buffer_B_sound) {
								  rt_kprintf("Malloc failed: buffer_B_sound\n");
								  continue;
													}															 																		
                  offset = 0; 
									motor = motor_change(motor_flag);
                  offset+= snprintf(buffer_B_sound + offset,BUFFER_SIZE-offset,"%s\n%s\n%s\n","FF FF",motor,"2");
                  for (int i = 3073; i < 6143; i+=3)
                  {
                  offset+= snprintf(buffer_B_sound + offset,BUFFER_SIZE-offset,"%.5f\n",ele_buffer[i]-0.16f);
                  }
                  offset+= snprintf(buffer_B_sound + offset,BUFFER_SIZE-offset,"%s\n","FF FE");
								  while (uart_tx_busy) 
								  rt_thread_mdelay(1);
								  uart_tx_busy = 1;												
                  HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buffer_B_sound, offset);
                  UART_1_ENTIRE_COMPLETE_SOUND = 1;
                  rt_thread_mdelay(3000);

                  /* 发送震动 */
                  if(buffer_B_shock) rt_free(buffer_B_shock);
                  buffer_B_shock = (char *)rt_malloc(1080 * 4 * sizeof(char));
								  if (!buffer_B_shock) {
								  rt_kprintf("Malloc failed: buffer_B_shock\n");
								  continue;
													}															 																											
                  offset = 0; 
									motor = motor_change(motor_flag);
                  offset+= snprintf(buffer_B_shock + offset,BUFFER_SIZE-offset,"%s\n%s\n%s\n","FF FF",motor,"3");
                  for (int i = 3074; i < 6144; i+=3)
                  {
                  offset+= snprintf(buffer_B_shock + offset,BUFFER_SIZE-offset,"%.5f\n",ele_buffer[i]);
                  }
                  offset+= snprintf(buffer_B_shock + offset,BUFFER_SIZE-offset,"%s\n","FF FE");
								  while (uart_tx_busy) 
								  rt_thread_mdelay(1);
								  uart_tx_busy = 1;												
                  HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buffer_B_shock, offset);
                  UART_1_ENTIRE_COMPLETE_SHOCK = 1;
                  rt_thread_mdelay(3000);             

            }

            if (recved & EVENT_FLAG_ADC_2_entire)
            {
                 /* 发送foc电流 */                
                 if(buffer_B_foc) rt_free(buffer_B_foc);
                 buffer_B_foc = (char *)rt_malloc(1080 * 4 * sizeof(char));
								 if (!buffer_B_foc) {
								 rt_kprintf("Malloc failed: buffer_B_foc\n");
								 continue;
													}													
                 int offset = 0;//sniprintf格式化数组
								 const char* motor = motor_change(motor_flag);
                 offset+= snprintf(buffer_B_foc + offset,BUFFER_SIZE-offset,"%s\n%s\n%s\n","FF FF",motor,"1");
                 for (int i = 3072; i < 6142; i+=3)
                 {
                 offset+= snprintf(buffer_B_foc + offset,BUFFER_SIZE-offset,"%.5f\n",ele_buffer_foc[i]);
                 }
                 offset+= snprintf(buffer_B_foc + offset,BUFFER_SIZE-offset,"%s\n","FF FE");
								 while (uart_tx_busy) 
								 rt_thread_mdelay(1);
								 uart_tx_busy = 1;											 
                 HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buffer_B_foc, offset);
                 UART_2_ENTIRE_COMPLETE_ELE = 1;
                 rt_thread_mdelay(3000);

                 /* 发送foc声音 */
                 if(buffer_B_foc_sound) rt_free(buffer_B_foc_sound);
                 buffer_B_foc_sound = (char *)rt_malloc(1080 * 4 * sizeof(char));
								 if (!buffer_B_foc_sound) {
								 rt_kprintf("Malloc failed: buffer_B_foc_sound\n");
								 continue;
													}																					 
                 offset = 0;//sniprintf格式化数组
								 motor = motor_change(motor_flag);
                 offset+= snprintf(buffer_B_foc_sound + offset,BUFFER_SIZE-offset,"%s\n%s\n%s\n","FF FF",motor,"2");
                 for (int i = 3073; i < 6143; i+=3)
                 {
                 offset+= snprintf(buffer_B_foc_sound + offset,BUFFER_SIZE-offset,"%.5f\n",ele_buffer_foc[i]);
                 }
                 offset+= snprintf(buffer_B_foc_sound + offset,BUFFER_SIZE-offset,"%s\n","FF FE");
								 while (uart_tx_busy) 
								 rt_thread_mdelay(1);
								 uart_tx_busy = 1;											 
                 HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buffer_B_foc_sound, offset);
                 UART_2_ENTIRE_COMPLETE_SOUND = 1;
                 rt_thread_mdelay(3000);

                 /* 发送foc震动 */
                 if(buffer_B_foc_shock) rt_free(buffer_B_foc_shock);
                 buffer_B_foc_shock = (char *)rt_malloc(1080 * 4 * sizeof(char));
								 if (!buffer_B_foc_shock) {
								 rt_kprintf("Malloc failed: buffer_B_foc_shock\n");
								 continue;
													}																					 								 
                 offset = 0;//sniprintf格式化数组
								 motor = motor_change(motor_flag);
                 offset+= snprintf(buffer_B_foc_shock + offset,BUFFER_SIZE-offset,"%s\n%s\n%s\n","FF FF",motor,"3");
                 for (int i = 3074; i < 6144; i+=3)
                 {
                 offset+= snprintf(buffer_B_foc_shock + offset,BUFFER_SIZE-offset,"%.5f\n",ele_buffer_foc[i]);
                 }
                 offset+= snprintf(buffer_B_foc_shock + offset,BUFFER_SIZE-offset,"%s\n","FF FE");
								 while (uart_tx_busy) 
								 rt_thread_mdelay(1);
								 uart_tx_busy = 1;			
                 HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buffer_B_foc_shock, offset);
                 UART_2_ENTIRE_COMPLETE_SHOCK = 1;
                 rt_thread_mdelay(3000);
            }
            }
}
}

//数据处理线程
static void event_handler_thread(void *parameter)
{
    rt_uint32_t recved;
    if(ele_buffer) rt_free(ele_buffer);
    ele_buffer = (float *)rt_malloc(2048 * 3 * sizeof(float));
    if(ele_buffer_foc) rt_free(ele_buffer_foc);
    ele_buffer_foc = (float *)rt_malloc(2048 * 3 * sizeof(float));
    if (ele_buffer == RT_NULL || ele_buffer_foc == RT_NULL)
        rt_kprintf("UART buffer malloc failed!\n");

    while (1)
    {
        if (rt_event_recv(global_event,
                          EVENT_FLAG_ADC_1_half | EVENT_FLAG_ADC_2_half | 
                          EVENT_FLAG_ADC_1_entire | EVENT_FLAG_ADC_2_entire,
                          RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                          RT_WAITING_FOREVER,
                          &recved) == RT_EOK)//等待接受事件标志+自清除标志
        {
            if (recved & EVENT_FLAG_ADC_1_half)
            {
                rt_kprintf("[Event] ADC_1_half completed\n");
                // 处理 ADC1前半 数据
                for (int i = 0; i < 3072; i+=3)
            {
               float v1 = adc_buffer[i] * ADC_SCALE * RVE;
               float v2 = adc_buffer[i + 1] * ADC_SCALE;
               float v3 = adc_buffer[i + 2] * ADC_SCALE;

               ele_buffer[i]     = v1 * RVE;  // 电流通道
               ele_buffer[i + 1] = v2;  // 声音通道
               ele_buffer[i + 2] = v3;  // 震动通道
            }
            rt_event_send(global_event,EVENT_FLAG_bufferA);
            rt_thread_mdelay(2);//处理完数据让出CPU，执行发送线程
            }

            if (recved & EVENT_FLAG_ADC_2_half)
            {

                rt_kprintf("[Event] ADC_2_half completed\n");
                // 处理 ADC2前半 数据
               for (int i = 0; i < 3072; i+=3)
            {
               float v1 = adc_buffer_foc[i] * ADC_SCALE * RVE;
               float v2 = adc_buffer_foc[i + 1] * ADC_SCALE;
               float v3 = adc_buffer_foc[i + 2] * ADC_SCALE;

               ele_buffer_foc[i]     = v1 * RVE;  // 电流通道
               ele_buffer_foc[i + 1] = v2;  // 声音通道
               ele_buffer_foc[i + 2] = v3;  // 震动通道
            }
            rt_event_send(global_event,EVENT_FLAG_bufferA_foc);
            rt_thread_mdelay(2);
            }

            if (recved & EVENT_FLAG_ADC_1_entire)
            {
                rt_event_recv(global_event,
                              EVENT_FLAG_SENT,
                              RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                              RT_WAITING_FOREVER,
                              &recved);
                rt_kprintf("[Event] ADC_1_entire completed\n");//接收前半发送完毕信号
                // 处理 ADC1后半 数据
                for (int i = 3072; i < 6144; i+=3)
           {
               float v1 = adc_buffer[i] * ADC_SCALE * RVE;
               float v2 = adc_buffer[i + 1] * ADC_SCALE;
               float v3 = adc_buffer[i + 2] * ADC_SCALE;

               ele_buffer[i]     = v1 * RVE;  // 电流通道
               ele_buffer[i + 1] = v2;  // 声音通道
               ele_buffer[i + 2] = v3;  // 震动通道

            }
               rt_event_send(global_event,EVENT_FLAG_bufferB);      
               rt_thread_mdelay(2);
            }

            if (recved & EVENT_FLAG_ADC_2_entire)
            {
                 rt_event_recv(global_event,
                              EVENT_FLAG_SENT_FOC,
                              RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                              RT_WAITING_FOREVER,
                              &recved);
                rt_kprintf("[Event] ADC_2_)entire completed\n");//接受前半发送信号
                // 处理 ADC2后半 数据
            for (int i = 3072; i < 6144; i+=3)
        {
               float v1 = adc_buffer_foc[i] * ADC_SCALE * RVE;
               float v2 = adc_buffer_foc[i + 1] * ADC_SCALE;
               float v3 = adc_buffer_foc[i + 2] * ADC_SCALE;

               ele_buffer_foc[i]     = v1 * RVE;  // 电流通道
               ele_buffer_foc[i + 1] = v2;  // 声音通道
               ele_buffer_foc[i + 2] = v3;  // 震动通道
        }
               rt_event_send(global_event,EVENT_FLAG_bufferB_foc);    
               rt_thread_mdelay(2);
            }
        }
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_Base_Init(&htim3);
  HAL_TIM_Base_Start(&htim3);
  HAL_ADC_Start_DMA(&hadc1,(uint32_t*)adc_buffer,2048*3);//启动adc采集 
  HAL_ADC_Start_DMA(&hadc2,(uint32_t*)adc_buffer_foc,2048*3);//启动adc采集 
	
	rt_components_board_init();//rtt初始化 
	
	HAL_UART_Receive_DMA(&huart1, uart_rx_buf, UART_RX_LEN);
__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE); // 使能空闲中断

	rt_kprintf("System START!\n"); // 添加启动日志



/* rt-thread任务调度 */

    global_event = rt_event_create("evt", RT_IPC_FLAG_FIFO);//创建事件
    if (global_event == RT_NULL)
  {
    rt_kprintf("Failed to create event!\n");
    return -1;
  }
    uart_tid = rt_thread_create("uart", uart_thread_entry, RT_NULL, 2048, 8, 50); // 创建动态线程 uart_thread
    if (uart_tid != RT_NULL)
        rt_thread_startup(uart_tid);                                               // 创建动态线程 dma_thread
    event_tid = rt_thread_create("evt_handler", event_handler_thread, RT_NULL,1024, 12, 50);//创建事件处理线程 event_handler_thread               
    if (event_tid != RT_NULL)
        rt_thread_startup(event_tid);

    return 0; // 不再进入while(1)，主控权移交线程  

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//  while (1) {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//  } 

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
        rt_event_send(global_event, EVENT_FLAG_ADC_1_half);
    }
    else if (hadc->Instance == ADC2)
    {
       rt_event_send(global_event, EVENT_FLAG_ADC_2_half);
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)//adc中断回调函数；
{
        if (hadc->Instance == ADC1)
    {
        rt_event_send(global_event, EVENT_FLAG_ADC_1_entire);
    }
        else if (hadc->Instance == ADC2)
    {
        rt_event_send(global_event, EVENT_FLAG_ADC_2_entire);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
			
				uart_tx_busy = 0;//清除串口工作标志位
        if(UART_1_HALF_COMPLETE_SHOCK)
        {
          rt_event_send(global_event, EVENT_FLAG_SENT);//发送完一轮dcm数据
					UART_1_HALF_COMPLETE_SHOCK = 0;
        }

        if(UART_2_HALF_COMPLETE_SHOCK)
        {
          rt_event_send(global_event, EVENT_FLAG_SENT_FOC);
					UART_2_HALF_COMPLETE_SHOCK = 0;
        }               
    }

    
}

void uart_rx_idle_callback(void)//切换电机数据
{
    uint16_t len = UART_RX_LEN - __HAL_DMA_GET_COUNTER(huart1.hdmarx); // 实际长度
    HAL_UART_DMAStop(&huart1); // 停止DMA

    // 简单命令识别
    if (len >= 4)
    {
        if (len >= 4 && memcmp(uart_rx_buf, "foc1", 4) == 0)
      {
        send_AM(0xC0); 
        motor_flag = 4;
      }
        else if (len >= 4 && memcmp(uart_rx_buf, "foc2", 4) == 0)
      {
        send_AM(0xD0);
        motor_flag = 5;
      }
        else if (len >= 4 && memcmp(uart_rx_buf, "foc3", 4) == 0)
      {
        send_AM(0x30);
        motor_flag = 6;
      }
        else if (len >= 4 && memcmp(uart_rx_buf, "foc4", 4) == 0)
      {
        send_AM(0x70);
        motor_flag = 7;
      }
        else if (len >= 4 && memcmp(uart_rx_buf,"dcm1", 4) == 0)
      {
        send_DM(0x0C); 
        motor_flag = 0;
      }
        else if (len >= 4 && memcmp(uart_rx_buf, "dcm2", 4) == 0)
      {
        send_DM(0x0D);
        motor_flag = 1;
      }
        else if (len >= 4 && memcmp(uart_rx_buf,"dcm3", 4) == 0)
      {
        send_DM(0x03); 
        motor_flag = 2;
      }
        else if (len >= 4 && memcmp(uart_rx_buf, "dcm4", 4) == 0)
      {
        send_DM(0x07);    
        motor_flag = 3; 
      }
        else
        printf("error");
    }
    memset(uart_rx_buf, 0, UART_RX_LEN); // 清空
    HAL_UART_Receive_DMA(&huart1, uart_rx_buf, UART_RX_LEN); // 重启DMA
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
