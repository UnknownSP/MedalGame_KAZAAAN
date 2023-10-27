/*
 * LED.c
 *
 *  Created on: Aug 24, 2023
 *      Author: UnknownSP
 */


#include "../Inc/LED.h"
#include <stdlib.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

int D_LED_Handler(uint32_t ballState, int brightness){
	static uint32_t recent_System_counter = 0;
	static uint32_t processTime = 0;
	static uint32_t processTimeOut = 0;
	static uint32_t processCaseTime = 0;
	static uint32_t deltaTime = 0;
	static uint32_t recent_ballState = B_WAIT_LAUNCH;

	static bool _out1st_process = false;
	static bool _out2nd_process = false;
	static bool _out3rd_process = false;
	static bool _through_launching2nd = false;
	static bool _through_launching3rd = false;

	static bool _firstTime = true;

	deltaTime = G_System_counter - recent_System_counter;
	processTime += deltaTime;
	recent_System_counter = G_System_counter;

	if(brightness > 1000) brightness = 1000;
	if(brightness < 0) brightness = 0;

	switch(ballState){
	case B_WAIT_LAUNCH:
		_firstTime = true;
		if(!_out1st_process){
			D_LED_SetBrightness(PWM_LED_1ST, brightness);
			D_LED_SetBrightness(PWM_LED_2ND, 0);
			D_LED_SetBrightness(PWM_LED_3RD, 0);
		}
		break;
	case B_LAUNCHING_1ST:
	case B_LAUNCHED_1ST:
		if(ballState != recent_ballState && ballState == B_LAUNCHING_1ST){
			_out1st_process = false;
			processTime = 0;
			D_LED_1stBlink(0, 0, 0, true);
		}
		D_LED_1stBlink(&processTime, 0, brightness, false);
		break;
	case B_OUT_1ST:
		_out1st_process = true;
		if(ballState != recent_ballState){
			processTime = 0;
			processTimeOut = 0;
			D_LED_1stBlink(0, 0, 0, true);
			D_LED_1stBlink(0, 10, 0, false);
		}
		break;
	case B_LAUNCHING_2ND:
		if(ballState != recent_ballState){
			processTime = 0;
			_through_launching2nd = true;
			D_LED_123Blink(0, 0, 0, true);
		}
		D_LED_123Blink(&processTime, 0, brightness, false);
		break;
	case B_LAUNCHED_2ND:
		if(ballState != recent_ballState){
			D_LED_2ndBlink(0, 0, 0, true);
			processCaseTime = 0;
			_firstTime = true;
		}
		processCaseTime += deltaTime;
		if(processCaseTime >= 1500){
			if(_firstTime){
				_firstTime = false;
				processTime = 0;
			}
			D_LED_2ndBlink(&processTime, 0, brightness, false);
		}else if(_through_launching2nd){
			D_LED_123Blink(&processTime, 0, brightness, false);
		}else{
			D_LED_1stBlink(&processTime, 0, brightness, false);
		}
		break;
	case B_OUT_2ND:
		_out2nd_process = true;
		if(ballState != recent_ballState){
			processTime = 0;
			processTimeOut = 0;
			D_LED_2ndBlink(0, 0, 0, true);
			D_LED_2ndBlink(0, 10, 0, false);
		}
		break;
	case B_LAUNCHING_3RD:
		if(ballState != recent_ballState){
			processTime = 0;
			_through_launching3rd = true;
			D_LED_123Blink(0, 0, 0, true);
			D_LED_123Blink(0, 10, 0, false);
		}
		D_LED_123Blink(&processTime, 0, brightness, false);
		break;
	case B_LAUNCHED_3RD:
		if(ballState != recent_ballState){
			D_LED_3rdBlink(0, 0, 0, true);
			processCaseTime = 0;
			_firstTime = true;
		}
		processCaseTime += deltaTime;
		if(processCaseTime >= 2000){
			if(_firstTime){
				_firstTime = false;
				processTime = 0;
			}
			D_LED_3rdBlink(&processTime, 0, brightness, false);
		}else if(_through_launching3rd){
			D_LED_123Blink(&processTime, 0, brightness, false);
		}else{
			D_LED_2ndBlink(&processTime, 0, brightness, false);
		}
		break;
	case B_OUT_3RD:
		_out3rd_process = true;
		if(ballState != recent_ballState){
			processTime = 0;
			processTimeOut = 0;
			D_LED_3rdBlink(0, 0, 0, true);
			D_LED_3rdBlink(0, 10, 0, false);
			D_LED_SetBrightness(PWM_LED_2ND, 0);
		}
		break;
	case B_UP_JPC:
		if(ballState != recent_ballState){
			processTime = 0;
			D_LED_SetBrightness(PWM_LED_1ST, 0);
			D_LED_SetBrightness(PWM_LED_2ND, 0);
			D_LED_SetBrightness(PWM_LED_3RD, 0);
		}
		break;
	}
	if(_out1st_process){
		processTimeOut += deltaTime;
		if(D_LED_1stBlink(&processTimeOut, 0, brightness, false)){
			_out1st_process = false;
			processTimeOut = 0;
		}
	}
	if(_out2nd_process){
		processTimeOut += deltaTime;
		if(D_LED_2ndBlink(&processTimeOut, 0, brightness, false)){
			_out2nd_process = false;
			processTimeOut = 0;
		}
	}
	if(_out3rd_process){
		processTimeOut += deltaTime;
		if(D_LED_3rdBlink(&processTimeOut, 0, brightness, false)){
			_out3rd_process = false;
			processTimeOut = 0;
		}
	}

	recent_ballState = ballState;
	return 0;
}

