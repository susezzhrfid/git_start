#ifndef _ESAMCMD_H_
#define _ESAMCMD_H_
//#include "bios.h"
#include "sysarch.h"
//#include "sysapi.h"
//#include "apptypedef.h"


typedef struct SWTab
{
	BYTE bSW1;
	BYTE bSW2;
	char *str;//状态说明性文字
}TSWTab;

extern BYTE g_bEsamTxRxBuf[1800];		//发送接收共用
extern TSem g_semEsam;

bool EsamInit();
void EsamClose();
bool EsamReset();
int EsamRead(BYTE* pbBuf, WORD wBufSize);
int EsamRead2(BYTE* pbBuf, WORD wExpLen);
int EsamWrite(BYTE* pbBuf, WORD wBufSize);
int EsamTxRxCmd(const BYTE* pbTx, BYTE* pbRx, WORD wExpLen);
int EsamGetTermInfo(BYTE* pbRx);
int EsamInitSession(BYTE* pbTx, WORD wTxLen, BYTE* pbRx);
int EsamNegotiateKey(BYTE* pbTx, WORD wTxLen, BYTE* pbRx);
bool EsamUpdSymKey(BYTE bKeyNum, BYTE* pbKey);
int EsamUpdCert(BYTE* pbRx);
bool EsamUpdCA(BYTE* pbTx, WORD wTxLen);
int EsamIntCert(BYTE* pbTx, WORD wTxLen, BYTE* pbRx);
int EsamExtCert(BYTE* pbTx, WORD wTxLen, BYTE* pbRx);
bool EsamSwitchState(BYTE bP1, BYTE* pbTx, WORD wTxLen);
bool EsamSetOfflineCnt(BYTE* pbTx, WORD wTxLen);
bool EsamTransEncrAuth(BYTE* pbTx, WORD wTxLen);
int EsamVerifyMac(BYTE bAFN, BYTE* pbData, WORD wDataLen, BYTE* pbMac, BYTE* pbRx);
bool EsamGrpBroadcastVerifyMac(BYTE bAFN, WORD wGrpAddr, BYTE* pbData, WORD wDataLen, BYTE* pbRx);
int EsamGetRandom(BYTE* pbRx, BYTE bRandomLen);
bool EsamGetMtrAuth(BYTE bP2, BYTE* pbMtrCiph, BYTE* pbMtrAddr, BYTE* pbTskData, BYTE* pbR1, BYTE* pbER1);
int EsamGetAdjTmCiph(const BYTE* pbTskFmt, BYTE* pbTskData, BYTE bTskLen, BYTE* pbMtrKeyCiph, BYTE* pbR2, BYTE* pbRx);

#endif /* _ESAMCMD_H_ */
