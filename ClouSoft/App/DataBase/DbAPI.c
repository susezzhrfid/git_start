/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbAPI.cpp
 * ժ    Ҫ�����ļ���Ҫʵ��Э����ص����ݿ��׼�ӿ�֮�����չ�ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
 *********************************************************************************************************/
#include "Info.h"
#include "FaConst.h"
#include "DbConst.h"
#include "ComStruct.h"
#include "FaAPI.h"
#include "DbAPI.h"
#include "ComAPI.h"
#include "MtrAPI.h"
#include "DataManager.h"
#include "SysDebug.h"
#include "LibDbAPI.h"
#include "ExcTask.h"

//����汾 
// const BYTE g_bSoftVer[SOFT_VER_LEN] = 
// {
// 	'C', 'L', 'O', 'U',	 //���̴��� 4
// 	'C', 'L', '8', '1', '8', 'K', '5', '-', //�豸�ͺ� 8
// 	'0', '0', '1', 'n', //�ն�����汾 4 x.xx A ���汾.���汾 ���԰汾
// 	0x05, 0x09, 0x14,   //�ն������������
// 	'9', '2', '7', '0', '3', '2', '6', '4', 0, 0, 0, //�ն�����������Ϣ 11 CPU�汾(3), RAM��С(2), FLASH��С(2) 
// 	'1', '3', '7', '6', //�ն�ͨѶ��Լ�汾�� 4
// 	'v', '0', '0', '1', //�ն�Ӳ���汾 4
// 	0x01, 0x12, 0x12,   //�ն�Ӳ���������� 3
// };
const BYTE g_bSoftVer[SOFT_VER_LEN] = 
{
	'C', 'L', '8', '1', '8', 'K', '5', '-', //�豸�ͺ� 8  
	'0', '0', '2', 'i', //�ն�����汾 4 x.xx A ���汾.���汾 ���԰汾  
	0x3d,0x5a,	 //���̴��� 2 
	0x04,//�ն����� 01��ʾ��վ��02��ʾר�䣬03��ʾ���䣬04��ʾ������ 1
	0x20, 0x10, 0x14,   //�ն������������ 3

	0x00,0x01, //�ն�ͨѶ��Լ�汾�� 2

	0x00,0x12, //�ն�Ӳ���汾 2
	0x15, 0x09, 0x14,   //�ն�Ӳ���������� 3
	'9', '2', '7', '0', '3', '2', '6', '4', 0, 0, 0, //�ն�����������Ϣ 11 CPU�汾(3), RAM��С(2), FLASH��С(2) 
};
//�ڲ�����汾
// const BYTE g_bInnerSoftVer[INN_SOFT_VER_LEN] = 
// {
// 	 '3', '0', '1', '2',     //�汾4���ֽ� x.xx A ���汾.���汾 ���԰汾����ʽ�鵵�İ汾���԰汾��Ϊ0
// 	 0x13, 0x01, 0x14,        //����3���ֽ� BCD�룬�ն�����������ڡ�
// };

const WORD g_FmtARDLen[23] = 
{
	0,
	7,
	91,
	159,
	27,
	31,
	35,
	55,
	35,
	37,
	10,
	58,
	9,
	20,
	34,
	25,
	25,
	25,
	10,
	23,
	12,
	263,//�澯״̬(1)+ʱ��(6)+�������(��������3��)
	9,
};

