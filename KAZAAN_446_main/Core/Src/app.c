/*
 * app.c
 *
 *  Created on: Mar 11, 2023
 *      Author: UnknownSP
 */

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "app.h"

static bool JP_Lift_Up(void);
static bool JP_Lift_Down(void);
static void Lottery_1st2nd_SetSpeed(int speed);
static void Lottery_3rd_SetSpeed(int speed);
static void Lottery_JP_SetSpeed(int speed, int direction);
static void Encoder_1st(void);
static void Encoder_2nd(void);
static void Encoder_3rd(void);
static void Encoder_JP(void);
static int JPC_Processing(int station, uint8_t* pocket, int speed,int direction, bool skipPerformance, bool skipButton);
static int JPC_DecelerationSpeed_cal(int time);
static int JPC_Pocket_cal(int front, int rear, int direction);
static int JPC_Pocket(int num);

static int Encoder_Num_1st = 1;
static int Encoder_Num_2nd = 1;
static int Encoder_Num_3rd = 1;
static int Encoder_Num_JP = 1;
static int JPC_stations[4] = {0};

const char* JP_PocketName[6] = {
    "POCKET_JP",
    "POCKET_6_R",
    "POCKET_12_R",
    "POCKET_Q",
    "POCKET_12_L",
    "POCKET_6_L"
};

const int FrontDetect_Pockets[2][6] = {
    {7,5,3,1,11,9}, //右方向回転(ホリアテールと同じ)
    {6,8,10,12,2,4},  //左方向回転(ホリアテールと同じ)
};

const int RearDetect_Pockets[2][6] = {
    {1,11,9,7,5,3}, //右方向回転(ホリアテールと同じ)
    {12,2,4,6,8,10},  //左方向回転(ホリアテールと同じ)
};

//初期化
int appInit(void){
	D_CAN_SetReceiveAddress(CAN_ST1_ADDRESS,CAN_ST2_ADDRESS,CAN_ST3_ADDRESS,CAN_ST4_ADDRESS);
	Lottery_1st2nd_SetSpeed(0);
	Lottery_3rd_SetSpeed(0);
	Lottery_JP_SetSpeed(0,0);
	for(int i=0; i<8; i++){
		for(int j=0; j<4; j++){
			rcvData[j][i] = 0;
			sndData[j][i] = 0;
		}
	}
	IO_SET_ROOMLIGHT();
	return 0;
}

