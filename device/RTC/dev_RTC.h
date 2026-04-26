#ifndef __DEV_RTC_H
#define __DEV_RTC_H

#include "com_type.h"

extern s16 MyRTC_Time[];

void MyRTC_Init(void);
void MyRTC_SetTime(void);
void MyRTC_ReadTime(void);

#endif
