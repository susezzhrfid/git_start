/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�CctPara.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�ֶԼ���������װ��
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2009��9��
 * ��    ע: ���ļ���Ҫ�������θ��汾������Ĳ�����
 *********************************************************************************************************/
//#include "stdafx.h"
//#include "syscfg.h"
#include "AutoReader.h"

#include "DbAPI.h"
#include "CctHook.h"
#include "FaAPI.h"
#include "CctAPI.h"
#include "DrvCfg.h"
//����:װ���Զ�����������
//��ע:���Ը��ݲ�ͬ��ͨ��bLink����һ�����⻯
//����:·��������ģʽ

void LoadAutoRdrPara(BYTE bLink, TAutoRdrPara* pRdrPara)
{
	DWORD dwAddr = 0;
	BYTE bBuf[16];
	pRdrPara->bDayFrzDelay = 1;		//�ն��᳭���ӳ�ʱ��,��λ����,�������������ն�ʱ��ȵ���

// 	//��Ϣ�궨��,�����������п��õ��ĺ궨���Ӧ�ó���һ��
// 	pRdrPara->wInfoPlcPara = INFO_PLC_PARA;   //�ز���������
// 	pRdrPara->wInfoPlcClrRt = INFO_PLC_CLRRT; //��·��
// 	pRdrPara->wInfoPlcStopRd = INFO_PLC_STOPRD; //ֹͣ����
// 	pRdrPara->wInfoPlcResumeRd = INFO_PLC_RESUMERD;	 //�ָ�����
// 	pRdrPara->wInfoPlcRdAllRtInfo = INFO_PLC_RDALLRTINFO;	//��ȡ���нڵ���м�·����Ϣ
// 	pRdrPara->wInfoUpdateRT = INFO_PLC_UPDATE_ROUTE;       //�ز�·��������
// 	pRdrPara->wInfoChannelChange = INFO_PLC_WIRLESS_CHANGE;       //����ͨ�Ų������
// 	//pRdrPara->wInfoManualNetwork = INFO_RF_MANUAL_NETWORK;
	//pRdrPara->wInfoPlcSetRfCh = INFO_RF_SET_CH;	//���������ŵ���
	//pRdrPara->wInfoCmdNetwork = INFO_RF_CMD_NETWORK;//������������
	//pRdrPara->wInfoCmdStopNetwork = INFO_RF_STOP_NETWORK;//����ֹͣ����
	//pRdrPara->wInfoDelRfNode = INFO_RF_DEL_NODE;
	
	pRdrPara->fUseLoopBuf = true;		//�Ƿ�ʹ��ѭ��������
	
	memset(pRdrPara->bMainAddr, 0, sizeof(pRdrPara->bMainAddr));	//���ڵ��ַ
	ReadItemEx(BN0, PN0, 0x8020, pRdrPara->bMainAddr); //����������A1
	ReadItemEx(BN0, PN0, 0x8021, (BYTE*)&dwAddr);	//�ն˵�ַA2
	//IntToBCD(dwAddr, pRdrPara->bMainAddr+3 ,3);
	memcpy(pRdrPara->bMainAddr+3,&dwAddr,3);

	/*ReadItemEx(BN0, PN0, 0x231f, bBuf);
	memcpy(bBuf, pRdrPara->bMainAddr, 6);
	WriteItemEx(BN0, PN0, 0x231f, bBuf);*/
	//pRdrPara->bMainAddr+2
	////������ѡ����ʱ�α�־
	//if (bLink==AR_LNK_ES || bLink==AR_LNK_STD_ES)
	//{
	//   ReadItemEx(BN15, PN0, 0x5314, pRdrPara->bReRdTmFlg);
	//   ReadItemEx(BN15, PN0, 0x5315, pRdrPara->bSelRdTmFlg);
	//}
	//else
	//{
 //      ReadItemEx(BN15, PN0, 0x5312, pRdrPara->bReRdTmFlg);
	//   ReadItemEx(BN15, PN0, 0x5313, pRdrPara->bSelRdTmFlg);
	//}
}

//����:װ��698.42·�ɲ���
void LoadStdPara(BYTE bLink, TStdPara* pStdPara)
{
	BYTE bRunMode = 1;
	memset(pStdPara, 0, sizeof(TStdPara));
	LoadAutoRdrPara(bLink, &pStdPara->RdrPara);
    
	pStdPara->wFrmLenBytes = 2;
	pStdPara->bLink = bLink;

	//if (bLink == AR_LNK_STD_ES)
	//{
		ReadItemEx(BN15, PN0, 0x5321, &bRunMode);
		pStdPara->bRunMode = bRunMode;
	//}
	
	pStdPara->RdrPara.bLogicPort = PORT_CCT_PLC;	//�߼��˿ں�,ָ����ͨ��Э���ϸ�ÿ��ͨ���涨�Ķ˿ں�,����������˿ں�
	pStdPara->RdrPara.bPhyPort = COMM_METER3;	//����˿ں�
}