int D_LED_1stBlink(uint32_t *processTime, int setState, int brightness, bool _init){
	static int state = 0;
	static int count = 0;
	if(_init){
		state = 0;
		count = 0;
		return;
	}
	if(setState != 0){
		state = setState;
		return;
	}
	switch(state){
	case 0:
		if(*processTime <= 120){
			D_LED_SetBrightness(PWM_LED_1ST, 0);
		}else if(*processTime <= 240){
			D_LED_SetBrightness(PWM_LED_1ST, brightness);
		}else{
			count += 1;
			*processTime = 0;
		}
		if(count >= 4){
			count = 0;
			state = 1;
			*processTime = 0;
		}
		break;
	case 1:
		if(*processTime <= 200){
			D_LED_SetBrightness(PWM_LED_1ST, 0);
		}else{
			D_LED_SetBrightness(PWM_LED_1ST, brightness);
			state = 2;
			*processTime = 0;
		}
		break;
	case 2:
		if(*processTime <= 1100){
			D_LED_SetBrightness(PWM_LED_1ST, brightness);
		}else if(*processTime <= 1200){
			D_LED_SetBrightness(PWM_LED_1ST, 0);
		}else{
			*processTime = 0;
		}
		break;
	case 10:
		if(*processTime <= 70){
			D_LED_SetBrightness(PWM_LED_1ST, 0);
		}else if(*processTime <= 140){
			D_LED_SetBrightness(PWM_LED_1ST, brightness);
		}else{
			count += 1;
			*processTime = 0;
		}
		if(count >= 5){
			count = 0;
			state = 11;
			*processTime = 0;
		}
		break;
	case 11:
		D_LED_SetBrightness(PWM_LED_1ST, brightness);
		*processTime = 0;
		return 1;
		break;
	}
	return 0;
}

int D_LED_123Blink(uint32_t *processTime, int setState, int brightness, bool _init){
	static int state = 0;
	static int count = 0;
	if(_init){
		state = 0;
		count = 0;
		return;
	}
	if(setState != 0){
		state = setState;
		return;
	}
	switch(state){
	case 0:
		if(*processTime <= 110){
			D_LED_SetBrightness(PWM_LED_1ST, brightness);
			D_LED_SetBrightness(PWM_LED_2ND, 0);
			D_LED_SetBrightness(PWM_LED_3RD, 0);
		}else if(*processTime <= 220){
			D_LED_SetBrightness(PWM_LED_1ST, 0);
			D_LED_SetBrightness(PWM_LED_2ND, brightness);
			D_LED_SetBrightness(PWM_LED_3RD, 0);
		}else if(*processTime <= 330){
			D_LED_SetBrightness(PWM_LED_1ST, 0);
			D_LED_SetBrightness(PWM_LED_2ND, 0);
			D_LED_SetBrightness(PWM_LED_3RD, brightness);
		}else{
			count += 1;
			*processTime = 0;
		}
		if(count >= 10){
			count = 0;
			state = 1;
			*processTime = 0;
			D_LED_SetBrightness(PWM_LED_1ST, brightness);
			D_LED_SetBrightness(PWM_LED_2ND, 0);
			D_LED_SetBrightness(PWM_LED_3RD, 0);
		}
		break;
	case 1:
		if(*processTime <= 1100){
			D_LED_SetBrightness(PWM_LED_1ST, brightness);
		}else if(*processTime <= 1200){
			D_LED_SetBrightness(PWM_LED_1ST, 0);
		}else{
			*processTime = 0;
		}
		D_LED_SetBrightness(PWM_LED_2ND, 0);
		D_LED_SetBrightness(PWM_LED_3RD, 0);
		break;
	case 10:
		if(*processTime <= 110){
			D_LED_SetBrightness(PWM_LED_1ST, brightness);
			D_LED_SetBrightness(PWM_LED_2ND, 0);
			D_LED_SetBrightness(PWM_LED_3RD, 0);
		}else if(*processTime <= 220){
			D_LED_SetBrightness(PWM_LED_1ST, 0);
			D_LED_SetBrightness(PWM_LED_2ND, brightness);
			D_LED_SetBrightness(PWM_LED_3RD, 0);
		}else if(*processTime <= 330){
			D_LED_SetBrightness(PWM_LED_1ST, 0);
			D_LED_SetBrightness(PWM_LED_2ND, 0);
			D_LED_SetBrightness(PWM_LED_3RD, brightness);
		}else{
			count += 1;
			*processTime = 0;
		}
		if(count >= 10){
			count = 0;
			state = 11;
			*processTime = 0;
			D_LED_SetBrightness(PWM_LED_1ST, brightness);
			D_LED_SetBrightness(PWM_LED_2ND, brightness);
			D_LED_SetBrightness(PWM_LED_3RD, 0);
		}
		break;
	case 11:
		if(*processTime <= 1100){
			D_LED_SetBrightness(PWM_LED_2ND, brightness);
		}else if(*processTime <= 1200){
			D_LED_SetBrightness(PWM_LED_2ND, 0);
		}else{
			*processTime = 0;
		}
		D_LED_SetBrightness(PWM_LED_3RD, 0);
		break;
	}
	return 0;
}

