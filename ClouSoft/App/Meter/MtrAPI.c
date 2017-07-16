/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MeterAPI.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�ֳ���Ĺ����ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
 *********************************************************************************************************/
#include "MtrAPI.h"
#include "DbGbAPI.h"
#include "DbAPI.h"
#include "FaAPI.h"
#include "DbConst.h"
#include "ComAPI.h"
#include "MtrStruct.h"
#include "SysDebug.h"
#include "DrvCfg.h"
#include "MtrCtrl.h"
#include "ExcTask.h"
#include "GbPro.h"

DWORD GbValToBaudrate(BYTE val)
{
	static const DWORD dwBaudrate[] = {0, CBR_600, CBR_1200, CBR_2400, 
								  CBR_4800, CBR_4800, CBR_9600, CBR_19200};
	if (val <= 7)
		return dwBaudrate[val];
	else
		return CBR_1200;
}

BYTE GbBaudrateToVal(DWORD dwBaudRate)
{
	BYTE i;
	static const DWORD dwBaudrate[] = {0, CBR_600, CBR_1200, CBR_2400, 
								  CBR_4800, CBR_4800, CBR_9600, CBR_19200};
	for (i=0; i<sizeof(dwBaudrate)/sizeof(DWORD); i++)
	{
		if (dwBaudrate[i] == dwBaudRate)
			return i;
	}
	return 0;
}

BYTE GbValToParity(BYTE val)
{
	static const BYTE bParityTab[] = {NOPARITY, ODDPARITY, EVENPARITY}; 

	if (val < 3)
		return bParityTab[val];
	else	
		return NOPARITY;
}


BYTE GbValToStopBits(BYTE val)
{
	static const BYTE bStopBitsTab[] = {ONESTOPBIT, TWOSTOPBITS, ONE5STOPBITS};
	if (val>0 && val<=3)
		return bStopBitsTab[val-1];
	else
		return ONESTOPBIT;
}

BYTE GbValToByteSize(BYTE val)
{
	if (val>=5 && val<=8)
		return val;
	else
		return 8;
}

//����:�������㴮�ڲ�����ȱʡ����
void GetDefaultCommPara(TMtrPara* pMtrPara)
{	
	TCommPara* pCommPara = &pMtrPara->CommPara;
	switch (pMtrPara->bProId)
	{	
#ifdef PROTOCOLNO_HT3A
		case PROTOCOLNO_HT3A://�麣��ͨ		
			if (pCommPara->dwBaudRate == 0) //�����Լ,��������Ϊ0ʱ,ȡ��ȱʡ����
				pCommPara->dwBaudRate = CBR_1200;//������
			pCommPara->bParity = 1;//��У��
			pCommPara->bByteSize = 8;//����λ
			break;
#endif

#ifdef PROTOCOLNO_ABB2
		case PROTOCOLNO_ABB2://ABBԲ��
			//������F10
			if (pCommPara->dwBaudRate == 0) //�����Լ,��������Ϊ0ʱ,ȡ��ȱʡ����
				pCommPara->dwBaudRate = CBR_1200;//������
			pCommPara->bParity = 0;//��У��
			pCommPara->bByteSize = 8;//����λ	
			break;	
#endif			
#ifdef PROTOCOLNO_ABB
		case PROTOCOLNO_ABB://ABB����
			//������F10
			if (pCommPara->dwBaudRate == 0) //�����Լ,��������Ϊ0ʱ,ȡ��ȱʡ����
				pCommPara->dwBaudRate = CBR_1200;//������
			pCommPara->bParity = 0;//��У��
			pCommPara->bByteSize = 8;//����λ			
			break;	
#endif	
		
#ifdef PROTOCOLNO_EDMI
		case PROTOCOLNO_EDMI://�����
			//������F10
			if (pCommPara->dwBaudRate == 0) //�����Լ,��������Ϊ0ʱ,ȡ��ȱʡ����
				pCommPara->dwBaudRate = CBR_9600;//������
			pCommPara->bParity = 0;//��У��
			pCommPara->bByteSize = 8;//����λ	
			break;
#endif	
			
#ifdef PROTOCOLNO_1107
		case PROTOCOLNO_1107://	A1700 1107��Լ
			//������F10
			if (pCommPara->dwBaudRate == 0) //�����Լ,��������Ϊ0ʱ,ȡ��ȱʡ����
				pCommPara->dwBaudRate = CBR_1200;//������
			pCommPara->bParity = 2;//żУ��
			pCommPara->bByteSize = 7;//����λ
			break;
#endif				

#ifdef PROTOCOLNO_LANDIS
		case PROTOCOLNO_LANDIS://������1107��Լ
			if (pCommPara->dwBaudRate == 0) //�����Լ,��������Ϊ0ʱ,ȡ��ȱʡ����
				pCommPara->dwBaudRate = CBR_300;//������
			pCommPara->bParity = 2;//żУ��
			pCommPara->bByteSize = 7;//����λ
			break;
#endif	

#ifdef PROTOCOLNO_LANDIS_ZMC
		case PROTOCOLNO_LANDIS_ZMC://������ZMC��Լ
			//������F10
			if (pCommPara->dwBaudRate == 0) //�����Լ,��������Ϊ0ʱ,ȡ��ȱʡ����
				pCommPara->dwBaudRate = CBR_9600;//������
			pCommPara->bParity = 0;//��У��
			pCommPara->bByteSize = 8;//����λ	
			break;
#endif

#ifdef PROTOCOLNO_DLMS
		case PROTOCOLNO_DLMS://������DLMS��Լ
			//������F10
			if (pCommPara->dwBaudRate == 0) //�����Լ,��������Ϊ0ʱ,ȡ��ȱʡ����
				pCommPara->dwBaudRate = CBR_9600;//������
			pCommPara->bParity = 0;//��У��
			pCommPara->bByteSize = 8;//����λ	
			break;
#endif				

#ifdef PROTOCOLNO_LANDIS_DLMS
		case PROTOCOLNO_LANDIS_DLMS://������DLMS��Լ
			//������F10
			if (pCommPara->dwBaudRate == 0) //�����Լ,��������Ϊ0ʱ,ȡ��ȱʡ����
				pCommPara->dwBaudRate = CBR_300;//������
			pCommPara->bParity = 2;//żУ��
			pCommPara->bByteSize = 7;//����λ	
			break;
#endif

#ifdef PROTOCOLNO_HND
		case PROTOCOLNO_HND://�������Լ
			if (pCommPara->dwBaudRate == 0) //�����Լ,��������Ϊ0ʱ,ȡ��ȱʡ����
				pCommPara->dwBaudRate = CBR_1200;//������
			pCommPara->bParity = 0;//��У��
			pCommPara->bByteSize = 8;//����λ	
			break;
#endif	

#ifdef PROTOCOLNO_EMAIL
		case PROTOCOLNO_EMAIL://EMAIL��
			//������F10
			if (pCommPara->dwBaudRate == 0) //�����Լ,��������Ϊ0ʱ,ȡ��ȱʡ����
				pCommPara->dwBaudRate = CBR_1200;//������
			pCommPara->bParity = 0;//��У��
			pCommPara->bByteSize = 8;//����λ	
			break;	
#endif	

#ifdef PROTOCOLNO_DLT645_V07
		case PROTOCOLNO_DLT645_V07://2007��645��
			//������F10
			if (pCommPara->dwBaudRate == 0) //�����Լ,��������Ϊ0ʱ,ȡ��ȱʡ����
				pCommPara->dwBaudRate = CBR_2400;//������
			pCommPara->bParity = 2;//żУ��
			pCommPara->bByteSize = 8;//����λ	
			break;	
#endif	

	default:
		if (pCommPara->dwBaudRate == 0) //�����Լ,��������Ϊ0ʱ,ȡ��ȱʡ����
			pCommPara->dwBaudRate = CBR_1200;//������
		pCommPara->bParity = 2;//żУ��
		pCommPara->bByteSize = 8;//����λ	
		break;
	}
	pCommPara->bStopBits = GbValToStopBits(1);//ֹͣλ =1

	pMtrPara->bRateTab[0] = 1;//����˳��	//47
	pMtrPara->bRateTab[1] = 2;			//48
	pMtrPara->bRateTab[2] = 3;			//49
	pMtrPara->bRateTab[3] = 4;			//50	
}