int appTask(void){
	static int errorHandle = 0;
	static uint32_t recent_System_counter = 0;
	static uint32_t sndTime = 0;
	static uint8_t stationHoldPocket[4] = {0};
	static uint8_t allHoldPocket = 0;
	static uint8_t JPC_IN_Pocket = 0;
	static uint8_t JPC_ProcessState = 0;

	//エンコーダーの値を更新
	Encoder_1st();
	Encoder_2nd();
	Encoder_3rd();
	Encoder_JP();

	//CANでエンコーダの値を送信
	for(int i=0; i<4; i++){
		sndData[i][0] = Encoder_Num_1st;
		sndData[i][1] = Encoder_Num_2nd;
		sndData[i][2] = Encoder_Num_3rd;
	}

	sndTime += G_System_counter - recent_System_counter;
	recent_System_counter = G_System_counter;
	//CAN送信タイミングの場合送信
	if(sndTime >= CAN_SEND_INTERVAL){
		D_CAN_Transmit(CAN_MAIN_ADDRESS,sndData[0],8);
		sndTime = 0;
	}
	//CAN受信処理
	D_CAN_Receive(CAN_ST1_ADDRESS,rcvData[0],8);
	//各ステーションの3段目ホールドポケットを受信
	for(int i=0; i<4; i++){
		stationHoldPocket[i] = rcvData[i][0];
		allHoldPocket |= stationHoldPocket[i];
	}
	//全体のホールドポケット情報を送信
	for(int i=0; i<4; i++){
		sndData[i][3] = allHoldPocket;
	}
	//JPCの待機をスタック処理
	for(int i=0; i<4; i++){
		if(rcvData[i][1] != 0){
			bool _stacked = false;
			for(int j=0; j<4; j++){
				if(JPC_stations[j] == i+1) _stacked = true;
			}
			if(_stacked) continue;
			for(int j=0; j<4; j++){
				if(JPC_stations[j] == 0){
					JPC_stations[j] = i+1;
					break;
				}
			}
		}
	}
	//D_Mess_printf("%d\n", G_System_counter);
	//D_PWM_Set(1,100);
	//D_PWM_Set(2,100);
	//IO_SET_BLDC2_DIR();
	if(IO_READ_USERBUTTON()){
		IO_SET_USERLED();
		//IO_SET_BLDC1_ENA();
		//IO_SET_BLDC2_ENA();
		//D_PWM_Set(BLDC1,50);
		//D_PWM_Set(BLDC2,50);
		//D_PWM_Set(BLDC3,300);
		//IO_SET_JP_LED();
		//Lottery_1st2nd_SetSpeed(500);
		//Lottery_3rd_SetSpeed(800);
		//Lottery_JP_SetSpeed(300,0);
		//Lottery_JP_SetSpeed(JPC_MAX_SPEED,0);

		JPC_stations[0] = 1;
		//IO_SET_ROOMLIGHT();
		//JP_Lift_Down();
	}else{
		IO_RESET_USERLED();
		//IO_RESET_BLDC1_ENA();
		//IO_RESET_BLDC2_ENA();
		//IO_RESET_BLDC3_ENA();
		//D_PWM_Set(BLDC1,300);
		//D_PWM_Set(BLDC2,3000);
		//D_PWM_Set(BLDC3,3500);
		//IO_RESET_JP_LED();
		
		//Lottery_1st2nd_SetSpeed(30);
		//IO_RESET_ROOMLIGHT();
		//Lottery_3rd_SetSpeed(0);
		//Lottery_JP_SetSpeed(10,0);

		//JP_Lift_Up();
	}

	//1段目UPポケットを回避するような動き
	/*
	if(Encoder_Num_1st % 6 == 3 || Encoder_Num_1st % 6 == 4){
		Lottery_1st2nd_SetSpeed(1000);
	}else{
		Lottery_1st2nd_SetSpeed(20);
	}
	*/

	//回転スピードは固定
	//後々対策バージョンのような動作を実装
	Lottery_1st2nd_SetSpeed(30);
	Lottery_3rd_SetSpeed(300);

	//JPCプロセスの実行
	JPC_ProcessState = JPC_Processing(JPC_stations[0], &JPC_IN_Pocket, JPC_SLOW_SPEED, 1, true, true);
	//一回目二回目で入ったポケットを送信
	if(JPC_ProcessState == JPC_WAIT_2 || JPC_ProcessState == JPC_WAIT_4){
		if(JPC_stations[0] != 0) sndData[JPC_stations[0]-1][5] = JPC_IN_Pocket;
	}
	//JPC中ならJPC中のステーションにJPCの状況を送信
	if(JPC_stations[0] != 0) sndData[JPC_stations[0]-1][4] = JPC_ProcessState;
	//JPCのスタックの処理
	if(JPC_ProcessState == JPC_STACK_PROCESS && rcvData[JPC_stations[0]-1][1] == 0){
		sndData[JPC_stations[0]-1][4] = 0;
		sndData[JPC_stations[0]-1][5] = 0;
		JPC_Processing(0, &JPC_IN_Pocket, 0, 1, true, true);
		for(int i=0; i<3; i++){
			JPC_stations[i] = JPC_stations[i+1];
		}
		JPC_stations[3] = 0;
	}

	//JPC時の部屋の電気の同期処理
	if(JPC_ProcessState >= 0 && JPC_ProcessState < 20){
		IO_RESET_ROOMLIGHT();
	}else{
		IO_SET_ROOMLIGHT();
	}

	//デバッグ用
	int16_t debug_bits = 0;
	debug_bits &= 0;
	debug_bits |= ((int)IO_READ_1ST_HOME() << 5);
	debug_bits |= ((int)IO_READ_1ST_ENC() << 4);
	debug_bits |= ((int)IO_READ_2ND_HOME() << 3);
	debug_bits |= ((int)IO_READ_2ND_ENC() << 2);
	debug_bits |= ((int)IO_READ_3RD_HOME() << 1);
	debug_bits |= ((int)IO_READ_3RD_ENC() << 0);
	debug_bits |= ((int)IO_READ_JP_HOME() << 13);
	debug_bits |= ((int)IO_READ_JP_ENC() << 12);
	debug_bits |= ((int)IO_READ_JP_FRONT() << 11);
	debug_bits |= ((int)IO_READ_JP_REAR() << 10);
	debug_bits |= ((int)IO_READ_JP_UPPER() << 9);
	debug_bits |= ((int)IO_READ_JP_LOWER() << 8);
	D_Mess_printf("\033[1;1H");
	D_Mess_printf("%016b\n", debug_bits);
	D_Mess_printf("%02d\n",Encoder_Num_1st);
	D_Mess_printf("%02d\n",Encoder_Num_2nd);
	D_Mess_printf("%02d\n",Encoder_Num_3rd);
	D_Mess_printf("%02d\n",Encoder_Num_JP);
	D_Mess_printf("%02d\n", errorHandle);
	D_Mess_printf("%3d,%3d,%3d,%3d,%3d,%3d\n", sndData[0][0],sndData[0][1],sndData[0][2],sndData[0][3],sndData[0][4],sndData[0][5]);
	D_Mess_printf("%3d,%3d,%3d,%3d\n", rcvData[0][0],rcvData[0][1],rcvData[0][2],rcvData[0][3]);

	return 0;
}

