/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrAPI.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֳ���Ĺ����ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2007��4��
 *********************************************************************************************************/
#ifndef METERAPI_H
#define METERAPI_H
#include "sysarch.h"
#include "MtrAPIEx.h"
#include "TypeDef.h"
#include "ThreadMonitor.h"

#define METERING_AUTO_MODE	0
#define METERING_RULE_1		1
#define METERING_RULE_2		2
#define METERING_RULE_3		3

void SetMtrChgFlg(WORD wPn);
void ClrMtrChgFlg(WORD wPn);
bool IsMtrParaChg(WORD wPn);

bool GetMeterPara(WORD wPn, TMtrPara* pMtrPara);
BYTE GetMeterInterv(WORD wPn);
//BYTE GetMeterInterv();
//bool GetMtrDate(BYTE* pbBuf);
//bool GetMtrDate(BYTE bPort, BYTE* pbBuf);
//bool GetPnDate(WORD wPn, BYTE* pbBuf);
bool GetMeterAddr(WORD wPn, BYTE* pbAddr);
BYTE GetMtrNumByPort(WORD wPort, WORD* pbTotalMtrNum, WORD* pwMtrRdSuccNum);
BYTE GetMtrNum();
bool SaveSearchPnToPointSect(BYTE* pbMtrAddr, BYTE bPro, BYTE bPort);
bool SaveSearchPnToF10();
void NewMtrThread();
bool InitMeter();

DWORD GbValToBaudrate(BYTE val);
BYTE GbBaudrateToVal(DWORD dwBaudRate);
BYTE GbValToParity(BYTE val);
BYTE GbValToStopBits(BYTE val);
BYTE GbValToByteSize(BYTE val);

void GetLogicPortNum(WORD* pwNum, WORD* pwMin, WORD* pwMax);
int MeterPortToPhy(BYTE bPortNo);
BYTE MeterProtoLocalToInner(BYTE bProto);
//����:����������
WORD SearchPnFromMask(const BYTE* pbPnMask, WORD wStartPn);
BYTE GetMtrClassNum(WORD wId, WORD wExpPn, BYTE bMtrClass);
BYTE GetVitalPn(BYTE* p);
#endif //METERAPI_H