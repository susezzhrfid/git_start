/*******************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�FaAPI.c
 * ժ    Ҫ��ϵͳʵ����Ҫ��API
 * ��ǰ�汾��1.0.0
 * ��    �ߣ����
 * ������ڣ�2011��1��
 * ��    ע��
 ******************************************************************************/
#include "Sysarch.h"
#include <string.h>
#include "Sysapi.h"
#include "SysDebug.h"
#include "Comm.h"
#include "DrvConst.h"
#include "Drivers.h"
#include "FaConst.h"
#include "FaCfg.h"
#include "FaAPI.h"
#include "StatMgr.h"
#include "FaStruct.h"
#include "ComStruct.h"
#include "Info.h"
#include "DbAPI.h"
#include "Trace.h"
#include "TaskDB.h"
#include "MtrCtrl.h"
#include "MtrAPI.h"
#include "ProAPI.h"
#include "FlashMgr.h"
#include "MtrProAPI.h"
#include "CommonTask.h"
#include "DrvCfg.h"
#include "ProIf.h"
#include "FlashIf.h"
#include "TermnExc.h"
#include "FileMgr.h"
#include "TermnExc.h"
#include "EsamCmd.h"
#include "BatMtrTask.h"
#include "DbFmt.h"
#include "Gbpro.h"
#ifndef SYS_WIN
#include "Sample.h"
#include "YX.h"
#include "CctAPI.h"
#endif

DWORD g_dwExtCmdFlg = 0;
DWORD g_dwExtCmdClick = 0;
DWORD g_dwUpdateTime = 0;
bool g_fUpdFatOk = true;
BYTE g_b485RxLed=0;
BYTE g_b485TxLed=0;
BYTE g_bMsRxLed=0;
BYTE g_bMsTxLed=0;
bool g_fAlertLed=false;
bool g_fStopMtrRd=false;
bool g_fFaInitOK=false; //FA��ʼ����־
DWORD g_dwLastStopMtrClick=0;
WORD g_wStopSec = 0;
bool g_fDownSoft = false;   //�Ƿ�����״̬
bool g_fTmrRptState = false;
//TSem g_semRemoteIf;
TSem g_semTask;
TCommPara g_CommPara;
//TPro645Ex g_ProEx;
BYTE g_bRdMtrAlr[12];//485����״̬:0����,1����ʧ��,2����ָ�(�澯��ͨ��ʧ��);ÿһ��bit��ʾһ�����״̬
BYTE g_bRdMtrAlrStatus[12];

WORD g_wLedBurstCnt[MAX_LED_NUM];
BYTE g_bLedCtrlMode[MAX_LED_NUM];
bool g_fRxLedTestCmd = false;
bool g_fTermTestMode = false;
//BYTE g_bRxBatTask0Cnt = 0;
//BYTE g_bRxBatTask1Cnt = 0;
DWORD g_dwTimes[3] = {0};
TSoftVerChg g_SoftVerChg;	 //����İ汾����¼�
BYTE g_bDefaultCfgID = 0;
bool g_fDownLoad = false;

#ifndef SYS_WIN
TYxPara g_tYx;   //ң��
#endif

TPowerOffTmp g_PowerOffTmp;     //�����ݴ����
const TPowerOffTmp g_DefaultPowerOffTmp = {PWROFF_VER, //�汾 
									 true,  //�����ݴ������Ч��־
									 false,   //GPRS������
									 {0, 0, 0, 0},  //Զ����������ķ�����IP��ַ
                                     4097,          //�ն˱��ض˿ںţ�ÿ��ʹ��������4097-9200
									 false,			//bool fAlrPowerOff	����ǰ�ϱ���ͣ��澯
									 0,				//WORD wRstNum;	��λ����
									 0,				//�̼߳�ظ�λ����
									 {0, 0, 0, 0},		//�̼߳�ظ�λ���һ�θ�λ���߳�����
									 0,				//iRemoteDownPort
									{0, 0, 0, 0, 0, 0, 0},//tPoweroff �ϴ�����ʱ��
									{0, 0, 0, 0, 0, 0, 0},//tPoweroff �ϴ�ͣ��ʱ��
									 0,				//��¼������ʼ���¼�����Ч��Ч
                                     {0, {0, 0, 0, 0, 0, 0, 0, 0, 0},},
                                     {0xee,0xee,0xee,0xee,0xee,0xee,0xee,0xee,0xee,0xee,
                                      0xee,0xee,0xee,0xee,0xee,0xee,0xee,0xee,0xee,0xee,
                                      0xee,0xee,0xee,0xee,0xee,0xee,0xee,0xee,0xee,0xee},//����ֵ��ʼ��Ϊ��Ч
                                     0, //�ն�ͳ�ƣ��������¹���ʱ�䡢��λ����������
                                     0, //�յ�ѹͳ��
                                     0, //�µ�ѹͳ��
									}; 

void StopMtrRd(WORD wStopSec)
{
    g_fStopMtrRd = true;
    g_dwLastStopMtrClick = GetClick();
    if (wStopSec == 0)
        wStopSec = 300;

    g_wStopSec = wStopSec;
}

void DoFapCmd()
{
	if (g_dwExtCmdFlg==0 || GetClick()-g_dwExtCmdClick<2)
		return;
		
	if (g_dwExtCmdFlg == FLG_FORMAT_DISK)
	{
		StopMtrRd(0);
		Sleep(200);
		//InFlashFormat();  
        ClearWDG();
		EraseAllInFile();
        ClearWDG();
		ExFlashEraseChip();
        DTRACE(DB_CRITICAL, ("finished to format disk, rebooting...\r\n"));
		ResetCPU();
	}
	else if (g_dwExtCmdFlg == FLG_DEFAULT_CFG)
	{
	}
	else if (g_dwExtCmdFlg==FLG_HARD_RST || g_dwExtCmdFlg==FLG_REMOTE_DOWN)
	{
		SavePoweroffTmp(true);	//�յ������������󲻱���ϵͳ���ļ�,�Ᵽ��������,����������Ӧʱ�����
		ResetCPU();
	}
	else if (g_dwExtCmdFlg == FLG_DISABLE_METEREXC)
	{
	}
	else if (g_dwExtCmdFlg == FLG_ENERGY_CLR)
	{
	}
	else if(g_dwExtCmdFlg == FLG_APP_RST && GetClick()-g_dwExtCmdClick>60)
	{
		SavePoweroffTmp(true);	//�յ������������󲻱���ϵͳ���ļ�,�Ᵽ��������,����������Ӧʱ�����
		Sleep(100);
		ResetCPU();
	}
	if(g_dwExtCmdFlg != FLG_APP_RST)
	    g_dwExtCmdFlg = 0;
}


//TSem   g_semMtrPnMap;
void SemInit()
{
	//g_semRemoteIf  = NewSemaphore(1, 1);
	g_semTask     = NewSemaphore(1, 1);
	//g_semMtrPnMap = NewSemaphore(1, 1);
}

