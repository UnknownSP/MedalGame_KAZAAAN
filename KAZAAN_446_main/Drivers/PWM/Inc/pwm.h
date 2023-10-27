/*
 * pwm.h
 *
 *  Created on: Mar 11, 2023
 *      Author: UnknownSP
 */

#ifndef PWM_INC_PWM_H_
#define PWM_INC_PWM_H_

#include "main.h"

void D_PWM_Init(void);
void D_PWM_Set(int channel, int value);

#endif /* PWM_INC_PWM_H_ */
