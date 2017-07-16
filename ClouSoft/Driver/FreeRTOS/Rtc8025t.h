#ifndef RTC8025T_H
#define RTC8025T_H

#include "ComStruct.h"

#define RTC8025T        1

void TwiInit(void);
void Rtc8025Init(void);

bool Set8025Time(const TTime *time);
bool Get8025Time(TTime *time);

#endif