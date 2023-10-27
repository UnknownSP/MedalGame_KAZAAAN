#include "../Inc/xprintf.h"
#include "stm32f4xx_hal.h"
#include "../Inc/message.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include "main.h"

extern UART_HandleTypeDef huart2;

#define message(type, fmt, ...) _msg(type, __FUNCTION__, __LINE__, fmt, ## __VA_ARGS__)

static
void _xprintf(char *fmt, ...);

static
char buff[1024];

static
volatile bool had_completed = true;


void D_Mess_printf(const char* fmt, ...){
  va_list arp;

  //MW_waitForMessageTransitionComplete(100);
  va_start(arp, fmt);
  xvprintf(fmt, arp);
  va_end(arp);
  if( outptr - buff > MAX_STRING_LENGTH ){
	  D_Mess_flush();
  }
}

void _msg(const char* type,
          const char* func,
          int line,
          const char* fmt,
          ...){
  //MW_waitForMessageTransitionComplete(100);
  _xprintf("%-10s %-3d %-8s", func, line, type);

  {
    va_list arp;
    va_start(arp, fmt);
    xvprintf(fmt, arp);
    va_end(arp);
  }
  _xprintf("\n");

  D_Mess_flush();
}

void D_Mess_flush(void){
//  if( MW_waitForMessageTransitionComplete(100) != EXIT_SUCCESS ){
//    return;
//  }
	if(!had_completed){
		outptr = buff;
		return;
	}
	if( outptr != 0 ){
		*outptr++ = '\n';
		HAL_UART_Transmit_DMA(&huart2, (uint8_t*)buff, outptr - buff);
		had_completed = false;
	}
	outptr = buff;
}

static
void _xprintf(char *fmt, ...){
  va_list arp;
  va_start(arp, fmt);
  xvprintf(fmt, arp);
  va_end(arp);
}
//
//int MW_waitForMessageTransitionComplete(uint32_t timeout){
//  uint32_t time;
//  time = g_SY_system_counter;
//  while( !had_completed && time + timeout > g_SY_system_counter ){
//    SY_wait(1);
//  }
//  if( time + timeout <= g_SY_system_counter ){
//    return EXIT_FAILURE;
//  }
//  return EXIT_SUCCESS;
//}

void D_Mess_TransitionCompletedCallBack(void){
  had_completed = true;
}