void GetLogicPortNum(WORD* pwNum, WORD* pwMin, WORD* pwMax)
{
	*pwNum = LOGIC_PORT_NUM;
	*pwMin = LOGIC_PORT_MIN;
	*pwMax = LOGIC_PORT_MAX;
}


/*
//����:������߼��˿ڵ�����˿ڵ�ӳ��
int MeterPortToPhy(BYTE bPortNo)
{
	BYTE bPortFun = PORT_FUN_RDMTR;
	if (bPortNo == LOGIC_PORT_MIN)
		return COMM_METER2;	    
	else if (bPortNo == (LOGIC_PORT_MIN+1))
	{
		ReadItemEx(BN10, PN0, 0xa180, (BYTE*)&bPortFun);
		//3�����ڵ������
		if (bPortFun != PORT_FUN_DEBUG)
			return COMM_METER;	
		else
			return -1;
	}
    else
    	return -1;
}*/



int MeterPortToPhy(BYTE bMeterPort)
{
	BYTE b = 0, b1 = 0, b3 = 0, bPortFun=0;
	ReadItemEx(BN0, PN0, 0x8710, (BYTE *)&b);			//�����
	ReadItemEx(BN0, PN0, 0x8720, (BYTE *)&b1);		//������
	//ReadItemEx(BN0, PN0, 0x8750, (BYTE *)&b2);			//���Կ�

//ReadItemEx(BANK7,POINT0, 0x7510, (BYTE *)&b3);//485˳�� //0-����  �ҳ���    1-�󳭱�  �Ҽ���

	if (bMeterPort == PORT_CCT_PLC || bMeterPort == PORT_CCT_WIRELESS)
		return COMM_METER3;
	
	b3=0;	//����չ����
	if (b3 == 0) 
	{
		if(bMeterPort == 1)
		{
			if(b1 == 0) // ���Կ�û�б�ռ��
			{
				ReadItemEx(BN10, PN0, 0xa180, (BYTE*)&bPortFun);	//�������ã��Ƿ����ڵ������
				if (bPortFun != PORT_FUN_DEBUG)
					return COMM_METER;
			}
		}
		else
		{
			if (b == 0)//�����û�б�ռ��
				return COMM_METER2;
		}
	}
	else
	{
		if(bMeterPort == 1)
		{
			if(b1 == 0)//�����û�б�ռ��
				return COMM_METER2;
		}
		else
		{
			if (b == 0)//���Կ�û�б�ռ��
				return COMM_METER;
		}
	}

	return -1;//û�оͷ��ظ���
}


