/*
 * I2C.c
 *
 *  Created on: Aug 19, 2023
 *      Author: UnknownSP
 */


#include "../Inc/I2C.h"
#include <stdlib.h>
#include "stm32f4xx_hal.h"

int D_I2C3_Transmit(const uint8_t *data, uint16_t size){
	if(I2C3_Completed_tx){ //送信完了フラグが立っていれば送信を行う
	    if( HAL_I2C_Slave_Transmit_DMA(&hi2c3, (uint8_t*)data, size) != HAL_OK ){
	      return -1;
	    }
	    I2C3_Completed_tx = false; //送信完了フラグを下げる
	}else{
		return -2;
	}
	return 0;
}

int D_I2C3_Receive(uint8_t *data, uint16_t size){
	if(I2C3_Completed_rx){ //送信完了フラグが立っていれば送信を行う
		if( HAL_I2C_Slave_Receive_DMA(&hi2c3, (uint8_t*)data, size) != HAL_OK ){
	      return -1;
	    }
	    I2C3_Completed_rx = false; //送信完了フラグを下げる
	}else{
		return -2;
	}
	return 0;
}

void D_I2C3_TransitionCompletedCallBack(void){
	I2C3_Completed_tx = true;
}

void D_I2C3_ReceptionCompletedCallBack(void){
	I2C3_Completed_rx = true;
}