//�澯����
const TAlrTaskCtrl g_AlrTaskCtrl[ALR_NUM] = 
{
	// 	{0xE2000001,	ARD2},//����װ���ſ���
	// 	{0xE2000002,	ARD2},//����������(��Լ��ɾ������)
	// 	{0xE2000003,	ARD2},//��ѹ������
	// 	{0xE2000004,	ARD2},//������ƽ��
	//	{0xE2000005,	ARD2},//��ѹ��ƽ��
	//	{0xE2000006,	ARD2},//�������ƫ��
	//	{0xE2000007,	ARD2},//A��CT���β��·
	//	{0xE2000008,	ARD2},//B��CT���β��·
	//	{0xE2000009,	ARD2},//C��CT���β��·
	// 	{0xE200000A,	ARD2},//A��CT���β࿪·
	//	{0xE200000B,	ARD2},//B��CT���β࿪·
	//	{0xE200000C,	ARD2},//C��CT���β࿪·
	{0xE200000D,	ARD2},//A�ೱ������
	{0xE200000E,	ARD2},//B�ೱ������
	{0xE200000F,	ARD2},//C�ೱ������

	//{0xE2000010,	ARD2},//A���������
	//{0xE2000011,	ARD2},//B���������
	//{0xE2000012,	ARD2},//C���������
	{0xE2000013,	ARD2},//A�����ʧ��
	{0xE2000014,	ARD2},//B�����ʧ��
	{0xE2000015,	ARD2},//C�����ʧ��
	{0xE2000016,	ARD2},//A���ѹʧѹ
	{0xE2000017,	ARD2},//B���ѹʧѹ
	{0xE2000018,	ARD2},//C���ѹʧѹ
	{0xE2000019,	ARD2},//ȫʧѹ
	// 	{0xE200001A,	ARD2},//A���ѹ��ѹ
	// 	{0xE200001B,	ARD2},//B���ѹ��ѹ
	// 	{0xE200001C,	ARD2},//C���ѹ��ѹ
	// 	{0xE200001D,	ARD2},//A���ѹ����
	// 	{0xE200001E,	ARD2},//B���ѹ����
	// 	{0xE200001F,	ARD2},//C���ѹ����

	// 	{0xE2000020,	ARD14},//A���ѹ����
	// 	{0xE2000021,	ARD14},//B���ѹ����
	// 	{0xE2000022,	ARD14},//C���ѹ����
	// 	{0xE2000023,	ARD14},//A���������
	// 	{0xE2000024,	ARD14},//B���������
	// 	{0xE2000025,	ARD14},//C���������
	// 	{0xE2000026,	ARD2},//�޹�������
	// 	{0xE2000027,	ARD2},//�޹�Ƿ����
	// 	{0xE2000028,	ARD2},//���ʳ���ֵ
	// 	{0xE2000029,	ARD2},//���ɹ���
	// 	{0xE200002A,	ARD2},//����ͬ�����õ�
	{0xE200002B,	ARD4},//ʣ���Ѳ���
	{0xE200002C,	ARD3},//ʾ���½�
	// 	{0xE200002D,	ARD3},//���ܱ����
	{0xE200002E,	ARD2},//���ܱ�ͣ��
	// 	{0xE200002F,	ARD1},//���ܱ�ͨѶʧ��

	// 	{0xE2000030,	ARD3},//��澯
	// 	{0xE2000031,	ARD7},//��������ֶ�����
	{0xE2000032,	ARD1},//ʱ�ӵ�ص�ѹ����
	{0xE2000033,	ARD2},//�ն˵���
	{0xE2000034,	ARD2},//�ն��ϵ�
	{0xE2000035,	ARD2},//���ܱ���ʱ�����
	{0xE2000036,	ARD1},//���ܱ�ʱ�λ���ʸ���
	// 	{0xE2000037,	ARD6},//���ܱ����峣������
	// 	{0xE2000038,	ARD5},//���ܱ�Ļ��������ʸ���
 	{0xE2000039,	ARD9},//ң�ű�λ
	{0xE200003A,	ARD10},//��ͨ������Խ��
	{0xE200003B,	ARD12},//�̵�����λ
	{0xE200003C,	ARD12},//���ܱ�����բʧ��
	{0xE200003D,	ARD21},//����ʧ��
	{0xE200003E,	ARD13},//���ܱ�ʱ���쳣
	// 	{0xE200003F,	ARD1},//���ܱ�Уʱʧ��
	// 
	// 	{0xE2000040,	ARD15},//���ܱ�A��B��C��ʧѹ�ܴ���
	// 	{0xE2000041,	ARD16},//���ܱ�A��B��C��ʧ���ܴ���
	// 	{0xE2000042,	ARD17},//���ܱ�A��B��C�ೱ�������ܴ���
	// 	{0xE2000043,	ARD18},//���ܱ����ܴ���
	// 	{0xE2000044,	ARD14},//A���ѹƫ��Խ��
	// 	{0xE2000045,	ARD14},//B���ѹƫ��Խ��
	// 	{0xE2000046,	ARD14},//C���ѹƫ��Խ��
	// 	{0xE2000047,	ARD14},//Ƶ��ƫ��Խ��
	// 	{0xE2000048,	ARD14},//A������Խ��
	// 	{0xE2000049,	ARD14},//B������Խ��
	// 	{0xE200004A,	ARD14},//C������Խ��
	// 	{0xE200004B,	ARD14},//��ѹ��ƽ��Խ��(��Լ��ɾ������)
	// 	{0xE200004C,	ARD22},//����δ֪���
	// 	{0xE200004D,	ARD11},//���ť�п����澯
	// 	{0xE200004E,	ARD11},//��ǿ����澯
	//�¼���¼����
	// 	{0xE2010001,	ARD1},//���ܱ��̼�¼
	// 	{0xE201000F,	ARD1},//���������¼
	// 	{0xE2010010,	ARD1},//Уʱ��¼

	/**/{0x00000000,	ARD},/**///������־
};
WORD GetEventLen(DWORD dwId)
{
	if(dwId==0xE201000A)
		return 12;
	else if(dwId==0xE201000E)
		return 15;
	else if(dwId==0xE2010014)
		return 38;
	else
		return 0;

	return 0;
}