//����:���ļ��г�ʼ�������ݴ����,�����Ч,��ʹ��Ĭ��
void InitPoweroffTmp()
{
	TPowerOffTmp* pDefTmp;
	bool fResult;
	BYTE bData[2];
	g_PowerOffTmp.fTmpValid = false;

    fResult = ReadPwrOffTmp(&g_PowerOffTmp, sizeof(g_PowerOffTmp));

	if (!fResult || g_PowerOffTmp.bVer!=PWROFF_VER || !g_PowerOffTmp.fTmpValid)
	{
		DTRACE(DB_GLOBAL, ("InitPoweroffTmp : use default power off tmp.\r\n"));
		memcpy(&g_PowerOffTmp, &g_DefaultPowerOffTmp, sizeof(g_PowerOffTmp));
	}
	if(g_PowerOffTmp.bRemoteDownIP[0] != 0 || g_PowerOffTmp.bRemoteDownIP[1] != 0 || 
	   g_PowerOffTmp.bRemoteDownIP[2] != 0 || g_PowerOffTmp.bRemoteDownIP[3] != 0)
	{
		//g_fDownSoft = true;
		g_PowerOffTmp.bRemoteDownIP[0] = 0;
		g_PowerOffTmp.bRemoteDownIP[1] = 0;
		g_PowerOffTmp.bRemoteDownIP[2] = 0;
		g_PowerOffTmp.bRemoteDownIP[3] = 0;
	}

//g_fDownSoft = true;    
    if(g_PowerOffTmp.bRemoteDownIP[4] == 0xff && g_PowerOffTmp.bRemoteDownIP[5] == 0xff && 
	   g_PowerOffTmp.bRemoteDownIP[6] == 0xff )
	{
		g_PowerOffTmp.bRemoteDownIP[4] = 0x00;
		g_PowerOffTmp.bRemoteDownIP[5] = 0x00;
		g_PowerOffTmp.bRemoteDownIP[6] = 0x00;
        //ClearTdbFile();
        //TdbUnLock();		
		//g_fDownSoft = true;
	}

	memcpy(g_bEsamTxRxBuf, &g_DefaultPowerOffTmp, sizeof(g_DefaultPowerOffTmp));
	pDefTmp = (TPowerOffTmp* )g_bEsamTxRxBuf;

	if (IsInvalidTime(&g_PowerOffTmp.tPowerOn))
		GetCurTime(&g_PowerOffTmp.tPowerOn);

	//��λ����
	g_PowerOffTmp.wRstNum++;	
	pDefTmp->wRstNum = g_PowerOffTmp.wRstNum;
	DWORDToBCD(g_PowerOffTmp.wRstNum, bData, 2);
	//WriteItemEx(BANK2, PN0, 0x1021, (BYTE *)&g_PowerOffTmp.wRstNum);	//0x1021 2 ��λ����,HEX
	WriteItemDw(BN0, PN0, 0xE1800016, bData);

	//�̼߳�ظ�λ����
	pDefTmp->wMonitorRstNum = g_PowerOffTmp.wMonitorRstNum;	
	memcpy(pDefTmp->szMonitorRstThrd, g_PowerOffTmp.szMonitorRstThrd, sizeof(g_PowerOffTmp.szMonitorRstThrd));
	WriteItemEx(BANK2, PN0, 0x1029, (BYTE *)&g_PowerOffTmp.wMonitorRstNum); //0x1029 2 �̼߳�ظ�λ����,HEX
	WriteItemEx(BANK2, PN0, 0x102a, (BYTE* )g_PowerOffTmp.szMonitorRstThrd); 	//0x102a 1 �̼߳�ظ�λID,HEX
	
	//pDefTmp->fAlrPowerOff = g_PowerOffTmp.fAlrPowerOff;		//��ͣ��澯��־������ͨ��Э���и澯��ȷ�ϲ���
    if(((GetCurMinute()/15*15) - (TimeToMinutes(&g_PowerOffTmp.tTermStat.tmLastRun)/15*15)) >= 15)
    {//���ͣ�絽�ϵ�ʱ���ͬһ��15���Ӽ�������ԭ������
        memset(g_PowerOffTmp.bAcAvrVolt, 0xee, 30);
    }

    WritePwrOffTmp(pDefTmp, sizeof(g_DefaultPowerOffTmp));
}

//����:��������ݴ����
bool SavePoweroffTmp(bool fSaveAc)
{
	if(GetAcPn()>0 && fSaveAc)
	{//����ǰ��һ��
		TTime tmNow;
		DWORD dwMin;
		BYTE bData[36];
		GetCurTime(&tmNow);
		dwMin = TimeToMinutes(&tmNow);
		dwMin = dwMin/15*15;
		MinutesToTime(dwMin, &tmNow);
		TimeToFmt15(&tmNow, bData);
		bData[5] = GetAcPn() & 0xff;
		//DTRACE(0, ("PoweroffTmp: Save Ac U.....\r\n")); 
		memcpy(bData+6, g_PowerOffTmp.bAcAvrVolt, sizeof(g_PowerOffTmp.bAcAvrVolt));
		PipeAppend(TYPE_COMM_TASK, 89, bData, sizeof(bData));
	}
	WritePwrOffTmp(&g_PowerOffTmp, sizeof(g_PowerOffTmp));
	return true;
}

//����:��������ݴ�����Ĳ�����ʼ����Ϣ
bool SavePwroffTmpVerChg()
{
	TSoftVerChg ParaInit;	//������ʼ���¼�
	memcpy((BYTE*)&ParaInit, (BYTE*)&g_PowerOffTmp.ParaInit, sizeof(TSoftVerChg));
	memcpy((BYTE*)&g_PowerOffTmp, 0x00, sizeof(g_PowerOffTmp));
	memcpy((BYTE*)&g_PowerOffTmp.ParaInit, (BYTE*)&ParaInit, sizeof(TSoftVerChg));
	WritePwrOffTmp(&g_PowerOffTmp, sizeof(g_PowerOffTmp));
	return true;
}

bool IsPowerOff()
{    
    //return false;
#ifndef SYS_WIN
    #ifdef AC_DETECT_POWER_OFF
	bool fPowerOff= false;
	DWORD dwVolt;
    BYTE bBuf[4];
    memset(bBuf, 0, sizeof(bBuf));
	int nRead = ReadItemEx(BN2, PN0, 0x1031, bBuf);
	if (nRead <= 0 || GetClick() < 30)
		return false;
    
    dwVolt = BcdToDWORD(bBuf, sizeof(bBuf));
    
	if (dwVolt < 200)
    {
		fPowerOff = true;
        //DTRACE(0, ("PowerOff\n"));
    }
	else
		fPowerOff = false;

	return fPowerOff;
 	//return IsPwrOff();
    #else
    return AcPowerOff();
    #endif
#else
	return false;
#endif
}

//ȡ�ÿ���ģʽ
int GetLedCtrlMode(BYTE bID)
{
	if (bID >= MAX_LED_NUM)
		return -1;

	return g_bLedCtrlMode[bID];	//ע���ʼ״̬
}

//���ÿ���ģʽ[ ����:ON_MODE, ����:OFF_MODE, ��˸:TOGGLE_MODE ]
bool SetLedCtrlMode(BYTE bID, BYTE bCtrlMode)
{		
	if (bID >= MAX_LED_NUM)
		return false;

	g_bLedCtrlMode[bID] = bCtrlMode;
	return true;
}

void InitAllLed()
{
    WORD i;    
    SetLedCtrlMode(LED_RUN, LED_MODE_OFF);//K5û�����е�
    SetLedCtrlMode(LED_ONLINE, LED_MODE_OFF);//���ߵ�
    SetLedCtrlMode(LED_ALARM, LED_MODE_OFF);//�澯��
    SetLedCtrlMode(LED_SIGNAL1G, LED_MODE_OFF);//�źŵ� �ź�ǿ�����
    SetLedCtrlMode(LED_SIGNAL2R, LED_MODE_OFF);//�źŵ� �ź�ǿ������
	SetLedCtrlMode(LED_LOCAL_RX, LED_MODE_OFF);//���ؽ��յ� 
	SetLedCtrlMode(LED_LOCAL_TX, LED_MODE_OFF);//���ط��͵� 
	SetLedCtrlMode(LED_REMOTE_RX, LED_MODE_OFF);//Զ�̽��յ� 
	SetLedCtrlMode(LED_REMOTE_TX, LED_MODE_OFF);//Զ�̷��͵� 

    
    for (i=0; i<MAX_LED_NUM; i++)
    {
        SetLed(false, i);	//���״̬
        g_wLedBurstCnt[i] = 0;
    }
}

//����һ��
void DoLedBurst(BYTE bID)
{
    if (bID < MAX_LED_NUM)
    {
        SetLedCtrlMode(bID, LED_MODE_BURST);
        g_wLedBurstCnt[bID] = 1;
        SetLed(true, bID);        
    }
}