//JPCでどこのポケットに入ったか
static int JPC_Pocket_cal(int front, int rear, int direction){
	int return_num = 0;
	for(int i=0; i<6; i++){
		if(front == FrontDetect_Pockets[direction][i] && rear == RearDetect_Pockets[direction][i]){
			return_num = i+1;
			return return_num;
		}
	}
	return 0;
}

static int JPC_Pocket(int num){
	int return_num = num;
	if(return_num < 1){
		return_num += 12;
	}
	if(return_num > 12){
		return_num -= 12;
	}
	return return_num;
}

//JPCのプロセス
static int JPC_Processing(int station, uint8_t* pocket, int speed, int direction, bool skipPerformance, bool skipButton){
	static JPC_Process_State JPC_state = JPC_ROT_START_SLOW_1;
	static int RecentTime = 0;
	static int DeltaTime = 0;
	static int caseDeltaTime = 0;
	static int Rear_detect_num = 0;
	static int Front_detect_num = 0;
	static int IN_Pocket = 0;
	static int Slowing_EncNum = 0;
	static int Stop_EncNum = 0;
	static bool _slowing = false;
	if(station == 0){
		RecentTime = G_System_counter;
		Front_detect_num = 0;
		Rear_detect_num = 0;
		IN_Pocket = 0;
		Slowing_EncNum = 0;
		Stop_EncNum = 0;
		_slowing = false;
		JPC_state = JPC_ROT_START_SLOW_1;
		Lottery_JP_SetSpeed(0,0);
		return -1;
	}
	DeltaTime = G_System_counter - RecentTime;
	caseDeltaTime += DeltaTime;
	switch (JPC_state)
	{
	//JPCユニットの回転
	case JPC_ROT_START_SLOW_1:
		Lottery_JP_SetSpeed(speed,direction);
		caseDeltaTime = 0;
		JPC_state += 1;
		break;

	//メインユニットのLED点灯
	case JPC_LED_ON:
		IO_SET_JP_LED();
		if(caseDeltaTime >= 2000){
			caseDeltaTime = 0;
			JPC_state += 1;
		}
		break;

	//JPCリフト上昇
	case JPC_LIFT_UP:
		if(skipPerformance){
			JPC_state += 1;
			caseDeltaTime = 0;
			break;
		}
		if(JP_Lift_Up()){
			caseDeltaTime = 0;
			JPC_state += 1;
		}
		break;

	//待機
	case JPC_WAIT_1:
		if(skipPerformance){
			caseDeltaTime = 0;
			JPC_state += 1;
			break;
		}
		if(caseDeltaTime >= 2000){
			caseDeltaTime = 0;
			JPC_state += 1;
		}
		break;

	//ステーションからのJPCボタン入力待機(未実装)
	case JPC_WAIT_INPUT_1:
		if(skipButton){
			caseDeltaTime = 0;
			JPC_state += 1;
			break;
		}
		//Wait station JPC Button
		break;

	//抽選開始
	case JPC_ROT_START_1:
		Lottery_JP_SetSpeed(JPC_MAX_SPEED,direction);
		if(caseDeltaTime >= 10000){
			caseDeltaTime = 0;
			JPC_state += 1;
		}
		break;

	//抽選機の減速
	case JPC_DECELERATION_1:
		if(caseDeltaTime >= JPC_DECELERATOIN_TIME){
			Lottery_JP_SetSpeed(JPC_MIN_SPEED,direction);
		}else{
			int calSpeed = JPC_DecelerationSpeed_cal(caseDeltaTime);
			Lottery_JP_SetSpeed(calSpeed,direction);
		}
		if(IO_READ_JP_FRONT() || IO_READ_JP_REAR()){
			caseDeltaTime = 0;
			JPC_state += 1;
			Lottery_JP_SetSpeed(JPC_MIN_SPEED,direction);
			Front_detect_num = 0;
			Rear_detect_num = 0;
			IN_Pocket = 0;
			Slowing_EncNum = 0;
			Stop_EncNum = 0;
			_slowing = false;
		}
		break;

	//入賞ポケットの判定
	case JPC_JUDGE_1:
		if(Front_detect_num != 0 && Rear_detect_num != 0){
			IN_Pocket = JPC_Pocket_cal(Front_detect_num, Rear_detect_num, direction);
			if(caseDeltaTime >= 1500){
				caseDeltaTime = 0;
				JPC_state += 1;
				if(direction == 0){
					Stop_EncNum = JPC_Pocket(Front_detect_num + (station-1)*3);
					Slowing_EncNum = JPC_Pocket(Stop_EncNum + 1);
				}else if(direction == 1){
					Stop_EncNum = JPC_Pocket(Front_detect_num - (station-1)*3);
					Slowing_EncNum = JPC_Pocket(Stop_EncNum + 1);
				}
			}
			D_Mess_printf("JPC IN [%02d][%s]\n",IN_Pocket,JP_PocketName[IN_Pocket-1]);
		}else{
			if(IO_READ_JP_FRONT()){
				Front_detect_num = Encoder_Num_JP;
			}
			if(IO_READ_JP_REAR()){
				Rear_detect_num = Encoder_Num_JP;
			}
			caseDeltaTime = 0;
		}
		//D_Mess_printf("Front [%02d]\nRear  [%02d]\n",Front_detect_num, Rear_detect_num);
		//caseDeltaTime = 0;
		//JPC_state += 1;
		break;

	//抽選機停止
	case JPC_STOP_1:
		if(!_slowing){
			if(Encoder_Num_JP == Slowing_EncNum){
				_slowing = true;
				caseDeltaTime = 0;
			}
		}else{
			if(caseDeltaTime <= 1500){
				Lottery_JP_SetSpeed(JPC_MIN_SPEED - (int)(((double)(JPC_MIN_SPEED - JPC_SLOW_SPEED)/1500.0)*(double)caseDeltaTime),direction);
			}
			if(Encoder_Num_JP == Stop_EncNum){
				Lottery_JP_SetSpeed(0,direction);
				_slowing = false;
				caseDeltaTime = 0;
				//JPC_state += 1;
				JPC_state = JPC_WAIT_2;
				*pocket = IN_Pocket;
			}
		}
		break;
	case JPC_WAIT_2:
		if(IN_Pocket == 1){ //JP Pocket
			if(caseDeltaTime >= 5000){
				JPC_state = JPC_ROT_START_SLOW_2;
				caseDeltaTime = 0;
				Front_detect_num = 0;
				Rear_detect_num = 0;
				IN_Pocket = 0;
				Slowing_EncNum = 0;
				Stop_EncNum = 0;
				_slowing = false;
			}
		}else{
			if(caseDeltaTime >= 5000 || skipPerformance){
				JPC_state = JPC_LED_OFF;
				caseDeltaTime = 0;
			}
		}
		break;

	//二回目の抽選
	case JPC_ROT_START_SLOW_2:
			Lottery_JP_SetSpeed(speed,direction);
			caseDeltaTime = 0;
			JPC_state += 1;
			break;

	case JPC_WAIT_3:
		if(skipPerformance){
			caseDeltaTime = 0;
			JPC_state += 1;
			break;
		}
		if(caseDeltaTime >= 3000){
			caseDeltaTime = 0;
			JPC_state += 1;
		}
		break;
	case JPC_WAIT_INPUT_2:
		if(skipButton){
			caseDeltaTime = 0;
			JPC_state += 1;
			break;
		}
		//Wait station JPC Button
		break;
	case JPC_ROT_START_2:
		Lottery_JP_SetSpeed(JPC_MAX_SPEED,direction);
		if(caseDeltaTime >= 12500){
			caseDeltaTime = 0;
			JPC_state += 1;
		}
		break;
	case JPC_DECELERATION_2:
		if(caseDeltaTime >= JPC_DECELERATOIN_TIME){
			Lottery_JP_SetSpeed(JPC_MIN_SPEED,direction);
		}else{
			int calSpeed = JPC_DecelerationSpeed_cal(caseDeltaTime);
			Lottery_JP_SetSpeed(calSpeed,direction);
		}
		if(IO_READ_JP_FRONT() || IO_READ_JP_REAR()){
			caseDeltaTime = 0;
			JPC_state += 1;
			Lottery_JP_SetSpeed(JPC_MIN_SPEED,direction);
			Front_detect_num = 0;
			Rear_detect_num = 0;
			IN_Pocket = 0;
			Slowing_EncNum = 0;
			Stop_EncNum = 0;
			_slowing = false;
		}
		break;
	case JPC_JUDGE_2:
		if(Front_detect_num != 0 && Rear_detect_num != 0){
			IN_Pocket = JPC_Pocket_cal(Front_detect_num, Rear_detect_num, direction);
			if(caseDeltaTime >= 1500){
				caseDeltaTime = 0;
				JPC_state += 1;
				if(direction == 0){
					Stop_EncNum = JPC_Pocket(Front_detect_num + (station-1)*3);
					Slowing_EncNum = JPC_Pocket(Stop_EncNum + 1);
				}else if(direction == 1){
					Stop_EncNum = JPC_Pocket(Front_detect_num - (station-1)*3);
					Slowing_EncNum = JPC_Pocket(Stop_EncNum + 1);
				}
			}
			D_Mess_printf("JPC IN [%02d][%s]\n",IN_Pocket,JP_PocketName[IN_Pocket-1]);
		}else{
			if(IO_READ_JP_FRONT()){
				Front_detect_num = Encoder_Num_JP;
			}
			if(IO_READ_JP_REAR()){
				Rear_detect_num = Encoder_Num_JP;
			}
			caseDeltaTime = 0;
		}
		//D_Mess_printf("Front [%02d]\nRear  [%02d]\n",Front_detect_num, Rear_detect_num);
		//caseDeltaTime = 0;
		//JPC_state += 1;
		break;
	case JPC_STOP_2:
		if(!_slowing){
			if(Encoder_Num_JP == Slowing_EncNum){
				_slowing = true;
				caseDeltaTime = 0;
			}
		}else{
			if(caseDeltaTime <= 1500){
				Lottery_JP_SetSpeed(JPC_MIN_SPEED - (int)(((double)(JPC_MIN_SPEED - JPC_SLOW_SPEED)/1500.0)*(double)caseDeltaTime),direction);
			}
			if(Encoder_Num_JP == Stop_EncNum){
				Lottery_JP_SetSpeed(0,direction);
				_slowing = false;
				caseDeltaTime = 0;
				//JPC_state += 1;
				JPC_state = JPC_WAIT_4;
				*pocket = IN_Pocket;
			}
		}
		break;

	case JPC_WAIT_4:
		if(IN_Pocket == 1){ //JP Pocket
			if(caseDeltaTime >= 10000){
				JPC_state = JPC_LED_OFF;
				caseDeltaTime = 0;
			}
		}else{
			if(caseDeltaTime >= 5000 || skipPerformance){
				JPC_state = JPC_LED_OFF;
				caseDeltaTime = 0;
			}
		}
		break;

	//メインユニットのLED消灯
	case JPC_LED_OFF:
		if(caseDeltaTime >= 2000){
			IO_RESET_JP_LED();
			caseDeltaTime = 0;
			JPC_state += 1;
		}
		//JPC_state = JPC_LIFT_DOWN;
		break;

	//メインユニットの下降
	case JPC_LIFT_DOWN:
		if(skipPerformance){
			caseDeltaTime = 0;
			JPC_state += 1;
			break;
		}
		if(caseDeltaTime >= 500){
			if(JP_Lift_Down()){
				caseDeltaTime = 0;
				JPC_state += 1;
			}
		}
		break;

	case JPC_STACK_PROCESS:
		caseDeltaTime = 0;
		*pocket = 0;
		break;

	default:
		break;
	}
	RecentTime = G_System_counter;

	return JPC_state;
}