int D_LED_2ndBlink(uint32_t *processTime, int setState, int brightness, bool _init){
	static int state = 0;
	static int count = 0;
	if(_init){
		state = 0;
		count = 0;
		return;
	}
	if(setState != 0){
		state = setState;
		return;
	}
	switch(state){
	case 0:
		D_LED_SetBrightness(PWM_LED_1ST, brightness);
		D_LED_SetBrightness(PWM_LED_3RD, 0);
		if(*processTime <= 80){
			D_LED_SetBrightness(PWM_LED_2ND, 0);
		}else if(*processTime <= 160){
			D_LED_SetBrightness(PWM_LED_2ND, brightness);
		}else{
			count += 1;
			*processTime = 0;
		}
		if(count >= 3){
			count = 0;
			state = 1;
			*processTime = 0;
		}
		break;
	case 1:
		if(*processTime <= 1100){
			D_LED_SetBrightness(PWM_LED_2ND, brightness);
		}else if(*processTime <= 1200){
			D_LED_SetBrightness(PWM_LED_2ND, 0);
		}else{
			*processTime = 0;
		}
		break;
	case 10:
		if(*processTime <= 120){
			D_LED_SetBrightness(PWM_LED_2ND, 0);
		}else if(*processTime <= 240){
			D_LED_SetBrightness(PWM_LED_2ND, brightness);
		}else{
			count += 1;
			*processTime = 0;
		}
		if(count >= 4){
			count = 0;
			state = 11;
			*processTime = 0;
		}
		break;
	case 11:
		D_LED_SetBrightness(PWM_LED_2ND, 0);
		*processTime = 0;
		return 1;
		break;
	}
	return 0;
}

int D_LED_3rdBlink(uint32_t *processTime, int setState, int brightness, bool _init){
	static int state = 0;
	static int count = 0;
	if(_init){
		state = 0;
		count = 0;
		return;
	}
	if(setState != 0){
		state = setState;
		return;
	}
	switch(state){
	case 0:
		D_LED_SetBrightness(PWM_LED_1ST, brightness);
		D_LED_SetBrightness(PWM_LED_2ND, brightness);
		if(*processTime <= 130){
			D_LED_SetBrightness(PWM_LED_3RD, 0);
		}else if(*processTime <= 260){
			D_LED_SetBrightness(PWM_LED_3RD, brightness);
		}else{
			count += 1;
			*processTime = 0;
		}
		if(count >= 5){
			count = 0;
			state = 1;
			*processTime = 0;
		}
		break;
	case 1:
		if(*processTime <= 1100){
			D_LED_SetBrightness(PWM_LED_3RD, brightness);
		}else if(*processTime <= 1200){
			D_LED_SetBrightness(PWM_LED_3RD, 0);
		}else{
			*processTime = 0;
		}
		break;
	case 10:
		if(*processTime <= 120){
			D_LED_SetBrightness(PWM_LED_3RD, 0);
		}else if(*processTime <= 240){
			D_LED_SetBrightness(PWM_LED_3RD, brightness);
		}else{
			count += 1;
			*processTime = 0;
		}
		if(count >= 5){
			count = 0;
			state = 11;
			*processTime = 0;
		}
		break;
	case 11:
		D_LED_SetBrightness(PWM_LED_3RD, 0);
		*processTime = 0;
		return 1;
		break;
	}
	return 0;
}

void D_LED_SetBrightness(int Led_num, int brightness){
	if(brightness >= 1000) brightness = 1000;
	if(brightness <= 0) brightness = 0;
	int setValue = (TIM_MAX_COUNT / 1000.0) * brightness;
	if(!(Led_num == PWM_LED_1ST || Led_num == PWM_LED_2ND || Led_num == PWM_LED_3RD)){
		return;
	}
	D_PWM_Set(Led_num,setValue);
}