void CheckSignStrength()
{
    //BYTE bConnState = 0;
    static BYTE bLastSign = 0;
    BYTE bSignStren = GetSignStrength();
//	if (IsOnlineState(&g_ifGprs))    //����״̬�������ź�ǿ��
//        return;
//   ReadItemEx(BN5, PN0, 0x5002, &bConnState);
//  if (bConnState != 0)
//       return;
    
    if (bSignStren != bLastSign)
    {
        bLastSign = bSignStren;
        if (bSignStren >= 100)//TD��ģ���ź�ǿ�ȷ�ΧΪ100-199
        {
            bSignStren -= 100;		
            if (bSignStren != 99)
            {
                bSignStren = (bSignStren)*31/98 + 10;
                if (bSignStren > 31)
                    bSignStren = 31;
            }
        }	
        if (bSignStren > 31)
            bSignStren = 0;	
        //WriteItemEx(BN5, PN0, 0x5001, (&bSignStren));

        #ifndef SYS_WIN
        if (bSignStren == 0)  //0ȫ�������Ϊ�ε�ģ�飬������̫��ʱʹ��
        {
            SetLedCtrlMode(LED_SIGNAL2R, LED_MODE_OFF);
            SetLedCtrlMode(LED_SIGNAL1G, LED_MODE_OFF);
        }
        else if (bSignStren < 8)
        {
            SetLedCtrlMode(LED_SIGNAL2R, LED_MODE_ON);
            SetLedCtrlMode(LED_SIGNAL1G, LED_MODE_OFF);
        }
        else if ((bSignStren >= 8) && (bSignStren < 16))
        {
            SetLedCtrlMode(LED_SIGNAL2R, LED_MODE_ON);
            SetLedCtrlMode(LED_SIGNAL1G, LED_MODE_ON);
        }
        else
        {
            SetLedCtrlMode(LED_SIGNAL2R, LED_MODE_OFF);
            SetLedCtrlMode(LED_SIGNAL1G, LED_MODE_ON);
        }
        #endif
    }
}

void DoAllLedCtrl()
{
  /*  WORD i;
    static BYTE bLed = LED_RMTCON;
    if (g_fRxLedTestCmd)
    {
        for (i=LED_RMTCON; i<MAX_LED_NUM; i++)
            SetLed(false, i);   //ȫ�ص�

        SetLed(true, bLed++);
        if (bLed >= MAX_LED_NUM)
            bLed = LED_RMTCON;
        return;
    }
    */
	//DoLedCtrl(LED_LOCAL);	//���غ�ƿ���
	//DoLedCtrl(LED_RMTDAT);	//Զ���̵ƿ���
  	//if (GetLedCtrlMode(LED_RMTDAT) == LED_MODE_OFF)	//Զ���̵���ʱ��ˢ�º��
    	//DoLedCtrl(LED_RMTCON);	//Զ�̺�ƿ���
	DoLedCtrl(LED_ONLINE);
	DoLedCtrl(LED_SIGNAL1G);
	DoLedCtrl(LED_SIGNAL2R);
	DoLedCtrl(LED_LOCAL_TX);
	DoLedCtrl(LED_LOCAL_RX);
	DoLedCtrl(LED_REMOTE_TX);
	DoLedCtrl(LED_REMOTE_RX);
}

//���ݿ���ģʽ ���еƿ���
void DoLedCtrl(BYTE bID)
{
	int nCtrlMode = GetLedCtrlMode(bID);
	if (nCtrlMode < 0)
		return;

	if (nCtrlMode == LED_MODE_TOGGLE)	//��˸ģʽ	
	{		
		#ifndef SYS_WIN
        ToggleLed(bID);
		#endif
	}
    else if (nCtrlMode == LED_MODE_BURST)	//����ģʽ	
    {
        if (g_wLedBurstCnt[bID] > 0)
        {
            #ifndef SYS_WIN
            ToggleLed(bID);
            #endif
            if (GetLed(bID))    //����һ�μ�1
                g_wLedBurstCnt[bID]--;
        }
        else
        {        
            #ifndef SYS_WIN
            SetLed(false, bID);	//���״̬		
            #endif
            SetLedCtrlMode(bID, LED_MODE_OFF);  //�ָ��ر�ģʽ
        }
    }
	else	//��������ģʽ
	{
        #ifndef SYS_WIN
        SetLed((nCtrlMode==LED_MODE_ON), bID);	//���״̬		
        #endif
	}
}

//ͬ����������
void SyncGPRSPara()
{
    BYTE bModemType=0;
    BYTE bBuf[125];
	ReadItemEx(BN0,PN0,0x888f,bBuf);//���120�ֽ�
	if (0x02 == bBuf[8])//GPRS����,���ñ���ģ���ͺ�	//20131225-2
	{
		ReadItemEx(BN10, PN0, 0xa1c5, &bBuf[124]);
		if (8 == bBuf[124])//������ݵ�����̫��ģʽ,�޸�ΪĬ�ϵ�M590E��ģ��
			bBuf[124] = 22;
        
   		ReadItemEx(BN1,PN0,0x2012,&bModemType);
        if (bModemType != bBuf[124])
    		WriteItemEx(BN1,PN0,0x2012,&bBuf[124]);
	}
	else if (0x04 == bBuf[8])//��̫��
	{
		bBuf[124] = 8;
		WriteItemEx(BN1,PN0,0x2012,&bBuf[124]);
	}

	WriteItemEx(BN0, PN0, 0x801f, bBuf);
    
	ReadItemEx(BN0, PN0, 0x889f, bBuf);
	WriteItemEx(BN0, PN0, 0x802f, bBuf);
}

void DpStatAc()
{
	//static DWORD dwVolt = 0;
	//DWORD  dwCurMin, dwOldMin;
	//BYTE bData[36];
	//static BYTE bCount = 0;
	//static TTime tmNow;
	TTime now;
	GetCurTime(&now);

	/////////////////��ʼ��////////////////////////////////////////
	//memset(bData, 0x00, sizeof(bData));
	//if (IsTimeEmpty(&tmNow))	//��һ������
	//{
	//	tmNow = now;	//�������һ�ε�����ʱ��
	//}
    ///////////////////////////////////////////////////////////////

	//if((now.nMinute!=tmNow.nMinute) || (now.nHour!=tmNow.nHour) || (now.nDay!=tmNow.nDay)
	//	||(now.nYear!=tmNow.nYear))
	//{
	  //���ʱ��
	//	if (bCount > 20) //��һ�������ۼƵ���û����ƽ��ֵ
	//	{
	//		//��ÿ��ȡ��ʱ�������������⣬Ϊ�ˣ��ڴ˷���
			//DTRACE(0, ("####Err: %dsecond %d\r\n",tmNow.nSecond , dwVolt)); 
	//		dwVolt = dwVolt/bCount;
	//		DWORDToBCD(dwVolt, bData, 2);
	//		memcpy(g_PowerOffTmp.bAcAvrVolt+tmNow.nMinute%15*2,bData,2);
	//		WriteItemEx(BN2, PN0, 0x1032, bData);//һ���ӵ�ѹƽ��ֵ
	//		dwVolt = 0;
			DoMgrDataStat(&now);
	//	}
	//	dwOldMin = TimeToMinutes(&tmNow);
	//	dwCurMin = TimeToMinutes(&now);
	//	dwOldMin = (dwOldMin-15)/15*15;
	//	dwCurMin = (dwCurMin-15)/15*15;
	//	if((GetAcPn() != PN0) && ((now.nMinute%15 == 0)||
	//		                       ((dwCurMin-dwOldMin)>=15)))//��Խ���
	//	{
	//		if((dwCurMin-dwOldMin) >= 15)
	//			dwCurMin = dwOldMin +15;
	//		MinutesToTime(dwCurMin, &tmNow);
	//		TimeToFmt15(&tmNow, bData);
	//		bData[5] = GetAcPn() & 0xff;
	//		memcpy(bData+6, g_PowerOffTmp.bAcAvrVolt, 30);
	//		PipeAppend(TYPE_COMM_TASK, 89, bData, 36);
	//		memset(g_PowerOffTmp.bAcAvrVolt, 0xee, sizeof(g_PowerOffTmp.bAcAvrVolt));
	//	}
		//DoMgrDataStat();
	//	bCount = 0;
	//}
	//tmNow = now;
	//ReadItemEx(BN2, PN0, 0x1031,bData);
	//if(/*(tmNow.nSecond<=15) && */(bCount==0))
	//{
		//dwVolt = ByteToDWORD(bData, 3);
   //     dwVolt = BcdToDWORD(bData, 3);
	//	bCount++;
		//DTRACE(0, ("%dsecond %d\r\n",tmNow.nSecond , dwVolt)); //У׼��ֵ
	//}
	//else if((tmNow.nSecond == 59)&&(bCount != 0))
	//{
		//dwVolt += ByteToDWORD(bData, 3);
  //      dwVolt += BcdToDWORD(bData, 3);
	//	bCount++;
	//	dwVolt = dwVolt / bCount;
		//DTRACE(0, ("%dsecond1 %d\r\n",tmNow.nSecond , dwVolt)); //У׼��ֵ
//		DWORDToBCD(dwVolt, bData, 2);
//		memcpy(g_PowerOffTmp.bAcAvrVolt+tmNow.nMinute%15*2,bData,2);
//		WriteItemEx(BN2, PN0, 0x1032, bData);//һ���ӵ�ѹƽ��ֵ
		//WriteItemEx(BN0, GetAcPn(), 0xb611, bData);//һ���ӵ�ѹƽ��ֵ
//		bCount = 0;
//		dwVolt = 0;
//		DoMgrDataStat(&tmNow);
//	}
//	else if(bCount != 0)
//	{
		//DTRACE(0, ("%dsecond1 %d\r\n",tmNow.nSecond , dwVolt)); //У׼��ֵ
		//dwVolt += ByteToDWORD(bData, 3);
  //      dwVolt += BcdToDWORD(bData, 3);
	//	bCount++;
	//}
	//else if(bCount==0 && tmNow.nSecond>15)
	//{
	  //DTRACE(0, ("Tm = %d:%d:%d   bCount = %d\r\n",tmNow.nHour, tmNow.nMinute, tmNow.nSecond , bCount)); 
	//}
	return;
}