//JPCユニット減速時の減速カーブを定義　s
static int JPC_DecelerationSpeed_cal(int time){
	double x = ((double)JPC_DECELERATOIN_TIME-(double)time)/(double)JPC_DECELERATOIN_TIME;
	double f_x = x*x*x;
	int return_speed = JPC_MIN_SPEED + (int)((double)(JPC_MAX_SPEED - JPC_MIN_SPEED)*f_x);
	return return_speed;
}

//エンコーダ更新処理
static void Encoder_1st(void){
	static int enc_change_count = 0;
	static int home_change_count = 0;
	static int recent_enc_state = 0;
	static int recent_home_state = 0;
	int enc_state = (int)IO_READ_1ST_ENC();
	int home_state = (int)IO_READ_1ST_HOME();
	if(enc_state != recent_enc_state){
		enc_change_count++;
	}else{
		enc_change_count = 0;
	}
	if(enc_change_count >= 5){
		recent_enc_state = enc_state;
		enc_change_count = 0;
		if(enc_state == 1 || enc_state == 0){
			Encoder_Num_1st++;
		}
	}
	if(home_state != recent_home_state){
		home_change_count++;
	}else{
		home_change_count = 0;
	}
	if(home_change_count >= 5){
		recent_home_state = home_state;
		home_change_count = 0;
		if(home_state == 1){
			Encoder_Num_1st = 48;
		}
	}
	if(Encoder_Num_1st > 48){
		Encoder_Num_1st = 1;
	}
}

