#include "data.h"
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>

uint16_t adc_buffer_foc[6200];
volatile uint8_t DMA_half_foc = 0;//volatile初始化标志位，防止被更改
volatile uint8_t DMA_finish_foc = 0;
#define RVE 1.333f

static unsigned char old_dat = 0xCC; // 保存上一次的数据，初始值为11001100

void send_595(unsigned char dat)
{
    unsigned char i;
    unsigned char temp = dat; // 使用临时变量避免修改原始数据
    
    for(i = 0; i < 8; i++)
    {
        if(temp & 0x80)
            HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_5, GPIO_PIN_RESET);HAL_GPIO_WritePin(GPIOF, GPIO_PIN_5, GPIO_PIN_SET); // 数据移位
        temp = temp << 1;
    }
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4, GPIO_PIN_RESET);HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4, GPIO_PIN_RESET); // 输出
    
    old_dat = dat; // 保存当前发送的数据
}

// 切换直流电机（保持高位）
void send_DM(unsigned char new_dat)
{
    // 组合：新数据的低4位 + 旧数据的高4位
    unsigned char combined_dat = (new_dat & 0x0F) | (old_dat & 0xF0);
    send_595(combined_dat);
}

// 切换FOC（保持低位）
void send_AM(unsigned char new_dat)
{
    // 组合：新数据的高4位 + 旧数据的低4位
    unsigned char combined_dat = (new_dat & 0xF0) | (old_dat & 0x0F);
    send_595(combined_dat);
}

////切换电机，保持原来状态
//send_DM(0x0C); // DM1: 00001100 → 最终数据 11001100
//send_DM(0x0D); // DM2: 00001101 → 最终数据 11001101
//send_DM(0x03); // DM3: 00000011 → 最终数据 11000011
//send_DM(0x07); // DM4: 00000111 → 最终数据 11000111

//send_AM(0xC0); // AM1: 11000000 → 最终数据 11001100
//send_AM(0xD0); // AM2: 11010000 → 最终数据 11011100
//send_AM(0x30); // AM3: 00110000 → 最终数据 00111100
//send_AM(0x70); // AM4: 01110000 → 最终数据 01111100
