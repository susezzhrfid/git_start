/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�FapAPI.c
 * ժ    Ҫ�����ļ���Ҫ����FaProtoĿ¼��API������ȫ�ֱ����Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
 * ��    ע��
 *********************************************************************************************************/
#include "Pro.h"
#include "DrvCfg.h"
#include "ProIf.h"
#include "Modem.h"
#include "M590.h"
#include "CommIf.h"
#include "GprsIf.h"
#include "ProPara.h"
#include "ProAPI.h"
#include "GbPro.h"
#include "CommIf.h"
#include "SysDebug.h"
#include "ThreadMonitor.h"

//#include "socketIf.h"


//struct TProIf g_ifSock;	//Socketͨ�Žӿڻ���ṹ
TSocketIf g_SocketIf;	//Socket�ӿ�����

struct TProIf g_ifGprs;	//GPRSͨ�Žӿڻ���ṹ
TGprsIf g_GprsIf;	//GPRSͨ�Žӿ�����ṹ
struct TModem g_Modem;	//MODEM����
TM590 g_M590;		//GC864����

#define REMOTE_BUF_MAX			1860
#define LOCAL_BUF_MAX			1100

struct TProIf g_ifCommLocal;	//����ͨ�Žӿڻ���ṹ
TCommIf g_Comm232If;		//����ͨ�Žӿ�����ṹ
TCommIf g_CommIrIf;		//����ͨ�Žӿ�����ṹ


struct TPro g_proMaster;	//��վͨ��Э�����
TSgPro g_gbMaster;		//��վͨ��Э������

struct TPro g_proLocal;		//����ά��ͨ��Э�����
TSgPro g_gbLocal;			//����ά��ͨ��Э������

/*
struct TPro g_proLocalIr;		//���غ����ά��ͨ��Э�����
TGbPro g_gbLocalIr;			//���غ����ά��ͨ��Э������*/

BYTE g_bRemoteRxBuf[REMOTE_BUF_MAX];		//Զ��ͨѶ���ջ�����
BYTE g_bRemoteTxBuf[REMOTE_BUF_MAX];		//Զ��ͨѶ���ͻ�����

BYTE g_bLocalRxBuf[LOCAL_BUF_MAX];		//����ͨѶ���ջ�����
BYTE g_bLocalTxBuf[LOCAL_BUF_MAX];		//����ͨѶ���ͻ�����

void InitMasterPro()
{
	//Socket�ӿ�����
	memset(&g_SocketIf, 0, sizeof(g_SocketIf));
	//LoadSockPara(&g_SocketIf);

	//Socketͨ�Žӿڻ���ṹ
	//g_ifSock.pvIf = &g_SocketIf;
		
	//SocketIfInit(&g_ifSock);
	//SetMaxFrmBytes(&g_ifSock, REMOTE_BUF_MAX);
	
	//Э����ӿڵĹҽ�	
	//g_ifSock.pPro = &g_proMaster;	
    //g_proMaster.pIf = &g_ifSock;	

	memset(&g_Modem, 0, sizeof(g_Modem));
	memset(&g_M590, 0, sizeof(g_M590));
	memset(&g_GprsIf, 0, sizeof(g_GprsIf));

	//MODEM�������
	g_Modem.bComm = COMM_GPRS;		//���ں�
	g_Modem.pProIf = &g_ifGprs;	//ָ��GPRS�ӿڵ�ָ��
	g_Modem.pvModem = &g_M590;		//MODEM��������,ָ�����ʵ��
	g_Modem.bModuleVer = MODULE_ME590;

	//GC864�����������
#ifndef SYS_WIN
	M590Init(&g_Modem); //Wing 2014/07/31
	//GL868Init(&g_Modem);
#endif

    g_GprsIf.pSocketIf = &g_SocketIf;
    
	//GPRSͨ�Žӿ�����ṹ
	LoadGprsPara(&g_GprsIf);
	g_GprsIf.pModem = &g_Modem;

	//GPRSͨ�Žӿڻ���ṹ
	g_ifGprs.pvIf = &g_GprsIf;

	GprsIfInit(&g_ifGprs);
	//SetMaxFrmBytes(&g_ifGprs, REMOTE_BUF_MAX);

	//Э����ӿڵĹҽ�
    g_ifGprs.pPro = &g_proMaster;
	g_proMaster.pIf = &g_ifGprs;		

	//Э���ʼ��
	g_proMaster.pvPro = &g_gbMaster;
	SgInit(&g_proMaster, g_bRemoteRxBuf, g_bRemoteTxBuf, false);   //��������û�г�ʼ��������������
}