//エンコーダ更新処理
static void Encoder_2nd(void){
	static int enc_change_count = 0;
	static int home_change_count = 0;
	static int recent_enc_state = 0;
	static int recent_home_state = 0;
	int enc_state = (int)IO_READ_2ND_ENC();
	int home_state = (int)IO_READ_2ND_HOME();
	if(enc_state != recent_enc_state){
		enc_change_count++;
	}else{
		enc_change_count = 0;
	}
	if(enc_change_count >= 5){
		recent_enc_state = enc_state;
		enc_change_count = 0;
		if(enc_state == 1 || enc_state == 0){
			Encoder_Num_2nd++;
		}
	}
	if(home_state != recent_home_state){
		home_change_count++;
	}else{
		home_change_count = 0;
	}
	if(home_change_count >= 5){
		recent_home_state = home_state;
		home_change_count = 0;
		if(home_state == 1){
			Encoder_Num_2nd = 30;
		}
	}
	if(Encoder_Num_2nd > 30){
		Encoder_Num_2nd = 1;
	}
}

//エンコーダ更新処理
static void Encoder_3rd(void){
	static int enc_change_count = 0;
	static int home_change_count = 0;
	static int recent_enc_state = 0;
	static int recent_home_state = 0;
	int enc_state = (int)IO_READ_3RD_ENC();
	int home_state = (int)IO_READ_3RD_HOME();
	if(enc_state != recent_enc_state){
		enc_change_count++;
	}else{
		enc_change_count = 0;
	}
	if(enc_change_count >= 5){
		recent_enc_state = enc_state;
		enc_change_count = 0;
		if(enc_state == 1 || enc_state == 0){
			Encoder_Num_3rd++;
		}
	}
	if(home_state != recent_home_state){
		home_change_count++;
	}else{
		home_change_count = 0;
	}
	if(home_change_count >= 5){
		recent_home_state = home_state;
		home_change_count = 0;
		if(home_state == 1){
			Encoder_Num_3rd = 1;
		}
	}
	if(Encoder_Num_3rd > 12){
		Encoder_Num_3rd = 1;
	}
}