//static bool g_fYxInit = false;
//static BYTE g_bYxVal = 0;

//����������һ������F9ң�ŵ�״̬��
/*
void DoYX()
{
	BYTE bYxVal = 0; //ң��״̬��־
    BYTE bChgFlg;	 //״̬��λ��־
	BYTE bBuf[10];
    TYxPara YxPara;
    TYxPara* pYxPara = &YxPara;
    static BYTE bLastYxVal = 0;    
	WORD wYXInput, wVal;
	int iLen;

#ifndef SYS_WIN
	if (IsPowerOff())
	{
		return;
	}
#endif
    
    memset(bBuf, 0, sizeof(bBuf));
    iLen = ReadItemEx(BN0, PN0, 0x00cf, bBuf);	//C4F12
	if (iLen > 0)
	{	
	    DTRACE(DB_FA, ("YXLoadPara : bBuf[0]=%x, bBuf[1] =%x.\n", bBuf[0], bBuf[1]));
		pYxPara->wYxFlag = bBuf[0] & 0x1;

        bBuf[1] = bBuf[1]&0x01;
		pYxPara->wYxPolar = ~bBuf[1];
		DTRACE(DB_FA, ("YXLoadPara : pYxPara->wYxFlag=%x, pYxPara->wYxPolar =%x.\n", pYxPara->wYxFlag, pYxPara->wYxPolar));
	}
    else
    {
        pYxPara->wYxFlag = 0;           //Ĭ�ϲ�����
		pYxPara->wYxPolar = 0x1;		//Ĭ��a�ʹ���
    }

    if (pYxPara->wYxFlag == 0)
        return;

    wYXInput = GetYxInput(); //Ӳ����YX����
    wVal = (wYXInput^pYxPara->wYxPolar) & 0x1;
    //WORD wYxVal = (wYxVal & (~0x1)) | wVal;
    bYxVal = (BYTE ) wVal;
    if (bLastYxVal != bYxVal)
    {
        bLastYxVal = bYxVal;
        WriteItemEx(BN2, PN0, 0x1100, &bYxVal);
    }    

	bBuf[0] = bYxVal;
    if (!g_fYxInit)
    {//��һ�ν�����ȡң��״̬λ�����жϣ�
        g_bYxVal = bYxVal;
        g_fYxInit = true;
		WriteItemEx(BN0, PN0, 0x108f, bBuf);  //�洢ң��״̬����
        return;
    }
 
	bChgFlg = g_bYxVal ^ bYxVal;
    g_bYxVal = bYxVal;

    if (bChgFlg != 0)
    {
	   bBuf[1] = bChgFlg;
	   WriteItemEx(BN0, PN0, 0x108f, bBuf);  //�洢ң��״̬������λ��־��һ������F9)��
	}
}*/

//#pragma warning(disable:111)
WORD g_wTcpipRunCnt;                      //todo:���TCPIP�̣߳������������˵�����߳������ˡ�

TThreadRet WdgThread(void* pvPara)
{
	int iMonitorID;
    WORD i=0;
	bool fForbitWDG = false;
    InitAllLed();
    while (1)
    {
		if (!fForbitWDG)
			ClearWDG();

        Sleep(100);

		if (GetInfo(INFO_STOP_FEED_WDG))
			fForbitWDG = true;

	#ifndef SYS_WIN
        PowerOffProtect();
	#endif

		i++;
		DoAllLedCtrl();
		if (i == 10)
		{			
            DoLedCtrl(LED_RUN);	//���еƿ���
			DoLedCtrl(LED_ALARM);
			
		    iMonitorID = DoThreadMonitor();	//�̼߳��,ÿ��ִ��һ��
			if (iMonitorID < 0)
			{
				iMonitorID = -iMonitorID-1;
				GetMonitorThreadName(iMonitorID, g_PowerOffTmp.szMonitorRstThrd); 
				g_PowerOffTmp.wMonitorRstNum++;
				DTRACE(DB_CRITICAL, ("WdgThread:reset %s.\r\n", g_PowerOffTmp.szMonitorRstThrd));
				SavePoweroffTmp(true);
                ResetCPU();
				while(1);
			}
			i = 0;
		}
    }
    
    //return THREAD_RET_OK;
}

//���֧�ֵ�ID���ն�ҲҪ��Ӧ��Ч���ݵ�ID
bool IsNorSuportId(WORD wID)
{
	if (wID>=0x2011 && wID<=0x2016)
		return true;
	if (wID>=0x2021 && wID<=0x2023)
		return true;
	if (wID>=0x2031 && wID<=0x203f)
		return true;

	return false;
}

//�Ƿ�Ϊ����ģʽ
bool IsDownSoft()
{
    return g_fDownSoft;
}

bool IsMtr485Port(BYTE bPort)
{
	if (bPort>=LOGIC_PORT_MIN && bPort<LOGIC_PORT_MIN+LOGIC_PORT_NUM)
		return true;

	return false;
}

void LoadInMtrCommPara(TCommPara* pCommPara)
{
	pCommPara->wPort = LOGIC_PORT_MIN;
	pCommPara->dwBaudRate = INMTR_BAUD;
	pCommPara->bByteSize = 8;
	pCommPara->bStopBits = ONESTOPBIT;
	pCommPara->bParity = EVENPARITY;
}

#ifdef EN_PROEX
static const TModuleParaDesc g_ModuleParaDesc[] = {
//	wBn				wID					wLen		wOffset		bFn
	{BN1,			0x2012,				1,			0,			0,      PN0},		//ģ���ͺ�
	{BN0,			0x003f,				28,			1,			3,      PN0},		//��վIP����IPAPN		F3
	{BN0,			0x004f,				16,			29,			4,      PN0},		//�������ĺ���			F4
	{BN10,			0xA040,				2,			45,			0,      PN0},		//��������				
	{BN10,			0xA041,				2,			47,			0,      PN0},		//�ն˵�ַ
	{BN0,			0x006f,				16,			49,			6,      PN0},		//���ַ				F6
	{BN0,			0x008f,				8,			65,			8,      PN0},		//����ͨѶ��ʽ			F8
	{BN0,			0x005f,				3,			73,			5,      PN0},		//�����㷨				F5
	{BN0,			0x024f,				4,			76,			36,     PN0},	    //��������				F36
	{BN0,			0x001f,				6,			80,			1,      PN0},	    //�ն�ͨ�Ų���			F1
	{BN0,			0x010f,				64,			86,         16,     PN0},		//ppp�û���������
	{BN10,			0xa1b3,				12,			150,        0,      PN0},		//�豸��
	{BN0,			0x019f,				11,			162,        25,     PN1},		//������1�ڱ���߷�ʽ����PTCT		F25
	//{BN0,			0x021f,				110,		},					//������				F33	
};

const TModuleParaDesc* GetParaDesc(int nIdx)
{
	if (nIdx < sizeof(g_ModuleParaDesc)/sizeof(TModuleParaDesc))
		return &g_ModuleParaDesc[nIdx];
	else
		return NULL;
}

int GetModuleParaIndex(WORD wBn, WORD wID)
{
	WORD i;
	for (i=0; i<sizeof(g_ModuleParaDesc)/sizeof(TModuleParaDesc); i++)
	{
		if (wBn == g_ModuleParaDesc[i].wBn && wID == g_ModuleParaDesc[i].wID)
			return i;
	}

	return -1;
}


