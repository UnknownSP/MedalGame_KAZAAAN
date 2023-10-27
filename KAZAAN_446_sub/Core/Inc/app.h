/*
 * app.h
 *
 *  Created on: Mar 11, 2023
 *      Author: UnknownSP
 */

#ifndef INC_APP_H_
#define INC_APP_H_

#include "main.h"

int appTask(void);
int appInit(void);

static uint8_t rcvData[8] = {0}; //CAN受信データ
static uint8_t sndData[8] = {0}; //CAN送信データ

static volatile uint8_t rcvData_UART[8] = {0}; //PCからの受信データ

#define STATION_NUMBER 1

#define CAN_SEND_INTERVAL 10
#define UART_RECEIVE_INTERVAL 10

#define CAN_ST1_ADDRESS 0x100
#define CAN_ST2_ADDRESS 0x200
#define CAN_ST3_ADDRESS 0x300
#define CAN_ST4_ADDRESS 0x400
#define CAN_MAIN_ADDRESS 0x500
#define CAN_ST1 0
#define CAN_ST2 1
#define CAN_ST3 2
#define CAN_ST4 3
#define CAN_MAIN 4

#define IO_SET_USERLED() 		(D_GPIO_Set(GPIOA,GPIO_PIN_5))
#define IO_RESET_USERLED() 		(D_GPIO_Reset(GPIOA,GPIO_PIN_5))
#define IO_READ_USERBUTTON() 	(!D_GPIO_Read(GPIOC,GPIO_PIN_13))

#define IO_SET_SHUTTER_SOL() 	(D_GPIO_Set(GPIOB,GPIO_PIN_6))
#define IO_RESET_SHUTTER_SOL() 	(D_GPIO_Reset(GPIOB,GPIO_PIN_6))

#define IO_READ_1ST_IN() 		(D_GPIO_Read(GPIOA,GPIO_PIN_15))
#define IO_READ_1ST_COLLECT() 	(D_GPIO_Read(GPIOB,GPIO_PIN_7))
#define IO_READ_1ST_SLOPE() 	(D_GPIO_Read(GPIOA,GPIO_PIN_0))
#define IO_READ_2ND_IN() 		(D_GPIO_Read(GPIOA,GPIO_PIN_1))
#define IO_READ_2ND_COLLECT() 	(D_GPIO_Read(GPIOA,GPIO_PIN_4))
#define IO_READ_2ND_SLOPE() 	(D_GPIO_Read(GPIOC,GPIO_PIN_2))
#define IO_READ_3RD_IN() 		(D_GPIO_Read(GPIOC,GPIO_PIN_1))
#define IO_READ_3RD_COLLECT() 	(D_GPIO_Read(GPIOC,GPIO_PIN_3))
#define IO_READ_3RD_SHUTTER() 	(D_GPIO_Read(GPIOC,GPIO_PIN_0))

#define IO_READ_L_MECHA() 		(D_GPIO_Read(GPIOD,GPIO_PIN_2))
#define IO_READ_L_BALLWAIT() 	(D_GPIO_Read(GPIOC,GPIO_PIN_11))
#define IO_READ_L_START() 		(D_GPIO_Read(GPIOC,GPIO_PIN_10))
#define IO_READ_L_BALLSUPPLY() 	(D_GPIO_Read(GPIOC,GPIO_PIN_12))

#define IO_READ_SIG_1() 		(!D_GPIO_Read(GPIOB,GPIO_PIN_14))

#define IO_1ST_IN		0
#define IO_1ST_COLLECT	1
#define IO_1ST_SLOPE	2
#define IO_2ND_IN		3
#define IO_2ND_COLLECT	4
#define IO_2ND_SLOPE 	5
#define IO_3RD_IN		6
#define IO_3RD_COLLECT 	7
#define IO_3RD_SHUTTER 	8
#define IO_L_MECHA		9
#define IO_L_BALLWAIT 	10
#define IO_L_START		11
#define IO_L_BALLSUPPLY 12
#define IO_SIG_1		13

#define TIM_MAX_COUNT 5000

#define ERRORTIME_L_ROTATE 2000
#define ERRORTIME_L_SUPPLY 8000
#define ERRORTIME_SHUTTER_OPEN 500
#define ERRORTIME_SHUTTER_CLOSE 500

#define PWM_LED_1ST 3
#define PWM_LED_2ND 1
#define PWM_LED_3RD 2
#define PWM_L_MOTOR 4

//抽選の状態管理
typedef enum{
	B_WAIT_LAUNCH	= 0,
	B_LAUNCHING_1ST	= 1,
	B_LAUNCHED_1ST	= 2,
	B_OUT_1ST		= 3,
	//B_UP_WAIT_1ST,
	B_LAUNCHING_2ND = 4,
	B_LAUNCHED_2ND	= 5,
	B_OUT_2ND		= 6,
	//B_UP_WAIT_2ND,
	B_LAUNCHING_3RD	= 7,
	B_LAUNCHED_3RD	= 8,
	B_OUT_3RD		= 9,
	B_UP_JPC		= 10,
	B_JPC_1ST		= 11,
	B_JPC_1ST_WAIT	= 12,
	B_JPC_2ND		= 13,
	B_JPC_2ND_WAIT	= 14,
	B_GET_JP		= 15,
}BallProcess_State;

#endif /* INC_APP_H_ */
