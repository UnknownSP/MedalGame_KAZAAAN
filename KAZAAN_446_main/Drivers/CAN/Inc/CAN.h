/*
 * CAN.h
 *
 *  Created on: Aug 23, 2023
 *      Author: UnknownSP
 */

#ifndef CAN_INC_CAN_H_
#define CAN_INC_CAN_H_

#include "main.h"

void D_CAN_Init(void);
int D_CAN_Transmit(uint16_t ownAddress, const uint8_t *data, uint16_t size);
int D_CAN_SetReceiveAddress(uint16_t Address1, uint16_t Address2, uint16_t Address3, uint16_t Address4);
int D_CAN_Receive(uint16_t SenderAddress, uint8_t *data, uint16_t size);
void D_CAN_ReceiveCallBack(CAN_HandleTypeDef *hcan);

CAN_TxHeaderTypeDef   TxHeader;
CAN_RxHeaderTypeDef   RxHeader;
CAN_FilterTypeDef  sFilterConfig;

uint8_t               TxData[8];
uint8_t               RxData[8];
uint8_t               AllData[5][8];
uint32_t              TxMailbox;

#endif /* CAN_INC_CAN_H_ */
