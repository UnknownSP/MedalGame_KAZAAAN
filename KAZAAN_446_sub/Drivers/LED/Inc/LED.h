/*
 * LED.h
 *
 *  Created on: Aug 24, 2023
 *      Author: UnknownSP
 */

#ifndef LED_INC_LED_H_
#define LED_INC_LED_H_

#include "main.h"
#include "app.h"

int D_LED_Handler(uint32_t ballState, int brightness);
void D_LED_SetBrightness(int Led_num, int brightness);
int D_LED_1stBlink(uint32_t *processTime, int setState, int brightness, bool _init);
int D_LED_123Blink(uint32_t *processTime, int setState, int brightness, bool _init);
int D_LED_2ndBlink(uint32_t *processTime, int setState, int brightness, bool _init);
int D_LED_3rdBlink(uint32_t *processTime, int setState, int brightness, bool _init);

#endif /* LED_INC_LED_H_ */
