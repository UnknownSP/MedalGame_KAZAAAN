/*
 * CAN.c
 *
 *  Created on: Aug 23, 2023
 *      Author: UnknownSP
 */

#include "../Inc/CAN.h"
#include <stdlib.h>
#include "stm32f4xx_hal.h"

void D_CAN_Init(void){
	HAL_CAN_Start(&hcan1);
	HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
	for(int i=0; i<8; i++){
		TxData[i] = 0;
		RxData[i] = 0;
		for(int j=0; j<5; j++){
			AllData[j][i] = 0;
		}
	}
}

int D_CAN_Transmit(uint16_t ownAddress, const uint8_t *data, uint16_t size){
	if(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0){
		TxHeader.StdId = ownAddress;
		TxHeader.RTR = CAN_RTR_DATA;
		TxHeader.IDE = CAN_ID_STD;
		TxHeader.DLC = size;
		TxHeader.TransmitGlobalTime = DISABLE;
		for(int i=0; i<size; i++){
			TxData[i] = data[i];
		}
		if(HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK){
			return -2;
		}
	}else{
		return -1;
	}
	return 0;
}

int D_CAN_SetReceiveAddress(uint16_t Address1, uint16_t Address2, uint16_t Address3, uint16_t Address4){
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDLIST;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;
	sFilterConfig.FilterIdHigh = Address1<<5;
	sFilterConfig.FilterIdLow = Address2<<5;
	sFilterConfig.FilterMaskIdHigh = Address3<<5;
	sFilterConfig.FilterMaskIdLow = Address4<<5;
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.SlaveStartFilterBank = 14;

	if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK){
		return -1;
	}

	return 0;
}

int D_CAN_Receive(uint16_t SenderAddress, uint8_t *data, uint16_t size){
	int setIdx = 0;
	switch(SenderAddress){
	case CAN_ST1_ADDRESS:
		setIdx = CAN_ST1;
		break;
	case CAN_ST2_ADDRESS:
		setIdx = CAN_ST2;
		break;
	case CAN_ST3_ADDRESS:
		setIdx = CAN_ST3;
		break;
	case CAN_ST4_ADDRESS:
		setIdx = CAN_ST4;
		break;
	case CAN_MAIN_ADDRESS:
		setIdx = CAN_MAIN;
		break;
	default:
		setIdx = CAN_MAIN;
		break;
	}
	for(int i=0; i<size; i++){
		data[i] = AllData[setIdx][i];
	}


	return 0;
}

void D_CAN_ReceiveCallBack(CAN_HandleTypeDef *hcan){
	CAN_RxHeaderTypeDef RxHeader;
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK){
		uint32_t id = (RxHeader.IDE == CAN_ID_STD)? RxHeader.StdId : RxHeader.ExtId;
		uint32_t size = RxHeader.DLC;
		int setIdx = 0;
		switch(id){
		case CAN_ST1_ADDRESS:
			setIdx = CAN_ST1;
			break;
		case CAN_ST2_ADDRESS:
			setIdx = CAN_ST2;
			break;
		case CAN_ST3_ADDRESS:
			setIdx = CAN_ST3;
			break;
		case CAN_ST4_ADDRESS:
			setIdx = CAN_ST4;
			break;
		case CAN_MAIN_ADDRESS:
			setIdx = CAN_MAIN;
			break;
		default:
			setIdx = CAN_MAIN;
			break;
		}
		for(int i=0; i<size; i++){
			AllData[setIdx][i] = RxData[i];
		}
	}
}
