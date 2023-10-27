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

static bool Collect_3rdBall(int collectPocketNum, int enc_3rd, int *error, bool _init);
static bool Launch_OneShot(int speed, bool *_status, int *error, bool _init);
static bool IO_Read(uint8_t sensor);
static void LaunchMotor_SetSpeed(int speed);

//ホールドポケットでない場所の定義(激カザーンにも対応するため)
const bool notHold_Pocket[6] = {true,false,false,true,false,false};

//初期化
int appInit(void){
	D_CAN_SetReceiveAddress(CAN_MAIN_ADDRESS,CAN_ST2_ADDRESS,CAN_ST3_ADDRESS,CAN_ST4_ADDRESS);
	return 0;
}

int appTask(void){
	static BallProcess_State processState = B_WAIT_LAUNCH;

	static bool _LaunchBall = false;
	static bool _Launched1st = false;
	int error = 0;
	static int errorHandle = 0;
	static uint32_t recent_System_counter = 0;
	static uint32_t sndTime = 0;
	static uint32_t rcvTime = 0;
	static int inPocket_1st = 0;
	static int inPocket_2nd = 0;
	static int inPocket_3rd = 0;
	static bool _inPocket_1st_in = false;
	static bool _inPocket_1st_collect = false;
	static bool _through_1st_in = false;
	static bool _through_1st_collect = false;
	static bool _through_1st_collect_ena = false;
	static bool _inPocket_2nd_in = false;
	static bool _inPocket_2nd_collect = false;
	static bool _3rd_in_ena = false;
	static uint8_t JPC_IN_Pocket = 0;

	static uint8_t encoder_1st = 0;
	static uint8_t encoder_2nd = 0;
	static uint8_t encoder_3rd = 0;
	static uint8_t allHold_Pocket = 0;
	static uint8_t ownHold_Pocket = 0;
	static bool _collect3rdBall = false;
	static int8_t collect3rdQueue[6] = {-1};

	static bool _testMode = false;

	sndTime += G_System_counter - recent_System_counter;
	rcvTime += G_System_counter - recent_System_counter;
	recent_System_counter = G_System_counter;
	//自ステーションのホールドポケットを送信
	sndData[0] = ownHold_Pocket;
	//CAN送信タイミングなら送信
	if(sndTime >= CAN_SEND_INTERVAL){
		switch(STATION_NUMBER){
		case 1:
			D_CAN_Transmit(CAN_ST1_ADDRESS,sndData,8);
			break;
		case 2:
			D_CAN_Transmit(CAN_ST2_ADDRESS,sndData,8);
			break;
		case 3:
			D_CAN_Transmit(CAN_ST3_ADDRESS,sndData,8);
			break;
		case 4:
			D_CAN_Transmit(CAN_ST4_ADDRESS,sndData,8);
			break;
		}
		sndTime = 0;
	}
	D_CAN_Receive(CAN_MAIN_ADDRESS,rcvData,8);
	//メインからエンコーダの状態を受信
	encoder_1st = rcvData[0];
	encoder_2nd = rcvData[1];
	encoder_3rd = rcvData[2];
	allHold_Pocket = rcvData[3];
	if(rcvTime >= UART_RECEIVE_INTERVAL){
		D_Mess_Receive(8,rcvData_UART);
		rcvTime = 0;
	}

	//マイコンについてるボタンかスタートボタンが押されたらボール発射
	if(IO_READ_USERBUTTON() || IO_Read(IO_SIG_1)){
		if(processState == B_WAIT_LAUNCH){
			_LaunchBall = true;
		}
		IO_SET_USERLED();
		//IO_SET_SHUTTER_SOL();
		//LED_SetBrightness(PWM_LED_1ST,1000);
		//LED_SetBrightness(PWM_LED_2ND,1000);
		//LED_SetBrightness(PWM_LED_3RD,1000);

	}else{
		IO_RESET_USERLED();
		//IO_RESET_SHUTTER_SOL();
		//LED_SetBrightness(PWM_LED_1ST,0);
		//LED_SetBrightness(PWM_LED_2ND,0);
		//LED_SetBrightness(PWM_LED_3RD,0);
	}

	//PCから発射指令がきた場合も発射
	if(rcvData_UART[0] == 100){
		_LaunchBall = true;
	}
	//PCからテストモード指令が来ている場合はテストモードへ
	for(int i=1; i<7; i++){
		if(rcvData_UART[i] > 100){
			_testMode = true;
			processState = B_WAIT_LAUNCH;
			_LaunchBall = false;
			break;
		}else{
			_testMode = false;
		}
	}


	/*
	static int cal_count = 0;
	static int count_1st = 0;
	static int count_2nd = 300;
	static int count_3rd = 600;
	static int coeff_1st = 1;
	static int coeff_2nd = 1;
	static int coeff_3rd = 1;
	cal_count++;
	if(cal_count >= 30){
		cal_count = 0;
		count_1st += coeff_1st;
		if(count_1st >= 1000){
			coeff_1st = -1;
		}
		if(count_1st <= 0){
			coeff_1st = 1;
		}
		count_2nd += coeff_2nd;
		if(count_2nd >= 1000){
			coeff_2nd = -1;
		}
		if(count_2nd <= 0){
			coeff_2nd = 1;
		}
		count_3rd += coeff_3rd;
		if(count_3rd >= 1000){
			coeff_3rd = -1;
		}
		if(count_3rd <= 0){
			coeff_3rd = 1;
		}
	}
	LED_SetBrightness(PWM_LED_1ST,count_1st);
	LED_SetBrightness(PWM_LED_2ND,count_2nd);
	LED_SetBrightness(PWM_LED_3RD,count_3rd);
	*/

	//1段目発射処理 テストモードの場合は発射処理しない
	if(_LaunchBall && !_testMode){
		if(processState == B_WAIT_LAUNCH){
			processState = B_LAUNCHING_1ST;
		}
		if(Launch_OneShot(1000, &_Launched1st, &error, false)){
			_LaunchBall = false;
			_Launched1st = false;
			Launch_OneShot(1000, &_Launched1st, &error, true);
			if(error){
				//ここに発射エラーハンドラ
				errorHandle = error; //テスト用
			}
		}
		if(_Launched1st){
			processState = B_LAUNCHED_1ST;
		}
	}

	//1段目 1st_in 3の倍数 UPポケット
	//1段目 1st_collect 上記数に2+

	//1段目発射後
	if(processState == B_LAUNCHED_1ST){
		//入賞センサのどちらかが反応した場合かスロープセンサが反応した場合
		//_through_1st_inと_through_1st_collectは次ボール発射した時に前ボールの判定が入ってしまうことを防ぐ
		if(IO_Read(IO_1ST_IN)==1 && !_through_1st_in){
			inPocket_1st = encoder_1st;
			_inPocket_1st_in = true;
			_through_1st_in = true;
			_through_1st_collect = true;
		}else if(IO_Read(IO_1ST_COLLECT) == 1 && !_through_1st_collect){
			inPocket_1st = encoder_1st - 5;
			if(inPocket_1st < 1){
				inPocket_1st += 48;
			}
			_inPocket_1st_collect = true;
			_through_1st_collect = true;
			_through_1st_collect_ena = true;
		}else if(IO_Read(IO_1ST_SLOPE) == 1){
			processState = B_LAUNCHED_2ND;
		}
		if(_inPocket_1st_in || _inPocket_1st_collect){
			//入賞ポケットの数調整
			inPocket_1st = (int)(((double)inPocket_1st / 2.0) + 0.5);
			//入賞した場所ごとにUPかOUTかの判定
			if(_inPocket_1st_in){
				if(inPocket_1st % 3 != 0){
					processState = B_OUT_1ST;
				}else{
					processState = B_LAUNCHING_2ND;
				}
			}else if(_inPocket_1st_collect){
				if(inPocket_1st % 3 != 0){
					processState = B_OUT_1ST;
				}else{
					//ここにエラーハンドラ
					//collectセンサの場所でUPが反応することはない(はず)
				}
			}
			_inPocket_1st_in = false;
			_inPocket_1st_collect = false;
		}
	}
	//OUTに入ったボールのセンサ通過待ち
	if(_through_1st_in){
		if(IO_Read(IO_1ST_IN)==0){
			_through_1st_in = false;
		}
	}
	//OUTに入ったボールのセンサ通過待ち
	if(_through_1st_collect){
		if(!_through_1st_collect_ena && IO_Read(IO_1ST_COLLECT) == 1){
			_through_1st_collect_ena = true;
		}
		if(_through_1st_collect_ena && IO_Read(IO_1ST_COLLECT) == 0){
			_through_1st_collect = false;
			_through_1st_collect_ena = false;
		}
	}

	//1段目UPが反応した場合
	if(processState == B_LAUNCHING_2ND){
		if(IO_Read(IO_1ST_SLOPE) == 1){
			processState = B_LAUNCHED_2ND;
		}
	}

	//2段目 2nd_in 1,6,11 UPポケット
	//2段目 2nd_collect 上記数に2+

	//2段目発射後
	if(processState == B_LAUNCHED_2ND){
		//入賞センサのどちらかが反応した場合かスロープセンサが反応した場合
		if(IO_Read(IO_2ND_IN)==1){
			inPocket_2nd = encoder_2nd;
			_inPocket_2nd_in = true;
		}else if(IO_Read(IO_2ND_COLLECT) == 1){
			inPocket_2nd = encoder_2nd - 3;
			if(inPocket_2nd < 1){
				inPocket_2nd += 30;
			}
			_inPocket_2nd_collect = true;
		}else if(IO_Read(IO_2ND_SLOPE) == 1){
			processState = B_LAUNCHED_3RD;
		}
		if(_inPocket_2nd_in || _inPocket_2nd_collect){
			//入賞ポケットの数調整
			inPocket_2nd /= 2;
			if(inPocket_2nd == 0) inPocket_2nd = 15;
			//入賞した場所ごとにUPかOUTかの判定
			if(_inPocket_2nd_in){
				if(inPocket_2nd % 5 != 1){
					processState = B_OUT_2ND;
				}else{
					processState = B_LAUNCHING_3RD;
				}
			}else if(_inPocket_2nd_collect){
				if(inPocket_2nd % 5 != 1){
					processState = B_OUT_2ND;
				}else{
					//ここにエラーハンドラ
					//collectセンサの場所でUPが反応することはない(はず)
				}
			}
			_inPocket_2nd_in = false;
			_inPocket_2nd_collect = false;
			_3rd_in_ena = false;
		}
	}

	//2段目UPが反応した場合
	if(processState == B_LAUNCHING_3RD){
		if(IO_Read(IO_2ND_SLOPE) == 1){
			processState = B_LAUNCHED_3RD;
			_3rd_in_ena = false;
		}
	}

	//3段目 3rd_in 1 UPポケット
	//3段目発射後
	if(processState == B_LAUNCHED_3RD){
		//_3rd_in_enaはセンサが反応した場合のみだけ処理を行うためのフラグ
		if(!_3rd_in_ena && IO_Read(IO_3RD_IN) == 0){
			_3rd_in_ena = true;
		}
		//センサが反応した場合
		if(_3rd_in_ena && IO_Read(IO_3RD_IN) == 1){
			_3rd_in_ena = false;
			//値調整
			inPocket_3rd = encoder_3rd / 2;
			if(inPocket_3rd == 6) inPocket_3rd = 0;
			if(((allHold_Pocket >> inPocket_3rd) & 0x01) || ((ownHold_Pocket >> inPocket_3rd) & 0x01)){
				//既にホールドされている球が反応した場合の処理
			}else{
				//ホールドされていない場所で反応した場合の処理

				//ホールドポケットではない場合
				if(notHold_Pocket[inPocket_3rd]){
					//球回収処理
					//JPCに入った場合の回収処理とかぶる可能性があるためキューで処理
					//JPCの回収処理は先頭にいれる
					for(int i=5; i>=0; i--){
						if(collect3rdQueue[i]==-1){
							continue;
						}else{
							collect3rdQueue[i+1] = collect3rdQueue[i];
						}
					}
					collect3rdQueue[0] = inPocket_3rd;
					Collect_3rdBall(collect3rdQueue[0],encoder_3rd,&error,true);
					_collect3rdBall = true;
				}else{
					//ホールドポケットだった場合
					ownHold_Pocket |= 0x01 << inPocket_3rd;
				}
				if(inPocket_3rd != 0){
					processState = B_OUT_3RD;
				}else{
					processState = B_UP_JPC;
				}
			}
		}
	}

	//3段目球回収処理
	if(_collect3rdBall){
		if(Collect_3rdBall(collect3rdQueue[0],encoder_3rd,&error,false)){
			for(int i=0; i<5; i++){
				collect3rdQueue[i] = collect3rdQueue[i+1];
			}
			collect3rdQueue[5] = -1;
			if(collect3rdQueue[0] == -1){
				_collect3rdBall = false;
			}else{
				Collect_3rdBall(collect3rdQueue[0],encoder_3rd,&error,true);
			}
		}
	}

	//JPCポケットに入った場合
	if(processState == B_UP_JPC){
		sndData[1] = 99; //センターへJPC抽選スタック要求
		D_LED_Handler(processState, 300);
		processState = B_JPC_1ST;
	}

	//JPC一回目
	if(processState == B_JPC_1ST){
		if(rcvData[4] >= 9){ //センターのJPC_WAIT_2より後
			JPC_IN_Pocket = rcvData[5];
			processState = B_JPC_1ST_WAIT;
		}
	}

	//JPC一回目待機
	if(processState == B_JPC_1ST_WAIT){
		if(JPC_IN_Pocket == 1){
			processState = B_JPC_2ND;
		}else{
			if(rcvData[4] >= 20){ //センターのJPC_STACK_PROCESSより後
				processState = B_WAIT_LAUNCH;
				sndData[1] = 0;
			}
		}
	}

	//JPC二回目
	if(processState == B_JPC_2ND){
		if(rcvData[4] >= 17){ //センターのJPC_WAIT_4より後
			JPC_IN_Pocket = rcvData[5];
			processState = B_JPC_2ND_WAIT;
		}
	}

	//JPC二回目待機
	if(processState == B_JPC_2ND_WAIT){
		if(JPC_IN_Pocket == 1){
			processState = B_GET_JP;
		}else{
			if(rcvData[4] >= 20){ //センターのJPC_STACK_PROCESSより後
				processState = B_WAIT_LAUNCH;
				sndData[1] = 0;
			}
		}
	}

	//JPC獲得した場合
	//演出未実装
	if(processState == B_GET_JP){
		if(rcvData[4] >= 20){ //センターのJPC_STACK_PROCESSより後
			processState = B_WAIT_LAUNCH;
			sndData[1] = 0;
		}
	}

	//テストモードではない場合はLEDの処理
	if(!_testMode) D_LED_Handler(processState, 300);

	//アウトポケットに入った場合は状態初期化
	if(processState == B_OUT_1ST && !_LaunchBall){
		processState = B_WAIT_LAUNCH;
	}
	if(processState == B_OUT_2ND){
		processState = B_WAIT_LAUNCH;
	}
	if(processState == B_OUT_3RD){
		processState = B_WAIT_LAUNCH;
	}

	//debug------------------------------------------------------

	/*
	if(IO_Read(IO_1ST_IN) == 1 || IO_Read(IO_1ST_COLLECT) == 1){
		inPocket_1st = encoder_1st;
	}
	if(IO_Read(IO_2ND_COLLECT) == 1){
		inPocket_2nd = encoder_2nd;
	}
	if(IO_Read(IO_3RD_IN) == 1){
		inPocket_3rd = encoder_3rd;
	}

	//*/
	//debug------------------------------------------------------

	//testMode---------------------------------------------------
	if(_testMode){
		//LEDTest
		if(rcvData_UART[1] == 131 && rcvData_UART[3] == 132 && rcvData_UART[5] == 133){
			D_LED_SetBrightness(PWM_LED_1ST, rcvData_UART[2] * 10);
			D_LED_SetBrightness(PWM_LED_2ND, rcvData_UART[4] * 10);
			D_LED_SetBrightness(PWM_LED_3RD, rcvData_UART[6] * 10);
		}
	}
	//testMode---------------------------------------------------

	//デバッグ用
	int16_t debug_bits = 0;
	debug_bits &= 0;
	debug_bits |= ((int)IO_READ_1ST_IN() << 0);
	debug_bits |= ((int)IO_READ_1ST_COLLECT() << 1);
	debug_bits |= ((int)IO_READ_1ST_SLOPE() << 2);
	debug_bits |= ((int)IO_READ_2ND_IN() << 3);
	debug_bits |= ((int)IO_READ_2ND_COLLECT() << 4);
	debug_bits |= ((int)IO_READ_2ND_SLOPE() << 5);
	debug_bits |= ((int)IO_READ_3RD_IN() << 6);
	debug_bits |= ((int)IO_READ_3RD_COLLECT() << 7);
	debug_bits |= ((int)IO_READ_3RD_SHUTTER() << 8);
	debug_bits |= ((int)IO_READ_L_MECHA() << 9);
	debug_bits |= ((int)IO_READ_L_BALLWAIT() << 10);
	debug_bits |= ((int)IO_READ_L_START() << 11);
	debug_bits |= ((int)IO_READ_L_BALLSUPPLY() << 12);
	D_Mess_printf("\033[1;1H");
	D_Mess_printf("%016b\n", debug_bits);
	D_Mess_printf("%02d\n", errorHandle);
	D_Mess_printf("%3d,%3d,%3d,%3d\n", sndData[0],sndData[1],sndData[2],sndData[3]);
	D_Mess_printf("%3d,%3d,%3d,%3d,%3d,%3d\n", rcvData[0],rcvData[1],rcvData[2],rcvData[3],rcvData[4],rcvData[5]);
	//D_Mess_printf("%02d\n", inPocket_1st);
	//D_Mess_printf("%02d\n", inPocket_2nd);
	//D_Mess_printf("%02d\n", inPocket_3rd);
	//D_Mess_printf("%08b\n", ownHold_Pocket);
	//D_Mess_printf("%08b\n", allHold_Pocket);
	D_Mess_printf("processState: ,%2d\n", processState);
	D_Mess_printf("PC: ,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d\n",rcvData_UART[0],rcvData_UART[1],rcvData_UART[2],rcvData_UART[3],rcvData_UART[4],rcvData_UART[5],rcvData_UART[6],rcvData_UART[7]);

	return 0;
}

