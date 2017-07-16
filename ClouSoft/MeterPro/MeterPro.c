/*********************************************************************************************************
* Copyright (c) 2011,深圳科陆电子科技股份有限公司
* All rights reserved.
*
* 文件名称：MeterPro.h
* 摘    要：本文件主要包含抄表协议的基本公共函数的实现
* 当前版本：1.0
* 作    者：潘香玲
* 完成日期：2011年3月
* 备    注：
*********************************************************************************************************/

//#include "stdafx.h"
#include "MeterPro.h"
#include "DbAPI.h"
#include "FaAPI.h"
#include "Drivers.h"
#include "MtrCtrl.h"
#include "DrvCfg.h"
#include "StdReader.h"

#define VER_STR	"Ver1.1.86"

const WORD CRC_16[] = 
	{0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF, 
	 0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE, 
	 0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D, 
	 0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC, 
	 0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823, 0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B, 
	 0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A, 
	 0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49, 
	 0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78, 
	 0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067, 
	 0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256, 
	 0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 
	 0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634, 
	 0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3, 
	 0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, 0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92, 
	 0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1, 
	 0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
	};

/*WORD g_wBlockId[] =
	{
	 0x901f, 0x902f, 0x907f, 0x908f, 0x911f, 0x912f, 0x913f, 0x914f, 0x915f, 0x916f, 0x917f, 0x918f,
	 0xa01f, 0xa02f, 0xa11f, 0xa12f, 0xa13f, 0xa14f, 0xa15f, 0xa16f, 
	 0xb01f, 0xb02f, 0xb11f, 0xb12f, 0xb13f, 0xb14f, 0xb15f, 0xb16f, 
	 0x941f, 0x942f, 0x947f, 0x948f, 0x951f, 0x952f, 0x953f, 0x954f, 0x955f, 0x956f, 0x957f, 0x958f, 	
	 0x9a1f, 0x9a2f, 0x9b1f, 0x9b2f, 0x9b3f, 0x9b4f, 0x9b5f, 0x9b6f,
	 0x9c0f, 0x9c1f, 0x9c2f, 0x9c3f, 0x9c8f, 0x9c9f, 0x9caf, 0x9cbf,
	 0xa41f, 0xa42f, 0xa51f, 0xa52f, 0xa53f, 0xa54f, 0xa55f, 0xa56f, 
	 0xb41f, 0xb42f, 0xb51f, 0xb52f, 0xb53f, 0xb54f, 0xb55f, 0xb56f, 
	 0xb61f, 0xb62f, 0xb63f, 0xb64f, 0xb65f, 0xb66f, 0xb67f,
	 0xb71f, 0xb72f, 0xb73f, 0xb74f, //北京97表扩展失压等事件ID
	 0xb31f, 0xb32f, 0xb33f, 0xb34f, 	 	
	 0xc31f, 0xc32f, 
	 0xc33f, 0xc34f, 0xc35f, 0xc36f, 0xc37f, 0xc38f, 0xc39f, 0xc3af, 
	 //以下为按2007版645协议扩展的块数据ID
	 0xc86f, 0xc8bf,
	 0xc90f, 0xc91f, 0xc92f, 0xc93f, 0xc94f, 0xc95f, 0xc96f, 0xc97f, 0xc98f, 
	 0xd02f, 0xd03f,
	 0, 
	};*/

////////////////////////////////////////////////////////////////////////////////////////////
//MtrProIf私有成员变量
BYTE m_bInvdData; 	 //系统无效数据的定义
//共用收发缓存
BYTE m_MtrTxBuf[DYN_PN_NUM][MTR_FRM_SIZE];
BYTE m_MtrRxBuf[DYN_PN_NUM][MTR_FRM_SIZE];