int GetIndexFromFn(BYTE bFn)
{
	WORD i;
	if (bFn != 0)
	{	
		for (i=0; i<sizeof(g_ModuleParaDesc)/sizeof(TModuleParaDesc); i++)
		{
			if (bFn == g_ModuleParaDesc[i].bFn)
				return i;
		}
	}

	return -1;
}


//nOffset:��ʼ��ַ
//wLen:����
WORD MakeModuleParaFrm(int nOffset, WORD wDataLen, BYTE* pbTx)
{
	WORD i;
	//DWORD dwNeedSendFlag = 0;
	BYTE bBuf[200];	//	
	BYTE* pbBuf = bBuf;

	*pbTx++ = nOffset&0xff;
	*pbTx++ = (nOffset>>8)&0xff;	//ƫ��,2bytes
	*pbTx++ = wDataLen;				//����,1byte
	if (wDataLen > Module_PARA_LEN || !wDataLen)
		return 0;

	for (i=0; i<sizeof(g_ModuleParaDesc)/sizeof(TModuleParaDesc); i++)
	{
		TModuleParaDesc* pDesc = (TModuleParaDesc*)&g_ModuleParaDesc[i];
		if (pDesc == NULL)
			continue;

		ReadItemEx(pDesc->wBn, pDesc->bPn, pDesc->wID, pbBuf);
		pbBuf += pDesc->wLen;
	}

	memcpy(pbTx, bBuf+nOffset, wDataLen);
	return wDataLen+3;
}

//���ڱ��ȡģ�����
bool UpdateModulePara(TPro645Ex* pProEx)	//��δ���³ɹ���ô����
{
	WORD i;
	int  nDataLen;
	BYTE bInValid[200];	//BUF���Ȳ���С��Module_PARA_LEN
	BYTE bInValidData[] = {0, 0xEE, 0xFF};
	bool  fInvalidPara = false;	//�ڱ�����Ƿ���Ч
	bool fSave = false;
	int nRet = -1;
	DWORD dwNeedSendFlag=0;
	BYTE bDataBuf[200];
	BYTE bBuf[300];
	BYTE* pbBuf = &bBuf[12];	//F003��������ʼ��ַ

	bBuf[0] = 0;
	bBuf[1] = 0;
	bBuf[2] = Module_PARA_LEN;	//�����ܳ���

	nRet = CallInMeterDat( pProEx, ADDTYPEID_READ, bBuf, 3);	//��ȡ�����ڱ��ģ�����
	if (nRet <= 0)
	{
		DTRACE(DB_FA, ("UpdateModulePara: fail to Read Inn Meter Module Para.\r\n"));
		return false;
	}

	for (i=0; i<sizeof(bInValidData); i++)
	{
		memset(bInValid, bInValidData[i], sizeof(bInValid));
		if (memcmp(pbBuf, bInValid, Module_PARA_LEN) == 0)
		{
			fInvalidPara = true;
			break;
		}
	}

	ReadItemEx(BN5, PN0, 0x5007, (BYTE* )&dwNeedSendFlag);
	memset(bInValid, 0xFF, sizeof(bInValid));   //Ŀǰ�������Ч����Ϊ0xFF

	fSave = false;
	if (fInvalidPara == false)
	{
		for (i=1; i<sizeof(g_ModuleParaDesc)/sizeof(TModuleParaDesc); i++)      //��1��ʼѭ��������ģ���ͺţ���ģ���ͺŲ������ӵ��ȡ 2011.05.27 zq modify
		{
			TModuleParaDesc* pDesc = (TModuleParaDesc*)&g_ModuleParaDesc[i];
			if (pDesc == NULL)
				continue;

			ReadItemEx(pDesc->wBn, pDesc->bPn, pDesc->wID, bDataBuf);	//2
			if (memcmp(pbBuf+pDesc->wOffset, bInValid, pDesc->wLen) == 0)   //���ڴ洢�ĸò���Ϊ��Ч����0xff
			{
				fInvalidPara = true;
			}
			else
			{
				if (memcmp(pbBuf+pDesc->wOffset, bDataBuf, pDesc->wLen) != 0)	//��ͬ
				{
					if ((!(dwNeedSendFlag & (1<<i))) && (pbBuf[pDesc->wOffset] != 0xee))	//�����ѱ��ڱ�ȷ��
					{
						fSave = true;
						WriteItemEx(pDesc->wBn, PN0, pDesc->wID, pbBuf+pDesc->wOffset);
					}
				}
				else	//��ͬ
				{
					if (dwNeedSendFlag & (1<<i))
						dwNeedSendFlag &= ~(0x1<<i);
				}
			}
		}

		if (fSave)
		{
			DTRACE(DB_FA, ("UpdateModulePara: Get&Save Module Para from Inn Meter.\r\n"));
			WriteItemEx(BN5, PN0, 0x5007, (BYTE* )&dwNeedSendFlag);		
			TrigerSavePara();
			//TrigerSavePara();???
		}
	}

	if (fInvalidPara == true)		//���ڲ�����Ч����ģ�����д����ڡ�
	{
		nDataLen = MakeModuleParaFrm(0, Module_PARA_LEN, pbBuf);
		CallInMeterDat( pProEx, ADDTYPEID_WRITE, pbBuf, nDataLen);	//ģ�����д���ڱ�
		DTRACE(DB_FA, ("UpdateModulePara: Inn Meter Para Invalid, Set Module Para into inn meter!\r\n"));
	}

	DTRACE(DB_FA, ("UpdateModulePara: Update Inn Meter Para successfully!\r\n"));
	return true;
}

void InitProtoEx()
{
	WORD i;
	LoadInMtrCommPara(&g_CommPara);
	ProExInit(&g_ProEx, &g_CommPara);
	CommOpen(g_CommPara.wPort, g_CommPara.dwBaudRate, g_CommPara.bByteSize, g_CommPara.bStopBits, g_CommPara.bParity);		//���ڱ��

	for (i=0; i<2; i++)
	{
		if (UpdateModulePara(&g_ProEx))
			break;
	}
    
    if (ReadInMtrTime(&g_ProEx) == false)   
       	InitSysClock();	 ////�ӻ����ȡʱ��ʧ�ܣ����ϵͳ���ݿ�ȡʱ��

	CommClose(g_CommPara.wPort);
}

#endif

void FaInitStep1()
{
//	BYTE bA165Val;
	BYTE bMode = 0;
	
    ClearWDG();
	DTRACE(DB_FA, ("\r\n\r\n/**********************************************/\r\n"));
	DTRACE(DB_FA, ("/***           "FA_NAME" www.szclou.com     ***/\r\n")); 
    DTRACE(DB_FA, ("/**********************************************/\r\n\r\n"));
    ClearWDG();
	SemInit();
	SyncTimer(); //�����߳�����ǰ��ʼ��

//	InitInfo();

	ClearWDG();
	TdbInit();	//����ֻ�漰���ײ��ļ�ϵͳ���ź����ĳ�ʼ������û�õ�ϵͳ��

    ClearWDG();
	InitDB(); //�汾����¼��õ������	
    ClearWDG();
	InitPoweroffTmp();	//��ʼ���������

	ClearWDG();
	g_fUpdFatOk = UpdFat();	//����Flash��̬���䣬һ��Ҫ����InitDB֮��

	
	ReadItemEx(BN10, PN0, 0xa1a8,  &bMode);
	if(bMode == 1)
	{
		g_fTermTestMode = true;
	}
	else
	{
		g_fTermTestMode = false;
	}
	InitTestMode();
    /*if (IsDownSoft())
    {
        CommClose(COMM_DEBUG);  //COMM_TEST
    	CommOpen(COMM_DEBUG, CBR_19200, 8, ONESTOPBIT, NOPARITY);		//COMM_TESTά���ں�Debug�ڹ���
    }*/

	InitDebug();   //������Ϣ��������ݿ��ʼ����Ž���,��Ϊ�õ������ݿ�

#ifndef SYS_WIN
//	ResetESAM();
//	EsamReset();
	EsamInit();
#endif

#ifndef SYS_WIN
	/*BYTE bBuf[64];
    BYTE bNetContTye;
	ReadItemEx(BN0, PN0, 0x007f, bBuf);
    ReadItemEx(BN10, PN0, 0xa1b6, &bNetContTye); //��̫�����ӷ�ʽ
    bBuf[12] = bNetContTye;   //0-11��IP,���룬����
	InitTCPIP((void* )bBuf);*/
    InitTCPIP(NULL);
#endif

    InitTermExc(&g_TermAlrCtrl);
    //InitProtoEx();
    g_fFaInitOK = true;   
#ifndef SYS_WIN
    UpdatBL();
#endif
}

