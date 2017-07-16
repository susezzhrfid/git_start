/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrStruct.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֳ�����ṹ����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��1��
 *********************************************************************************************************/
#ifndef MTRSTRUCT_H
#define MTRSTRUCT_H
#include "TypeDef.h"
#include "Comm.h"
#include "MtrConst.h"
#include "TypeDef.h"
#include "ComStruct.h"


//TCctRdCtrl���Ʊ�־λ����
#define PH1			(1<<0)		//���������
#define PH3			(1<<1)		//����3�����
#define M97			(1<<2)		//����97����
#define M07			(1<<3)		//����07����

#define DMCUR		(1<<4)		//DAY&MONTH FROZEN CURRENT �����¶������óɳ����ǰ����ʱ����
#define DMMTR		(1<<5)		//DAY&MONTH FROZEN METER FRZ �����¶������óɳ������ʱ����
#define DFCUR		(1<<6)		//DAYFLG FROZEN CURRENT �ڳ����ն������óɳ����ǰ����ʱ����
#define DFMTR		(1<<7)		//DAYFLG FROZEN METER �ڳ����ն������óɳ������ʱ����
#define PFCUR		(1<<8)		//PROFILE FROZEN CURRENT ���������óɳ����ǰ����ʱ����
#define PFMTR		(1<<9)		//PROFILE FROZEN METER ���������óɳ������ʱ����
#define DEMDMCUR	(1<<10)		//DEMAND DAY&MONTH FROZEN CURRENT ���������¶������óɳ����ǰ����ʱ����
#define DEMDMMTR	(1<<11)		//DEMAND DAY&MONTH FROZEN METER FRZ ���������¶������óɳ������ʱ����
#define DEMDFCUR	(1<<12)		//DEMAND DAYFLG FROZEN CURRENT �����ڳ������¶������óɳ����ǰ����ʱ����
#define DEMDFMTR	(1<<13)		//DEMAND DAYFLG FROZEN METER FRZ �����ڳ������¶������óɳ������ʱ����
#define MSETT		(1<<14)		//MONTH FROZEN SETT ���¶������óɳ�������ʱ����
#define DEMMSETT	(1<<15)		//MONTH FROZEN SETT �������¶������óɳ�������ʱ����

#define PH13		(PH1|PH3)	//������/3�����
#define M0797		(M97|M07)	//����07/97����

#define MTR_FN_MAX	8

#define ALLPN		0xff
#define ERCPN		161 //����¼���8��ʼ213+8>FN_MAX && 213+41<255

typedef struct{
	WORD  wID;					//�ڲ�ID
	BYTE  bIntervU;				//��������λ
	BYTE  bIntervV;				//������ֵ
	WORD  wInterID;				//����ʼʱ��ID(����Ϊ0����ȡtmStartTimeΪ��ʼʱ��)
	TTime tmStartTime;			//��ʼʱ��
	//BYTE  bC1Fn[MTR_FN_MAX];	//��wID��ص�1��FN,��һ���ֽڱ�ʾFN�ĸ���,�������һ����FN
	//							//��������������Ӧ��FN�����F38���ó�������wID��Ҫ��
	BYTE  bFn[MTR_FN_MAX];		//��wID��ص�1��FN,��һ���ֽڱ�ʾFN�ĸ���,�������һ����FN
								//��������������Ӧ��FN�����F39���ó�������wID��Ҫ��
								//dwFnFlg[n]==ALLPN ���в�����
								//dwFnFlg[n]==PNCHAR_VIP �ص㻧������
	DWORD dwFnFlg[MTR_FN_MAX];	//���Ʊ�־λ,dwC2Flg[]��ÿ��Ԫ����bC2Fn[]һһ��Ӧ
								//�ֱ����ÿ��FN��ʲô����½��г���
}TMtrRdCtrl;	//��������ƽṹ

//������
typedef struct
{
	WORD	wPn;				//�������		
	BYTE	bProId;				//Э���
	//BYTE	bSubProId;			//��Э���
	BYTE	bRateNum;			//������
	BYTE	bAddr[6];			//���ַ	
	BYTE	bPassWd[6];			//����
	BYTE	bRateTab[4];		//����˳��

	TCommPara CommPara;			//����ͨ�Ų���	
}TMtrPara; 

#define MTR_COMSET_DEFAULT	0x4b	//����ȱʡ���� 1200,E,8,1

typedef struct{
	BYTE* pbData;		
	WORD  wLen;
}TPnTmp;	//�����㵼�뵼������ʱ����

#endif	//MTRSTRUCT_H