void InitMeterPro()
{
	//DTRACE(DB_CRITICAL, ("InitMeterPro: "VER_STR" init ok.\r\n"));
	m_bInvdData = GetInvalidData(ERR_APP_OK);
	return;
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool MtrProOpenComm(TCommPara* pCommPara)
{
	//pComm->SetTimeouts(1000); //串口缺省延时是1S,以防被某些表协议改到很短了

	bool fCommOpen = false;
	TCommPara CommPara;
	if ( CommIsOpen(pCommPara->wPort) )
	{
		if (CommGetPara(pCommPara->wPort, &CommPara)) //函数内部已有检测串口是否打开	
		{
			fCommOpen = true;
			if ( pCommPara->wPort==CommPara.wPort
				&& pCommPara->dwBaudRate==CommPara.dwBaudRate
				&& pCommPara->bByteSize==CommPara.bByteSize
				&& pCommPara->bParity==CommPara.bParity
				&& pCommPara->bStopBits==CommPara.bStopBits )
			{
				return true;	//串口波特率相同的情况下不用重新再打开串口			
			}
		}
	}
	
	if ( fCommOpen ) 
	{
        if ((pCommPara->wPort!=COMM_GPRS) && (pCommPara->wPort!=COMM_LOCAL))//不能误关GPRS与红外
        {
            if ( !CommClose(pCommPara->wPort) )
            {
                DTRACE(DB_METER, ("CMeterPro::OpenComm : fail to close COM=%d.\r\n", CommPara.wPort));
                return false;
            }
        }
	}

	if ( CommOpen(pCommPara->wPort, pCommPara->dwBaudRate, pCommPara->bByteSize, pCommPara->bStopBits, pCommPara->bParity) )
	{
		return true;
	}
	else
	{
		DTRACE(DB_METER, ("CMeterPro::OpenComm : fail to open COM=%d.\r\n", pCommPara->wPort));
		return false;
	}
}


WORD MtrProSend(WORD wPortNo, BYTE* pbTxBuf, WORD wLen)
{
    static DWORD dwLastClick=0;
	WORD wTmpLen = 0;
	BYTE bTmpBuf[10] = {0,};
	//添加打印信息???
	//char szHeader[32];
	//sprintf(szHeader, "%s rx <--", szProName);
	//#ifdef SYS_WIN
	TraceBuf(DB_645FRM, "MtrPro --> ", pbTxBuf, wLen);
	//#endif
	if (COMM_METER3 == wPortNo)//载波端口
	{
		if (NULL == g_ptStdReader)
		{
			return false;
		}
		memcpy(bTmpBuf, pbTxBuf, sizeof(bTmpBuf));
		wTmpLen = CctMake3762Unlocked(g_ptStdReader, pbTxBuf, wLen);
	}

    if (GetClick()-dwLastClick > 2)
    {
        dwLastClick = GetClick();
        g_b485TxLed = 2;
//        SetLed(true, LED_DOWNT);  //开灯
    }
	
	if (COMM_METER3 == wPortNo)
	{
		if(wTmpLen == CommWrite(wPortNo, g_ptStdReader->m_pbCctTxBuf, wTmpLen, COM_TIMEOUT))
		{
			memcpy(pbTxBuf, bTmpBuf, sizeof(bTmpBuf));
			return wLen;
		}
		else
			return 0;
	}
	else
		return CommWrite(wPortNo, pbTxBuf, wLen, COM_TIMEOUT);
}


WORD GetCRC16(BYTE* pbyt, int iCount)
{
    WORD wCRC = 0;
	int i;

    for (i=0; i<iCount; i++)
	{
        wCRC = ((WORD)(wCRC<<8)) ^ (CRC_16[(wCRC>>8) ^ pbyt[i]]);
    }
    
    return wCRC;
}

void CheckRate(BYTE* pbRateTab, BYTE* pbData, BYTE nlen)
{
	BYTE tBuf[50];
	memset (tBuf, m_bInvdData, 50);
	if (pbRateTab[0]!=0 && pbRateTab[0]<5)
		memcpy(tBuf+(pbRateTab[0]-1)*nlen, pbData, nlen);
	if (pbRateTab[1]!=0 && pbRateTab[1]<5)
		memcpy(tBuf+(pbRateTab[1]-1)*nlen, pbData+nlen, nlen);
	if (pbRateTab[2]!=0 && pbRateTab[2]<5)
		memcpy(tBuf+(pbRateTab[2]-1)*nlen, pbData+nlen*2, nlen);
	if (pbRateTab[3]!=0 && pbRateTab[3]<5)
		memcpy(tBuf+(pbRateTab[3]-1)*nlen, pbData+nlen*3, nlen);
	memcpy(pbData, tBuf, 4*nlen);
}


//描述：是否分相电能的ID
bool IsPhaseEngId(WORD wID)
{
	if ((wID>>4)==0x907 || (wID>>4)==0x908
		|| (wID>>4)==0x917 || (wID>>4)==0x918
		|| (wID>>4)==0x947 || (wID>>4)==0x948
		|| (wID>>4)==0x957 || (wID>>4)==0x958)
	{		
		return true;
	}
	else 
		return false;
}

//描述：是否需量数据（含需量及需量时间）
//描述：是否日冻结需量ID
bool IsDayFrzDemdId(WORD wID)
{
	if ((wID>>4)>=0x9c0 && (wID>>4)<=0x9c3)
		return true;
	else
		return false;
}

//描述：是否日冻结需量时间
bool IsDayFrzDemdTime(WORD wID)
{
	if ((wID>>4)>=0x9c8 && (wID>>4)<=0x9cb)
		return true;
	else
		return false;
}

bool IsDemdId(WORD wID)
{
	if ((wID>>8)==0xa0 || (wID>>8)==0xa1
		|| (wID>>8)==0xa4 || (wID>>8)==0xa5
		|| (wID>>8)==0xb0 || (wID>>8)==0xb1
		|| (wID>>8)==0xb4 || (wID>>8)==0xb5
		|| IsDayFrzDemdId(wID) || IsDayFrzDemdTime(wID)) //日冻结需量
		return true;
	else 
		return false;
}

//描述：是否上月数据
bool IsLastMonthId(WORD wID)
{
	if ((wID>>8)==0x94 || (wID>>8)==0x95
		|| (wID>>8)==0xa4 || (wID>>8)==0xa5
		|| (wID>>8)==0xb4 || (wID>>8)==0xb5)
		return true;
	else 
		return false;
}

//描述：是否需量时间
bool IsDemdTime(WORD wID)
{
	if ((wID>>8)==0xb0 || (wID>>8)==0xb1		
		|| (wID>>8)==0xb4 || (wID>>8)==0xb5
		|| IsDayFrzDemdTime(wID))
		return true;
	else 
		return false;
}

//是否跟费率有关的计量ID
bool  IsRateId(WORD wID)
{
	if ((wID>>8)==0x90 || (wID>>8)==0x91		
		|| (wID>>8)==0xa0 || (wID>>8)==0xa1
		|| (wID>>8)==0xb0 || (wID>>8)==0xb1
		|| (wID>>8)==0x94 || (wID>>8)==0x95
		|| (wID>>8)==0xa4 || (wID>>8)==0xa5
		|| (wID>>8)==0xb4 || (wID>>8)==0xb5
		|| (wID>>8)==0x98 || (wID>>8)==0x99
		|| (wID>>8)==0xa8 || (wID>>8)==0xa9
		|| (wID>>8)==0xb8 || (wID>>8)==0xb9)
		return true;
	else 
		return false;
}


BYTE GetBlockIdNum(WORD wID)
{
	BYTE num = 0;

	if ( IsPhaseEngId(wID) ) //分相电能
		num = 3;
	else if ((wID>>12)==0x9 || (wID>>12)==0xa)
		num = TOTAL_RATE_NUM;
	else if ((wID>>8)==0xb0 || (wID>>8)==0xb1
		|| (wID>>8)==0xb4 || (wID>>8)==0xb5)
		num = TOTAL_RATE_NUM;
	else if ((wID>>4)==0xb61 || (wID>>4)==0xb62)
		num = 3;
	else if ((wID>>4)==0xb63 || (wID>>4)==0xb64 || (wID>>4)==0xb65 || (wID>>4)==0xb67)
		num = 4;
	else if ((wID>>4)==0xb66) //电压电流相位角
		num = 6;
	else if ((wID>>8)==0xb3) //断相
		num = 4;
	else if (wID == 0xc01f)
		num = 2;

	return num;
}


BYTE Get645TypeLength(WORD wID)
{
	BYTE len = 0;

	if ((wID>>12)==0x9 && !IsDayFrzDemdId(wID) && !IsDayFrzDemdTime(wID))	//电量
	{
		len = 4;
	}
	else if ((wID>>12)==0xa || IsDayFrzDemdId(wID))	//需量
	{
		len = 3;
	}
	else if ((wID>>4)==0xB61 //电压
		|| (wID>>4)==0xB62  //电流
		|| (wID>>4)==0xB64  //无功功率
		|| (wID>>4)==0xB65	//功率因素
		|| (wID>>4)==0xB31	//断相次数
		|| wID==0xB212 || wID==0xB213 //编程次数
		|| wID==0xC117 || wID==0xC211 || wID==0xC511) //自动抄表日
	{
		len = 2;
	}
	else if ((wID>>4)==0xB63  //有功功率
		|| (wID>>4)==0xB67	//视在功率
		|| (wID>>4)==0xB32	//断相时间
		|| wID==0xB214	//电池时间
		|| wID==0xC011	//时间
		|| wID==0xC030 || wID==0xC031)//脉冲常数   
	{
		len = 3;
	}
	else  if ( IsDemdTime(wID) )  //需量时间
	{
		len = 4;
	}
	else if ((wID>>4)==0xB33  //断相时刻
		|| (wID>>4)==0xB34
		|| wID==0xC010 //日期
		|| wID==0xB210 || wID==0xB211 //编程时间  
		|| wID==0xC119 || wID==0xC11A //有/无功电能起始读数
		|| wID==0xC212 || wID==0xC510)
	{
		len = 4;
	}
	else if ((wID>>4)==0xC02 //状态字
		|| (wID>>4)==0xC31 //年时区
		|| (wID>=0xC111 && wID<=0xC118 && wID!=0xC117)
		|| wID==0xC41E)
	{
		len = 1;
	}
	else if (((wID>>4)>=0xc32 && (wID>>4)<=0xc3a) || (wID>>4)==0xC41) //费率参数
	{
		len = 3;
	}
	else if (wID>=0xc032 && wID<=0xc034)
	{
		len = 6;
	}
	else if (wID == 0xC040)
	{
		len = 7;
	}

	return len;
}


WORD  SetCommDefault(WORD wID, BYTE* pbBuf)
{
	int i=0;
	BYTE num = 0;
	WORD len = 0;
	int32 Int32TmpVal[6];
	int64 DDTmpVal[TOTAL_RATE_NUM];
	//int32 Int32TmpVal[TOTAL_RATE_NUM];

	if ((wID&0xf) == 0xf)
		num = GetBlockIdNum(wID);
	else 
		num = 1;

	if ((wID>>12)==0x9 && !IsDayFrzDemdId(wID) && !IsDayFrzDemdTime(wID)) //电量
	{		
		//SetArrVal64((int64*)pbBuf, INVALID_VAL64, num);				
		len = num*sizeof(int64);
		for (i=0; i<num; i++)
			DDTmpVal[i] = INVALID_VAL64;
		memcpy(pbBuf, &DDTmpVal, len);
	}
	else if ((wID>>12)==0xa || IsDayFrzDemdId(wID))	//需量DWORD
	{		
		//SetArrVal32((int32*)pbBuf, INVALID_VAL, num);			
		len = num*sizeof(int32);
		for (i=0; i<num; i++)
			Int32TmpVal[i] = INVALID_VAL;
		memcpy(pbBuf, &Int32TmpVal, len);
	}
	else if ((wID>>4)==0xB61  //电压
		|| (wID>>4)==0xB62  //电流
		|| (wID>>4)==0xB63  //有功功率
		|| (wID>>4)==0xB64  //无功功率
		|| (wID>>4)==0xB65 //功率因素
		|| (wID>>4)==0xB66 //相位角
		|| (wID>>4)==0xB67) //视在功率
	{
		//SetArrVal32((int32*)pbBuf, INVALID_VAL, num);	
		len = num*sizeof(int32);
		for (i=0; i<num; i++)
			Int32TmpVal[i] = INVALID_VAL;
		memcpy(pbBuf, &Int32TmpVal, len);
	}
	else  if ( IsDemdTime(wID) )  //需量时间
	{	
		len = num*4;
		memset(pbBuf, (BYTE)m_bInvdData, len);			
	}
	else if (wID == 0xc010)
	{	
		len = 3;
		memset(pbBuf, (BYTE)m_bInvdData, len);
	}
	else if (wID == 0xc011)
	{		
		len = 4;
		memset(pbBuf, (BYTE)m_bInvdData, len);
	}
 
	return len;

}

float POW10(signed char n)//10的n次方
{
	float f = 1;
	BYTE sign = 0;
	int i;
	
	if (n < 0)
	{
		n = (0xff-n)+1;
		sign = 1;
	}	
	
	for (i=0; i<n; i++)
	{
		if (sign == 1)
			f = f/10;
		else 
			f = f*10;
	}	
	
	return f;
}

int64 POW10N(signed char n)//10的n次方（n> 0）
{
	int64 iVal64 = 1, i;	
	for (i=0; i<n; i++)
	{
		iVal64 = iVal64*10;
	}	
	
	return iVal64;
}

int64 Round(int64 iVal64)
{
	return (iVal64%10<5) ? iVal64/10:(iVal64/10 + 1); //四舍//五入	
}

void CheckDecimal(BYTE bToltLen, BYTE bItemLen, BYTE bNewDec, BYTE bOldDec, BYTE* pbBuf)
{
	int64 iVal64;
	float f;
	BYTE i,j;

	if (bOldDec == bNewDec) //如果小数位与缺省的相同则不用转换
		return;
    if (bItemLen == 0)
        return;
	for (i=0; i<bToltLen/bItemLen; i++)
	{
		if ( !IsBcdCode(pbBuf+i*bItemLen, bItemLen) ) //有非法数据
			continue;

		iVal64 = 0;
		for (j=0; j<bItemLen; j++)
		{
			iVal64 = iVal64+(((*(pbBuf+bItemLen*i+j))&0xf) + ((*(pbBuf+bItemLen*i+j))>>4)*10)*POW10N(2*j);  
		}	

		if( bOldDec >= bNewDec )
			iVal64 = iVal64*POW10N(bOldDec-bNewDec);
		else
			f = iVal64*POW10(bOldDec-bNewDec);

		if (bOldDec != bNewDec)
		{
			if (bOldDec >= bNewDec)
			{				
				iVal64 = Round(iVal64*10);
			}
			else
			{
				iVal64 = Round((DWORD)(f*10));				
			}
		}
		DWORDToBCD((DWORD)iVal64, pbBuf+bItemLen*i, bItemLen);	
	}
}

void CheckDecimalNew(BYTE bDstLen, BYTE bSrcLen, BYTE bDstDec, BYTE bSrcDec, BYTE* pbDstBuf, BYTE* pbSrcBuf)
{
	uint64 iVal64;
	float f;	
	BYTE j;

	if (bDstLen==bSrcLen && bDstDec==bSrcDec)
	{
		memcpy(pbDstBuf, pbSrcBuf, bSrcLen);
		return;
	}
	
	iVal64 = 0;
	for (j=0; j<bSrcLen; j++)
	{
		iVal64 = iVal64+(((*(pbSrcBuf+j))&0xf) + ((*(pbSrcBuf+j))>>4)*10)*POW10N(2*j);  
	}	

	if (bDstDec >= bSrcDec)
	{
		iVal64 = iVal64*POW10N(bDstDec-bSrcDec);
		iVal64 = Round(iVal64*10);	
	}
	else
	{
		f = iVal64*POW10(bDstDec-bSrcDec);
		iVal64 = Round((DWORD)(f*10));
	}	

	Uint64ToBCD(iVal64, pbDstBuf, bDstLen);		
}

//描述:	从字符串中找出对应字符的位置
//参数:	@pStr		需要查找的目标字符串
//		@iStrLen	目标字符串的长度
//		@c			标志字符
//返回: 返回位置 -1 表示查找失败
int SchStrPos(char* pStr, int iStrLen, char c)
{
	int i;
	for (i=0; i<iStrLen; i++)
	{
		if (pStr[i] == c)
			break;
	}

	if (i == iStrLen)
		return -1;
	else 
		return i;
}

//描述：处理DLT645数据格式向通用数据格式的转换
//参数：@wID 		数据项ID号
//		@pbBuf 		指向数据数组的指针, 既为输入的数组也做输出的数组
//      @wLen		输入645格式数据的长度
//返回：转换成功则返回通用数据的长度.否则返回0
WORD Data645ToComm(WORD wID, BYTE* pbBuf, WORD wLen)
{		
	WORD i;	
	WORD tLen = 0;//新总长
	WORD oLen = 0;//原子ID长
	WORD nLen = 0;//新子ID长
	DWORD dwVal;

	int32 iVal32;
	uint64 iVal64;	
	BYTE bItemLen;

	BYTE mBuf[80];
	memset(mBuf, 0, 80);		
	
	switch (wID>>8)
	{
		case 0x90://可兼容分相电能
		case 0x91:			
		case 0x94:
		case 0x95://电能+2小数位
		case 0x9a:			
		case 0x9b://日冻结的电量
				oLen = 4;
				nLen = oLen;
				for (i=0; i<wLen/oLen; i++)
				{					
					if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //无效数据	
					{
						memset(&mBuf[i*nLen], m_bInvdData, nLen);
						continue;
					}									
					CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 2, 2, &mBuf[i*nLen], &pbBuf[i*oLen]);				
				}
				tLen = i*nLen;
				memcpy(pbBuf, mBuf, tLen);
				break;
		case 0x9c:	//日冻结的需量及需量发生时间
			if ( IsDayFrzDemdId(wID) )			
				oLen = 3;			
			else if ( IsDayFrzDemdTime(wID) )			
				oLen = 4;			
			nLen = oLen;

			if ( IsDayFrzDemdId(wID) )	
			{
				for (i=0; i<wLen/oLen; i++)
				{					
					if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //无效数据						
						continue;									
					CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 4, 4, &mBuf[i*nLen], &pbBuf[i*oLen]);				
				}
				tLen = i*nLen;
				memcpy(pbBuf, mBuf, tLen);				
			}

			tLen = wLen;
			break;	
		case 0xa0:
		case 0xa1:	
		case 0xa4:
		case 0xa5:		
				oLen = 3;
				nLen = oLen;
				for (i=0; i<wLen/oLen; i++)
				{					
					if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //无效数据						
						continue;									
					CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 4, 4, &mBuf[i*nLen], &pbBuf[i*oLen]);				
				}
				tLen = i*nLen;
				memcpy(pbBuf, mBuf, tLen);
				tLen = wLen;
				break;
		case 0xb0:
		case 0xb1:	
		case 0xb4:
		case 0xb5:
				oLen = 5;
				nLen = oLen;
				tLen = wLen;
				break;
		case 0xb6:
				switch (wID>>4)
				{
					case 0xb61://电压+1小数位
							oLen = 2;	
							nLen = oLen;							
							for (i=0; i<wLen/oLen; i++)
							{					
								if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //无效数据						
									continue;												
								CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 1, 0, &mBuf[i*nLen], &pbBuf[i*oLen]);				
							}
							tLen = i*nLen;
							memcpy(pbBuf, mBuf, tLen);								
							break;
					case 0xb62://电流+1符号位
					case 0xb66://相位角+1符号位
							oLen = 2;
							nLen = oLen;
							for (i=0; i<wLen/oLen; i++)
							{
								if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //无效数据
									continue;
								if ((wID>>4) == 0xb62)
								{
									nLen = 3;
									CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 3, 2, &mBuf[i*nLen], &pbBuf[i*oLen]);		
								}
								else
									CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 2, 2, &mBuf[i*nLen], &pbBuf[i*oLen]);	
								*(mBuf+(i+1)*nLen-1) &= 0x7f;
							}
							tLen = i*nLen;
							memcpy(pbBuf, mBuf, tLen);
							break;
					case 0xb63://有功功率+1符号位
							oLen = 3;
							nLen = oLen;
							for (i=0; i<wLen/oLen; i++)
							{
								if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //无效数据
									continue;
								CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 4, 4, &mBuf[i*nLen], &pbBuf[i*oLen]);		
								*(mBuf+(i+1)*nLen-1) &= 0x7f;
							}
							tLen = i*nLen;
							memcpy(pbBuf, mBuf, tLen);
							break;					
					case 0xb64://无功功率+1符号位+2小数位
							oLen = 2;	
							nLen = oLen+1;			
							for (i=0; i<wLen/oLen; i++)
							{	
								if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //无效数据	
								{
									memset(&mBuf[i*nLen], m_bInvdData, nLen);
									continue;
								}			
								CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 4, 2, &mBuf[i*nLen], &pbBuf[i*oLen]);	
								*(mBuf+(i+1)*nLen-1) &= 0x7f;									
							}
							tLen = i*nLen;
							memcpy(pbBuf, mBuf, tLen);
							break;
					case 0xb65://功率因素-2数位+1符号位
							oLen = 2;	
							nLen = oLen;
							for (i=0; i<wLen/oLen; i++)
							{						
								if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //无效数据
									continue;
								CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 3, 3, &mBuf[i*nLen], &pbBuf[i*oLen]);
								*(mBuf+(i+1)*nLen-1) &= 0x7f;
							}
							tLen = i*nLen;
							memcpy(pbBuf, mBuf, tLen);
							break;
					case 0xb67://视在功率
							oLen = 3;	
							nLen = oLen;
							for (i=0; i<wLen/oLen; i++)
							{
								if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //无效数据
									continue;
								CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 4, 4, &mBuf[i*nLen], &pbBuf[i*oLen]);		
								*(mBuf+(i+1)*nLen-1) &= 0x7f;
							}
							tLen = i*nLen;
							memcpy(pbBuf, mBuf, tLen);							
							break;						
					default:
							tLen = wLen;		
							break;				
					}
					break;		
			default:
					tLen = wLen;		
					break;					
	}
	
	if (nLen!=0 && oLen!=0) //公共格式需要转为16进制的数据
	{	
		memcpy(mBuf, pbBuf, tLen);
		
		if ((wID&0xf000)==0x9000 && !IsDayFrzDemdId(wID) && !IsDayFrzDemdTime(wID)) //电量
		{	
			bItemLen = sizeof(uint64);
			for (i=0; i<wLen/oLen; i++)
			{
				if (IsAllAByte(mBuf+i*nLen, m_bInvdData, nLen) || !IsBcdCode(mBuf+i*nLen, nLen)) //无效数据 或有非法数据
					iVal64 = (uint64)INVALID_VAL64;
				else
					iVal64 = BcdToUint64(mBuf+i*nLen, nLen);	
				memcpy(pbBuf+i*bItemLen, (BYTE*)&iVal64, bItemLen);
			}
			tLen = i*bItemLen;
		}
		else if ((wID&0xf000)==0xa000 ||  IsDayFrzDemdId(wID)) //需量
		{
			bItemLen = sizeof(DWORD);
			for (i=0; i<wLen/oLen; i++)
			{
				if (IsAllAByte(mBuf+i*nLen, m_bInvdData, nLen) || !IsBcdCode(mBuf+i*nLen, nLen)) //无效数据 或有非法数据
					dwVal = (DWORD)INVALID_VAL;
				else
					dwVal = BcdToDWORD(mBuf+i*nLen, nLen);	
				memcpy(pbBuf+i*bItemLen, (BYTE*)&dwVal, bItemLen);
			}
			tLen = i*bItemLen;
		}
		else if ( IsDemdTime(wID) ) //需量时间
		{
			for (i=0; i<wLen/oLen; i++)
			{
				if ( !IsBcdCode(mBuf+i*nLen, nLen)	//无效数据 或有非法数据
					/*|| BcdToByte(mBuf[i*nLen])>59 
					|| BcdToByte(mBuf[i*nLen+1])>23		
					|| (BcdToByte(mBuf[i*nLen+2])<1 || BcdToByte(mBuf[i*nLen+2])>31) 
					|| (BcdToByte(mBuf[i*nLen+3])<1 || BcdToByte(mBuf[i*nLen+3])>12)*/ )
					memset(mBuf+i*nLen, m_bInvdData, nLen);

				memcpy(pbBuf+i*nLen, mBuf+i*nLen, nLen);		
			}
			tLen = wLen;
		}
		else if ((wID&0xff00) == 0xb600) //瞬时量(电压/电流/功率/功率因素)
		{
			bItemLen = sizeof(int32);
			for (i=0; i<wLen/oLen; i++)
			{
				if (IsAllAByte(mBuf+i*nLen, m_bInvdData, nLen) || !IsBcdCode(mBuf+i*nLen, nLen)) //无效数据 或有非法数据
					iVal32 = INVALID_VAL;
				else
					iVal32 = BcdToDWORD(mBuf+i*nLen, nLen);	//实际最大只有3字节正数,不会超出值域
				memcpy(pbBuf+i*bItemLen, (BYTE*)&iVal32, bItemLen);
			}
			tLen = i*bItemLen;
		}			
	}
	
	return tLen;
}



