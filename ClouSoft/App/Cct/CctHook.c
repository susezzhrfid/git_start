/*********************************************************************************************************
* Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
*
* �ļ�����: CctHook.cpp
* ժ    Ҫ: ���ļ���Ҫʵ�ּ����ҹ��ӿ�,��д�Ĺҹ���Ҫ���Լ�����,������ͨ�����
* ��ǰ�汾: 1.1
* ��    ��: 
* �������: 2009��9��
* ��    ע: 
*********************************************************************************************************/
//#include "stdafx.h"
#include "CctAPI.h"
#include "MtrAPI.h"
#include "CctHook.h"
#include "ComAPI.h"
#include "LibDbConst.h"
#include "LibDbAPI.h"
#include "DbConst.h"

//����:ȡ���ز��ڵ�ĵ�ַ,����ͨ���ɼ��ն˵ĵ��,��ȡ�ɼ��ն˵ĵ�ַ
//����:@wPn �������
//	   @pbAddr �������ص�ַ
//����:����ɹ��򷵻�true,���򷵻�false
bool GetPlcNodeAddr(WORD wPn, BYTE* pbAddr)
{
	BYTE bLink;
	BYTE bBuf[12];

	if (!IsCctPn(wPn))
		return false;
	
	if (ReadItemEx(BN0, wPn, 0x8902, bBuf)<=0)  //�������ַ
		return false;

	ReadItemEx(BN0,wPn,0x8909,bBuf+6);  //�������Ӧ�ɼ��ն˵�ַ

// 	if (bLink & LINK_ACQ)
// 	{
// 		memcpy(pbAddr, bBuf+6, 6);
// 	}
// 	else
	{
		memcpy(pbAddr, bBuf, 6);
	}

	//��ַ��Ч���ж�
	if(IsAllAByte(pbAddr, 0, 6) || IsAllAByte(pbAddr, 0xff, 6))
		return false;
	else
		return true;
}

// ����:ȡ�㲥Уʱ����ʱ���֡
// BYTE CctGetAdjTimeFrm(BYTE* pbFrm)
// {
// 	const BYTE bAddr[6] = {0x99, 0x99, 0x99, 0x99, 0x99, 0x99};
// 	TTime time;
// 
// 	GetCurTime(&time);
// 	pbFrm[10] = ByteToBcd(time.nSecond&0xff);
// 	pbFrm[11] = ByteToBcd(time.nMinute&0xff);
// 	pbFrm[12] = ByteToBcd(time.nHour&0xff);
// 	pbFrm[13] = ByteToBcd(time.nDay&0xff);
// 	pbFrm[14] = ByteToBcd(time.nMonth&0xff);
// 	pbFrm[15] = ByteToBcd((time.nYear-2000)&0xff);
// 
// 	return (BYTE )Make645Frm(pbFrm, bAddr, 0x08, 6);
// }

//����:	��ȡ����ַ,
//����:	@wPn �������
//		@pbAddr �������ص�ַ
//����:	����ɹ��򷵻�true,���򷵻�false
//��ע:	�����ز���, �ز������ַ�����ַһ��,
//		���ڲɼ���ģ��,����ȡĿ�ĵ���ַ.
bool GetCctMeterAddr(WORD wPn, BYTE* pbAddr)
{
	BYTE bBuf[100];
	if (!IsCctPn(wPn))
		return false;

	if (ReadItemEx(BN0, wPn, 0x8902, bBuf)<=0)
		return false;

	memcpy(pbAddr, bBuf, 6);
	return true;
}

//����:ȡ���ز�����������λ,�������ɼ����ڵ�,�����������ز���ڵ��ͨ���ɼ����ɼ���485��
void GetPlcPnMask(BYTE* pbPnMask)
{
	ReadItemEx(BN17, PN0, 0x7005, pbPnMask); 
}

//����:ȡ���ز�����������λ�Ķ�ָ��,�������ɼ����ڵ�,�����������ز���ڵ��ͨ���ɼ����ɼ���485��
const BYTE* GetCctPnMask()
{
	return GetItemRdAddr(BN17, PN0, 0x7005); 
}

