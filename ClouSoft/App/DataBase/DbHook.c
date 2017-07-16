/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbHook.c
 * ժ    Ҫ�����ļ���Ҫ��������ϵͳ��Ĺҹ�/�ص�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
 * ��    ע��$���ļ���Ҫ�������׼��ӿ�,�벻Ҫ������صĴ�����뵽���ļ�
 *			 $���ļ�����ĺ���,��ʽһ��,����������ͬ�汾�Ĳ�Ʒʱ,����������Ҫ�޸�
 *********************************************************************************************************/
#include "Info.h"
#include "FaConst.h"
#include "DbConst.h"
#include "ComStruct.h"
#include "FaAPI.h"
#include "DbAPI.h"
#include "DbFmt.h"
#include "DataManager.h"
#include "ComAPI.h"
#include "SysDebug.h"
#include "MtrAPI.h"
#include "M590.h"
#include "AutoReader.h"
#include "FaAPI.h"
//�����в��ֵ��ȡ����˲ʱ���ʣ���Ҫͨ���������15���ӵĵ���������
//�����־Ҫ��ʶ���Ƿ���Ҫȡ���ID��Ϊ�����(bit0-7���������1-8)
//	BYTE g_bRedirectPwrID = 0; 

#define CMB_TO_SUB_ID_MAX	16
static const WORD g_wCmbToSubID[][CMB_TO_SUB_ID_MAX] =		//����BANK0�ڲ�ID��645ID��ӳ��
{
//	{0x126f, 0xb66f, 0}, //��λ��

	//���¶�������ȡ��ǰ
	//����
	{0x20cf, 0x8908, 0xa010, 0xb010, 0xa011, 0xb011, 0xa012, 0xb012, 0xa013, 0xb013, 0xa014, 0xb014, 0},
	//����
	{0x20df, 0x8908, 0xa020, 0xb020, 0xa021, 0xb021, 0xa022, 0xb022, 0xa023, 0xb023, 0xa024, 0xb024, 0},
	//����
	{0x20ef, 0x8908, 0xa110, 0xb110, 0xa111, 0xb111, 0xa112, 0xb112, 0xa113, 0xb113, 0xa114, 0xb114, 0},
	//����
	{0x210f, 0x8908, 0xa120, 0xb120, 0xa121, 0xb121, 0xa122, 0xb122, 0xa123, 0xb123, 0xa124, 0xb124, 0},


	//���¶���ȡ��ǰ����
	{0x166f, 0x8908, 0x9010, 0x9011, 0x9012, 0x9013, 0x9014, 0}, 	//��ǰ�����й�
	{0x167f, 0x8908, 0x911f, 0}, 	//��ǰ�����޹�	 
	{0x168f, 0x8908, 0x902f, 0}, 	//��ǰ�����й�
	{0x169f, 0x8908, 0x912f, 0}, 	//��ǰ�����޹�
	{0x16af, 0x8908, 0x913f, 0},	//��ǰһ�����޹�
	{0x16bf, 0x8908, 0x915f, 0},	//��ǰ�������޹�
	{0x16cf, 0x8908, 0x916f, 0},	//��ǰ�������޹�
	{0x16df, 0x8908, 0x914f, 0},	//��ǰ�������޹�

	//�ն���ȡ��1������
	{0x3761, 0x8908, 0x9a10, 0x9a11, 0x9a12, 0x9a13, 0x9a14, 0},	//��1�������й�
	{0x3762, 0x8908, 0x9b1f, 0},	//��1�������޹�	 
	{0x3763, 0x8908, 0x9a2f, 0},	//��1�շ����й�
	{0x3764, 0x8908, 0x9b2f, 0},	//��1�շ����޹�
	{0x3765, 0x8908, 0x9b3f, 0},	//��1��һ�����޹�
	{0x3766, 0x8908, 0x9b5f, 0},	//��1�ն������޹�
	{0x3767, 0x8908, 0x9b6f, 0},	//��1���������޹�
	{0x3768, 0x8908, 0x9b4f, 0}, 	//��1���������޹�


	//�¶���ȡ��1������
	{0x3777, 0x8908, 0x9a10, 0x9a11, 0x9a12, 0x9a13, 0x9a14, 0},	//��1�������й�
	{0x3778, 0x8908, 0x9b1f, 0},	//��1�������޹�  
	{0x3779, 0x8908, 0x9a2f, 0},	//��1�շ����й�
	{0x3780, 0x8908, 0x9b2f, 0},	//��1�շ����޹�
	{0x3781, 0x8908, 0x9b3f, 0},	//��1��һ�����޹�
	{0x3782, 0x8908, 0x9b5f, 0},	//��1�ն������޹�
	{0x3783, 0x8908, 0x9b6f, 0},	//��1���������޹�
	{0x3784, 0x8908, 0x9b4f, 0},	//��1���������޹�

	//�ն�������ȡ��1������
	{0x3785, 0x8908, 0x9c00, 0x9c80, 0x9c01, 0x9c81, 0x9c02, 0x9c82, 0x9c03, 0x9c83, 0x9c04, 0x9c84, 0},	//��1�������й��������������ʱ�䣨�ܡ�����1~M��
	{0x3786, 0x8908, 0xa110, 0xb110, 0xa111, 0xb111, 0xa112, 0xb112, 0xa113, 0xb113, 0xa114, 0xb114, 0},	//��1�������޹��������������ʱ�䣨�ܡ�����1~M��
	{0x3787, 0x8908, 0x9c20, 0x9ca0, 0x9c21, 0x9ca1, 0x9c22, 0x9ca2, 0x9c23, 0x9ca3, 0x9c24, 0x9ca4, 0},	//��1�շ����й��������������ʱ�䣨�ܡ�����1~M��
	{0x3788, 0x8908, 0xa120, 0xb120, 0xa121, 0xb121, 0xa122, 0xb122, 0xa123, 0xb123, 0xa124, 0xb124, 0},	//��1�շ����޹��������������ʱ�䣨�ܡ�����1~M��


	//�¶�������ȡ����������
	{0x3789, 0x8908, 0xa410, 0xb410, 0xa411, 0xb411, 0xa412, 0xb412, 0xa413, 0xb413, 0xa414, 0xb414, 0},	//�����������й��������������ʱ�䣨�ܡ�����1~M��
	{0x3790, 0x8908, 0xa510, 0xb510, 0xa511, 0xb511, 0xa512, 0xb512, 0xa513, 0xb513, 0xa514, 0xb514, 0},	//�����������޹��������������ʱ�䣨�ܡ�����1~M��
	{0x3791, 0x8908, 0xa420, 0xb420, 0xa421, 0xb421, 0xa422, 0xb422, 0xa423, 0xb423, 0xa424, 0xb424, 0},	//�����շ����й��������������ʱ�䣨�ܡ�����1~M��
	{0x3792, 0x8908, 0xa520, 0xb520, 0xa521, 0xb521, 0xa522, 0xb522, 0xa523, 0xb523, 0xa524, 0xb524, 0},	//�����շ����޹��������������ʱ�䣨�ܡ�����1~M��
	//�¶�������ȡ��1������
	{0x3793, 0x8908, 0x9c00, 0x9c80, 0x9c01, 0x9c81, 0x9c02, 0x9c82, 0x9c03, 0x9c83, 0x9c04, 0x9c84, 0},	//��1�������й��������������ʱ�䣨�ܡ�����1~M��
	{0x3794, 0x8908, 0xa110, 0xb110, 0xa111, 0xb111, 0xa112, 0xb112, 0xa113, 0xb113, 0xa114, 0xb114, 0},	//��1�������޹��������������ʱ�䣨�ܡ�����1~M��
	{0x3795, 0x8908, 0x9c20, 0x9ca0, 0x9c21, 0x9ca1, 0x9c22, 0x9ca2, 0x9c23, 0x9ca3, 0x9c24, 0x9ca4, 0},	//��1�շ����й��������������ʱ�䣨�ܡ�����1~M��
	{0x3796, 0x8908, 0xa120, 0xb120, 0xa121, 0xb121, 0xa122, 0xb122, 0xa123, 0xb123, 0xa124, 0xb124, 0},	//��1�շ����޹��������������ʱ�䣨�ܡ�����1~M��


	//�¶���ȡ����������
	{0x3877, 0x8908, 0x941f, 0},	//�����������й�
	{0x3878, 0x8908, 0x951f, 0},	//�����������޹� 
	{0x3879, 0x8908, 0x942f, 0},	//�����շ����й�
	{0x3880, 0x8908, 0x952f, 0},	//�����շ����޹�
	{0x3881, 0x8908, 0x953f, 0},	//������һ�����޹�
	{0x3882, 0x8908, 0x955f, 0},	//�����ն������޹�
	{0x3883, 0x8908, 0x956f, 0},	//�������������޹�
	{0x3884, 0x8908, 0x954f, 0},	//�������������޹�





	{0x8e70, 0xb6a0, 0}, //�������
};