//描述：获取通用格式块ID数据信息
//参数：@wID 		块数据项ID号
//		bInx		对应g_wBlockId块ID列表中的位置
//		@pbNum 		块中子数据项的个数
//      @pbLen		块中子数据项的长度
//返回：获取成功返回true.
//备注: 其中有功功率块与DLT645有差异, 去掉了上下限值的内容
/*bool GetCommBlockIdInfo(WORD wID, BYTE* pbInx, BYTE* pbNum, BYTE* pbLen)
{
	bool find = false;		
	BYTE i;
	
	*pbNum = 0;
	*pbLen = 0;
	
	for (i=0; i<sizeof(g_wBlockId)/sizeof(WORD); i++)
	{
		if (g_wBlockId[i] == wID)
		{
			find = true;
			*pbInx = i;
			break;
		}		
	}
	
	if ( !find )
	{
		return false;
	}
	
	find = false;

	
	if ((wID&0xf) == 0xf) 
	{
		switch (wID>>8) 
		{
			case 0x90:
			case 0x94:
			case 0x91:		
			case 0x95:	
			case 0x9a:		
			case 0x9b:
				if ( IsPhaseEngId(wID) ) //分相电能
					*pbNum = 3;
				else
					*pbNum = TOTAL_RATE_NUM;
				*pbLen = sizeof(uint64);
				find = true;
				break;	
			case 0x9c: //日冻结需量
				if ( IsDayFrzDemdId(wID) ) //需量
				{
					*pbNum = TOTAL_RATE_NUM;
					*pbLen = sizeof(DWORD);
					find = true;
				}
				else if ( IsDayFrzDemdTime(wID) ) //需量时间
				{
					*pbNum = TOTAL_RATE_NUM;
					*pbLen = 4;
					find = true;
				}
				break;
			case 0xb0:
			case 0xb1:
			case 0xb4:
			case 0xb5:
				*pbNum = TOTAL_RATE_NUM;
				*pbLen = 4;
				find = true;
				break;
			case 0xa0:
			case 0xa1:
			case 0xa4:
			case 0xa5:
				*pbNum = TOTAL_RATE_NUM;
				*pbLen = sizeof(DWORD);
				find = true;
				break;
			case 0xb3:
				switch (wID>>4)
				{
					case 0xb31:
							*pbNum = 4;
							*pbLen = 2;
							find = true;
							break;
					case 0xb32:
							*pbNum = 4;
							*pbLen = 3;
							find = true;
							break;
					case 0xb33:
					case 0xb34:
							*pbNum = 4;
							*pbLen = 4;
							find = true;
							break;
					default:
							break;
				}
				break;
			case 0xb6:
				switch (wID>>4)
				{
					case 0xb61:
					case 0xb62:
							*pbNum = 3;
							*pbLen = sizeof(int32);
							find = true;
							break;
					case 0xb63:
					case 0xb64:												
					case 0xb65:
					case 0xb67:
							*pbNum = 4;
							*pbLen = sizeof(int32);
							find = true;
							break;
					case 0xb66:
							*pbNum = 6;
							*pbLen = sizeof(int32);
							find = true;
							break;
					default:
							break;		
				}
				break;
			case 0xb7:
				switch (wID>>4)
				{
					case 0xb71:
							*pbNum = 4;
							*pbLen = 6;
							find = true;
							break;
					case 0xb72:
					case 0xb73:
							*pbNum = 10;
							*pbLen = 15;
							find = true;
							break;
					case 0xb74:
							*pbNum = 1;
							*pbLen = 1;
							find = true;
							break;
					default:
							break;		
				}
				break;
			case 0xc0:
				if (wID == 0xc02f)
				{
					*pbNum = 3;
					*pbLen = 1;
					find = true;
				}
				break;
			case 0xc3:			 
				switch (wID>>4)
				{
					case 0xc31:
						*pbNum = 5;
						*pbLen = 1;
						find = true;
						break;
					case 0xc32:							
					case 0xc33:
					case 0xc34:
					case 0xc35:
					case 0xc36:
					case 0xc37:
					case 0xc38:
					case 0xc39:
					case 0xc3a:
						*pbNum = 10;
						*pbLen = 3;
						find = true;
						break;						
					default:
						break;		
				}
				break;	
			//以下为按2007版645协议扩展的块数据ID
			case 0xc8:	
				if (wID == 0xc8bf) //电压电流的夹角
				{
					*pbNum = 3;
					*pbLen = 2;
					find = true;
				}
				else if (wID == 0xc86f) //电表状态字的块ID
				{
					*pbNum = 7;
					*pbLen = 2;
					find = true;
				}
				break;	
			case 0xc9:			 
				if (wID>=0xc90f && wID<=0xc98f) //第二套费率时段块
				{						
					*pbNum = 10;
					*pbLen = 3;
					find = true;					
				}
				break;			
			case 0xd0:			 
				if (wID==0xd02f || wID==0xd03f) //结算信息(组合有功电能量)
				{						
					*pbNum = TOTAL_RATE_NUM;
					*pbLen = 4;
					find = true;			
				}
				break;		
			default:
				break;
		}	

	}
	
	return	find;	
}*/

