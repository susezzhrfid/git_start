/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ProThread.c
 * ժ    Ҫ�����ļ���Ҫʵ�ֽӿڵı�׼ͨ���߳�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
 * ��    ע��
 *********************************************************************************************************/
//#include "FaAPI.h"
#include "FaCfg.h"
#include "ProAPI.h"
#include "Pro.h"
#include "SysArch.h" 
#include "ThreadMonitor.h"
#include "SysDebug.h"

//����:�ӿڵı�׼ͨ���߳�
TThreadRet StdProThread(void* pvArg)
{
	//DWORD dwClick;
	int iMonitorID;
	struct TPro* pPro = (struct TPro* )pvArg;
	struct TProIf* pIf = pPro->pIf;		//ͨ�Žӿ�
	
	//DTRACE(DB_FAPROTO, ("StdProThread : If (%s) started!\n", pIf->pszName));
	
	iMonitorID = ReqThreadMonitorID(pIf->pszName, 60*60);	//�����̼߳��ID,���¼��Ϊһ��Сʱ

	pIf->fExitDone = false;
	while (1)
	{
		Sleep(100);
		UpdThreadRunClick(iMonitorID);
		if (pIf->fExit)
			break;

		pIf->LoadUnrstPara(pIf); 	//�Ǹ�λ���������ı�
		//pProto->LoadUnrstPara(); 

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
				if (pIf->pfnReset(pIf))
                {
					pIf->pfnOnResetOK(pIf);
                }
				else
                {
					pIf->pfnOnResetFail(pIf);
                }
				break;
					
			case IF_STATE_TRANS:  //����
				//if (pIf->bIfType==IF_GPRS || pIf->bIfType==IF_SOCKET)
					//pPro->pfnAutoSend(pPro);

				pIf->pfnTrans(pIf); //���յ���һ֡,���Ѿ�������д���
				break;
				
			default:
				DTRACE(DB_FAPROTO, ("StdProtoThread : enter unkown state!\n"));	
                pIf->bState = IF_STATE_RST;  //ֱ�Ӹ�λ
				//Sleep(5000);
				break;
		}
	}

	ReleaseThreadMonitorID(iMonitorID);
	pIf->fExitDone = true;

	return 0;
}