//����ʱ��ID���
bool IsDemandTimeId(WORD wID)
{
	if (wID>=0xe910 && wID<=0xe930)//��ǰA/B/C�������й������������ʱ��
		return true;
	if (wID>=0xe911 && wID<=0xe931)//��ǰA/B/C�෴���й������������ʱ��
		return true;
	if (wID>=0xe912 && wID<=0xe932)//��ǰA/B/C������޹�1�����������ʱ��
		return true;
	if (wID>=0xe913 && wID<=0xe933)//��ǰA/B/C������޹�2�����������ʱ��
		return true;
	if (wID>=0xe914 && wID<=0xe934)//��ǰA/B/C���һ�����޹������������ʱ��
		return true;
	if (wID>=0xe915 && wID<=0xe935)//��ǰA/B/C��ڶ������޹������������ʱ��
		return true;
	if (wID>=0xe916 && wID<=0xe936)//��ǰA/B/C����������޹������������ʱ��
		return true;
	if (wID>=0xe917 && wID<=0xe937)//��ǰA/B/C����������޹������������ʱ��
		return true;
	if (wID>=0xe918 && wID<=0xe938)//��ǰA/B/C���������������������ʱ��
		return true;
	if (wID>=0xe919 && wID<=0xe939)//��ǰA/B/C�෴�����������������ʱ��
		return true;

	if (wID>=0xec10 && wID<=0xec30)//�¶���A/B/C�������й������������ʱ��
		return true;
	if (wID>=0xec11 && wID<=0xec31)//�¶���A/B/C�෴���й������������ʱ��
		return true;
	if (wID>=0xec12 && wID<=0xec32)//�¶���A/B/C������޹�1�����������ʱ��
		return true;
	if (wID>=0xec13 && wID<=0xec33)//�¶���A/B/C������޹�2�����������ʱ��
		return true;
	if (wID>=0xec14 && wID<=0xec34)//�¶���A/B/C���һ�����޹������������ʱ��
		return true;
	if (wID>=0xec15 && wID<=0xec35)//�¶���A/B/C��ڶ������޹������������ʱ��
		return true;
	if (wID>=0xec16 && wID<=0xec36)//�¶���ǰA/B/C����������޹������������ʱ��
		return true;
	if (wID>=0xec17 && wID<=0xec37)//�¶���A/B/C����������޹������������ʱ��
		return true;
	if (wID>=0xec18 && wID<=0xec38)//�¶���A/B/C���������������������ʱ��
		return true;
	if (wID>=0xec19 && wID<=0xec39)//�¶���A/B/C�෴�����������������ʱ��
		return true;

	return false;
}

