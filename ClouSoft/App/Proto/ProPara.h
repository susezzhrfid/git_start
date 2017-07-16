/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ProPara.h
 * 摘    要：本文件主要用来把各协议不同的参数装载到相同的参数结构中去,
 *			 如TSocket,TGprs等,使共用的通信代码不用直接面对各种
 *			 协议的差异
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2011年3月
 * 备    注：
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