//エンコーダ更新処理
static void Encoder_JP(void){
	static int enc_change_count = 0;
	static int home_change_count = 0;
	static int recent_enc_state = 0;
	static int recent_home_state = 0;
	int enc_state = (int)IO_READ_JP_ENC();
	int home_state = (int)IO_READ_JP_HOME();
	if(enc_state != recent_enc_state){
		enc_change_count++;
	}else{
		enc_change_count = 0;
	}
	if(enc_change_count >= 5){
		recent_enc_state = enc_state;
		enc_change_count = 0;
		if(enc_state == 1){
			Encoder_Num_JP++;
		}
	}
	if(home_state != recent_home_state){
		home_change_count++;
	}else{
		home_change_count = 0;
	}
	if(home_change_count >= 5){
		recent_home_state = home_state;
		home_change_count = 0;
		if(home_state == 1){
			Encoder_Num_JP = 12;
		}
	}
	if(Encoder_Num_JP > 12){
		Encoder_Num_JP = 1;
	}
}


static void Lottery_1st2nd_SetSpeed(int speed){
	if(speed >= 1000) speed = 1000;
	if(speed <= 0) speed = 0;
	int setSpeed = (BLDC_MAX_SPEED / 1000.0) * speed;
	if(setSpeed == 0){
		IO_RESET_BLDC1_ENA();
	}else{
		IO_SET_BLDC1_ENA();
	}
	D_PWM_Set(BLDC1,setSpeed);
}