//Ϊ����ʱ�������
bool AddDemandTimeYear(BYTE* pbBuf)
{
	TTime tmTimeNow;
	GetCurTime(&tmTimeNow);

	if(pbBuf==NULL)
		return false;

	if(0==BcdToDWORD(pbBuf,4))
	{
		memset(pbBuf,0,5);
		return false;
	}

	if(BcdToByte(pbBuf[3])<tmTimeNow.nMonth)
		pbBuf[4] = ByteToBcd(tmTimeNow.nYear%100-1);
	else
		pbBuf[4] = ByteToBcd(tmTimeNow.nYear%100);

	return true;
}

//����:��ֱ��ģ�����������Ƿ���Ч
bool IsPnValid(WORD wPn)
{
	int iRet;
	BYTE bBuf[4];

	if (wPn == PN0)
		return true;

	iRet = ReadItemEx(BN0, wPn, 0x8900, bBuf);
	if (iRet <= 0)
		return false;

	return (bBuf[0] != 0x00);
}


//����:ȡ���ID����ID��ӳ������
const WORD* CmbToSubID(WORD wBn, WORD wID)
{
	WORD i, wNum;
	if (wBn != BN0)
		return NULL;

	//����BANK��BANK0��ͬ�������ID����
	wNum = sizeof(g_wCmbToSubID) / (sizeof(WORD)*CMB_TO_SUB_ID_MAX);
	for (i=0; i<wNum; i++)
	{
		if (wID == g_wCmbToSubID[i][0])
		{
			return &g_wCmbToSubID[i][1];
		}
	}
	
	return NULL;
}