static bool Collect_3rdBall(int collectPocketNum, int enc_3rd, int *error, bool _init){
	static bool _3rd_in = true;
	static int pocketNum = -1;
	static int errCount_shutter = 0;
	static bool _waitShutter = false;
	if(_init){
		_3rd_in = true;
		pocketNum = -1;
		errCount_shutter = 0;
		_waitShutter = false;
		*error = 0;
		return true;
	}
	if(IO_Read(IO_3RD_IN) == 0 && !_3rd_in){
		_3rd_in = true;
	}
	if(IO_Read(IO_3RD_IN) == 1 && _3rd_in){
		pocketNum = enc_3rd / 2;
		if(pocketNum == 6) pocketNum = 0;
		_3rd_in = false;
	}
	if(pocketNum == collectPocketNum && !_waitShutter){
		IO_SET_SHUTTER_SOL();
		errCount_shutter ++;
		if(errCount_shutter >= ERRORTIME_SHUTTER_OPEN){
			*error = 1;
		}
		if(IO_Read(IO_3RD_SHUTTER) == 1){
			errCount_shutter = 0;
		}
		if(IO_Read(IO_3RD_COLLECT) == 1){
			errCount_shutter = 0;
			IO_RESET_SHUTTER_SOL();
			_waitShutter = true;
		}
	}
	if(_waitShutter){
		errCount_shutter ++;
		if(errCount_shutter >= ERRORTIME_SHUTTER_CLOSE){
			*error = 1;
		}
		if(IO_Read(IO_3RD_SHUTTER) == 0){
			errCount_shutter = 0;
			return true;
		}
	}
	return false;
}

