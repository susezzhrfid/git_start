#ifndef DrvHook_H
#define DrvHook_H

#include "Typedef.h"

extern void UartReadHook(WORD wPort, DWORD dwLen);
extern void UartWriteHook(WORD wPort, DWORD dwLen);

void WlReadHook(DWORD dwLen);
void WlWriteHook(DWORD dwLen);

#endif