//描述：将非2007版645协议读取的扩展ID转为读相应645ID，以兼容698终端上的读取
//返回：返回相应645ID
WORD Id645V07toDL645(WORD wExtId)
{
	WORD w645Id;

	switch(wExtId)
	{
		case 0xc860://电表运行状态字1
		case 0xc86f://电表运行状态字块
			w645Id = 0xc020;			
			break;
		case 0xc870://电池工作时间
			w645Id = 0xb214;			
			break;
		case 0xc810://编程总次数
			w645Id = 0xb212;	
			break;
		case 0xc811://最近一次编程时间	
			w645Id = 0xb210;
			break;
		case 0xc830://需量清零总次数
			w645Id = 0xb213;		
			break;
		case 0xc831://最近一次清需量时间
			w645Id = 0xb211;		
			break;
		case 0xc871://第一自动结算日
			w645Id = 0xc117;		
			break;
		default:
			w645Id = wExtId;			
			break;
	}

	return w645Id;
}

//描述：将97版645读回的数据转为2007版64的数据格式，以兼容698终端上的读取
//返回：2007版64的格式数据的长度
WORD Data645to645V07(WORD w645Id, BYTE* pbBuf, WORD wLen)
{
	WORD wExtLen;
	BYTE mBuf[20];
	memset(mBuf, 0, sizeof(mBuf));

	switch(w645Id)
	{
		case 0xc020: //高位补一字节0
		case 0xb214:
		case 0xb212:
		case 0xb213:
			pbBuf[wLen] = 0;
			wExtLen = wLen+1;
			break;
		case 0xb210://时间补上秒及年
		case 0xb211:
			memcpy(mBuf+1, pbBuf, wLen);
			memcpy(pbBuf, mBuf, wLen+2);
			wExtLen = wLen+2;
			break;
		case 0xc862://组0xC86F块			
			memcpy(mBuf+4, pbBuf, wLen);
			memcpy(pbBuf, mBuf, 14);
			wExtLen = 14;
			break;
		default:
			wExtLen = wLen;			
			break;
	}

	return wExtLen;
}