static bool Launch_OneShot(int speed, bool *_status, int *error, bool _init){
	static bool _firstFlag = true;
	static int startTime = 0;
	static int recentTime = 0;
	static int calTime = 0;
	static int errTime_Rotate = 0;
	static int errTime_Supply = 0;
	int deltaTime = 0;
	static bool _recent_L_Mecha = false;
	static bool _now_L_Mecha = false;
	static bool _isLaunched = false;
	bool _returnFlag = false;

	//初期化
	if(_init){
		_firstFlag = true;
		startTime = 0;
		recentTime = 0;
		calTime = 0;
		errTime_Rotate = 0;
		errTime_Supply = 0;
		_recent_L_Mecha = false;
		_now_L_Mecha = false;
		_isLaunched = false;
		LaunchMotor_SetSpeed(0);
		return true;
	}
	//エラー応答用
	*error = 0;
	//発射確認用
	*_status = false;
	//初回更新
	if(_firstFlag){
		startTime = G_System_counter;
		_firstFlag = false;
	}
	if(speed <= 0) speed = 1;
	//時刻計算
	calTime = G_System_counter - startTime;
	deltaTime = calTime - recentTime;

	LaunchMotor_SetSpeed(speed);

	//エラー用時間積算及び更新
	errTime_Rotate += deltaTime;
	_now_L_Mecha = IO_Read(IO_L_MECHA);
	if(_now_L_Mecha != _recent_L_Mecha){
		errTime_Rotate = 0;
	}
	errTime_Supply += deltaTime;
	if(IO_Read(IO_L_BALLSUPPLY)){
		errTime_Supply = 0;
	}

	//発射判定
	if(!_isLaunched && IO_Read(IO_L_START)) {
		_isLaunched = true;
		*_status = true;
	}
	//発射後処理
	if(_isLaunched){
		if(!IO_Read(IO_L_BALLWAIT)){
			_returnFlag = true;
			LaunchMotor_SetSpeed(0);
		}
	}

	//エラー判定用
	if(errTime_Rotate >= (int)((double)ERRORTIME_L_ROTATE * (double)(1000.0/speed))){
		*error = 1;
		LaunchMotor_SetSpeed(0);
		_returnFlag = true;
	}
	if(errTime_Supply >= (int)((double)ERRORTIME_L_SUPPLY * (double)(1000.0/speed))){
		*error = 2;
		LaunchMotor_SetSpeed(0);
		_returnFlag = true;
	}

	_recent_L_Mecha = _now_L_Mecha;
	recentTime = calTime;

	return _returnFlag;
}