static void Lottery_3rd_SetSpeed(int speed){
	IO_SET_BLDC2_DIR();
	if(speed >= 1000) speed = 1000;
	if(speed <= 0) speed = 0;
	int setSpeed = (BLDC_MAX_SPEED / 1000.0) * speed;
	if(setSpeed == 0){
		IO_RESET_BLDC2_ENA();
	}else{
		IO_SET_BLDC2_ENA();
	}
	D_PWM_Set(BLDC2,setSpeed);
}

static void Lottery_JP_SetSpeed(int speed, int direction){
	if(speed >= 1000) speed = 1000;
	if(speed <= 0) speed = 0;
	int setSpeed = (BLDC_MAX_SPEED / 1000.0) * speed;
	if(setSpeed == 0){
		IO_RESET_BLDC3_ENA();
	}else{
		IO_SET_BLDC3_ENA();
	}
	if(direction == 0){
		IO_RESET_BLDC3_DIR();
	}else if(direction == 1){
		IO_SET_BLDC3_DIR();
	}
	D_PWM_Set(BLDC3,setSpeed);
}

static bool JP_Lift_Up(void){
	if(IO_READ_JP_UPPER()){
		IO_RESET_JPTOWER_ENA1();
		IO_RESET_JPTOWER_ENA2();
		return true;
	}else{
		IO_SET_JPTOWER_ENA1();
		IO_RESET_JPTOWER_ENA2();
		return false;
	}
}

static bool JP_Lift_Down(void){
	if(IO_READ_JP_LOWER()){
		IO_RESET_JPTOWER_ENA1();
		IO_RESET_JPTOWER_ENA2();
		return true;
	}else{
		IO_RESET_JPTOWER_ENA1();
		IO_SET_JPTOWER_ENA2();
		return false;
	}
}