bool IsEventId(DWORD dwId)
{
	//��ֻ֧��������ID���¼���ͣ�ϵ硢���ơ���Σ�
	if(dwId==0xE201000A || dwId==0xE201000E || dwId==0xE2010014)
		return true;

	return false;
}

WORD GetFmtARDLen(DWORD dwId)
{
	BYTE i;
	for(i=0; GetAlrID(i)!=0;i++)
	{
		if (GetAlrID(i) == dwId)
		{
			return g_FmtARDLen[g_AlrTaskCtrl[i].bFmt];
		}
	}
	
	return 0;
}

DWORD GetAlrID(BYTE i)
{
	return g_AlrTaskCtrl[i].dwAlrId;
}


//����:��ȡ��ֱ��ģ��������������
//����:0��ʾ��Ч��1��ʾ���ɣ�2��ʾ���3��ʾ����//��4��ʾģ���� 
BYTE GetPnProp(WORD wPn)
{
	BYTE bProp;
	if (wPn >= POINT_NUM)// PN_NUM)
		return INVALID_POINT;
	
#if MTRPNMAP!=PNUNMAP		
	if (SearchPnMap(MTRPNMAP, wPn) < 0) //����ҵ��򷵻ض�Ӧ��ӳ���,���򷵻�-1
		return INVALID_POINT;
#endif //MTRPNMAP!=PNUNMAP,
	
	if (ReadItemEx(BN0, wPn, 0x8900, &bProp) <= 0)	//��������Ч��־
		return INVALID_POINT;

	if (bProp != 1)
		return INVALID_POINT;

	if (ReadItemEx(BN0, wPn, 0x8901, &bProp) <= 0)
		return INVALID_POINT;

	return bProp;
}


//����:ȡPN��Ӧ�Ķ˿ں�
//����:�����ȷ�򷵻ض˿ں�,���򷵻�0
BYTE GetPnPort(WORD wPn)
{
	BYTE bBuf[10];
	BYTE bPort;
	if (ReadItemEx(BN0, wPn, 0x890a, bBuf) >= 0)
	{	
		bPort = bBuf[0]&0x3f;

		return bPort;
	}

	return 0;
}

//����:ȡ�ò�����ĵ��Э������
BYTE GetPnMtrPro(WORD wPn)
{
	BYTE bBuf[4];
	memset(bBuf, 0, sizeof(bBuf));
	ReadItemEx(BN0, wPn, 0x8903, bBuf);
	return MeterProtoLocalToInner(bBuf[0]);	
}