BYTE MeterProtoLocalToInner(BYTE bProto)
{
	switch (bProto)
	{
	case 0x00:
	case 0x02:
		return PROTOCOLNO_DLT645;
	case 0x01:
	case 0x03:
		return PROTOCOLNO_DLT645_V07;
	/*case 0x04:	   
		return PROTOCOLNO_WS;
	case 0x05:
	case 0x06:
		return PROTOCOLNO_LANDIS;
	case 0x07:
		return PROTOCOLNO_EDMI;
	case 0x08:
		return PROTOCOLNO_ABB;
	case 0x09:
		return PROTOCOLNO_DLMS;
	case 0x0A:
		return PROTOCOLNO_EMAIL;
	case 0x0B:
		return PROTOCOLNO_HND;
	case 0x0C:
		return PROTOCOLNO_LANDIS_DLMS;*/
	default:
		return PROTOCOLNO_DLT645_V07;
	}
}


//����:���ɿ��ƹ���2005Э��ĵ�������ʼ��
//����:@pMtrInf	ָ���ŵ���������ĵ������ṹָ��
//	   @&bNum	���ص���Ч����������Ŀ
//����:�ɹ��򷵻�true		
bool GetMeterPara(WORD wPn, TMtrPara* pMtrPara)
{	
	int iPort=0;
	BYTE bProId, bProp = 0;
	BYTE bBuf[32];

	if (wPn == PN0)//������0����Ҫȥ����
		return false;

	if (ReadItemEx(BN0, wPn, 0x8900, bBuf) <= 0)
		return false;

	if (bBuf[0] == 0)
		return false;

	if (ReadItemEx(BN0, wPn, 0x8901, &bProp) <= 0)
		return false;

	if (bProp != PN_PROP_METER)
		return false;

	pMtrPara->wPn = wPn;
	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
		return false;

	memcpy(pMtrPara->bAddr,bBuf,6);//�˿ڵ�ַ
	if (ReadItemEx(BN0, wPn, 0x8903, bBuf) <= 0)
		return false;	

	bProId = bBuf[0];
	pMtrPara->bProId = MeterProtoLocalToInner(bProId);
	//ReadItemEx(BANK3, POINT0, 0x30d0+wPn, bBuf);
	//pMtrPara->bSubProId = bBuf[0];

	if (ReadItemEx(BN0, wPn, 0x890b, bBuf) <= 0)
		return false;

	pMtrPara->CommPara.dwBaudRate = ValToBaudrate(bBuf[0]);//������
	pMtrPara->CommPara.bParity = ValToParity(bBuf[1]);//У��λ
	pMtrPara->CommPara.bByteSize = ValToByteSize(bBuf[2]);//����λ��
	pMtrPara->CommPara.bStopBits = ValToStopBits(bBuf[3]);//ֹͣλ

	if (ReadItemEx(BN0, wPn, 0x8930, bBuf) <= 0)
		return false;

	memcpy(pMtrPara->bPassWd, bBuf, 8);//����	

	if (ReadItemEx(BN0, wPn, 0x890a, bBuf) <= 0) //�˿ں�
		return false;
	//DTRACE(DB_METER, ("GetMeterPara :Point%d 0x8901=%d,8930=%d,8932=%d,8931=%d,8903=%d,8905=%d\n", wPn,bProp,pMtrInf->CommPara.bParity, pMtrInf->CommPara.bStopBits, pMtrInf->CommPara.bByteSize,pMtrInf->bProId, pMtrInf->CommPara.dwBaudRate));

	iPort = MeterPortToPhy(bBuf[0]); //����:������߼��˿ڵ�����˿ڵ�ӳ��
	if (COMM_METER3 == iPort)
	{
		pMtrPara->CommPara.dwBaudRate = CBR_9600;//������
		pMtrPara->CommPara.bParity = EVENPARITY;//У��λ
		pMtrPara->CommPara.bByteSize = 8;//����λ��
		pMtrPara->CommPara.bStopBits = 1;//ֹͣλ
	}

	if (iPort < 0)
	{
		DTRACE(DB_METER, ("GetMeterPara : fail to map port %d to physic\n", iPort));
		return false;
	}

	pMtrPara->CommPara.wPort = (WORD )iPort;
	/*pMtrInf->wMeterPort = bBuf[0]+1;
	//�������ֻ��645�����ã�Ҫ�����ĵط�ͳһ����
	if (pMtrInf->bProId == PROTOCOLNO_DLT645_V07)//2007��645
	{
		pMtrInf->bExtValid = 0xff;
		pMtrInf->bWakeUpCtrl = 4;//�����ַ�
		pMtrInf->bEnergyPtPos = 2;//����С��λ
		pMtrInf->bNeedPtPos = 4;//����С��λ
		pMtrInf->bVolPtPos = 1;//��ѹС��λ
		pMtrInf->bCurPtPos = 3;//����С��λ
		pMtrInf->bActPowerPtPos = 4;//�й�����С��λ
		pMtrInf->bReActPowerPtPos = 4;//�޹�����С��λ
		pMtrInf->bPowerFactorPtPos = 3;//��������С��λ
	}
	else//97��645
	{
		pMtrInf->bExtValid = 0xff;
		pMtrInf->bWakeUpCtrl = 0;//�����ַ�
		pMtrInf->bEnergyPtPos = 2;//����С��λ��   Ĭ�� 2
		pMtrInf->bNeedPtPos = 4;////����С��λ��   Ĭ�� 4
		pMtrInf->bVolPtPos = 0;//��ѹС��λ��  Ĭ�� 0
		pMtrInf->bCurPtPos = 2;//����С��λ��  Ĭ�� 2
		pMtrInf->bActPowerPtPos = 4;//�й�����С��λ��/  Ĭ��4
		pMtrInf->bReActPowerPtPos = 2;//�޹�����С��λ�� Ĭ��2
		pMtrInf->bPowerFactorPtPos = 3;//��������С��λ��/  Ĭ�� 3
	}

	ReadItemEx(BANK1, POINT0, 0x2050+wPn, pMtrInf->bRateTab);  //0x2050 4 ���->645���ʶ��ձ�0��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	if (pMtrInf->bRateTab[0]==0 || pMtrInf->bRateTab[0]>4)
	{
		pMtrInf->bRateTab[0] = 1;
		pMtrInf->bRateTab[1] = 2;
		pMtrInf->bRateTab[2] = 3;
		pMtrInf->bRateTab[3] = 4;
	}	*/

	//if (pMtrPara->CommPara.dwBaudRate == 0) //�����Լ,��������Ϊ0ʱ,ȡ��ȱʡ����
	{
		GetDefaultCommPara(pMtrPara);
	}

	return true;
}


