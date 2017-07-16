/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrExcTask.h
 * ժ    Ҫ��ʵ�ָ澯���жϣ���¼����ѯ
 *
 * ��    ��: 1.0 1
 * ��    �ߣ�����
 * ������ڣ�2008-06-
 *
 * ȡ���汾��1.0 0
 * ԭ �� ��:
 * ������ڣ�
 * ��    ע:
 
************************************************************************************************************/

#ifndef EXCTASK_H
#define EXCTASK_H

//#include "FaStruct.h"
//#include "TaskDB.h"
#include "MtrExc.h"
#include "LibDbStruct.h"


//���澯�¼�����
//========�¼�����========�¼�ID=================����������===========
#define ERC_INIT_VER        1					//���ݳ�ʼ���Ͱ汾�����¼
#define ERC_PARACHG			3                   //�䳤�¼�
#define ERC_YXCHG			4					//״̬��λ��¼
#define ERC_YK             	5					//ң����բ��¼
#define ERC_PWRCTL          6					//���ؿ���բ��¼
#define ERC_ENGCTL          7					//��ؿ���բ��¼
#define ERC_MTRPARA         8                   //���������
#define ERC_IEXC        	9					//������·�쳣
#define ERC_VEXC           	10					//��ѹ��·�쳣
#define ERC_DISORDER    	11					//�����쳣
#define ERC_MTRTIME         12					//���ܱ�ʱ�䳬��
#define ERC_MTRERR        	13					//��������Ϣ
#define ERC_PWRONOFF        14					//�ն�ͣ/�ϵ��¼�
#define ERC_HARMONIC        15					//г��Խ��
#define ERC_DC        		16					//ֱ��ģ�����¼�#
#define ERC_UNBALANCE    	17					//��ѹ/������ƽ���Խ�޼�¼
#define ERC_BUYPARA         19					//����������ü�¼
#define ERC_AUTH           	20					//��Ϣ��֤����
#define ERC_TERMERR			21					//�ն˹��ϼ�¼
#define ERC_ENGDIFF         22					//����¼�#
#define ERC_ENGALARM        23					//��ظ澯��¼
#define ERC_VOLTEX          24					//��ѹԽ�޼�¼
#define ERC_IEX          	25					//����Խ�޼�¼
#define ERC_SEX          	26					//���ڹ���Խ�޼�¼
#define ERC_ENGDEC       	27					//����¼���
#define ERC_ENGOVER       	28					//����������
#define ERC_MTRFLEW         29					//����¼���
#define ERC_MTRSTOP         30					//����¼���
#ifdef PRO_698
#define ERC_MTRRDFAIL		31					//����ʧ��
#define ERC_FLUXOVER		32					//�ն�����վͨ�������������¼���¼
#define ERC_MTRSTATUSCHG	33					//���״̬�ֱ�λ�¼�
#endif
#define ERC_CTEXC			34					//CT�쳣

#define ERC_MTRTAILLID		37					//���ܱ�����¼���¼
#define ERC_MTRBUTTONLID	38					//���ܱ���ť���¼���¼
#define ERC_MTRREFAIL		39					//����ʧ���¼���¼
#define ERC_FIELD_ERR		40					//�ų��쳣�¼�
#define ERC_SETTIME			41					//��ʱ�¼���¼
//����Ϊ���ܱ���չ�¼�
#define ERC_VMISS			40				//ʧѹ
//#define ERC_VLACK			41				//Ƿѹ
#define ERC_VDISORD			42				//��ѹ������
#define ERC_IDISORD			43				//����������
#define ERC_VEXCEED			44				//��ѹ
#define ERC_VBREAK			45				//����

#define ERC_IMISS			46				//ʧ��
#define ERC_IEXCEED			47				//����
#define ERC_VUNBALANCE		48				//��ѹ��ƽ��
#define ERC_IUNBALANCE		49				//������ƽ��
#define ERC_FLOW_REVERSE	50				//��������
#define ERC_OVER_LOAD		51				//����
#define ERC_TEST_CONNECT	60				//ͨ�Ų�������



// #define CONNTYPE_1P      3
// #define CONNTYPE_3P3W    1   //�����߷�ʽ-��������
// #define CONNTYPE_3P4W    2	 //��������
// #define EXC_REC_NUM     5   //��ȡĳ���͸澯�¼�������¼����
// #define EXC_REC_LENTH   50  //�����澯�¼���¼����󳤶ȣ�(�����Խ�����õ��ܼ���Ĳ��������>3��һ�㲻�ᳬ�����˶��峤���㹻��)


bool CreatExcTaskTab(char* pszName);
BYTE CopyAlrData(BYTE *bSaveBuf, BYTE *pbBuf, BYTE bLen);
extern bool SaveAlrData(DWORD dwAlrID, WORD wPn, BYTE* pbBuf, DWORD dwLen);
extern bool HandleAlr(DWORD dwAlrID, WORD wPn, TBankItem* pBankItem, WORD wIdNum, DWORD dwCurMin, TTime tmNow, BYTE* bPreData, WORD wDataLen);
//extern BYTE ReadClass3Item(BYTE bAlrID, BYTE bRecNum, BYTE* pbBuf, BYTE& bLen);
extern BYTE ReadAlrItem(BYTE bAlrType, BYTE bAlrID, BYTE bRecNum, BYTE* pbBuf, BYTE* pbLen);
BYTE SortArray(DWORD dwData[], BYTE bDataNum, BYTE bRecNum, BYTE* bIndex);



//
//*******************************�����¼�***************************************
//

//#define EXC_TASK_NUM	51		//34	//�澯�¼���������Ŀ
//BYTE m_bPnProp[PN_NUM+1];
//bool IsAlrAttriChg(BYTE bERC, BYTE& bAlrType);
//bool IsPnPropChg(BYTE bPn, BYTE& rbPnProp);

//extern BYTE m_bAlrType[EXC_TASK_NUM+1];

extern bool InitExcTask();	//��ʼ���澯����
extern bool ExcTaskInit();	
extern bool DoExcTask(BYTE bPn);	//ִ�в�����Pn�ĸ澯����

//�ն˸澯
//�ն˸澯�������ն˷����¼���ʱ��ִ��,���ڶ����������ݵ�ʱ�����

#endif 

