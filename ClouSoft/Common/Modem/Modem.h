/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Modem.h
 * ժ    Ҫ�����ļ�ʵ����ͨ��MODEM�Ļ��ඨ��
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��12��
 * ��    ע������ΪGPRS,CDMA�͵绰MODEM�Ļ���
 *********************************************************************************************************/
#ifndef MODEM_H
#define MODEM_H
#include "DrvConst.h"
#include "Comm.h"
//#include "drivers.h"
#include "ProIfCfg.h"

#define GSN_LEN    		 15
#define PHONE_NUM_LEN    32
#define ACTIVE_SMS_LEN   32

#define MODEM_STEP_INIT	 0	//��ʼ��
#define MODEM_STEP_RST	 1	//��λģ��
#define MODEM_STEP_SIM	 2	//���SIM��
#define MODEM_STEP_REG	 3	//ע������

#define MODEM_NO_ERROR	 0	//û�д���
#define MODEM_RST_FAIL	 1	//ģ�鸴λʧ��
#define MODEM_SIM_FAIL	 2  //���SIMʧ��
#define	MODEM_REG_FAIL	 3	//ע������ʧ��
#define MODEM_CSQ_FAIL   4  //���³�ǿʧ��
#define MODEM_OTHER_FAIL 5  //��������


typedef struct {
	BYTE bManuftr[4];	//���̴���	ASCII	4
	BYTE bModel[8];		//ģ���ͺ�	ASCII	8
	BYTE bSoftVer[4];	//����汾��	ASCII	4
	BYTE bSoftDate[3];	//����������ڣ�������	����¼A.20	3
	BYTE bHardVer[4];	//Ӳ���汾��	ASCII	4
	BYTE bHardDate[3];	//Ӳ���������ڣ�������	����¼A.20	3
	BYTE bCCID[20];					//�ӣɣͿ�ICCID	ASCII	20
}TModemInfo;	//ģ����Ϣ

struct TModem{
	//����
	BYTE  bComm;	//���ں�
	struct TProIf* pProIf;	//ָ��GPRS�ӿڵ�ָ��
	void* pvModem;		//MODEM��������,ָ�����ʵ��
	BYTE bModuleVer;
    TModemInfo tModemInfo;

	//�麯������Ҫʵ����Ϊ����ӿڵĶ�Ӧ����
	int (* pfnResetModem)(struct TModem* pModem);			//��λģ��
	int (* pfnUpdateSignStrength)(struct TModem* pModem);	//���³�ǿ
	int (* pfnInitAPN)(struct TModem* pModem);				//��ʼ��APN

	bool (* pfnOpenPpp)(struct TModem* pModem);				//PPP��������
	bool (* pfnClosePpp)(struct TModem* pModem);			//�Ͽ�PPP����
	bool (* pfnConnect)(struct TModem* pModem, bool fUdp, DWORD dwRemoteIP, WORD wRemotePort); //��Ϊ�ͻ������ӷ�����
	bool (* pfnCloseCliSock)(struct TModem* pModem);	//�رտͻ���socket
	int (* pfnCliSend)(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen); 	//��Ϊ�ͻ��˷�������           //int֧�ִ��󷵻�
	int (* pfnCliReceive)(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen); 	//��Ϊ�ͻ��˽�������
	bool (* pfnChkCliStatus)(struct TModem* pModem);	//���ͻ��˵�socket�����Ƿ���Ȼ��Ч��Ҫ�����������ѯ

	bool (* pfnSpecHandle)(struct TModem* pModem, char* pszRxBuf, WORD wRxLen, WORD wBufSize); //��ģ�������������ĵ����ݽ������⴦��,��TCP/UDP���ġ��յ����Լ����˿ڵ�������

#ifdef EN_PROIF_SVR		//֧�ַ�����
	bool (* pfnListen)(struct TModem* pModem, bool fUdp, WORD wLocalPort);  //��Ϊ�����������˿�
	bool (* pfnCloseSvrSock)(struct TModem* pModem);	//�رշ������Ѿ����ӵ�socket
	bool (* pfnCloseListen)(struct TModem* pModem);	//�رռ���

	int (* pfnSvrSend)(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen); 	//��Ϊ��������������
	int (* pfnSvrReceive)(struct TModem* pModem, BYTE* pbTxBuf, WORD wLen); 	//��Ϊ��������������

	bool (* pfnIsSvrAcceptOne)(struct TModem* pModem);		//��Ϊ�������Ƿ���յ�һ���ͻ��˵�����
	bool (* pfnChkSvrStatus)(struct TModem* pModem);	//����������socket�����Ƿ���Ȼ��Ч��Ҫ�����������ѯ
#endif //EN_PROIF_SVR		//֧�ַ�����

	//����
	BYTE bStep;		//��ǰ�Ĳ�������
	//char* pszCSQ;
    
    //int pd;    
};  //MODEM����

void ModemInit(struct TModem* pModem);
int ATCommand(struct TModem* pModem, char* pszCmd, char* pszAnsOK, char* pszAnsErr1, char* pszAnsErr2, WORD nWaitSeconds);
int WaitModemAnswer(struct TModem* pModem, char* pszAnsOK, char* pszAnsErr1, char* pszAnsErr2, WORD nWaitSeconds);
bool ATCmdTest(struct TModem* pModem, WORD wTimes);
int UpdateSignStrength(struct TModem* pModem);
//bool IsSignValid(WORD wSignStrength);
//void SignLedCtrl(BYTE bSignStrength);

int ATCmdGetInfo(struct TModem* pModem, char* pszCmd, char* pszAnsOK,
			  char* pszAnsErr, char* psRxHead, char *psBuf, WORD wBufSize,
              WORD nWaitSeconds);
bool GetModemVer(struct TModem* pModem);
bool GetModemCCID(struct TModem* pModem);

#endif  //MODEM_H