//����1�ʼ�¼-->ֱ��дFLASH�����¼
bool PipeAppend(BYTE bType, BYTE bFn, BYTE* pbData, WORD wDataLen)
{
	char str[20];
	if (bType == TYPE_FRZ_TASK)
	{	
		if (TdbOpenTable(bFn) == TDB_ERR_OK)
		{
			//#ifdef SYS_WIN
			if (IsDebugOn(DB_TASK))
			{
				char szBuf[64];
				sprintf(szBuf, "SaveRec:bFn=%d,bType=%s,Len=%d,Time:",bFn, TaskTypeToStr(bType, str), wDataLen);
				if((11 <= bFn) && (bFn <= 22))  //�����ݵ�ʱ����һ���ֽ�
				{
					TraceBuf(DB_TASK, szBuf, pbData, 2);
					memset(szBuf, 0, sizeof(szBuf));
					sprintf(szBuf, "Pn and Data:");
					TraceBuf(DB_TASK, szBuf, pbData+4, wDataLen-2);
				}
				else
				{
					TraceBuf(DB_TASK, szBuf, pbData, 4);
					memset(szBuf, 0, sizeof(szBuf));
					sprintf(szBuf, "Pn and Data:");
					TraceBuf(DB_TASK, szBuf, pbData+4, wDataLen-3);
				}
			}
			//#endif

			TdbAppendRec(bFn, pbData, wDataLen);			
		}
		TdbCloseTable(bFn);
	}
	else if (bType == TYPE_COMM_TASK)
	{	
		if (TdbOpenTable(bFn + FN_COMSTAT) == TDB_ERR_OK)
		{
			//#ifdef SYS_WIN
			if (IsDebugOn(DB_TASK))
			{
				//char szBuf[64];
				//sprintf(szBuf, "SavePipeRec: bFn=%d, bType=%s, bRecLen=%d, append rec --> ",bFn, TaskTypeToStr(bType, str), wDataLen);
				//TraceBuf(DB_TASK, szBuf, pbData, wDataLen);
			}
			//#endif

			TdbAppendRec(bFn + FN_COMSTAT, pbData, wDataLen);			
		}
		TdbCloseTable(bFn + FN_COMSTAT);
	}
	else if (bType == TYPE_FWD_TASK)
	{	
		if (TdbOpenTable(bFn + FN_FWDSTAT) == TDB_ERR_OK)
		{
			//#ifdef SYS_WIN
			if (IsDebugOn(DB_TASK))
			{
				//char szBuf[64];
				//sprintf(szBuf, "SavePipeRec: bFn=%d, bType=%s, bRecLen=%d, append rec --> ",bFn, TaskTypeToStr(bType, str), wDataLen);
				//TraceBuf(DB_TASK, szBuf, pbData, wDataLen);
			}
			//#endif

			TdbAppendRec(bFn + FN_FWDSTAT, pbData, wDataLen);			
		}
		TdbCloseTable(bFn + FN_FWDSTAT);
	}
	else if (bType == TYPE_ALR_EVENT)
	{
		//#ifdef SYS_WIN
		if (IsDebugOn(DB_TASK))
		{
			//char szBuf[64];
			//sprintf(szBuf, "SavePipeRec: bErc=%d, bType=%d, bRecLen=%d, append rec --> ",bFn, bType, wDataLen);
			//TraceBuf(DB_TASK, szBuf, pbData, wDataLen);
		}
		//#endif

// 		WriteAlrRec(bType, pbData, wDataLen);
	}

	return true;
}


//���������ϵͳ�������ڲ�Flash�ĳ�����վ����������������������
//������NONE
//���أ���ȷ�������1�����򷵻���Ӧ������
void FaResetPara()
{
	WORD i;
	
	LockDB();
	ClearWDG();
	for (i=0; i<SECT_NUM; i++)
	{
		if (i==SECT_KEEP_PARA) //���������ļ��ͽ��ɼ������ݳ���
			continue;

		DbClearBankData(BN0, i);
	}
}

void FaResetAllPara()
{
//#ifndef SYS_WIN
//	ClrAllPara();
//#endif
    
    WORD i;
	
	LockDB();
	ClearWDG();
	for (i=0; i<SECT_NUM; i++)
	{
		DbClearBankData(BN0, i);
	}
}

void FaResetData()
{
    BYTE bOrder = 0;
	WORD wLastAlarmNo=0;
	ReadItemEx(BN24, PN0, 0x5025, &bOrder);
	bOrder++;
	WriteItemEx(BN24, PN0, 0x5025, &bOrder); //�ı��־���´λ������

	WriteItemEx(BN24, PN0, 0x5030, (BYTE* )&wLastAlarmNo);		//������ζ��澯ָ��

	LockDB();
	ClearWDG();

	//ExFlashEraseChip();//��Ҫ��������Ĳ����������FAT�Ϳ����ˡ��������̫��̨�����ȥ
    ClrAllData();
	ResetTaskInfo();
    //DTRACE(DB_CRITICAL, ("FaResetData ok!\r\n"));
}

void FaResetExPara()
{
	LockDB();
	ClearWDG();
 
	DbClearBankData(BN1, 0);
    DbClearBankData(BN10, 0);
    DbClearBankData(BN24, 0);
}

//�����߳� ��ջ��С256
TThreadRet SlowSecondThread(void* pvPara)
{
#ifndef SYS_WIN
    YXInit(&g_tYx);
#endif
	while(1)
	{
		if (GetInfo(INFO_APP_RST))	//CPU��λ
		{
			DTRACE(DB_CRITICAL, ("SlowSecondThread : rx INFO_APP_RST......\r\n"));
            StopMtrRd(0);
			ResetCPU();
		}
		
		if (GetInfo(INFO_RST_DATA)) //���ݸ�λ
		{
			DTRACE(DB_CRITICAL, ("SlowSecondThread : rx INFO_RST_DATA at click=%d......\n", GetClick()));
            StopMtrRd(0);
			FaResetData();		//����������м�����
			//SavePoweroffTmp(false);
			Sleep(1000);
			ResetCPU();
		}
		
		if (GetInfo(INFO_RST_PARA)) //������λ
		{
			DTRACE(DB_CRITICAL, ("SlowSecondThread : rx INFO_RST_PARA......\n"));
            StopMtrRd(0);
			FaResetPara();		//ֻ������������صĲ������������ȫ�����
			FaResetData();		//����������м�����
			g_PowerOffTmp.wRstNum = 0;
			SavePoweroffTmp(false);
			Sleep(1000);
			ResetCPU();
		}

		if (GetInfo(INFO_RST_ALLPARA)) //���������ݸ�λ
		{
			DTRACE(DB_CRITICAL, ("SlowSecondThread : rx INFO_RST_ALLPARA......\n"));
            StopMtrRd(0);
			FaResetAllPara();	//���ȫ������
			FaResetData();		//����������м�����
			g_PowerOffTmp.wRstNum = 0;
			SavePoweroffTmp(false);
			Sleep(1000);
			ResetCPU();
		}
        
        DoFapCmd();

        if (g_fFaInitOK)
        {
            SyncTimer();
#ifndef SYS_WIN
            YXRun();
#endif
            DoTermExc(&g_TermAlrCtrl);
            CheckSignStrength();
        }

		Sleep(1000);
	}

	return THREAD_RET_OK;
}


TThreadRet TestThread(void* pvPara)
{
    BYTE bBuf[] = "ATE0\r\n";    
    WORD wLen = 0;       
    
    //CommOpen(COMM_TEST, CBR_1200, 8, ONESTOPBIT, EVENPARITY);
    //CommOpen(COMM_METER2, CBR_1200, 8, ONESTOPBIT, EVENPARITY);
    //CommOpen(COMM_METER, CBR_9600, 8, ONESTOPBIT, EVENPARITY);
	CommOpen(COMM_GPRS, CBR_9600, 8, ONESTOPBIT, EVENPARITY);
    while(1)
    {
    	CommWrite(COMM_GPRS, bBuf, sizeof(bBuf), 1000);        
        Sleep(1000); 
        /*wLen = CommRead(COMM_TEST, bBuf, sizeof(bBuf), 1000);
        if (wLen > 0)        
            CommWrite(COMM_TEST, bBuf, wLen, 1000);

        wLen = CommRead(COMM_METER2, bBuf, sizeof(bBuf), 1000);
        if (wLen > 0)        
            CommWrite(COMM_METER2, bBuf, wLen, 1000);*/

        //wLen = CommRead(COMM_METER, bBuf, sizeof(bBuf), 1000);
        //if (wLen > 0)
        //    CommWrite(COMM_METER, bBuf, wLen, 1000);
    }
}

