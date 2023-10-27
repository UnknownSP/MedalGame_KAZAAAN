/*
 * pwm.c
 *
 *  Created on: Mar 11, 2023
 *      Author: UnknownSP
 */

#include "../Inc/pwm.h"
#include <stdlib.h>
#include "stm32f4xx_hal.h"

//extern TIM_HandleTypeDef htim3;

void D_PWM_Init(void){
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
}

void D_PWM_Set(int channel, int value){
	switch(channel){
	case 1:
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, value);
		break;
	case 2:
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, value);
		break;
	case 3:
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, value);
		break;
	case 4:
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, value);
		break;
	}
}


