/*
 * GPIO.c
 *
 *  Created on: Mar 18, 2023
 *      Author: UnknownSP
 */

#include "../Inc/GPIO.h"
#include <stdlib.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

void D_GPIO_Set(GPIO_TypeDef* gpio_type, uint16_t gpio_pin){
	HAL_GPIO_WritePin(gpio_type,gpio_pin,GPIO_PIN_SET);
}

void D_GPIO_Reset(GPIO_TypeDef* gpio_type, uint16_t gpio_pin){
	HAL_GPIO_WritePin(gpio_type,gpio_pin,GPIO_PIN_RESET);
}

bool D_GPIO_Read(GPIO_TypeDef* gpio_type, uint16_t gpio_pin){
	return (bool)HAL_GPIO_ReadPin(gpio_type,gpio_pin);
}