#ifndef SYS_WIN
TThreadRet AcSampleTask(void *pvParameters)
{    
    LoadCalibCfg();
    InitAcSample();
	InitDcSample();
    ADStart();   
    
    while (1)
    {
        WaitSemaphore(g_semAcDone, 0);  //һ�������ɣ�ÿͨ��32����
        if (g_bJumpCyc != 0)  //�ӵ�һ���ܲ�,��Ϊ�����˲����������дFLASH����˲�����
        {
            g_bJumpCyc--; 
            continue;
        }
        memcpy(g_wAdBuf, g_tAdCtrl.pwData, sizeof(g_wAdBuf));   //BUFFER_SIZE*sizeof(WORD)     
        //CommWrite(COMM_LOCAL, (BYTE *)&g_tAdCtrl.tAdcSampleData[bDataReadyCh].wValue[i+j*NUM_CHANNELS], 2, 1);
                
        Calcu();
        if (g_fDcSample) 
			DoDCSample();
                
    }
}
#endif


void ResetPowerOffTmp()
{
  	//WORD wLocalPort = g_PowerOffTmp.wLocalPort; 
	TPowerOffTmp tPwrOffTmp;
	memcpy(&tPwrOffTmp, &g_PowerOffTmp, sizeof(g_PowerOffTmp));
	memcpy(&g_PowerOffTmp, &g_DefaultPowerOffTmp, sizeof(g_PowerOffTmp));
	g_PowerOffTmp.wRstNum = tPwrOffTmp.wRstNum;
	//�̼߳�ظ�λ����
	g_PowerOffTmp.wMonitorRstNum = tPwrOffTmp.wMonitorRstNum;	
	memcpy(g_PowerOffTmp.szMonitorRstThrd, tPwrOffTmp.szMonitorRstThrd, sizeof(g_PowerOffTmp.szMonitorRstThrd));
	//g_PowerOffTmp.wLocalPort = wLocalPort;
}

void InitTestMode()
{
	BYTE bMode = 0;
	DWORD  dwClick = 0;
	DWORD  dwClickLimit = 0;

	ReadItemEx(BN0, PN0, 0x344f, (BYTE*)&dwClick);
	ReadItemEx(BN1, PN0, 0x2120, (BYTE*)&dwClickLimit);
	if (dwClickLimit == 0)
	{
		dwClickLimit = 10*24*3600;
	}
	if (dwClick <= dwClickLimit)//�ڲ���ʱ����Ч����
	{
		bMode = 1;
	}
	else
	{
		bMode = 0;
	}
	WriteItemEx(BN2, PN0, 0x2040, &bMode);
}

void CountTestTm()
{
	BYTE bMode = 0;
	DWORD  dwClick = 0;
	DWORD  dwClickLimit = 0;
	static DWORD  dwLastClick = 0;

	if (GetClick() - dwLastClick < 5) //5��ִ��һ��
		return ;

	ReadItemEx(BN0, PN0, 0x344f, (BYTE*)&dwClick);

	ReadItemEx(BN1, PN0, 0x2120, (BYTE*)&dwClickLimit);
	if (dwClickLimit == 0)
	{
		dwClickLimit = 10*24*3600;
	}
	if (dwClick <= dwClickLimit)//�ڲ���ʱ����Ч����
	{
		bMode = 1;
		WriteItemEx(BN2, PN0, 0x2040, &bMode);
	}
	else
	{
		bMode = 0;
		WriteItemEx(BN2, PN0, 0x2040, &bMode);
		return ; //�����ۼ���
	}
	if (bMode == 1)
	{
		dwClick += GetClick() - dwLastClick;
		dwLastClick = GetClick();
		WriteItemEx(BN0, PN0, 0x344f, (BYTE*)&dwClick);
	}
}

TThreadRet MainThread(void)
{	
	int iMonitorID;
	DWORD dwTicks = 0;
	InitThreadMonitor(); //��ʼ���̼߳��
	iMonitorID = ReqThreadMonitorID("main-thrd", 90*60);	//�����̼߳��ID,���¼��Ϊ90����
        
	SysInit();
	DrvInit();

    NewThread("WDG", WdgThread, NULL, /*192*/224, THREAD_PRIORITY_ABOVE_NORMAL);
 //   Sleep(100);
    //InFlashFormat();
	//EraseAllInSpace();
    //ExFlashEraseChip();

	FaInitStep1();
    //NewThread("TEST", TestThread, NULL, 256, THREAD_PRIORITY_NORMAL);
    MtrCtrlInit();
	InitMeterPro();
	SyncGPRSPara();	//�������InitProto()ǰ,���ʼ����Ҫ�õ��ն˵�ַ����
	InitProto();
#ifdef EN_CCT	//����������
#ifndef SYS_WIN
	PlcReset();	//δ����
#endif
	DTRACE(DB_CRITICAL, ("MainThread : Reset Plc at %d!\n", GetClick()));
#endif
	
	DTRACE(DB_CRITICAL, ("MainThread : started V1.57!\n"));    
	NewProThread();

	//NewThread("SLOW", SlowSecondThread, NULL, 256, THREAD_PRIORITY_NORMAL);        //2048
    
    //NewThread("TEST", TestThread, NULL, 256, THREAD_PRIORITY_NORMAL);
#ifndef SYS_WIN
	//InitDcSample();
    NewThread("Ac", AcSampleTask, NULL, 256, THREAD_PRIORITY_ABOVE_NORMAL);	
#endif
   
#ifndef SYS_WIN
    YXInit(&g_tYx);
#endif

	NewMtrThread();

#ifdef EN_CCT	//����������
	InitCct();
	NewCctThread();
#endif

	//Sleep(1000);//�ý����߳�������
    
    while (1)
    {
        UpdThreadRunClick(iMonitorID);

        /*if (!IsDownSoft())
            MtrRdThread(NULL);*/

		Sleep(50); //��ǰ��SLEEP���ý����߳�������

		if (GetInfo(INFO_APP_RST))	//CPU��λ
		{
			DTRACE(DB_CRITICAL, ("SlowSecondThread : rx INFO_APP_RST......\r\n"));
			StopMtrRd(0);
			SavePoweroffTmp(true);
            Sleep(1000);
			ResetCPU();
		}
		
		if (GetInfo(INFO_RST_DATA)) //���ݸ�λ
		{
			DTRACE(DB_CRITICAL, ("SlowSecondThread : rx INFO_RST_DATA at click=%d......\n", GetClick()));
			StopMtrRd(0);
			FaResetData();		//����������м�����
			UnLockDB();
			ResetPowerOffTmp();
			SavePoweroffTmp(false);
   			Sleep(12*1000);     //���̨�������������
            SetInfo(INFO_TASK_PARA);    //g_fUpdFatOk = UpdFat();		//����Flash��̬���䣬һ��Ҫ����InitDB֮��
            Sleep(2000);
			//ResetCPU();
		}
		
		if (GetInfo(INFO_RST_PARA)) //������λ
		{
			DTRACE(DB_CRITICAL, ("SlowSecondThread : rx INFO_RST_PARA......\n"));
			StopMtrRd(0);
			FaResetPara();		//ֻ������������صĲ������������ȫ�����
			FaResetData();		//����������м�����
			ResetPowerOffTmp();
			SavePoweroffTmp(false);
			Sleep(1000);
			ResetCPU();
		}

		if (GetInfo(INFO_RST_ALLPARA)) //���������ݸ�λ
		{
			DTRACE(DB_CRITICAL, ("SlowSecondThread : rx INFO_RST_ALLPARA......\n"));
			StopMtrRd(0);
			FaResetAllPara();	//���ȫ������
			FaResetData();		//����������м�����
			ResetPowerOffTmp();
			SavePoweroffTmp(false);
			Sleep(1000);
			ResetCPU();
		}
        
        if (GetInfo(INFO_UPDATE_BOOTLOADER)) //Bootloader����
        {
            DTRACE(DB_CRITICAL, ("SlowSecondThread : Bootloader updating......\n"));
		#ifndef SYS_WIN
            ClearWDG();
			BootLoaderUpd();
            ClearWDG();
		#endif
            DTRACE(DB_CRITICAL, ("SlowSecondThread : Bootloader update is over!\r\n"));
		}
        
        DoFapCmd();
		DoBrcastMtrTm();
		CountTestTm();

        CheckNetStat();
       
        if (g_fFaInitOK)
        {
            SyncTimer();
		#ifndef SYS_WIN
			YXRun();
			//if (g_fDcSample)
				//DoDCSample();
		#endif
            CheckSignStrength();
        }

#ifndef SYS_WIN
        LocalThread();
		if(GetTick()-dwTicks >= 1000)
		{//һ������
			DpStatAc();
			DoTermExc(&g_TermAlrCtrl);
			dwTicks = GetTick();
		}
#else
		if(GetClick()-dwTicks >= 60)
		{//һ������
			DpStatAc();
			DoTermExc(&g_TermAlrCtrl);
			dwTicks = GetClick();
		}
#endif
        
		//Sleep(100);
    }

	return THREAD_RET_OK;
}

