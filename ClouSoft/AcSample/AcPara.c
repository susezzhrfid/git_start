/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AcSample.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�ֶԽ�������������װ�غͱ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2008��5��
 * ��    ע: ���ļ���Ҫ�������θ��汾������Ĳ�����
 *********************************************************************************************************/
#include "AcSample.h"
#include "Sample.h"
#include "ComAPI.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "LibDbAPI.h"

//extern	TSHJPara g_tSHJPara;	//Short-Circuit Judgement parameters

//��������������·���ϲ���
void LoadSHJPara(WORD wPn, TSHJPara *pSHJPara)
{
	if (pSHJPara == NULL)
		return;
/*
	BYTE bBuf[128];

���ϵ�����ֵ
������ֵ
����ʱ��
��һ����������Сʱ��
��һ�����������ʱ��
�ڶ������������ʱ��
�������ͻ������ֵ
�������Խ�޸澯��ֵ
�����ѹԽ�޸澯��ֵ
�������Խ�޷���/�ָ�ʱ����
�����ѹԽ�޷���/�ָ�ʱ����
�����ѹ��ֵ
���������ֵ
����ң�ų���ʱ��
*/
	/*ReadItemEx(BN0, PN0, 0x895f, bBuf);
    memset(bBuf, 0, sizeof(bBuf));
    
	pSHJPara->dwIh = BcdToDWORD(bBuf, 4);//���ϵ�����ֵ
	pSHJPara->dwIno = BcdToDWORD(bBuf+4, 4);//������ֵ
	pSHJPara->wT2 = BcdToDWORD(bBuf+8, 2);//�غ�բʱ������ֵ
	pSHJPara->wT1Min = BcdToDWORD(bBuf+10, 2);//����ʱ������ֵ(��Сֵ)
	pSHJPara->wT1Max = BcdToDWORD(bBuf+12, 2);//����ʱ������ֵ(���ֵ)
	pSHJPara->wT3 = BcdToDWORD(bBuf+14, 2);//�����ʱ������ֵ
	pSHJPara->dwI0 = BcdToDWORD(bBuf+16, 4);//�������ͻ��������ֵ
    pSHJPara->dwZeroI = BcdToDWORD(bBuf+20, 4);//�����������ֵ
    if (pSHJPara->dwZeroI == 0)
    	pSHJPara->dwZeroI = 1000;

    pSHJPara->dwZeroU = BcdToDWORD(bBuf+24, 4);//�����ѹ����ֵ
    pSHJPara->wInOverTime = BcdToDWORD(bBuf+28, 2);//����������������ӳ�ʱ��
    pSHJPara->wUnOverTime = BcdToDWORD(bBuf+30, 2);//�����ѹ���������ӳ�ʱ��
    pSHJPara->dwUOff = BcdToDWORD(bBuf+32, 4);//�����ѹ��ֵ
    if (pSHJPara->dwUOff == 0)
        pSHJPara->dwUOff = 20*100;

    pSHJPara->dwIOff = BcdToDWORD(bBuf+36, 4);//���������ֵ
    if (pSHJPara->dwIOff == 0)
        pSHJPara->dwIOff = 5*100;
    pSHJPara->wOffTime = BcdToDWORD(bBuf+40, 2);//�������ʱ��
	if (pSHJPara->wOffTime == 0)
		pSHJPara->wOffTime = 1;

	if (pSHJPara->wT1Min <= 20) 
	{
		pSHJPara->wT1Min = 250;
	}
	if (pSHJPara->dwIh < 50) 
	{
		pSHJPara->dwIh = 5*1000;
	}*/
}

//���ɲ����ĳ�ʼ��
bool AcLoadPara(WORD wPn, TAcPara* pAcPara)
{

	//WORD i;
	BYTE bBuf[64];
	//int iRet;

	memset(pAcPara, 0, sizeof(TAcPara));
	memset(bBuf, 0, sizeof(bBuf));
	pAcPara->wPoint = wPn;

	//����ģʽ
	/*ReadItemEx(BN0, PN0, 0x8903, bBuf);  //����ģʽ:0:����ģʽ;1������ģʽ;2:����ģʽ������������ģʽ
	pAcPara->wWorkMode = bBuf[0];*/
    //pAcPara->wWorkMode = 0;
/*
���ɻ�������
���ѹ
�����
��ѹ�ϸ�����
��ѹ�ϸ�����
�����ѹ��ƽ����ֵ
�����ѹ��ƽ����ֵ
Ƶ������
Ƶ������
�ܻ����ѹ����
�ܻ����������
���߷�ʽ                                                    
��Ƿ���1��ʾ�ǶȰ���˳ʱ�뷽���ʾ,Ua,Ub,Uc�ֱ�Ϊ0,120,240 */

	/*ReadItemEx(BN0, PN0, 0x894f, bBuf);
    memset(bBuf, 0, sizeof(bBuf));
    
	pAcPara->dwUn = BcdToDWORD(&bBuf[0], 4);
	if (pAcPara->dwUn == 0)
		pAcPara->dwUn = 22000;

	pAcPara->dwIn = BcdToDWORD(&bBuf[4], 4);
	if (pAcPara->dwIn == 0)
		pAcPara->dwIn = 5000;

	pAcPara->bConnectType = bBuf[40];	//���߷�ʽ
    if ((pAcPara->bConnectType != CONNECT_3P3W) || (pAcPara->bConnectType != CONNECT_1P)) 
        pAcPara->bConnectType = CONNECT_3P4W;

	pAcPara->bAngleClockwise = bBuf[41]; //1��ʾ�ǶȰ���˳ʱ�뷽���ʾ,Ua,Ub,Uc�ֱ�Ϊ0,120,240

	if (pAcPara->wWorkMode == 0) 
	{
		pAcPara->fCalcuHarmonic = true;
		pAcPara->wHarmNum = 15;
	} 
	else 
	{
		pAcPara->fCalcuHarmonic = false;
		pAcPara->wHarmNum = 0;
	}
*/
    pAcPara->fCalcuHarmonic = false;
    pAcPara->wHarmNum = 0;
	//pAcPara->wWorkMode = 6;   //TODO:���ݿ�������ó�ʼ��������ֻ��Ϊ�˲��Է���
	pAcPara->bConnectType = CONNECT_3P4W; 
	//LoadSHJPara(wPn, &g_tSHJPara);

	return true;
}

