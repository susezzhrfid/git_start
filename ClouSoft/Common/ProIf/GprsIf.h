/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�GprsIf.h
 * ժ    Ҫ�����ļ�ʵ����GPRSͨ�Žӿ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
 *********************************************************************************************************/
#ifndef GPRSIF_H
#define GPRSIF_H
#include "ProIf.h"
#include "Modem.h"
#include "SocketIf.h"

//ͨ����ͨ��ģʽ
#define CN_MODE_SOCKET      0	//����TCP/IP��ͨ��ģʽ
#define CN_MODE_SMS      	1	//����
#define CN_MODE_EMBED     	2	//ģ��Ƕ��ʽЭ��ջ
#define CN_MODE_COMM     	3	//����ͨ��ģʽ
#define CN_MODE_CMD     	4	//����ģʽ

#define CN_TYPE_GPRS        0   //GPRS
#define CN_TYPE_ET          1   //��̫��

//����ģʽ
#define ONLINE_M_PERSIST    1	//��������ģʽ
#define ONLINE_M_ACTIVE     2	//����ģʽ/����������ģʽ
#define ONLINE_M_PERIOD		3	//ʱ������ģʽ
#define ONLINE_M_SMS		4   //���ŷ�ʽ
#define ONLINE_M_JIT		5	//JUST IN TIME ����Ҫ��ʱ����,�絥�����ϱ��˿�
#define ONLINE_M_MIX		6	//�������Ϳͻ��˵Ļ��ģʽ

#define ONLINE_MODE			0
#define NONONLINE_MODE		1

typedef struct{
	//����
	BYTE bOnlineMode;		//����ģʽ
	BYTE bCliBeatMin;		//�ͻ����������,��λ����
	BYTE bSvrBeatMin;		//�������������,��λ����
	BYTE bRstToDormanNum;	//��λ�����ߵĴ���:�κ������ʧ�ܻᵼ��ģ�鸴λ����ģ�鸴λ����������������󣬽�������״̬
	BYTE bConnectNum;		//����ʧ���������ԵĴ���
	BYTE bLoginNum; 		//��¼ʧ���������ԵĴ���
	WORD wDormanInterv;		//����ʱ����, ��λ��, ,0��ʾ��ֹ����ģʽ
	WORD wActiveDropSec; 	 //����������ģʽ���Զ�����ʱ��,��λ��
	bool fEnableAutoSendActive; //���������ϱ�����
	struct TModem* pModem;		//GPRSģ��
    
    TSocketIf *pSocketIf;       //�ն�Э��ջ����̫���Ľӿ�
    	
	DWORD dwFluxOverClick;	//�����������ʼʱ��
   
	//����
	bool fSvrTrans;		//��ǰ���ڷ���������״̬����Ҫ���ڻ��ģʽ�£����������Ƿ��������ǿͻ����ڴ��䣬
						//Ҫ�ڵ��÷��ͽ��պ���ǰ�趨
	BYTE bCliState;		//�ͻ���״̬��������->��½->ͨ��->�����Ͽ�->����
	BYTE bSvrState;		//������״̬��������->��������->������ͨ��->
	BYTE bCnMode;		//ͨ��ģʽ,���ŷ�ʽ������ΪCN_MODE_SMS,
						//GPRS��ʽ������ΪCN_MODE_SOCKET��CN_MODE_EMBED,
						//���������߷�ʽ����ָGPRS��ͨ��ģʽ,����ΪCN_MODE_SOCKET��CN_MODE_EMBED,
    BYTE bCnType;       //ͨ��������̫����GPRS
	BYTE bSignStrength;	//�ź�ǿ��
	char cLastErr;		//���Ĵ���
    bool fCnSw;         //ͨ���л�
    int ipd;            //pppoe
    DWORD dwNoRxRstAppInterv;  //�޽��ո�λ�ն˼��,��λ��,0��ʾ����λ      ֻ��GPRS����̫���ڲ����������λ
}TGprsIf;	//GPRSͨ�Žӿ�����ṹ


////////////////////////////////////////////////////////////////////////////////////////////
//GprsIf������������
bool GprsIfInit(struct TProIf* pProIf);
bool GprsIfRxFrm(struct TProIf* pProIf, BYTE* pbRxBuf, int iLen, bool fSvr); //�ⲿ����������ݲ�����֡
extern BYTE GetSign(struct TProIf* pProIf);
extern BYTE GetSignStrength();
extern void UpdateSvrRxClick();
extern bool IsOnlineState(struct TProIf* pProIf);

void CheckNetStat(void);

#endif  //GPRSIF_H