int GetMeterPortFunc(WORD wMeterPort)
{
	BYTE b = 0, b1 = 0, b2 = 0, b3 = 0,b4 = 0;
	int iRet = -1;
	ReadItemEx(BN0, POINT0, 0x8710, (BYTE *)&b);//�����
	ReadItemEx(BN0, POINT0, 0x8720, (BYTE *)&b1);//������
	ReadItemEx(BN0, POINT0, 0x8750, (BYTE *)&b2);//���Կ�

	//ReadItemEx(BANK7,POINT0, 0x7510, (BYTE *)&b3);//485˳�� //0-����  �ҳ���    1-�󳭱�  �Ҽ���

	if (b3 == 0) 
	{
		if(wMeterPort == 3)
		{
			b4 = b2;
		}
		else if (wMeterPort == 2)
		{
			b4 = b1;
		}
		else
		{
			b4 = b;
		}
	}
	else
	{
		if(wMeterPort == 3)
		{
			b4 = b;
		}
		else if (wMeterPort == 2)
		{
			b4 = b1;
		}
		else
		{
			b4 = b2;
		}
	}


	return b4;
}

//����:�ж��߼����Ƿ�Ϊ485�ɼ�����
//����:��485�ɼ����ڷ���true
bool IsAcqLogicPort(BYTE bPort)
{
	int iPortFun = GetMeterPortFunc(bPort);
	if (iPortFun==COM_FUNC_ACQ || iPortFun==COM_FUNC_JC485)
		return true;

	return false;
}

//����:�߼��˿ںŵ��Զ�������(READER_PLC/READER_4851/READER_4852/READER_4853)��ת��
BYTE PortToReader(BYTE bPort)
{
	if (bPort == PORT_CCT_PLC)
	{
		return READER_PLC;
	}
	else
	{
		WORD wPortNum, wPortMin, wPortMax;
		GetLogicPortNum(&wPortNum, &wPortMin, &wPortMax);

		if (bPort>=wPortMin && bPort<=wPortMax)
			return bPort-wPortMin+1;
		else
			return READER_4851;
	}
}

//����:	͸��645��
int Transmit645Cmd(BYTE* pbCmdBuf, BYTE bLen, BYTE* pbData, BYTE* pbrLen, BYTE bTimeOut, WORD wPn, BYTE bPort)
{
	BYTE bPnValid = 0;
	int iRet = 1;

	if (ReadItemEx(BN0, wPn, 0x8900, &bPnValid) <= 0)
		return -1;
	if (bPnValid == 0)
		return -1;

	if (0x1f == bPort || 0x20 == bPort)
	{
		GetDirRdCtrl(bPort);	//ȡ��ֱ���Ŀ���Ȩ
		bTimeOut = TRANSMIT_TIME_OUT;
		iRet = CctDirectTransmit645Cmd(pbCmdBuf, bLen, pbData, pbrLen, bTimeOut);
		ReleaseDirRdCtrl(bPort); //�ͷ�ֱ���Ŀ���Ȩ
		return iRet;
	}

	memcpy(pbData, pbCmdBuf, bLen);
	memcpy(pbCmdBuf+8, pbData, bLen);
	pbCmdBuf[0] = pbCmdBuf[26];
	pbCmdBuf[1] = bPort;
	ReadItemEx(BN0, wPn, 0x890b, pbCmdBuf+2);
	if(bTimeOut>4 || bTimeOut==0)
		bTimeOut = 4;
	pbCmdBuf[6] = bTimeOut*60;
	pbCmdBuf[7] = bLen;
	*pbrLen = SgMtrFwdFrmCmd(pbCmdBuf, pbData, true);
	if (*pbrLen <= 0)
		return -1;

	return iRet;
}

//����:�ú���ʵ�ֱ�Ƽ�ز���ʱ645֡͸������,����Ч���������
//����:�ɹ�����1,ʧ�ܷ���0
int DirectTransmit645Cmd(WORD wPn, WORD wPort, BYTE* pbCmdBuf, BYTE bCmdLen, BYTE* pbRet, BYTE* pbRetLen, BYTE bTimeOut)
{
	int iRet = 0;
// 	BYTE bRdr = PortToReader((BYTE )wPort);

	iRet = Transmit645Cmd(pbCmdBuf, bCmdLen, pbRet, pbRetLen, bTimeOut, wPn, (BYTE)wPort);
	return iRet;
}

//����:�ú���ʵ�ֱ�Ƽ�ز���ʱ645֡͸������,��485�˿ڲ���,ȡЭ��˿�
//����:�ɹ�����1,ʧ�ܷ���0
int ComTransmit645Cmd(TCommPara tComPara, BYTE* pbCmdBuf, BYTE bCmdLen, BYTE* pbRet, BYTE* pbRetLen, BYTE bTimeOut)
{
	int iRet = 1;
	BYTE bPos = 0;
	
	if (0x1f == tComPara.wPort || 0x20 == tComPara.wPort)
	{
		GetDirRdCtrl(tComPara.wPort);	//ȡ��ֱ���Ŀ���Ȩ
		bTimeOut = TRANSMIT_TIME_OUT;
		iRet = CctDirectTransmit645Cmd(pbCmdBuf, bCmdLen, pbRet, pbRetLen, bTimeOut);
		ReleaseDirRdCtrl(tComPara.wPort); //�ͷ�ֱ���Ŀ���Ȩ
	}
	else
	{
		memcpy(pbRet, pbCmdBuf, bCmdLen);
		memcpy(pbCmdBuf+8, pbRet, bCmdLen);
		pbCmdBuf[0] = pbCmdBuf[26];
		pbCmdBuf[1] = tComPara.wPort;

		pbCmdBuf[2] = BaudrateToVal(tComPara.dwBaudRate);
		pbCmdBuf[3] = ParityToVal(tComPara.bParity);
		pbCmdBuf[4] = StopBitsToVal(tComPara.bByteSize);
		pbCmdBuf[5] = ByteSizeToVal(tComPara.bStopBits);

		if(bTimeOut>4 || bTimeOut==0)
			bTimeOut = 4;
		pbCmdBuf[6] = bTimeOut;//*60;
		pbCmdBuf[7] = bCmdLen;
		*pbRetLen = SgMtrFwdFrmCmd(pbCmdBuf, pbRet, true);//���޸ķ���buf
		if (*pbRetLen <= 0)
			iRet = -1;
	}
	
	if (*pbRetLen > 0)
	{
		for (bPos=0; bPos<*pbRetLen; bPos++)
		{
			if (pbRet[bPos] == 0x68 && pbRet[bPos+7] == 0x68)
				break;
		}
		if (bPos < *pbRetLen)
		{
			*pbRetLen = pbRet[bPos+9] + 12;//645���ĳ���:���ݳ���
			memcpy(pbCmdBuf, &pbRet[bPos], *pbRetLen); 	//ȥ��ǰ���ַ�
			memcpy(pbRet, pbCmdBuf, *pbRetLen); 	//ȥ��ǰ���ַ�
		}
	}
	
	return iRet;
}
