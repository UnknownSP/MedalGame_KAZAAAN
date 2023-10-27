/*
 * GPIO.h
 *
 *  Created on: Mar 18, 2023
 *      Author: UnknownSP
 */

#ifndef GPIO_INC_GPIO_H_
#define GPIO_INC_GPIO_H_

#include "main.h"

void D_GPIO_Set(GPIO_TypeDef* gpio_type, uint16_t gpio_pin);
void D_GPIO_Reset(GPIO_TypeDef* gpio_type, uint16_t gpio_pin);
bool D_GPIO_Read(GPIO_TypeDef* gpio_type, uint16_t gpio_pin);

#endif /* GPIO_INC_GPIO_H_ */