//����:ȡ������
/*BYTE GetMeterInterv(WORD wPn)
{
#ifdef PRO_698
	BYTE bBuf[110];
	BYTE bMeterInterv = 0;
	BYTE bPort = 2;
	BYTE bMode = 0;
	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
	{
		bMeterInterv = 60;	//15
		return bMeterInterv;
	}
	bPort = bBuf[MTR_PORT_OFFSET]&0x1f;
	if (ReadItemEx(BN0, bPort-1, 0x021f, bBuf) > 0)	//F33
	{	
		bMeterInterv = bBuf[9]; //8 bBuf[20]; //���û������,ϵͳ��Ĭ��ֵΪ0,������Զ�ȡĬ��ֵ
	}
	else
	{
		bMeterInterv = 60; //15
	}

	if (bMeterInterv==0 || bMeterInterv>60)
		bMeterInterv = 60; //15
	else if (bMeterInterv > 30)
		bMeterInterv = 60;
	else if (bMeterInterv > 15)
		bMeterInterv = 30;
	else if (bMeterInterv > 10)
		bMeterInterv = 15;

	ReadItemEx(BN2, PN0, 0x2040, &bMode);
	if (bMode==0 && bMeterInterv<60)
		bMeterInterv = 60; //��������Чʱ��ǿ�Ƴ�������С��60����

	return bMeterInterv;
#else
	return GetMeterInterv();
//#endif
}

//����:ȡ������
bool GetPnDate(WORD wPn, BYTE* pbBuf)
{
	BYTE bPort;
	BYTE bBuf[MTR_PARA_LEN];
    if (sizeof(bBuf) < GetItemLen(BN0, 0x8902))
    {
        DTRACE(DB_METER, ("bBuf is too small\r\n"));
        return false;
    }
	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
	{
		bPort = PN2;
	}
	else
	{
		bPort = bBuf[MTR_PORT_OFFSET]&0x1f;	//4
	}

	return GetMtrDate(bPort, pbBuf);
}*/
/*
//����:ȡ������
bool GetMtrDate(BYTE bPort, BYTE* pbBuf)
{
	static const BYTE bDefault[6] = {0x01, 0x00, 0x00, 0x00, 0x10, 0x00} ;
	BYTE bBuf[110];
    
    if (sizeof(bBuf) < GetItemLen(BN0, 0x021f))
    {
        DTRACE(DB_METER, ("bBuf is too small\r\n"));
        return false;
    }
	if (ReadItemEx(BN0, bPort-1, 0x021f, bBuf) > 0)	//F33
	{	
		if (IsAllAByte(&bBuf[3], 0, 6)) //û������ 2
			memcpy(pbBuf, bDefault, 6);
		else
			memcpy(pbBuf, &bBuf[3], 6);
	}
	else
	{
		memcpy(pbBuf, bDefault, 6);
	}
	return true;
}*/

//����:	��ȡ����ַ,
//����:	@wPn �������
//		@pbAddr �������ص�ַ
//����:	����ɹ��򷵻�true,���򷵻�false
//��ע:	
bool GetMeterAddr(WORD wPn, BYTE* pbAddr)
{
	BYTE bBuf[MTR_PARA_LEN];

	if (!IsMtrPn(wPn))
		return false;
	if (ReadItemEx(BN0, wPn, 0x8902, bBuf)<=0)
		return false;

	memcpy(pbAddr, bBuf+MTR_ADDR_OFFSET, 6);
	return true;
}


