/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�TaskStruct.h
 * ժ    Ҫ�����ļ���Ҫ���������������ݽṹ
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2009��9��
 *********************************************************************************************************/
#ifndef TASKSTRUCT_H
#define TASKSTRUCT_H
#include "TypeDef.h"
#include "TaskConst.h"

//#define MAX_ITEMS_PER_CTASK 8

#define TSK_DMCUR		1		//DAY&MONTH FROZEN CURRENT �����¶������óɳ����ǰ����ʱ����
#define TSK_DMMTR		2		//DAY&MONTH FROZEN METER FRZ �����¶������óɳ������ʱ����
#define TSK_DFCUR		3		//DAYFLG FROZEN CURRENT �ڳ����ն������óɳ����ǰ����ʱ����
#define TSK_DFMTR		4		//DAYFLG FROZEN METER �ڳ����ն������óɳ������ʱ����
#define TSK_PFCUR		5		//PROFILE FROZEN CURRENT ���������óɳ����ǰ����ʱ����
#define TSK_PFMTR		6		//PROFILE FROZEN METER ���������óɳ������ʱ����
#define TSK_DEMDMCUR	7		//DEMAND DAY&MONTH FROZEN CURRENT ���������¶������óɳ����ǰ����ʱ����
#define TSK_DEMDMMTR	8		//DEMAND DAY&MONTH FROZEN METER FRZ ���������¶������óɳ������ʱ����
#define TSK_DEMDFCUR	9		//DEMAND DAYFLG FROZEN CURRENT �����ڳ������¶������óɳ����ǰ����ʱ����
#define TSK_DEMDFMTR	10		//DEMAND DAYFLG FROZEN METER FRZ �����ڳ������¶������óɳ������ʱ����
#define TSK_MSETT   	11		//MONTH SETT FRZ �¶����ڳ���������ʱ����
#define TSK_DEMMSETT	12		//DEMAND MONTH SETT FRZ �¶��������ڳ���������ʱ����

#define MTR_WID_MAX		2        //ID��������ID��������ʱ��Ϊ2��


typedef struct
{
	BYTE	bFN;							//�������ʶbFNs[MAX_ITEMS_PER_CTASK]
	BYTE	bIDCnt;							//wID[MTR_WID_MAX]��������Ч��ID����
	WORD	wID[MTR_WID_MAX];				//����������������ID
	BYTE	bLen;							//����ID��������ܳ���
	DWORD	dwID;							//��Ӧ����ͨ��ID 4���ֽ�
	BYTE	bIntervU;						//��������λ
	WORD 	wRecSaveNum;					//��¼�������
	BYTE	bPnChar[2];						//Ҫִ�д�����Ĳ�����������,���ԭ��bPnType	[PNCHAR_MAX]
}TCommTaskCtrl;		//��ͨ������ƽṹ,�����븺�ص���ͨ������ƽṹͳһΪһ��

typedef struct
{
	BYTE	bSrcFn;		//һ��Сʱ�����������ʶ
	BYTE	bDstFn;		//��Ӧ�Ķ��������������ʶ
}TMaptoClass2Fn;

#endif //TASKSTRUCT_H