BYTE GetCctPnMtrPro(WORD wPn)
{
	BYTE bBuf[4];
	memset(bBuf, 0, sizeof(bBuf));
	ReadItemEx(BN0, wPn, 0x8903, bBuf);
	
	if (PROTOCOLNO_DLT645_V07 == MeterProtoLocalToInner(bBuf[0]))
		return CCT_MTRPRO_07;
	else
		return CCT_MTRPRO_97;
}


//����:�˲������Ƿ�֧�ִ�Fn
bool IsFnSupport(WORD wPn, BYTE bFn, BYTE bClass)
{
/*	const BYTE* pbCfg;
	const BYTE* p;	//ָ����Ӧ����û�С���
	WORD i, wID;
	WORD sub;
	BYTE bMain;
	BYTE bSub;
	BYTE n, bMark, pos;
//#ifdef SYS_WIN
	BYTE bBuf[C2_CFG_LEN+10];
//#endif

	if (!GetUserType(wPn, &bMain, &bSub)) //��ȡ�û��û�����ź�С���
		return false;

	wID = (bClass==1 ? 0x026f : 0x027f);

#ifdef SYS_WIN
	pbCfg = GetItemRdAddrID(BN0, bMain, wID, bBuf);
#else
	//pbCfg = GetItemRdAddrID(BN0, bMain, wID, NULL);
	pbCfg = bBuf;
	ReadItemEx(BN0, bMain, wID, bBuf);
#endif
	if (pbCfg==NULL)
		return false;

	if (bMain>=USR_MAIN_CLASS_NUM || bSub>=USR_SUB_CLASS_NUM || bFn==0)
		return false;

	if (pbCfg[0]==0xff && bMain==0)	//����û������,�û������Ϊ0,��ȫ��֧��
	{
		for (i=0; i<sizeof(g_bMultiFn); i++)
		{
			if (bFn == g_bMultiFn[i])
	        	return true;
		}

		return false;
	}
	else if (pbCfg[0] >= USR_MAIN_CLASS_NUM)
		return false;

	//m = pbCfg[1];
	bMark = 1 << ((bFn-1)&0x07);
	pos = (bFn-1)>>3;
	p = &pbCfg[2+(WORD )bSub*33];	//ָ����Ӧ����û�С���
	if ((bSub!=0) && (*p==bSub))	//�û�С������,���С���Ϊ0������,��������Ľ����ж�
	{
		n = p[1];	 //����n
        if (pos >= n)
			return false;

		if ((bMark & p[2+pos]) != 0)
			return true;
		else
			return false;
	}
	else if (bSub == 0)	//С���Ϊ0,Ĭ��Ϊ�û����ඨ�����������������
	{
		BYTE bBuf[32];
		memset(bBuf, 0, sizeof(bBuf));
		for (sub=1; sub<USR_SUB_CLASS_NUM; sub++)
		{
			p = &pbCfg[2+(WORD )sub*33]; //ָ����Ӧ����û�С���

			if (*p == sub)	//�û�С������
			{
				p++;	//����С���
				n = *p++;	 //����n
				if (n > 31)
					continue;

				for (i=0; i<n; i++)
				{
					bBuf[i] |= *p++;
				}
			}
		}

		if ((bMark & bBuf[pos]) != 0)
			return true;
		else
			return false;

	}*/

	return false;
}


//����:��ȡ�û��û�����ź�С���
/*bool GetUserType(WORD wPn, BYTE* pbMain, BYTE* pbSub)
{
	BYTE bType;
	BYTE bBuf[32];
	if (!IsMtrPn(wPn))
		return false;

	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
		return false;

	bType = bBuf[MTR_USR_TYPE_OFFSET];
	*pbMain = (bType>>4) & 0x0f; 
	*pbSub = bType&0x0f;

	return true;
}*/