BYTE GetRdMtrInterV(BYTE bPort)
{
	//BYTE b,b1,b3;	//20140517-2
	BYTE bMeterU;
	BYTE bBuf[8];
	//BYTE bMode;
	BYTE bRdMtrFeq;

	memset(bBuf,0,sizeof(bBuf));

	/*ReadItemEx(BN2, PN0, 0x2040, &bMode);
	if (bMode != 0)//����ģʽ
	{
		bRdMtrFeq = 1;
	}
	else*/
	{
		// 		ReadItemEx(BN1, PN0, 0x2060, bBuf);  //0x2060 1 �������,BCD,��λ����
		/*ReadItemEx(BN0, PN0,0x8710, (BYTE *)&b);
		ReadItemEx(BN0, PN0, 0x8720, (BYTE *)&b1);
		//ReadItemEx(BN0, PN0,0x8750, (BYTE *)&b2);
		//ReadItemEx(BANK7,POINT0, 0x7510, (BYTE *)&b3);//485˳�� //0-����  �ҳ���    1-�󳭱�  �Ҽ���
		b3 = 0;
		if (0==b3)
		{
			if (0==b)
				ReadItemEx(BN0, PN0, 0x8820, bBuf);
			else if (0==b1)
				ReadItemEx(BN0, PN0, 0x8821, bBuf);
			//else if (0==b2)
			//	ReadItem(PN0, 0x8822, bBuf);
		} 
		else
		{
			if (0==b)
				ReadItemEx(BN0, PN0, 0x8822, bBuf);
			else if (0==b1)
				ReadItemEx(BN0, PN0, 0x8821, bBuf);
			//else if (0==b2)
			//	ReadItemEx(BN0, PN0, 0x8820, bBuf);
		}*/

		if ((PORT_CCT_PLC != bPort && PORT_CCT_WIRELESS != bPort))
		{
			if (bPort >= LOGIC_PORT_NUM)
				bPort = 0;

			ReadItemEx(BN0, PN0, 0x8820+bPort, bBuf);
		}
		else
		{
			if (PORT_CCT_PLC == bPort)
			{
				ReadItemEx(BN0, PN0, 0x8841, bBuf);
			}

			if (PORT_CCT_WIRELESS == bPort)
			{
				ReadItemEx(BN0, PN0, 0x8842, bBuf);
			}
		}
		
		bMeterU = bBuf[5];
		switch(bMeterU+2)//��2��Ϊ����������Ӧ
		{
		case TIME_UNIT_MINUTE:
			bRdMtrFeq = bBuf[6];
			if(bRdMtrFeq>60)
				bRdMtrFeq = 60;
			break;
		case TIME_UNIT_HOUR:
		case TIME_UNIT_DAY:
		case TIME_UNIT_MONTH:
			bRdMtrFeq = 60;
			break;
		}

		if(0==bRdMtrFeq)
		{
			ReadItemEx(BN1, PN0, 0x2060, bBuf);  //0x2060 1 �������,BCD,��λ����
			bRdMtrFeq = BcdToByte(bBuf[0]);
		}
		if (bRdMtrFeq==0 ||bRdMtrFeq>60)
			bRdMtrFeq = 60;
	}

	return bRdMtrFeq;
}

//����:ȡ������
BYTE GetMeterInterv(WORD wPn)
{
	return GetRdMtrInterV(GetPnPort(wPn));
}


//����:ȡ�����в���������λ,
void GetAllPnMask(BYTE* pbPnMask)
{
	ReadItemEx(BN0, PN0, 0x056f, pbPnMask); //����������λ
}

//�����������¼�
//����: 
//����: 
//��ע: һ�����ѵ��µ���Ͳ����¼�
void PushEvtParaChanged()
{
	TTime now;
	BYTE bErcData[6], *pbBuf;

	pbBuf = bErcData;
	*pbBuf++ = 0;	//��վMAC��ַ

	*pbBuf++ = 0;//DA
	*pbBuf++ = 0;//DAG
	*pbBuf++ = 0x02;//DT
	*pbBuf++ = 0x01;//DTG

	GetCurTime(&now);

	g_dwUpdateTime = GetCurSec();
// 	SaveAlrData(ERC_PARACHG, now, bErcData, (int)(pbBuf-bErcData), 0);
}

//����:����������
WORD SearchPnFromMask(const BYTE* pbPnMask, WORD wStartPn)
{
	WORD i, j;
	BYTE bBitMask;
	i = wStartPn >> 3;
	j = wStartPn & 7;
	for (; i<PN_MASK_SIZE; i++)
	{
		if (pbPnMask[i] != 0)
		{
			bBitMask = 1 << j;
			for (; j<8; j++,bBitMask<<=1)
			{
				if (pbPnMask[i] & bBitMask)
					return (i<<3)+j;
			}
		}

		j = 0;
	}

	return POINT_NUM;
}

//����������һ���հ׵Ĳ������
WORD SearchUnUsedPnFromMask(const BYTE* pbPnMask, WORD wStartPn)
{
	WORD i, j;
	i = wStartPn >> 3;
	j = wStartPn & 7;
	for (; i<PN_MASK_SIZE; i++)
	{
		BYTE bBitMask = 1 << j;
		for (; j<8; j++,bBitMask<<=1)
		{
			if (!(pbPnMask[i] & bBitMask))
				return (i<<3)+j;
		}

		j = 0;
	}

	return POINT_NUM;
}

