#ifndef __MESSAGE_H
#define __MESSAGE_H

#include<stdint.h>

//auto transmit.(>900)
#define MAX_STRING_LENGTH 900

#define message(type, fmt, ...) _msg(type, __FUNCTION__, __LINE__, fmt, ## __VA_ARGS__)
void _msg(const char* type,
	  const char* func,
	  int line,
	  const char* fmt,
	  ...);

void D_Mess_printf(const char* fmt,...);

void D_Mess_flush(void);
int D_Mess_Receive(uint8_t size, uint8_t *data);

void D_Mess_TransitionCompletedCallBack(void);
void D_Mess_ReceptionCompletedCallBack(void);
//int MW_waitForMessageTransitionComplete(uint32_t timeout);


#endif
