#ifndef ESAM_H
#define ESAM_H

#include "Typedef.h"
#include "SysArch.h"

extern BYTE g_bEsamTxRxBuf[1800];		//发送接收共用
extern TSem g_semEsam;
void esam_init(void);
void esam_warm_reset(void);
int esam_write(BYTE * buffer, int count);
int esam_read(BYTE *buf, int count);
bool EsamGetMtrAuth(BYTE bP2, BYTE* pbMtrCiph, BYTE* pbMtrAddr, BYTE* pbTskData, BYTE* pbR1, BYTE* pbER1);
int EsamGetAdjTmCiph(BYTE* pbTskFmt, BYTE* pbTskData, BYTE bTskLen, BYTE* pbMtrKeyCiph, BYTE* pbR2, BYTE* pbRx);

#endif