//����:���ݲ���������ҳ��ö˿ڳ��������ͳ���ɹ���
BYTE GetMtrNumByPort(WORD wPort, WORD* pbTotalMtrNum, WORD* pwMtrRdSuccNum)
{
	WORD i;
	BYTE bPos, bMask;
	BYTE bMtrNum = 0;
	BYTE bMtrRdSuccNum = 0;

	for (i=1; i<POINT_NUM; i++)
	{
		if ((GetPnProp(i)==PN_PROP_METER && GetPnPort(i)==wPort) ||
			(GetPnProp(i)==PN_PROP_CCT && GetPnPort(i)==wPort))
		{
			bMtrNum++;

			bPos = i>>3;
			bMask = 1 << (i & 7);
			if (g_bMtrRdStatus[bPos] & bMask)
				bMtrRdSuccNum++;
		}
	}	

	*pbTotalMtrNum += bMtrNum;
	*pwMtrRdSuccNum += bMtrRdSuccNum;
	return bMtrNum;
}

//����:���ݲ���������ҳ��Ѿ��ҵ�(������)�˶��ٿ��
BYTE GetMtrNum()
{
	WORD wPn;
	BYTE bBuf[8];
	BYTE bMtrNum = 0;

	for (wPn=1; wPn<POINT_NUM; wPn++)
	{
		if (GetPnProp(wPn) == PN_PROP_METER) 
			bMtrNum++;
	}	

	for (wPn=1; wPn<POINT_NUM; wPn++)
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		ReadItemEx(BN0, wPn, 0x8920, bBuf);

		if(!IsAllAByte(bBuf, 0, sizeof(bBuf)))
			bMtrNum++;
	}	

	return bMtrNum;	
}

BYTE GetMtrClassNum(WORD wId, WORD wExpPn, BYTE bMtrClass)
{
	WORD wPn;
	BYTE bBuf[2];
	BYTE bMtrNum = 0;

	for (wPn=1; wPn<POINT_NUM; wPn++)
	{
		if(wPn == wExpPn)
			continue;

		memset(bBuf, 0x00, sizeof(bBuf));
		ReadItemEx(BN0, wPn, 0x8900, bBuf);
		ReadItemEx(BN0, wPn, wId, bBuf+1);

		if(bBuf[0]==1 && bBuf[1]==bMtrClass)
			bMtrNum++;
	}	

	return bMtrNum;	

}


BYTE GetVitalPn(BYTE* p)
{
	WORD wPn;
	BYTE bBuf[2];
	BYTE bLen = 0;
	BYTE* pTemp = p;

	for (wPn=1; wPn<POINT_NUM; wPn++)
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		ReadItemEx(BN0, wPn, 0x8900, bBuf);
		ReadItemEx(BN0, wPn, 0x8906, bBuf+1);

		if(bBuf[0]==1 && bBuf[1]==1)
		{
			DWORDToBCD(wPn, pTemp, 2);
			pTemp += 2;
			bLen  += 2;
		}
	}	

	return bLen;	

}
/*
//����:ɾ��30��δ�ɹ������pn
bool DelReadFailPn()
{
	WORD wPn, wSn;
	BYTE bBuf[8];
	BYTE bTemp[3] = {0};
	TTime tLastTime, tCurTime;
	GetCurTime(&tCurTime);

	for(wPn = 0; wPn<POINT_NUM; wPn++)
	{
		memset(bBuf , 0x00, sizeof(bBuf));
		ReadItemEx(BN0, wPn ,0x8921, bBuf);
		if (memcmp(bBuf, bTemp, 3)==0 || !IsMtrPn(wPn))
			continue;

		tLastTime.nYear = bBuf[0]+2000;
		tLastTime.nMonth = bBuf[1];
		tLastTime.nDay = bBuf[2];

		memset(bBuf , 0x00, sizeof(bBuf));
		if (DaysPast(&tLastTime, &tCurTime) >= 30)
		{ 
			bBuf[0] = 1;
			bBuf[1] = 0;
			wSn = MtrPnToSn(wPn);
			WordToByte(wSn, &bBuf[2]);
			bBuf[4] = 0;
			bBuf[5] = 0;
			WriteItemEx(BN0, BN0, 0x00af, bBuf);
			//WriteItemEx(BN0, wPn ,0x8921, bTemp); //ɾ���ò����������ʱ��
			DTRACE(DB_POINT, ("DelReadFailPn: Delete Pn:%d due to Read meter fail 30 days!!\r\n",wPn));
		}
	}

	return true;
}

//��������һ������ʧ��ʱ����ı�ɾ��
WORD  SearchOnePnReadFail(WORD wMinPn, WORD wMaxPn)
{
	WORD i, wSn, wPn = wMinPn;
	BYTE bBuf[8];
	BYTE bTemp[3] = {0};
	DWORD dwReadDays = 0, dwtemp = 0;

	TTime tLastTime;
	GetCurTime(&tLastTime);

	//�ҳ���һ������ʱ��
	for(i= wMinPn; i<wMaxPn; i++)
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		ReadItemEx(BN0, i ,0x8921, bBuf);
		if(memcmp(bBuf, bTemp, 3)==0 || !IsMtrPn(i))
			continue;

		tLastTime.nYear = bBuf[0]+2000;
		tLastTime.nMonth = bBuf[1];
		tLastTime.nDay = bBuf[2];

		dwReadDays = DaysFrom2000(&tLastTime);
		break;
	}

	for(i = wMinPn; i<wMaxPn; i++)
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		ReadItemEx(BN0, i ,0x8921, bBuf);
		if(memcmp(bBuf, bTemp, 3)==0 || !IsMtrPn(i))
		{
			continue;
		}

		tLastTime.nYear = bBuf[0]+2000;
		tLastTime.nMonth = bBuf[1];
		tLastTime.nDay = bBuf[2];

		dwtemp = DaysFrom2000(&tLastTime);
		if(dwtemp < dwReadDays) //�������
		{
			wPn = i;
			dwReadDays = dwtemp;
		}
	}

	//ɾ���ñ�
	bBuf[0] = 1;
	bBuf[1] = 0;
	wSn = MtrPnToSn(wPn);
	WordToByte(wSn, &bBuf[2]);
	bBuf[4] = 0;
	bBuf[5] = 0;
	WriteItemEx(BN0, BN0, 0x00af, bBuf);
	//WriteItemEx(BN0, wPn ,0x8921, bTemp); //ɾ���ò����������ʱ��
	DTRACE(DB_POINT, ("SearchDelReadFailPn: Delete Pn:%d due to Read Meter fail %ld days!!\r\n",wPn, dwReadDays));

	return wPn;
}*/