//����ά����Э���ʼ��
void InitLocalPro()
{
    memset(&g_CommIrIf, 0, sizeof(g_CommIrIf));
	LoadLocalIrPara(&g_CommIrIf);
	g_ifCommLocal.pvIf = &g_CommIrIf;
	CommIfInit(&g_ifCommLocal);

	memset(&g_Comm232If, 0, sizeof(g_Comm232If));	
	LoadLocal232Para(&g_Comm232If);
    g_ifCommLocal.pvIf = &g_Comm232If;
	CommIfInit(&g_ifCommLocal);

	//SetMaxFrmBytes(&g_ifCommLocal, LOCAL_BUF_MAX);

	//Э����ӿڵĹҽ�
	g_proLocal.pIf = &g_ifCommLocal;
	g_ifCommLocal.pPro = &g_proLocal;
	
	//Э���ʼ��
	g_proLocal.pvPro = &g_gbLocal;
	SgInit(&g_proLocal, g_bLocalRxBuf, g_bLocalTxBuf, true);
}


void InitProto()
{
	InitMasterPro();

//#ifndef SYS_WIN
	InitLocalPro();    
//#endif
	
	SftpInit();
	TransFileInit();
}

void NewProThread()
{
	NewThread("STD", StdProThread, &g_proMaster, 768/*1024*/, THREAD_PRIORITY_NORMAL);  //1024
        
    //sys_thread_new("StdProThread",  (pdTASK_CODE)StdProThread, &g_proMaster, 1024, THREAD_PRIORITY_BELOW_NORMAL);//ͨ���̱߳��뽫�߳̾������һ��ʱ����,�����LWIP�ṩ�ĺ���������
        
//#ifndef SYS_WIN
//    NewThread("LOCAL", LocalThread, &g_proLocal, 1024, THREAD_PRIORITY_BELOW_NORMAL);    
//#endif
} 

BYTE GetSignStrength()
{
#ifndef SYS_WIN
    return GetSign(&g_ifGprs);    
#else
	return 20;
#endif
}
        
    
//����ά���߳�
bool DoLocalService(struct TPro* pPro, TCommIf* pCommIf)
{
    //struct TProIf* pIf = &g_ifCommLocal;
    struct TProIf* pIf = pPro->pIf;     //����ͨ��
	pIf->pvIf = pCommIf;		//�Ȱ�ʵ��ͨ�Žӿ�
    
	pIf->pfnDoIfRelated(pIf);	//��һЩ�����ӿ���صķǱ�׼������
						//������������߷�ʽ��,GPRS��SMS����л�
	pPro->pfnDoProRelated(pPro);	//��һЩЭ����صķǱ�׼������
	
	//�ӿ�״̬��:����->��λ(MODEM��λ��PPP����)->����(�ͻ���״̬��������->��½->ͨ��->�����Ͽ�->����)
	switch (pIf->bState)  //�ӿڵ�״̬��
	{
		case IF_STATE_DORMAN:  //����
			pIf->pfnDorman(pIf);
			break;
		
		case IF_STATE_RST:  //��λ
			if (pIf->pfnReset(pIf) == IF_RST_OK)
			{
				pIf->pfnOnResetOK(pIf);
			}
			else
			{
				pIf->pfnOnResetFail(pIf);
			}
			break;
				
		case IF_STATE_TRANS:  //����
			pIf->pfnTrans(pIf); //���յ���һ֡,���Ѿ�������д���
			break;
			
		default:
			DTRACE(DB_FAPROTO, ("StdProtoThread : enter unkown state!\n"));	
			Sleep(5000);
			break;
	}

	return true;
}


//����:����ά���ı�׼ͨ���߳�(����ں�232ά���ڹ��ô��߳�)
void LocalThread()
{
//  struct TPro g_proLocal;		//����ά��ͨ��Э�����
	//int iMonitorID;
	//struct TPro* pPro = (struct TPro* )pvArg;
	//struct TProIf* pIf = pPro->pIf;		//ͨ�Žӿ�
	
	//DTRACE(DB_FAPROTO, ("LocalThread : if(%s) started!\n", pIf->pszName));
	//iMonitorID = ReqThreadMonitorID(pIf->pszName, 60*60);	//�����̼߳��ID,���¼��Ϊһ��Сʱ

	//pIf->fExitDone = false;
	//while (1)
	{
        //if (IsDownSoft())            
        //    Sleep(10);
        //else
    		//Sleep(100);    //����ҪSleep,CommRead���Ѿ�����

		//UpdThreadRunClick(iMonitorID);

        DoLocalService(&g_proLocal, &g_Comm232If);
        if (!IsDownSoft())    
            DoLocalService(&g_proLocal, &g_CommIrIf);
  		//if (pIf->fExit)
		//	break;
	}

	//ReleaseThreadMonitorID(iMonitorID);
	//pIf->fExitDone = true;
	
	return ;
}

BYTE GetGprsIfState(void)
{
    return g_ifGprs.bState;
}