//描述：从串口循环读取数据,退出条件如下:
//		1.当开始有读到数据后,后读到数据个数为0则结束
//		2.当开始有读到数据后,后读到数据个数大于给定的缓冲区长度则结束
//		3.当一直未读到数据,则按最小超时时间到则结束
//		4.其他情况最大超时时间到则结束
//参数：@dwDelayTime	读串口的间隔睡眠时间
//		@bMaxSec		最大读串口超时时间,单位:秒
//		@bMinSec		最小读串口超时时间,单位:秒
//		@dwTimeOut		传递到读串口函数的读取超时时间
//		@dwReadLength	每次读串口要读取的字节数
//		@dwBufSize		给定的缓冲区长度
//		@pbBuf			指向给定的缓冲区的指针
//返回：读到的串口数据的总个数
//DWORD ReadComm(struct TMtrPro* pMtrPro, DWORD dwDelayTime, BYTE bMaxSec, BYTE bMinSec, DWORD dwTimeOut, 
//						  DWORD dwReadLength, DWORD dwBufSize, BYTE* pbBuf)
//{
//	bool fBegin = false;
//	DWORD dwOldTick = GetTick();
//	DWORD dwTmpClick;
//	DWORD dwNewClick;	
//	DWORD dwLen = 0;
//	DWORD dwPtr = 0;
//	int i = 0, j = 0;
//	BYTE bBuf[1000];	
//	BYTE bPrintPro;
//	char szProName[20];
//	DWORD wErrCnt = 1;
//	
//	if (pbBuf == NULL)
//		return false;
//	
//	pMtrPro->pfnGetProPrintType(&bPrintPro, szProName);	
//
//	if (dwReadLength > ((dwBufSize>1000)?1000:dwBufSize))
//	{
//		DTRACE(bPrintPro, ("CMeterPro:: MtrPro=%s read length too long!\r\n", szProName));
//		return 0;
//	}
//
//	dwTmpClick = GetClick();
//	dwNewClick = dwTmpClick;	
//	while (dwNewClick-dwTmpClick > bMaxSec)    //n次尝试读取数据
//	{
//		i++;	
//		if (dwDelayTime > 0)
//		{
//			Sleep(dwDelayTime);
//		}	
//		dwLen = CommRead(bBuf, dwReadLength, dwTimeOut);
//	
//		if ((dwLen+dwPtr) >= dwBufSize)
//		{
//			DTRACE(bPrintPro, ("CMeterPro:: MtrPro=%s ReadComm Buffer not enough!\r\n", szProName));
//			break;
//		}
//		else
//			memcpy(pbBuf+dwPtr, bBuf, dwLen);				
//
//		if (dwLen > 0)
//		{
//		    j = 0;
//			dwPtr += dwLen;		
//			fBegin = true;			
//		}
//		else
//		{
//			j++; //连续无通信计数
//			
//		#ifdef SYS_LINUX
//			//20090817 ARM平台下长帧需要读3次才能完整
//			if (dwTimeOut <= 300) //若读延时较快的协议，可多读一次以保证断帧时收数据的可靠性
//				wErrCnt = 2;
//		#endif
//
//			if ((fBegin && j>wErrCnt) || (!fBegin && (dwNewClick-dwTmpClick > bMinSec)))
//				break;		
//		}			
//		dwNewClick = GetClick();
//	}	
//
//	//DTRACE(bPrintPro, ("CMeterPro:: read com , dwPtr=%d, dwclick = %d, RdCount=%d, time=%d, Pn=%d  MtrPro=%s\r\n", 
//	//				  dwPtr, dwNewClick-dwTmpClick, i, GetTick()-dwOldTick, m_pMeterPara->wPn, szProName));	
//
//	if (IsDebugOn(bPrintPro) && IsDebugOn(DB_645FRM))
//	{
//		char szHeader[32];
//		sprintf(szHeader, "%s rx <--", szProName);
//		TraceBuf(DB_645FRM, szHeader, pbBuf, dwPtr);
//	}
//
//	return dwPtr;	
//}
	