//�������洢�ѵ��ĵ����Ϣ
bool SaveSearchPnToPointSect(BYTE* pbMtrAddr, BYTE bPro, BYTE bPort)
{
	//BYTE bProp;
	WORD wPn;
	BYTE  bAddrTemp[6];
	BYTE bPnFlg[256], bBuf[8];

	if(pbMtrAddr == NULL)
		return false;

	memset(bPnFlg, 0, sizeof(bPnFlg));
	ReadItemEx(BN0, PN0, 0x0560, bPnFlg); //����������λ
	//�����Ƿ������ͬ�ĵ��
	for (wPn=1; wPn<POINT_NUM; wPn++)
	{	
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN0, wPn, 0x8920, bBuf);
		//����ַһ����˵���Ѿ�������
		if (memcmp(bBuf, pbMtrAddr, 6)==0 && !IsAllAByte(pbMtrAddr, INVALID_DATA, 6))
		{  //����ַ���                                 &&  ��Ч�ĵ���ַ
			return false;
		}
		if (GetMeterAddr(wPn, bAddrTemp))
		{
			if (memcmp(bAddrTemp, pbMtrAddr, 6) == 0)//��F10�Ѿ����úõĵ��
			{
				return false;
			}
		}
	}

	//ȫ�µĵ����һ����λ��������
	wPn = 1;
	while (wPn < POINT_NUM)
	{
		memset(bBuf, 0, sizeof(bBuf));

		//�����ѳ�������F10����û����Ŀհײ������
		wPn = SearchUnUsedPnFromMask(bPnFlg, wPn);

		if (wPn == POINT_NUM)
			break; //wPn = SearchOnePnReadFail(2, POINT_NUM); //һ�㽻�ɲ�����Ϊ1

		ReadItemEx(BN0, wPn, 0x8920, bBuf);
		if (IsAllAByte(bBuf, 0, sizeof(bBuf))) //���λ��Ϊ0��˵�����λ�ÿ��Դ��
		{ 
			memcpy(bBuf, pbMtrAddr, 6);
			bBuf[6] = bPro;
			bBuf[7] = bPort;
			WriteItemEx(BN0, wPn, 0x8920, bBuf);
			DTRACE(DB_POINT, ("SaveSearchPnToPointSect: Find a New Meter, New Pn No:%d, Meter addr :%02x%02x%02x%02x%02x%02x, bPro:%d, Port & Baud:%d .\r\n", wPn, bBuf[0], bBuf[1], bBuf[2], bBuf[3], bBuf[4], bBuf[5], bBuf[6], bBuf[7]));
			break;
		}

		wPn++;
	}
	return true;
}

