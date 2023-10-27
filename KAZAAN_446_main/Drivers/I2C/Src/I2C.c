/*
 * I2C.c
 *
 *  Created on: Aug 19, 2023
 *      Author: UnknownSP
 */


#include "../Inc/I2C.h"
#include <stdlib.h>
#include "stm32f4xx_hal.h"

int D_I2C1_Transmit(uint8_t address, const uint8_t *data, uint16_t size){

#if USE_I2C
	if(I2C1_Completed_tx){ //送信完了フラグが立っていれば送信を行う
		if( HAL_I2C_Master_Transmit_DMA(&hi2c1, address << 1, (uint8_t*)data, size) != HAL_OK ){
	      return -1;
	    }
	    I2C1_Completed_tx = false; //送信完了フラグを下げる
	}else{
		return -2;
	}
#endif
	return 0;
}

int D_I2C1_Receive(uint8_t address, uint8_t *data, uint16_t size){

#if USE_I2C
	if(I2C1_Completed_rx){ //送信完了フラグが立っていれば送信を行う
	    if( HAL_I2C_Master_Receive_DMA(&hi2c1, address << 1, (uint8_t*)data, size) != HAL_OK ){
	      return -1;
	    }
	    I2C1_Completed_rx = false; //送信完了フラグを下げる
	}else{
		return -2;
	}
#endif
	return 0;
}

void D_I2C1_TransitionCompletedCallBack(void){
	I2C1_Completed_tx = true;
}

void D_I2C1_ReceptionCompletedCallBack(void){
	I2C1_Completed_rx = true;
}