//����:ȡ���ID����ID�ĸ���
WORD CmbToSubIdNum(WORD wBn, WORD wID)
{
	const WORD* pwSubID = CmbToSubID(wBn, wID);
	WORD wNum;

	if (pwSubID == NULL)
		return 1;
	
	wNum = 0;	
	while (*pwSubID++ != 0)	//�����IDת�������ζ���ID�Ķ�
		wNum++;
	
	return wNum;
}


bool IsSP_TDMeter(BYTE bMain, BYTE bSub)
{
	if (bMain == MAINTYPE_CLASSD)
	{
		if (bSub==4 || bSub==5)
			return true;
		else
			return false;
	}
	else if (bMain == MAINTYPE_CLASSE)
	{
		if (bSub==14 || bSub==15)
			return true;
		else
			return false;
	}
	else
		return false;
}

//#ifndef SYS_WIN
//__no_init char g_szBootloader[32] @ 0x20001000;
//#endif

extern void ResetMtrUnsupIdFlg(WORD wPN);
//����:д������Ĺҹ�,�������д���Զ�����,����һЩ�Ǳ�׼�Ĳ���
//����:@nRet �ڵ���WriteItemEx()ʱ�ķ���ֵ,Ӧ��Ϊ����
int PostWriteItemExHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, int nRet)
{
	BYTE bBuf[10];
	DWORD dwSeconds=0, dwTimes = 0;

#ifdef SYS_WIN
	SYSTEMTIME tm;
	DWORD dwErr = 0;
#endif

	if (wBank==BN0 && wID==0x8030) //�ն�ʱ��
	{
		memset(bBuf, 0,sizeof(bBuf));
		ReadItemEx(BN0, PN0, 0xc850, bBuf);
		dwTimes = BcdToDWORD(bBuf, 3);
		dwTimes++;
		DWORDToBCD(dwTimes, bBuf, 3);
		WriteItemEx(BN0, PN0, 0xc850, bBuf);

	    //if (!IsPowerOff())
	    {//ͣ���ʱ����������ն�ʱ��
		    TTime time;
		    time.nSecond = BcdToByte(pbBuf[0]);
		    time.nMinute = BcdToByte(pbBuf[1]);
		    time.nHour = BcdToByte(pbBuf[2]);
		    time.nDay = BcdToByte(pbBuf[3]);
		    time.nMonth = BcdToByte(pbBuf[4]);
		    time.nYear = 2000 + BcdToByte(pbBuf[5]);

		    dwSeconds = TimeToSeconds(&time);
		    dwSeconds += 2;
		    SecondsToTime(dwSeconds, &time);

		    time.nWeek = DayOfWeek(&time);

#ifdef SYS_WIN			
			memset(&tm,0,sizeof(tm));

			tm.wYear = time.nYear;
			tm.wMonth = time.nMonth;
			tm.wDay = time.nDay;
			tm.wHour = time.nHour;
			tm.wMinute = time.nMinute;
			tm.wSecond = time.nSecond;
			tm.wDayOfWeek = DayOfWeek(&time) - 1;
			
			if (!SetLocalTime(&tm))	
			{
				nRet = -(ERR_INVALID + nRet*0x100); 
			}
#else
    		if (!SetSysTime(&time))
	    		nRet = -(ERR_INVALID + nRet*0x100); 
#endif
	    	/*if (nRet >= 0)//�����ʱ���˶���ʱ����Ҫ���в����ݶ���
			{
	    		//FrzAcData();

				DTRACE(DB_FAPROTO, ("PostWriteItemExHook: set system time ok!\r\n"));
			}*/


	   }
	  /* else	 
	   {
		   DTRACE(DB_FAPROTO, ("PostWriteItemExHook: due to terminal power off!\r\n"));
	        nRet = -(ERR_PERM + nRet*0x100);
	   }*/
	}
	else if (wBank==BN0 && wID==0x8032) //�ն˸�λ
	{
		if(*pbBuf == 0x00)
		{
			SetInfo(INFO_RST_PARA);
		}
		else if (*pbBuf == 0x01)
		{
			//SetInfo(INFO_RST_DATA);
			{
				DTRACE(DB_CRITICAL, ("SlowSecondThread : rx INFO_RST_DATA at Click=%d......\n", GetClick()));
				StopMtrRd(0);
				FaResetData();		//����������м�����
				UnLockDB();
				ResetPowerOffTmp();
				SavePoweroffTmp(false);
				SetInfo(INFO_TASK_PARA);    //g_fUpdFatOk = UpdFat();		//����Flash��̬���䣬һ��Ҫ����InitDB֮��
                Sleep(2000);                
				//ResetCPU();
			}
		}
		else if (*pbBuf == 0x02)
		{
			SetInfo(INFO_APP_RST);
		}
	}
	else if (wBank==BN0 && wPn==PN0 &&wID==0x8f04) //������
	{
		if (*pbBuf == 0x00)
			SetInfo(INFO_CLR_DEMAND);
	}
	else if(wBank==BN0 && wID==0x8040) //����������Ϊ �����գ� ������Ϊ����Ͷ�����
	{
		/*BYTE bBuf[4];
		ReadItemEx(BN3, PN0, 0x3200, bBuf);
		if (bBuf[0] != 0)
		{
			nRet = WriteItemEx(BN0, PN0, 0x806e, bBuf);
		}*/
	}
	else if (wBank==BN0 && (wID&0xfff0)==0x8900)//(wID==0x890f || wID==0x8902))
	{
		if(wID==0x890f || wID==0x8902)
		{
#ifdef EN_CCT
			SetInfo(INFO_PLC_PARA);
			g_fCctInitOk = false;
#endif
		}
		DoMtrRdStat();
	}
	else if (wBank==BN0 && wPn==PN0 && wID==0x80a1)
	{
		if (*pbBuf == 0x00)
		{
			bBuf[0] = 0x01;
			WriteItemEx(BN2, PN0, 0x2042, bBuf);
		}
		else
		{
			bBuf[0] = 0x00;
			WriteItemEx(BN2, PN0, 0x2042, bBuf);
		}
	}

	return nRet;
}