/*bool SaveSearchPnToF10()
{
	BYTE  bPnFlg[256];
	BYTE  bBuf[30];
	BYTE  bPnBuf[8];
	BYTE  bAddrTemp[6];
	bool  bIsSave = false;
	bool  bIsValid = false;
	WORD wPos, wPn, wPnTmp;
	BYTE bMask, bSchMtr;

	ReadItemEx(BN0, PN0, 0x07ef, &bSchMtr);	//--�Զ�ά��״̬��00���رգ�01�����ò�����F10��02�����õ�������F10��ȱʡΪ�ر�
	if (bSchMtr != 0x01)
		return false;

	//���������յ���ѯ������״̬���F150��n���ӣ�n=0~20��һ��ȡ10���ڲ�������F10�ն˵��ܱ�/��������װ�����ò������Զ����¡�
	//ʱ���ж�Ӧ�÷ŵ������Ϊһ��д��F10��F150�еĲ�������Ч��־�Զ�ͬ����Ч�����Ǳ����־ȴ���첽�ظ��µ�F150
	//�������������Ӧ����F10��F150ͬ������
	if (GetCurSec() - g_dwUpdateTime <= 10*60)
		return false;
	memset(bPnFlg, 0x00, sizeof(bPnFlg));
	//ReadItemEx(BN0, PN0, 0x056f, bPnFlg);	//F150

	memset(bPnBuf, 0x00, sizeof(bPnBuf));
	for (wPn=1; wPn<POINT_NUM; wPn++)
	{
		bIsSave= false;
		ReadItemEx(BN0, wPn, 0x8920, bPnBuf);
		if (IsAllAByte(bPnBuf, 0x00, 8))
			continue;

		//�����Ƿ������ͬ�ĵ����߲ɼ���
		for (wPnTmp=1; wPnTmp<POINT_NUM; wPnTmp++)
		{            
			if (GetMeterAddr(wPnTmp, bAddrTemp))
			{
				if (memcmp(bAddrTemp, bPnBuf, 6) == 0 )//F10������ͬ���
				{
					bIsSave = true;	//F10�Ѿ������
					memset(bPnBuf, 0x00, sizeof(bPnBuf));
					WriteItemEx(BN0, wPn, 0x8920, bPnBuf);
					break;
				}
			} //if (GetMeterAddr(wPnTmp, bAddrTemp))
		} //for (WORD wPnTmp=1; wPnTmp<POINT_NUM; wPnTmp++)

		if (bIsSave)
			continue;

		//��F10�в�������ͬ���
		memset(bBuf, 0x00, sizeof(bBuf));
		WordToByte(1, bBuf);           //���������
		WordToByte(wPn, &bBuf[2]);     //װ�����
		WordToByte(wPn, &bBuf[4]);     //�������
		bBuf[6] = bPnBuf[7];          //ͨ�����ʼ��˿ں�
		bBuf[7] = bPnBuf[6];          //ͨ��Э������
		memcpy(&bBuf[8], bPnBuf, 6);   //ͨ�ŵ�ַ
		memset(&bBuf[14], 0, 6);       //ͨ������
		bBuf[20] = 4;                  //���ʸ���
		bBuf[21] = 0;                  //������С��λ����
		bBuf[28] = 0x31;               //��С���

		if (WriteItemEx(BN0, PN0, 0x00af, bBuf) > 0)
		{	
			bMask = 1<<(wPn&0x07);
			wPos = wPn>>3;

			if(wPos<256)
			{
				ReadItemEx(BN0, PN0, 0x0560, bPnFlg);	//F150
				bPnFlg[wPos] |= bMask; //��Ч��־
				WriteItemEx(BN0, PN0, 0x0560, bPnFlg);
				ReadItemEx(BN0, PN0, 0x0561, bPnFlg);	//F150
				bPnFlg[wPos] |= bMask; //��λ��־
				WriteItemEx(BN0, PN0, 0x0561, bPnFlg);
			}
			DTRACE(DB_POINT, ("SaveSearchPnToF10: Add Meter To F10, Pn :%d, Meter addr :%02x%02x%02x%02x%02x%02x, bPro:%d, Port & Baud:%d .\r\n", wPn, bPnBuf[0], bPnBuf[1], bPnBuf[2], bPnBuf[3], bPnBuf[4], bPnBuf[5], bPnBuf[6], bPnBuf[7]));
		}

		memset(bPnBuf, 0x00, sizeof(bPnBuf));
		WriteItemEx(BN0, wPn, 0x8920, bPnBuf);
		bIsValid = true;
	}

	if (bIsValid)
	{
		//WriteItemEx(BN0, PN0, 0x056f, bPnFlg);
		DTRACE(DB_POINT, ("SaveSearchPnToF10: Update Pn's Flag OK!.###########\r\n"));
		PushEvtParaChanged();
	}

	return true;
}*/

static BYTE g_bMtrChgFlg[PN_MASK_SIZE];

void SetMtrChgFlg(WORD wPn)
{
	BYTE bPos = wPn>>3;
	BYTE bMask = 1<<(wPn & 7);
	
	g_bMtrChgFlg[bPos] |= bMask;
}

void ClrMtrChgFlg(WORD wPn)
{
	BYTE bPos = wPn>>3;
	BYTE bMask = 1<<(wPn & 7);
	
	g_bMtrChgFlg[bPos] &= ~bMask;
}

bool IsMtrParaChg(WORD wPn)
{
	BYTE bPos = wPn>>3;
	BYTE bMask = 1<<(wPn & 7);

	return (g_bMtrChgFlg[bPos]&bMask) != 0;
}

void NewMtrThread()
{
  	SaveSoftVerChg();
	NewThread("Mt0", MtrRdThread, (void * )0, 1024, THREAD_PRIORITY_LOWEST);		//��ջС��640,VC�����״����߳�ʧ��
	NewThread("Mt1", MtrRdThread, (void * )1, 1024, THREAD_PRIORITY_LOWEST);		//576
}

//��������ʼ������
bool InitMeter()
{
	memset(g_bMtrChgFlg, 0, sizeof(g_bMtrChgFlg));
	memset(g_bRdMtrAlr, 0, sizeof(g_bRdMtrAlr));
	memset(g_bRdMtrAlrStatus, 0, sizeof(g_bRdMtrAlrStatus));

	return true;
}
