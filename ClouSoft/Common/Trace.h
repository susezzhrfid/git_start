#ifndef TRACE_H
#define TRACE_H
#include "Typedef.h"

extern bool g_fDbInited;
extern bool g_fTraceEnable; 

extern bool IsDebugOn(BYTE bType);
bool InitDebug();

void EnableTrace(bool fEnable);
bool IsTraceEnable();

#ifndef SYS_WIN
WORD PrintBuf(BYTE* out, WORD wOutLen, BYTE* in, WORD wInLen);
void TraceBuf(WORD wSwitch, char* szHeadStr, BYTE* p, WORD wLen);
void TraceFrm(char* pszHeader, BYTE* pbBuf, WORD wLen);
#endif

#endif
