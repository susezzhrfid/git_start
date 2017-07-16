/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ProPara.h
 * ժ    Ҫ�����ļ���Ҫ�����Ѹ�Э�鲻ͬ�Ĳ���װ�ص���ͬ�Ĳ����ṹ��ȥ,
 *			 ��TSocket,TGprs��,ʹ���õ�ͨ�Ŵ��벻��ֱ����Ը���
 *			 Э��Ĳ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
 * ��    ע��
 *********************************************************************************************************/
#ifndef PROPARA_H
#define PROPARA_H
#include "ProStruct.h"
#include "Pro.h"
#include "DrvCfg.h"
#include "ProIf.h"
//#include "GbPro.h"
#include "CommIf.h"
#include "GprsIf.h"
//#include "SocketIf.h"
//#include "DbFmt.h"
#include "ComAPI.h"

void LoadGprsPara(TGprsIf* pGprs);
void LoadSockPara(TSocketIf* pSocket);

bool GetApn(char* pszApn);
bool GetUserAndPsw(char *psUser, char *psPsw);
bool GetMasterIp(TMasterIp* pMasterIp);
bool GetSvrPara(TSvrPara* pSvrPara);
void LoadLocal232Para(TCommIf* pCommIf);
void LoadLocalIrPara(TCommIf* pCommIf);

#endif //PROPARA_H


