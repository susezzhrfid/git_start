#ifndef DBSGAPI_H
#define DBSGAPI_H

#include "TypeDef.h"
#include "sysarch.h"
#include "sysapi.h"
#include "LibDbAPI.h"
#include "DbHook.h"
#include "DbFmt.h"


#define SUB_ID_MAX		15


#define T_SG			0x00			//����������,û�����Ӧ����ID������ID
#define T_FEE_BLK		0x01	 		//�򵥵Ŀ�������,��ʾ��ǰID������Ϊ������ID�������Ӧ����ID������ID,�����ʸ���
#define T_BLK			0x02	 		//�򵥵Ŀ�������,��ʾ��ǰID������Ϊ������ID�������Ӧ����ID������ID,�������ʸ���

#define C_01			0x00				//��ʾ��ID��Ӧ�ı��뷽ʽ,���ֱ����ʾ��ʼ��һ����ID���λ00��ʼ,�ڶ�����ID�����λ��01��ʼ
#define C_02			0x01				//��ʾ��ID��Ӧ�ı��뷽ʽ,���ֱ����ʾ��ʼ��һ����ID���λ00��ʼ,�ڶ�����ID�����λ��02��ʼ
#define C_01R			0x02				//��ʾ��ID��Ӧ�ı��뷽ʽ,���ֱ����ʾ��һ���ֽڵ�����Ϊ������������ʼ��һ����ID���λ00��ʼ,�ڶ�����ID�����λ��02��ʼ
#define C_03			0x03			//��ʾ��ID��Ӧ�ı��뷽ʽ,���ֱ����ʾ��ʼ��һ����ID���λ01��ʼ,�ڶ�����ID�����λ��02��ʼ

#define N_64		64					//��ID��Ӧ����ID����(��+����)
#define N_21		21					//��ID��Ӧ����ID����(��+����)
#define N_16		16					//��ID��Ӧ����ID����() �籾�յ�ѹƫ�����ֵ����Сֵ��ƽ��ֵ
#define N_5			5					//��ID��Ӧ����ID����(��+����) ���ݿ��������
#define N_4			4					//��ID��Ӧ����ID����(�繦�����������ݿ�)
#define N_6			6					//��ID��Ӧ����ID����(��������ݿ�)
#define N_7			7					//��ID��Ӧ����ID����(��״̬��)
#define N_9			9					//��ID��Ӧ����ID����(���ѹ���ݿ�)


typedef struct  
{
	DWORD	dwID;					//Э��ID
	DWORD dwBlkID;			//������ID��������ĳ����ID�ĸ�ֵΪ0
	WORD	wIdType;					//��־λ
	BYTE	bSubNum;				//��ID��������ID����
	BYTE	bCode;					//��ID����ID���ŷ�ʽ
	WORD	wSubId[SUB_ID_MAX];		//�ڲ���Ӧ��ID
}TProId2Sub;						//Э��ID����ID��ӳ��

typedef struct
{
	WORD	wPn;					//�������
	WORD	wBn;					//BANK��
	DWORD	dwID;					//������ID
}TProItem;							//��Ҫ���������������ʱ���ѯ

typedef struct
{
	DWORD dwId;             //��ID

	BYTE  bIdType;          //ID������          	
	DWORD dwProId;       	//��ID��Ӧ�����ñ��ϵ�Э��ID�ţ�������ñ�������ȫ��ȵ�ID�����dwId
	WORD  wProLen;          //��ID��Ӧ�����ݳ���
	BYTE	bCode;			//��ID����ID���ŷ�ʽ
	
	WORD  wOffset;          //��ID������ID�����е�ƫ��λ�ã������ID���������0
	WORD  wDataLen;         //��ID��Ӧ�����ݳ��ȣ������ID���������wProLen
	BYTE  bIdx;             //��ID�ڿ�ID�е������������ID���������0
}TProIdInfo;	//Э��ID����Ϣ

int WriteItemDw(WORD wBank, WORD wPn, DWORD dwID, BYTE* pbBuf, DWORD dwTime);

//����ID���������Ķ�
int ReadItemDw(WORD wBank, WORD wPn, DWORD dwID, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime); //ָ��ʱ���

//���ID�Ķ��ӿ�,Ŀǰֻ֧�ְ��������Ķ�
int ReadItemDwMid(WORD wBank, WORD wPn, DWORD *pdwID, WORD wNum, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime);//����ͬ������ʱ���
int ReadItemDwMbi(TProItem* pProItem, WORD wNum, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime);//����ͬʱ���

int GetItemLenDw(WORD wBn, DWORD dwID);
int GetItemsLenDw(DWORD *pdwItemID, WORD wNum);

//������ʱ��Ĳ�ѯ,���������Ƿ���ָ��ʱ���ڸ�����
int QueryItemTimeDwMbi(DWORD dwStartTime, DWORD dwEndTime, TProItem* pProItem, WORD wNum, BYTE* pbBuf, WORD* pwValidNum);  //����TBankItem��ͨ������
int QueryItemTimeDwMid(DWORD dwStartTime, DWORD dwEndTime, WORD wBn, WORD wPn, DWORD* pdwID, WORD wNum, BYTE* pbBuf, WORD* pwValidNum); //ͬһ��BANKͬһ�������㲻ͬID�Ĳ�ѯ
int QueryItemTimeDwMt(DWORD* pdwStartTime, DWORD* pdwEndTime, TProItem* pProItem, WORD wNum, BYTE* pbBuf, WORD* pwValidNum); //���ղ�ͬ�����ͬʱ��Ĳ�ѯ

WORD* ProId2SubId(DWORD dwID,WORD* pwSubNum);
WORD* ProId2RdId(DWORD dwID,WORD* pwSubNum);
bool InitProId2Sub();

//ȡ��Э��ID����Ϣ
bool GetProIdInfo(DWORD dwId, TProIdInfo* pInfo, bool fBlk);
WORD GetOnePnFromDA(BYTE bDA,BYTE bDAG);

#endif

