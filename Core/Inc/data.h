#ifndef __DATA_H__
#define __DATA_H__

#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>
extern uint16_t adc_buffer_foc[6200];
void send_595(unsigned char dat);
void send_DM(unsigned char new_dat);
void send_AM(unsigned char new_dat);

#endif