//����:ͨ���ز����ַת��Ϊ�������
//����:@pb485Addr ����ַ,
// 				  ���ڶ����ز���ڵ���ǵ���ַҲ���ز������ַ,
// 				  ����ͨ���ɼ����ɼ���485��,ֻ�ǵ���ַ
// 	   @pbAcqAddr �ɼ�����ַ,Ŀǰ��������Ҫ���������Ĳ���,ֻ�����������Ľӿ�
// 				  ���ڶ����ز���ڵ�����Ͳ��ô�,��ΪNULL����
// 				  ����ͨ���ɼ����ɼ���485��,���ݲɼ�����ַ
//����:����ҵ�ƥ��Ĳ������򷵻ز������,���򷵻�0
WORD PlcAddrToPn(const BYTE* pb485Addr, const BYTE* pbAcqAddr)
{
	BYTE bMtrAddr[6];
	WORD wPn;


	for (wPn = 1; wPn < POINT_NUM; wPn++)
	{
		if(GetPlcNodeAddr(wPn, bMtrAddr))
		{
			if (memcmp(bMtrAddr, pb485Addr, 6) == 0)
			{
				return wPn;
			}
		}
		else
			continue;
	}

// 	const BYTE* pbPnMask = GetCctPnMask();	//ȡ���ز�����������λ,�������ɼ����ڵ�,�����������ز���ڵ��ͨ���ɼ����ɼ���485��
// 	if (pbPnMask == NULL)
// 		return 0;
// 
// 	wPn = 0;
// 	while (1)
// 	{
// 		wPn = SearchPnFromMask(pbPnMask, wPn);	//�����ѳ��Ĳ����㶼���ز���
// 		if (wPn >= POINT_NUM)
// 			break;
// 
// 		GetCctMeterAddr(wPn, bMtrAddr);
// 		if (memcmp(pb485Addr, bMtrAddr, 6) == 0)
// 		{
// 			return wPn;
// 		}
// 
// 		wPn++;
// 	}

	return 0;
}

//����:�Ƿ��ڳ���ʱ����,����ͣ����ͻָ�����Ĺ����ñ�������ʵ��
bool IsInReadPeriod()
{
	WORD  wMinStart = 0;
	WORD  wMinEnd = 0;

	return GetCurRdPeriod(&wMinStart, &wMinEnd);
}

//����:ȡ��ǰ����ʱ�ε���ʼʱ��ͽ���ʱ��
bool GetCurRdPeriod(WORD* pwMinStart, WORD* pwMinEnd)
{
	TTime tmNow;
	WORD  wMinNow = 0;
	WORD  wMinStart = 0;	//����ʱ����ʼʱ��
	WORD  wMinEnd = 0;		//����ʱ�ν���ʱ��
	BYTE bBuf[120];

	//GetCurTime(&tmNow);
	//wMinNow = (WORD )tmNow.nHour*0x100 + tmNow.nMinute;

	//if (ReadItemEx(BN0, PORT_CCT_PLC, 0x021f, bBuf) > 0)	//FN33 ����ͨ�Ŷ˿ں�
	//{					//Ŀǰ�ٶ�ֻ���ز�ͨ�����õ�����ʱ��,���Թ̶��ö˿ں�PORT_CCT_PLC
	//	if (bBuf[0] == 0)	//û������
	//	{
	//		*pwMinStart = 0;
	//		*pwMinEnd = 24*0x100 + 0;
	//		return true;
	//	}

	//	if (bBuf[13] > 24)
	//		return false;

	//	for (WORD i=0; i<bBuf[13]; i++)
	//	{
	//		wMinStart = BcdToByte(bBuf[14+i*4]) + (WORD )BcdToByte(bBuf[15+i*4])*0x100;
	//		wMinEnd = BcdToByte(bBuf[16+i*4]) + (WORD )BcdToByte(bBuf[17+i*4])*0x100;

	//		if (wMinNow>=wMinStart && wMinNow<wMinEnd)
	//		{	
	//			*pwMinStart = wMinStart;
	//			*pwMinEnd = wMinEnd;
	//			return true;
	//		}
	//	}
	//}

	//*pwMinStart = 0;
	//*pwMinEnd = 0;
	//return false;	
	*pwMinStart = 0;
	*pwMinEnd = 24*0x100 + 0;
	return true;
}