//����:��ȡ���������ͺ��ص㻧����
bool GetUserTypeAndVip(WORD wPn, BYTE* bVip, BYTE* bType)
{
	BYTE bBuf[2];
	if (!IsMtrPn(wPn))
		return false;

	if ((ReadItemEx(BN0, wPn, 0x8904, bBuf) <= 0) || (ReadItemEx(BN0, wPn, 0x8906, bBuf+1) <= 0))
		return false;
	*bVip = bBuf[1] & 0x0f; 
	*bType = bBuf[0]&0x0f;

	return true;
}


bool IsMtrPn(WORD wPn)
{
	return GetPnProp(wPn)==PN_PROP_METER;
}


//����:��ȡ�¼�������
//����:0��ʾ���¼���Ч,1��ʾ��Ҫ�¼�,2��ʾһ���¼�
BYTE GetErcType(BYTE bErc)
{
	return IsAlrEnable(0xE2000001+bErc);
}


//�ն�����汾�������
void SaveSoftVerChg()
{
	/*TTime tm;
	//�а汾����¼����ȴ��γ�ʼ������д�汾����¼�	
	if (g_SoftVerChg.time != 0)
	{
		SecondsToTime(g_SoftVerChg.time, &tm);
// 		SaveAlrData(ERC_INIT_VER, tm, g_SoftVerChg.bVerInfo, 0, 0);	
 	}

	//д�������ʼ���¼������Ͱ汾���дһ���¼�����Ϊ����ǵ���ǰ�ķ���ʱ�䲻ͬ
	//�в�����ʼ���¼����ȴ��γ�ʼ������д	
	if (g_PowerOffTmp.bParaEvtType>0 && g_PowerOffTmp.ParaInit.time!=0)
	{
		SecondsToTime(g_PowerOffTmp.ParaInit.time, &tm);
// 		SaveAlrData(ERC_INIT_VER, tm, g_PowerOffTmp.ParaInit.bVerInfo, 0, g_PowerOffTmp.bParaEvtType);
		g_PowerOffTmp.bParaEvtType = 0;		
 	}*/
}

extern const BYTE g_bSoftVer[SOFT_VER_LEN];

//�ն�����汾������
void TermSoftVerChg()
{
	TTime tm;
	BYTE  bSoftVer[SOFT_VER_LEN];
    memcpy(bSoftVer, "SoftwareVer:", 12);
    bSoftVer[12] = 0;
    DTRACE(DB_DP, ("%s", bSoftVer));
    memcpy(bSoftVer, g_bSoftVer, 12);
    bSoftVer[12] = 0;
    DTRACE(DB_DP, ("%s\r\n", bSoftVer));

	WriteItemEx(BN0, PN0, 0x8817, (BYTE*)g_bSoftVer+18); //�ն�����汾��
	WriteItemEx(BN0, PN0, 0x8818, (BYTE*)g_bSoftVer+12); //�ն�����汾��
	WriteItemEx(BN0, PN0, 0x8819, (BYTE*)g_bSoftVer+20); //�ն�Ӳ���汾��

	memcpy(bSoftVer, "CL818K5-SG-Std", 14);
	memset(bSoftVer+14, 0x00, 2);
	memcpy(bSoftVer+16, g_bSoftVer+8, 4);
	memcpy(bSoftVer+20, g_bSoftVer+15, 3);
	WriteItemEx(BN2, PN0, 0x2107, (BYTE*)bSoftVer);


/*	memset((BYTE*)&g_SoftVerChg, 0, sizeof(g_SoftVerChg)); //ȫ�ֵĻ�����
 
 	ReadItemEx(BN0, PN0, 0x100f, bSoftVer);
 	if (memcmp(bSoftVer+12, g_bSoftVer+12, SOFT_VER_LEN-12) != 0)//ֻҪ�Ƚϰ汾�ţ���
 	{
		GetCurTime(&tm);
		g_SoftVerChg.time = TimeToSeconds(&tm);
		g_SoftVerChg.bVerInfo[0] = 0x02;
		memcpy(&g_SoftVerChg.bVerInfo[1], &bSoftVer[12], 4);
		memcpy(&g_SoftVerChg.bVerInfo[5], &g_bSoftVer[12], 4);
		//SaveAlrData(ERC_INIT_VER, tm, bBuf);	//д�汾����¼�			
		WriteItemEx(BN0, PN0, 0x100f, (BYTE *)g_bSoftVer);
		if (g_bSoftVer[12]=='V' && g_bSoftVer[13]=='0' && g_bSoftVer[14]=='.' && g_bSoftVer[15]=='5') //���������Զ�Ĭ��
		{
			BYTE bDayMonFrzSta = 1;
			WriteItemEx(BN24, PN0, 0x4111, &bDayMonFrzSta);
			WriteItemEx(BN24, PN0, 0x4115, &bDayMonFrzSta);
			WriteItemEx(BN24, PN0, 0x4116, &bDayMonFrzSta);
			WriteItemEx(BN0, PN0, 0x07bf, &bDayMonFrzSta); //���������ϱ�
		}
 	}

	WriteItemEx(BN2, PN0, 0x2107, (BYTE *)g_bInnerSoftVer);*/
}