//描述：从串口循环读取数据并检测是否有有效帧,退出条件如下:
//		1.当开始有读到数据后,则检测是否有有效帧,有效则返回TRUE
//		2.当开始有读到数据后,为检到有效后读到数据个数为0或大于给定的缓冲区长度返回false
//		3.当一直未读到数据,则按最小超时时间到则返回false
//		4.其他情况最大超时时间到则返回false
//参数：@dwDelayTime	读串口的间隔睡眠时间
//		@bMaxSec		最大读串口超时时间,单位:秒
//		@bMinSec		最小读串口超时时间,单位:秒
//		@dwTimeOut		传递到读串口函数的读取超时时间
//		@dwReadLength	每次读串口要读取的字节数
//		@dwBufSize		传出接收数据的缓冲区的长度,缺省为0,如果有需要传回数据才用到
//		@pbBuf			指向传出接收数据的缓冲区的指针,缺省为空,如果有需要传回数据才用到
//		@dwFrmSize		检验有效帧时所判断的数据的最大长度,缺省为0,目前只有ABB,EDMI协议检验有效帧时用到
//返回：返回true 表示检测到有效帧, false表示未检到有效帧
bool ReadCommFrm(struct TMtrPro* pMtrPro, void* pTmpInf, DWORD dwDelayTime, DWORD dwMaxSec, DWORD dwMinSec, DWORD dwTimeOut, 
						  DWORD dwReadLength, DWORD dwBufSize, BYTE* pbBuf, DWORD dwFrmSize)
{
	bool fBegin = false;
	DWORD dwOldTick = GetTick();
	DWORD dwTmpClick;
	DWORD dwNewClick;	
	//DWORD dwRxTick;
	DWORD dwLen = 0;
	DWORD dwPtr = 0;
	WORD i = 0, j = 0;
	BYTE bBuf[MTR_FRM_SIZE];
	BYTE bPrintPro;
	char szProName[20];
	DWORD wErrCnt = 1;
    static DWORD dwLastClick=0;
    if (COMM_METER3 == pMtrPro->pMtrPara->CommPara.wPort)//载波端口
	{
          dwMaxSec = 35;
		  dwDelayTime = 100;
	}
        
    pMtrPro->pfnGetProPrintType(&bPrintPro, szProName);
        
	dwReadLength = ((dwReadLength > MTR_FRM_SIZE) ? MTR_FRM_SIZE:dwReadLength);
	if(dwBufSize>0 && pbBuf!=NULL)
		dwReadLength = ((dwReadLength > dwBufSize) ? dwBufSize:dwReadLength);

	dwTmpClick = GetClick();
	dwNewClick = dwTmpClick;
	//dwRxTick = 0;
	while (dwNewClick-dwTmpClick < dwMaxSec)    //n次尝试读取数据
	{
		if (COMM_METER3 == pMtrPro->pMtrPara->CommPara.wPort)//载波端口
		{
			if (NULL == g_ptStdReader)
			{
				return false;
			}
			if(RxHandleFrm(g_ptStdReader, dwMaxSec))
			{//接收到376.2完整帧
				if (g_ptStdReader->m_tCctCommPara.m_wRxDataLen <= sizeof(bBuf))//不会溢出直接拷贝
				{
					memcpy(bBuf, g_ptStdReader->m_pbCctRxBuf, g_ptStdReader->m_tCctCommPara.m_wRxDataLen);
					dwLen = g_ptStdReader->m_tCctCommPara.m_wRxDataLen;
				}
				else//有溢出情况，直接退出
					break;
			}
			else//超时
				break;
			//CctRcvFrame(bBuf, (int)dwLen, &g_ptStdReader->m_tCctCommPara, g_ptStdReader->m_ptStdPara->wFrmLenBytes, g_ptStdReader->m_pbCctRxBuf, &g_ptStdReader->m_dwLastRxClick);

			//if (RcvBlock(pbBuf, dwPtr, dwFrmSize)) //检测到有完整帧则退出读数据,以使数据读取过程更快
			if (pMtrPro->pfnRcvBlock(pMtrPro, pTmpInf, bBuf, dwLen, dwFrmSize)) //检测到有完整帧则退出读数据,以使数据读取过程更快
			{
				DTRACE(bPrintPro, ("CMeterPro:: read com OK, dwPtr=%ld, dwclick=%ld, RdCount=%ld, time=%ld, Pn=%d, MtrPro=%s\r\n", 
					dwPtr, dwNewClick-dwTmpClick, i, GetTick()-dwOldTick, 
					pMtrPro->pMtrPara->wPn, szProName));

				//#ifdef SYS_WIN
				if (IsDebugOn(bPrintPro) && IsDebugOn(DB_645FRM))
				{
					char szHeader[32];
					sprintf(szHeader, "%s rx <--", szProName);
					TraceBuf(DB_645FRM, szHeader, bBuf, dwLen);
				}
				//#endif

				return true;
			}
			else
			{
				if (IsDebugOn(bPrintPro) && IsDebugOn(DB_645FRM))
				{
					DTRACE(bPrintPro, ("CMeterPro:: *****dwLen=%ld, dwclick=%ld, RdCount=%ld, time=%ld!\r\n", 
						dwLen, dwNewClick-dwTmpClick, i, GetTick()-dwOldTick));
					//#ifdef SYS_WIN
					TraceBuf(DB_645FRM, "rx <--", bBuf, dwLen);
					//#endif
				}
			}
		}

		i++;	
		if (dwDelayTime > 0)
		{
			Sleep(dwDelayTime);
		}	
		dwLen = CommRead(pMtrPro->pMtrPara->CommPara.wPort, bBuf, dwReadLength, dwTimeOut);
	
		if (pbBuf != NULL) //如果有需要则传回数据，否则直接用临时BUF解析，解析OK的数据已放入各类自己的接收缓存中
		{
			if ((dwLen+dwPtr) >= dwBufSize)
			{
				DTRACE(bPrintPro, ("CMeterPro:: MtrPro=%s ReadComm Buffer not enough!\r\n", szProName));
				break;
			}
			else
				memcpy(pbBuf+dwPtr, bBuf, dwLen);		
		}

		DTRACE(bPrintPro, ("CMeterPro:: read com i=%d tick=%u dwLen=%d, dwPtr=%d\r\n", i, GetTick(), dwLen, dwPtr));

		if (dwLen > 0)
		{
            if (GetClick()-dwLastClick > 2)
            {
                dwLastClick = GetClick();
                g_b485RxLed = 2;
//                SetLed(true, LED_DOWNR);  //开灯
            }    

			//dwRxTick = GetTick();
			j = 0;
			if ((dwLen+dwPtr) >= dwReadLength)
			{
				DTRACE(bPrintPro, ("CMeterPro:: MtrPro=%s ReadComm len over dwReadLength!\r\n", szProName));
				break;
			}
			dwPtr += dwLen;		
			fBegin = true;

// 			if (COMM_METER3 == pMtrPro->pMtrPara->CommPara.wPort)//载波端口
// 			{
// 				if (NULL == g_ptStdReader)
// 				{
// 					return false;
// 				}
//				CctRcvFrame(bBuf, (int)dwLen, &g_ptStdReader->m_tCctCommPara, g_ptStdReader->m_ptStdPara->wFrmLenBytes, g_ptStdReader->m_pbCctRxBuf, &g_ptStdReader->m_dwLastRxClick);
// 			}
			//if (RcvBlock(pbBuf, dwPtr, dwFrmSize)) //检测到有完整帧则退出读数据,以使数据读取过程更快
			if (pMtrPro->pfnRcvBlock(pMtrPro, pTmpInf, bBuf, dwLen, dwFrmSize)) //检测到有完整帧则退出读数据,以使数据读取过程更快
			{
				DTRACE(bPrintPro, ("CMeterPro:: read com OK, dwPtr=%ld, dwclick=%ld, RdCount=%ld, time=%ld, Pn=%d, MtrPro=%s\r\n", 
								   dwPtr, dwNewClick-dwTmpClick, i, GetTick()-dwOldTick, 
								   pMtrPro->pMtrPara->wPn, szProName));
				
				//#ifdef SYS_WIN
				if (IsDebugOn(bPrintPro) && IsDebugOn(DB_645FRM))
				{
					char szHeader[32];
					sprintf(szHeader, "%s rx <--", szProName);
					TraceBuf(DB_645FRM, szHeader, bBuf, dwLen);
				}
				//#endif

				return true;
			}
			else
			{
		    	if (IsDebugOn(bPrintPro) && IsDebugOn(DB_645FRM))
				{
					DTRACE(bPrintPro, ("CMeterPro:: *****dwLen=%ld, dwclick=%ld, RdCount=%ld, time=%ld!\r\n", 
										dwLen, dwNewClick-dwTmpClick, i, GetTick()-dwOldTick));
					//#ifdef SYS_WIN
					TraceBuf(DB_645FRM, "rx <--", bBuf, dwLen);
					//#endif
				}
			}				
		}
		else if (COMM_METER3 != pMtrPro->pMtrPara->CommPara.wPort)//载波端口
		{
#ifdef SYS_LINUX
			if ((dwRxTick!=0 && GetTick()-dwRxTick>2200) || 		//收到过字节后,等待2200毫秒都没收到字节的条件下退出 (fBegin && j>wErrCnt)  
				(dwRxTick==0 && dwNewClick-dwTmpClick>=dwMinSec))	//从没收到过字节,等待dwMinSec退出
			{
				break;
			}
#else
			j ++; //连续无通信计数			
			
			//20090817 ARM平台下长帧需要读3次才能完整
			if (dwTimeOut <= 300) //若读延时较快的协议，可多读一次以保证断帧时收数据的可靠性
				wErrCnt = 2;
			
			if ((fBegin && j>wErrCnt) || (!fBegin && dwNewClick-dwTmpClick>=dwMinSec))
				break;	
#endif
		}

        if (g_fDirRd && !g_bDirRdStep)
            break;

		dwNewClick = GetClick();
	}	

	DTRACE(bPrintPro, ("CMeterPro:: read com Fail, fBegin=%d, dwPtr=%ld, dwclick=%ld, RdCount=%ld, time=%ld Pn=%d, MtrPro=%s\r\n", 
					   fBegin, 
					   dwPtr, dwNewClick-dwTmpClick, i, GetTick()-dwOldTick, 
					   pMtrPro->pMtrPara->wPn, szProName));

	if (IsDebugOn(bPrintPro) && IsDebugOn(DB_645FRM) && pbBuf!=NULL)
	{
		DTRACE(bPrintPro, ("CMeterPro:: MtrPro=%s !", szProName));
		
		//#ifdef SYS_WIN
		TraceBuf(DB_645FRM, "rx <--", pbBuf, dwPtr);
		//#endif
	}

	return false;
}

//描述：是否97版645不支持的数据
bool Is645NotSuptId(WORD wID)
{
	if ( IsPhaseEngId(wID)	//分相电能
		 || ((wID>>8)==0xb6 && wID>0xb65f) //相位角、视在功率、零序电流等不支持	
		 || (wID>>8)==0xb7 //北京97版645表扩展失压等事件ID
		 || wID>=0xc600 //超出97版645的ID列表之外的ID（如07版645的ID或扩展ID）
		 || wID<0x9010)	//超出97版645的ID列表之外的ID
	{
		return true;
	}
	return false;
}