//����:��������Ĺҹ�,������������Զ�����,����һЩ�Ǳ�׼�Ĳ���
//����:@nRet �ڵ���ReadItemEx()ʱ�ķ���ֵ,Ӧ��Ϊ����
int PostReadItemExHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, int nRet)
{	
	if (wBank==BN0 && wID==0x8030) //�ն�ʱ��
	{
		TTime tmNow;
		GetCurTime(&tmNow);
		*pbBuf++ = ByteToBcd((BYTE)tmNow.nSecond);
		*pbBuf++ = ByteToBcd((BYTE)tmNow.nMinute);
		*pbBuf++ = ByteToBcd((BYTE)tmNow.nHour);
		*pbBuf++ = ByteToBcd((BYTE)tmNow.nDay);
		*pbBuf++ = ByteToBcd((BYTE)tmNow.nMonth);
		*pbBuf++ = ByteToBcd((BYTE)((tmNow.nYear - 2000)&0xff));
		return 6;
	}
	else if (wBank==BN2 && wID==0x1056)
	{
#ifndef SYS_WIN
		DWORD dwIP = GetLocalAddr("ppp0");
		*pbBuf++ = (dwIP>>24)&0xff;
		*pbBuf++ = (dwIP>>16)&0xff;
		*pbBuf++ = (dwIP>>8)&0xff;
		*pbBuf++ = (dwIP>>0)&0xff;
#endif
		return 4;
	}
    else if (wBank==BN0 && wID==0x8040) 
    {
       /* BYTE bBuf[4];
        ReadItemEx(BN3, PN0, 0x3200, bBuf);
        if (bBuf[0] != 0)
        {
            nRet = ReadItemEx(BN0, PN0, 0x806e, pbBuf);
        }*/
    }
    /*else if (wBank==BN0 && (wID&0xfff0)==0xb630)
    {
    	if (wPn>0 && wPn<PN_NUM)
    	{
	    	if ((g_bRedirectPwrID & (1<<(wPn-1))) != 0)
	    	{
	    		if (wID == 0xb630)	    
	    		{		
	    			nRet = ReadItemEx(BN11, wPn, 0x0221, pbBuf);
	    		}
	    		else if (wID == 0xb63f)
	    		{
	    			memset(pbBuf, 0xff, 12);
	    			nRet = ReadItemEx(BN11, wPn, 0x0221, pbBuf);
	    			return 12;
	    		}
	    		else
	    		{	
	    			memset(pbBuf, 0xff, 3);
	    			return 3;
	    		}
	    	}
    	}
    }
    else if (wBank==BN0 && wID==0x8e3f)
    {
    	if (wPn>0 && wPn<PN_NUM)
    	{    		
	    	if ((g_bRedirectPwrID & (1<<(wPn-1))) != 0)
	    	{
	    		if (wID == 0x8e30)
	    		{
	    			nRet = ReadItemEx(BN11, wPn, 0x0222, pbBuf);
	    		}
	    		else if (wID == 0x8e3f)
	    		{
		    		memset(pbBuf, 0xff, 16);
		    		ReadItemEx(BN11, wPn, 0x0222, pbBuf);
		    		return 16;
	    		}
	    		else
	    		{
	    			memset(pbBuf, 0xff, 4);
	    			return 4;	    			
	    		}
	    	}
	    }
    }*/
	//20140225-2
	/*else if (wBank==BN0 && (wID==0xb612 || wID==0xb61f) && wPn==GetAcPn() && GetConnectType(wPn)==CONNECT_3P3W)  //�������������ն�B���ѹ����
	{
		if (wID == 0xB612)
		{
			memset(pbBuf, 0, 2);
		}
		else
		{
			memset(pbBuf+2, 0, 2);
		}
	}
	else if (wBank==BN0 && (wID==0xb622 || wID==0xb62f) && wPn==GetAcPn() && GetConnectType(wPn)==CONNECT_3P3W)
	{
		if (wID == 0xB622)
		{
			memset(pbBuf, 0, 3);
		}
		else
		{
			memset(pbBuf+3, 0, 3);
		}
	}*/
	else if(wBank==BN0 && IsNorSuportId(wID))
	{
		nRet = GetItemLen(wBank,wID);
		memset(pbBuf, 0xff, nRet);
	}
	else if(wBank==BN0 && IsDemandTimeId(wID))
	{
// 		TraceBuf(DB_CRITICAL, "************* PostReadItemExHook: time buf is   ", pbBuf, 5);
		AddDemandTimeYear(pbBuf);
		nRet = GetItemLen(wBank,wID);
	}
    else if (wBank==BN0 && wID==0x8875)
    {
        nRet = ReadItemEx(BN2, PN0, 0x1044, pbBuf);
    }
	else if (wBank==BN0 && wID==0x8863)
	{
		DWORDToBCD(PN_NUM, pbBuf, 2);
		nRet = 2;
	}
	else if (wBank==BN0 && wID==0x8864)
	{
		DWORDToBCD(GetMtrNum(), pbBuf, 2);
		nRet = 2;
	}
	else if (wBank==BN0 && wID==0x8865)
	{
		DWORDToBCD(GetMtrClassNum(0x8906, 0, 0), pbBuf, 2);
		nRet = 2;
	}
	else if (wBank==BN0 && wID==0x8866)
	{
		DWORDToBCD(GetMtrClassNum(0x8904, 0, 2), pbBuf, 2);
		nRet = 2;
	}
	else if (wBank==BN0 && wID==0x8867)
	{
		DWORDToBCD(GetMtrClassNum(0x8906, 0, 1), pbBuf, 2);
		nRet = 2;
	}
	else if (wBank==BN0 && wID==0x8874)
	{
		GetVitalPn(pbBuf);
		nRet = 40;
	}
	
	return nRet;
}