void PostDbInit()
{ 	
#ifdef PRO_698
	//SetDefFnCfg();		//����Ĭ�ϴ�С��֧��������
	//InitMtrSnToPn();	//����������ŵ�InitAcPn()ǰ,��Ϊ�漰��װ����ŵ�������ŵ�ӳ��
#endif

	TermSoftVerChg();
#if MTRPNMAP!=PNUNMAP
	//NewPnMap(MTRPNMAP, PN1);	//Ϊ�ڱ�̶�ӳ��һ������PN1
#endif //MTRPNMAP!=PNUNMAP
    
	//if (!IsInMtrParaOk())
		//InitInMeterPn();
	//InitAcPn();
#if MTRPNMAP!=PNUNMAP
	UpdPnMap();
#endif
}

extern const TBankCtrl g_Bank0Ctrl[SECT_NUM];
extern const TBankCtrl g_BankCtrl[BANK_NUM];
//extern TPnMapCtrl g_PnMapCtrl[PNMAP_NUM];
TDbCtrl g_DbCtrl; //�������ݿ���в������õ����ݿ���ƽṹ

//����:ϵͳ���ݿ��ʼ��
bool InitDB(void)
{
	memset(&g_DbCtrl, 0, sizeof(g_DbCtrl));

	//BANK0�Ŀ����ֶ�
	g_DbCtrl.wSectNum = SECT_NUM;	//BANK0�е�SECT��Ŀ
	g_DbCtrl.pBank0Ctrl = g_Bank0Ctrl;

	//BANK�����ֶ�
	g_DbCtrl.wBankNum = BANK_NUM;	//֧�ֵ�BANK��Ŀ
	g_DbCtrl.pBankCtrl = g_BankCtrl;
								 
	//�����㶯̬ӳ������ֶ�
	g_DbCtrl.wPnMapNum = PNMAP_NUM;		//֧�ֵ�ӳ�䷽����Ŀ,�������ݿⲻ֧�ֲ����㶯̬ӳ������Ϊ0
	g_DbCtrl.pPnMapCtrl = NULL; //g_PnMapCtrl;	//�������ݿⲻ֧�ֲ����㶯̬ӳ������ΪNULL

	g_DbCtrl.wSaveInterv = 15;			//������,��λ����

	//g_DbCtrl.pDbUpgCtrl = &g_DbUpgCtrl;  ʹ���˲����㶯̬ӳ�䣬�ļ���С���ָ�ԭ��һ�£�������������

	if (!InitDbLib(&g_DbCtrl)) //�汾����¼��õ������
		return false;

	PostDbInit();

	//CheckHisProRptPara();
	
	return true;
}


//�������(���,����,����)����
void ClrPnData(WORD wPn)
{
}

