/*
 * I2C.h
 *
 *  Created on: Aug 19, 2023
 *      Author: UnknownSP
 */

#ifndef I2C_INC_I2C_H_
#define I2C_INC_I2C_H_

#include "main.h"

int D_I2C1_Transmit(uint8_t address, const uint8_t *data, uint16_t size);
int D_I2C1_Receive(uint8_t address, uint8_t *data, uint16_t size);
void D_I2C1_TransitionCompletedCallBack(void);
void D_I2C1_ReceptionCompletedCallBack(void);

static volatile bool I2C1_Completed_tx = true;
static volatile bool I2C1_Completed_rx = true;

#endif /* I2C_INC_I2C_H_ */