//����:�����������Ĺҹ�,������������Զ�����,����һЩ�Ǳ�׼�Ĳ���
//����:@nRet �ڵ���ReadItemEx()ʱ�ķ���ֵ,Ӧ��Ϊ����
int PostReadCmbIdHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwTime, int nRet)
{	
	const WORD* pwSubID = CmbToSubID(wBank, wID);	
	if (pwSubID == NULL)
		return nRet;

	//Ŀǰ�����ն˳���ʱ��ֱ���滻Ϊ��������������ʱ��
	if (*pwSubID==0x8910)
	{
		TTime time;
		if (dwTime == 0)
		{
			GetCurTime(&time);
		}
		else
		{
			SecondsToTime(dwTime, &time);
		}
		TimeToFmt15(&time, pbBuf);
	}
	else if (*pwSubID==0x8912)
	{
		TTime time;
		if (dwTime == 0)
		{
			GetCurTime(&time);
		}
		else
		{
			SecondsToTime(dwTime, &time);
		}
		TimeToFmt1(&time, pbBuf);
	}

	return nRet;
}

bool PswCheck(BYTE bPerm, BYTE* pbPassword)
{
	return true;
}


//����:��ȡ��ϵͳ����Ч���ݵĶ���
BYTE GetDbInvalidData()
{
	return INVALID_DATA;
}

//����:��ȡ��ϵͳ����Ч���ݵĶ���
BYTE GetInvalidData(BYTE bErr)
{
	if (bErr == ERR_FAIL)	//����ʧ��,���糭��ʧ�ܵ�
		return 0xef;	//���հ汾Ҫ���ֲ�֧��������ͳ���ʧ��������
	else
		return INVALID_DATA;
}

//����:�Ƿ�����Ч���ݣ���Ч���ݿ��ܴ��ڶ��ֶ���
bool IsInvalidData(BYTE* p, WORD wLen)
{
	if (IsAllAByte(p, INVALID_DATA, wLen))
		return true;

	//if (IsAllAByte(p, 0xef, wLen))
	//	return true;

	return false;
}


//��������ȡ��ϵͳ���ߵĶ�����
BYTE GetCurveInterv()
{
	return TIME_UNIT_HOUR;
}