#if MTRPNMAP!=PNUNMAP
void UpdPnMap()
{
	bool fPnMapFail = false;
	WORD wPn;
	BYTE bProp;

	//WaitSemaphore(g_semMtrPnMap, SYS_TO_INFINITE);
	//DTRACE(DB_DP, ("UpdPnMask()******1! \r\n"));

	for (wPn=1; wPn<PN_NUM; wPn++)
	{
		bProp = GetPnProp(wPn);

		//����ǰ���־λ�Ĳ�һ��,��������,��Ч�Ĳ����㶼��������һ�²�����ӳ��
		//��Ч�Ĳ����㶼ɾ��һ��ӳ��
		if (bProp == PN_PROP_METER)	//��������Ч
		{
			if (NewPnMap(MTRPNMAP, wPn) < 0)
				fPnMapFail = true;
		}
		else //��������Ч
		{
			DeletePnMap(MTRPNMAP, wPn);
		}
	}

	if (fPnMapFail)	//��һ������ʧ��,�п����Ǻ���Ĳ����㻹û�ͷ�,ǰ��Ĳ�������������
	{				//������һ�־�����
		for (wPn=1; wPn<PN_NUM; wPn++)
		{
			bProp = GetPnProp(wPn);

			if (bProp == PN_PROP_METER)	//��������Ч
			{
				NewPnMap(MTRPNMAP, wPn);
			}
		}
	}

	//SignalSemaphore(g_semMtrPnMap);
}
#endif


//����:�Ƿ�V2007��645Э�������
bool IsV07Mtr(WORD wPn)
{
	BYTE bType;
	BYTE bProp = 0;	

	if (ReadItemEx(BN0, wPn, 0x8901, &bProp) <= 0)
		return false;

	if (bProp != PN_PROP_METER)
		return false;

	if (ReadItemEx(BN0, wPn, 0x8903, &bType) <= 0)
		return false;	

	if (bType == PROTOCOLNO_DLT645_V07)
		return true;
	else
		return false;	
}

//����:��ȡ�����㶳���������
void GetFrzStatus(WORD wPn, BYTE* pbCurveStatus, BYTE* pbDayFrzStatus, BYTE* pbDayFlgFrzStatus)
{
	ReadItemEx(BN10, wPn, 0xa190, pbCurveStatus);
	ReadItemEx(BN10, wPn, 0xa191, pbDayFrzStatus);
	ReadItemEx(BN10, wPn, 0xa1b2, pbDayFlgFrzStatus);
}

//�������������������ÿ���96�����ã�������������ʱ
void ClearCurveFrzFlg(WORD wPn)
{
	BYTE bIsSaveFlg[19];//�����ݿ�ÿ��96�����״̬
	TTime tNow;		
	GetCurTime(&tNow);

	memset(bIsSaveFlg, 0, 12);
	bIsSaveFlg[12] = tNow.nYear-2000; 
	bIsSaveFlg[13] = tNow.nMonth;
	bIsSaveFlg[14] = tNow.nDay;

	WriteItemEx(BN0, wPn, 0xd881, bIsSaveFlg);	
	WriteItemEx(BN0, wPn, 0xd889, bIsSaveFlg);	
	WriteItemEx(BN0, wPn, 0xd901, bIsSaveFlg);	
	WriteItemEx(BN0, wPn, 0xd905, bIsSaveFlg);	
	WriteItemEx(BN0, wPn, 0xd945, bIsSaveFlg);	
	DTRACE(DB_DP, ("ClearCurveFrzFlg : day change has to clear wPn=%d\n", wPn));	
}

//���������ò����������
void SetPnRateNum(WORD wPn, BYTE bRateNum)
{ 
	WriteItemEx(BN0, wPn, 0x8903, &bRateNum);
}

//����:��ȡ������ķ�����
BYTE GetPnRateNum(WORD wPn)
{
	BYTE bRateNum = RATE_NUM;	
	WORD wBn = BN0;

	ReadItemEx(wBn, wPn, 0x8903, &bRateNum);	

	if (bRateNum==0 || bRateNum>RATE_NUM)
		bRateNum = RATE_NUM;

	return bRateNum;
}

//����:�Ƿ���ݵ���ص������ݵ�ʵ�ʳ����޸Ĳ�����ķ�����
bool IsChgRateNumByMtr()
{
	BYTE bVal = 0;
	ReadItemEx(BN10, PN0, 0xa1a1, &bVal);
	return ((bVal==0)?false:true);
}