static bool IO_Read(uint8_t sensor){
	static bool _io_State[14] = {false};
	static bool _io_returnState[14] = {false};
	static bool _io_recentState[14] = {false};
	static uint8_t io_Count[14] = {0};

	_io_State[IO_1ST_IN]		= IO_READ_1ST_IN();
	_io_State[IO_1ST_COLLECT]	= IO_READ_1ST_COLLECT();
	_io_State[IO_1ST_SLOPE]		= IO_READ_1ST_SLOPE();
	_io_State[IO_2ND_IN]		= IO_READ_2ND_IN();
	_io_State[IO_2ND_COLLECT]	= IO_READ_2ND_COLLECT();
	_io_State[IO_2ND_SLOPE]		= IO_READ_2ND_SLOPE();
	_io_State[IO_3RD_IN]		= IO_READ_3RD_IN();
	_io_State[IO_3RD_COLLECT]	= IO_READ_3RD_COLLECT();
	_io_State[IO_3RD_SHUTTER]	= IO_READ_3RD_SHUTTER();
	_io_State[IO_L_MECHA]		= IO_READ_L_MECHA();
	_io_State[IO_L_BALLWAIT]	= IO_READ_L_BALLWAIT(); 	
	_io_State[IO_L_START]		= IO_READ_L_START();
	_io_State[IO_L_BALLSUPPLY]	= IO_READ_L_BALLSUPPLY(); 
	_io_State[IO_SIG_1]			= IO_READ_SIG_1();

	for(int i=0; i<14; i++){
		if(_io_State[i] != _io_recentState[i]){
			io_Count[i]++;
		}else{
			io_Count[i] = 0;
		}
		if(io_Count[i] >= 5){
			_io_recentState[i] = _io_State[i];
			io_Count[i] = 0;
			_io_returnState[i] = _io_State[i];
		}
	}

	return _io_returnState[sensor];
}

static void LaunchMotor_SetSpeed(int speed){
	if(speed >= 1000) speed = 1000;
	if(speed <= 0) speed = 0;
	int setValue = (TIM_MAX_COUNT / 1000.0) * speed;
	D_PWM_Set(PWM_L_MOTOR,setValue);
}