//���������ò�����Ϊ�����ʱ��single-phase time-division meter�ı�־
void SetPnSPHTDMtr(WORD wPn, BYTE bSPMR)
{ 	
	WriteItemEx(BN24, wPn, 0x4107, &bSPMR);
}

//��������ѯ�������Ƿ�Ϊ�����ʱ��ı�־
bool IsPnSPHTDMtr(WORD wPn)
{ 	
	BYTE bVal = 0;
	ReadItemEx(BN24, wPn, 0x4107, &bVal);
	return ((bVal==0)?false:true);	
}

//����:���ݴ�С�������07�����,�Գ�������ʱת�ɳ������㶳������
bool IsSinglePhaseV07Mtr(WORD wPn)
{
	return false;
}


//����:����Э���������ֻ֧�ֵ�����645Э�黹��֧�ֿ鳭��97��645Э��
//����:1Ϊ֧�ֿ鳭��97��645Э��,2Ϊֻ֧�ֵ�����645Э��.3Ϊ����Э��
BYTE IsSIDV97Mtr(WORD wPn)
{
	BYTE bBuf[F10_LEN_PER_PN+10];
	BYTE bProp = 0;	

	if (ReadItemEx(BN0, wPn, 0x8901, &bProp) <= 0)
		return 0;

	if (bProp != PN_PROP_METER)
		return 0;

	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
		return 0;	

	if (bBuf[MTR_PRO_OFFSET] == PROTOCOLNO_DLT645)
		return 1;
	else if (bBuf[MTR_PRO_OFFSET] == PROTOCOLNO_DLT645_SID)
		return 2;

	return 0;	
}

//����:ȡ�ý��ɵĲ������
WORD GetAcPn()
{
	BYTE bProp = 0;
	WORD wPn;
	for (wPn=1; wPn<PN_NUM; wPn++)
	{		
		ReadItemEx(BN0, wPn, 0x8901, &bProp);
		if (bProp == PN_PROP_AC)
			return wPn;
	}

	return PN0;
}

//����:ȡ�ò�����Ľ��߷�ʽ
BYTE GetConnectType(WORD wPn)
{
	BYTE bBuf[16];

	ReadItemEx(BN0, wPn, 0x8910, bBuf);
	if (bBuf[0] == 3)		 //��������
		return CONNECT_3P3W; //�ն˽��߷�ʽ 1	1:����;3:��������;4:��������
	else if (bBuf[0] == 1) 	 //�����
		return CONNECT_1P;
	else //��������
		return CONNECT_3P4W; 
}

/*
�澯�ж������֣��������ֵ��
���λ0�����λ255�ֱ��Ӧ
�������E2000001H~E20000FFH
�ı�����0���Σ�1�����Ρ�
*/
bool IsAlrEnable(DWORD dwAlrID)
{
	BYTE bAlrMask[32];
	WORD n = (WORD)(dwAlrID - 0xe2000001);

	if (dwAlrID<0xe200001 || dwAlrID>0xe200004e)
		return false;

	//ReadItem(0xE0000137, bAlrMask);
	ReadItemEx(BN0, PN0, 0x8034, bAlrMask);

	if (bAlrMask[n/8] & (1<<n%8))   //1������
	{
// 		DTRACE(DB_TASK, ("%s : alr 0x%04x task enable.\r\n", __FUNCTION__, dwAlrID));
		return true;
	}
	else
	{
// 		DTRACE(DB_TASK, ("%s : alr 0x%04x task disable.\r\n", __FUNCTION__, dwAlrID));
		return false;
	}
}

bool IsCctPn(WORD wPn)
{

	BYTE bProp = GetPnProp(wPn); 
	if(bProp == PN_PROP_METER)
	{
		BYTE bPort = 0;
		if(ReadItemEx(BN0,wPn,0x890a,&bPort)>0)
		{
			if((bPort == PORT_CCT_PLC) || 
				(bPort == PORT_CCT_WIRELESS))
			{
				return true;
			}
		}
	}
	return false;
}
