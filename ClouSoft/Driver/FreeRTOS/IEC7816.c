#include "IEC7816.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <ctype.h> 
#include <stdarg.h> 

#include "Esam.h."
#include "FaCfg.h"

#include "SysDebug.h"
#include "Sysarch.h"
#include "ComAPI.h"

// IEC7816操作命令
#define WARMACTIVATE            0

#define CASE1  1
#define CASE2  2
#define CASE3  3
#define CASE4  4

#define P1     1
#define P2     2
const static TSWTable g_swTable[]=//顺序不能修改
{
	{0x90,0x00,"OK"},//指令执行成功
	{0x67,0x00,"Len wrong"},//数据长度错误
	{0x69,0x84,"no Random"},//没有可用随机数
	{0x90,0x71,"PubKeyFile write wrong"},//写公钥文件错误
	{0x90,0x72,"Command wrong"},//指令结构错误
	{0x90,0x73,"SM1Key wrong"},//SM1密钥错误
	{0x90,0x74,"CheckFileType no match"},//验签文件类型不匹配
	{0x90,0x75,"CheckFile no find"},//验签文件未找到
	{0x90,0x76,"PrivateKeyFile no find"},//产生RSA密钥对时私钥文件未找到
	{0x90,0x77,"PublicKeyFileForCipher no match"},//用来加密的公钥文件不匹配
	{0x90,0x78,"PublicKeyFileForCipher no find"},//用来加密的公钥文件没找到
	{0x90,0x79,"PublicKeyForDecipher no match"},//用来解密的公钥文件不匹配
	{0x90,0x7a,"PublicKeyFileForDecipher no find"},//用来解密的公钥文件没找到
	{0x90,0x82,"RSA cipher wrong"},//RSA加密错误
	{0x90,0x84,"RSA decipher wrong"},//RSA解密错误
	{0x90,0x86,"RSA checkSign wrong"},//RSA验签错误
	{0x90,0x88,"RSA keyPair wrong"},//RSA产生密钥对错误
	{0x90,0x8a,"RSA sign wrong"},//RSA签名错误
	{0x90,0x8c,"SM1 decipher data wrong"},//SM1解密数据错误
	{0x93,0x02,"Line protection wrong"},//SM1解密数据错误
	{0x6f,0x00,"Nothing to get"},//卡中无数据可返回
	{0x6e,0x00,"CLA no valid"},//无效的CLA
	{0x69,0x01,"Status no valid"},//无效的状态
	{0x69,0x81,"P1/P2 no pubfile"},//P1、P2所指的标识符不是响应的公钥文件/不支持此功能
	{0x69,0x82,"Rights no meet"},//增加或修改权限不满足
	{0x69,0x83,"Key locked"},//密钥被锁死
	{0x6a,0x84,"KeyFile space full"},//KEY文件空间已满
	{0x6a,0x86,"Key no find"},//密钥未找到
	{0x6a,0x82,"KeyFile no find"},//KEY文件未找到
    {0x6a,0x80,"Data format error"},//数据格式错误
};


typedef struct MacTable
{
	BYTE bP1;
	BYTE bP2;
}TMacTable;

const TMacTable macTable1[] =
{
	{0x00,0x01},
	{0x15,0x0b},
	{0x16,0x0b},
	{0x17,0x0b},
	{0x18,0x0b},
	{0x19,0x0b},
	{0x1a,0x0b},
	{0x1b,0x0b},
	{0x1c,0x0b},
};

const TMacTable macTable4[] =
{
	{0x00,0x02},
	{0x15,0x0c},
	{0x16,0x0c},
	{0x17,0x0c},
	{0x18,0x0c},
	{0x19,0x0c},
	{0x1a,0x0c},
	{0x1b,0x0c},
	{0x1c,0x0c},
};

const TMacTable macTable5[] =
{
	{0x00,0x03},
	{0x15,0x0d},
	{0x16,0x0d},
	{0x17,0x0d},
	{0x18,0x0d},
	{0x19,0x0d},
	{0x1a,0x0d},
	{0x1b,0x0d},
	{0x1c,0x0d},
};

const TMacTable macTablef[] =
{
	{0x00,0x04},
	{0x15,0x0e},
	{0x16,0x0e},
	{0x17,0x0e},
	{0x18,0x0e},
	{0x19,0x0e},
	{0x1a,0x0e},
	{0x1b,0x0e},
	{0x1c,0x0e},
};

const TMacTable macTable10[] =
{
	{0x00,0x05},
	{0x15,0x0f},
	{0x16,0x0f},
	{0x17,0x0f},
	{0x18,0x0f},
	{0x19,0x0f},
	{0x1a,0x0f},
	{0x1b,0x0f},
	{0x1c,0x0f},
};

TIEC7816 g_ESAM;

const static TMacTable *GetMacTable(BYTE bAFN)
{
	if (bAFN == 0x01) 
		return macTable1;
	if (bAFN == 0x04) 
		return macTable4;
	if (bAFN == 0x05) 
		return macTable5;
	if (bAFN == 0x0f) 
		return macTablef;
	if (bAFN == 0x10) 
		return macTable10;
	DTRACE(DB_FAPROTO,("AFN NO NEED MAC\n"));
	return NULL;
}

static BYTE GetKID(const TMacTable *macTable,BYTE bGrpAddr,BYTE bP12)
{
	if (bP12 == 1) 
		return macTable[bGrpAddr].bP1;
	else
		return macTable[bGrpAddr].bP2;
}

static BYTE memrcpy(BYTE *pbDst,BYTE *pbSrc,BYTE bLen)
{
	for (int i=0;i<bLen;i++) 
	{
		pbDst[i] = pbSrc[bLen-1-i];
	}
	return bLen;
}

int CheckSW1(int iCnt,BYTE *pbBuf)
{
	int i = 0;

	if (iCnt < 2) 
		goto err;

	for(i=0;i<sizeof(g_swTable)/sizeof(TSWTable);i++) 
	{
		if ((pbBuf[iCnt-1]==g_swTable[i].bSW2)&&(pbBuf[iCnt-2]==g_swTable[i].bSW1)) //
		{
            if (i != 0) //只打印错误
    			DTRACE(DB_FAPROTO,("SW: %s\n",g_swTable[i].str));
			return i;
		}
	}

err:
	DTRACE(DB_FAPROTO, ("SW failed\n"));
	return 0xff;//不明响应数据
}

int CheckSW2(int iCnt,BYTE *pbBuf,BYTE bSW1,BYTE bSW2)
{
	int i= 0;
	if (iCnt <= 0)
		return NOSW;

	if ((pbBuf[iCnt-1]==bSW2)&&(pbBuf[iCnt-2]==bSW1)) //
	{
		return 0;
	}
	else
	{
		for (i=0;i<iCnt;i++) 
		{
			if ((pbBuf[i+1]==bSW2)&&(pbBuf[i]==bSW1)) //
			{
				return i;
			}
		}
	}
	return NOSW;//TODo:错误码待处理
}

void CloseESAM(void)
{    
}

bool InitESAM(void)
{
	g_ESAM.fReseted = false;
	g_ESAM.fGetChallenge = false;
    memset(g_ESAM.bRandomNumber, 0, sizeof(g_ESAM.bRandomNumber));    
    
    esam_init();
	g_ESAM.semEsam = NewSemaphore(1, 1);
    	
	return true;
}

void ResetESAM(void)
{    
    esam_warm_reset();
	return;
}

void CheckStatus()
{
	BYTE bBuf[32];

//	if (!g_ESAM.fReseted) 
//		ResetESAM();

	if (!g_ESAM.fGetChallenge) 
	{
		DTRACE(DB_FAPROTO,("GetChallenge status\n"));
		GetChallenge(8,bBuf,2000);
	}
}

BYTE CulcLRC(BYTE *pbBuf, WORD wLen)
{
    WORD i;
    BYTE bLRC = pbBuf[0];
    for (i=1; i<wLen; i++)
        bLRC = bLRC ^ pbBuf[i];
    bLRC = ~bLRC;
    return bLRC;
}

int CheckFrm(BYTE *pbBuf, WORD wFrmLen);

int GetESAMSeris(BYTE *pbBuf,DWORD dwTimeout)
{
    BYTE bBuf[16];
    BYTE *p = bBuf;
    
    int iFrmLen;
    
    *p++ = 0x55;
    
    *p++ = 0x80;
    *p++ = 0x0e;
    *p++ = 0x00;
    *p++ = 0x02;
    *p++ = 0x00;
    *p++ = 0x00;
    
    *p++ = CulcLRC(&bBuf[1], 6);
    
    WaitSemaphore(g_ESAM.semEsam, SYS_TO_INFINITE);
    esam_write(bBuf, p-bBuf);    
    iFrmLen = esam_read(bBuf, sizeof(bBuf));
    SignalSemaphore(g_ESAM.semEsam);
    if (iFrmLen <= 0)
    {
        DTRACE(DB_FAPROTO,("Failed to read ESAM seris\r\n")); 
        return -1;
    }
    if (CheckFrm(bBuf, iFrmLen)<0)
    {
        DTRACE(DB_FAPROTO,("Failed to read ESAM seris\r\n")); 
        return -1;
    }
    memcpy(pbBuf,&bBuf[5],8); 
    return 8;
}

int CheckFrm(BYTE *pbBuf, WORD wFrmLen)
{    
    WORD wDataLen;
    if (pbBuf[0] != 0x55)
    {
        DTRACE(DB_FAPROTO,("Frame head error\r\n")); 
        return -1;
    }
    
    if(CheckSW1(2,&pbBuf[1]) != 0)
    {
   		DTRACE(DB_FAPROTO,("Frame state error\r\n"));
   		return -1;
   	}        
       
    wDataLen = (pbBuf[3]<<8) | pbBuf[4];
    if (wFrmLen != (wDataLen+6))
    {
        DTRACE(DB_FAPROTO,("Frame len error\r\n"));
        return -1;
    }
       
    if (pbBuf[wDataLen+5] != CulcLRC(&pbBuf[3], wDataLen+2))
    {
        DTRACE(DB_FAPROTO,("Frame LRC error\r\n"));
        return -1;
    }

    return 0;
}

int GetChallenge(BYTE bLen,BYTE *pbBuf,DWORD dwTimeout)
{
	BYTE bBuf[32];
	int iCnt = 0;	
	int iSW = 0;

	bBuf[0] = 0x00;
	bBuf[1] = 0x84;
	bBuf[2] = 0x00;
	bBuf[3] = 0x00;
	bBuf[4] = bLen;
    
	WaitSemaphore(g_ESAM.semEsam, SYS_TO_INFINITE);
	if (!g_ESAM.fReseted)//如果在去随机数之前没复位成功过，则在这里再复位下
	{
		ResetESAM();
	}
	if((iCnt=esam_write(bBuf, 5)) != 5)
	{
		DTRACE(DB_FAPROTO,("GetChallenge write failed %d\n",iCnt));
		SignalSemaphore(g_ESAM.semEsam);
		return SENDTIMEOUT;
	}
	TraceBuf(DB_FAPROTO, "GetChallenge write",bBuf,iCnt);

	Sleep(50);

	iCnt = esam_read(bBuf, sizeof(bBuf));
	TraceBuf(DB_FAPROTO, "GetChallenge read",bBuf,iCnt);
	if((iSW=CheckSW1(iCnt,bBuf)) != 0)
	{
		DTRACE(DB_FAPROTO,("%s failed %d~~~\n",__FUNCTION__,iSW));
		ResetESAM();
		SignalSemaphore(g_ESAM.semEsam);
		return -iSW;
	}

	memrcpy(pbBuf,&bBuf[1],bLen);
	memrcpy(g_ESAM.bRandomNumber,&bBuf[1],bLen);
	g_ESAM.fGetChallenge = true;
	SignalSemaphore(g_ESAM.semEsam);
	return bLen;
}

int GetESAMRandomNumber(BYTE *pbBuf,DWORD dwTimeout)
{
	if(IsAllAByte(g_ESAM.bRandomNumber,0,8))
	{
		DTRACE(DB_FAPROTO,("GetChallenge random\n"));
		GetChallenge(8,pbBuf,dwTimeout);
	}
	memcpy(pbBuf,g_ESAM.bRandomNumber,8);
	return 8;
}

int UpdateESAMKey(BYTE *pbKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	BYTE bBuf[300];
	int iCnt = 0;
	int iSW = 0;
	DWORD dwTick;
	bBuf[0] = 0x90;
	bBuf[1] = 0x40;
	bBuf[2] = 0x00;
	bBuf[3] = 0x00;
	bBuf[4] = 0x92;
	bBuf[5] = 0x62;
	bBuf[6] = 0x90;
	memrcpy(&bBuf[7],pbKey,144);//长度固定
	WaitSemaphore(g_ESAM.semEsam, SYS_TO_INFINITE);
    dwTick = GetTick();
    iCnt = 0;
    do 
    {
        Sleep(100);
        iCnt += esam_write(bBuf+iCnt, 151-iCnt);
        if(iCnt == 151) 
            break;
    }while (GetTick()-dwTick < 5*1000);
    TraceBuf(DB_FAPROTO, "UpdateESAMKey write1",bBuf,iCnt);

	dwTick = GetTick();
	do 
	{
		Sleep(100);
		iCnt = esam_read(bBuf, sizeof(bBuf));
		if ((iSW=CheckSW2(iCnt,bBuf,0x90,0x00)) >= 0) 
		{
			DTRACE(DB_FAPROTO,("UpdateESAMKey1 ok\n"));
			break;
		}
	}while (GetTick()-dwTick < dwTimeout);

	TraceBuf(DB_FAPROTO, "UpdateESAMKey read1",bBuf,iCnt);
	bBuf[0] = 0x80;
	bBuf[1] = 0x40;
	bBuf[2] = 0x00;
	bBuf[3] = 0x00;
	bBuf[4] = 0x82;
	bBuf[5] = 0x60;
	bBuf[6] = 0x80;
	memrcpy(&bBuf[7],pbSign,128);
    dwTick = GetTick();
    iCnt = 0;
    do 
    {
        Sleep(100);
        iCnt += esam_write(bBuf+iCnt, 135-iCnt);
        if(iCnt == 135) 
            break;
    }while (GetTick()-dwTick < 5*1000);
    TraceBuf(DB_FAPROTO, "UpdateESAMKey write2",bBuf,iCnt);

	dwTick = GetTick();
	do 
	{
		Sleep(100);
		iCnt = esam_read(bBuf, sizeof(bBuf));
//      TraceBuf(DB_FAPROTO, "UpdateESAMKey read2",bBuf,iCnt);
		if ((iSW=CheckSW2(iCnt,bBuf,0x90,0x00)) >= 0) 
		{
			DTRACE(DB_FAPROTO,("UpdateESAMKey2 ok\n"));
			break;
		}
	}while (GetTick()-dwTick < dwTimeout);

	TraceBuf(DB_FAPROTO, "UpdateESAMKey read2",bBuf,iCnt);

	if((iSW=CheckSW1(iCnt,bBuf)) != 0)
	{
		DTRACE(DB_FAPROTO,("%s failed %d~~~\n",__FUNCTION__,iSW));
		ResetESAM();

        SignalSemaphore(g_ESAM.semEsam);
		return -iSW;
	}

	SignalSemaphore(g_ESAM.semEsam);
	return 0;
}

int GetResponse(BYTE bLen,BYTE *pbBuf,DWORD dwTimeout)
{
	BYTE bBuf[300];
	DWORD dwTick;
	int iCnt = 0;
	int iSW = 0;

	if (dwTimeout==0 || dwTimeout>60*1000)
		dwTimeout = 30*1000;
	
	bBuf[0] = 0x00;
	bBuf[1] = 0xc0;
	bBuf[2] = 0x00;
	bBuf[3] = 0x00;
	bBuf[4] = bLen;
	if((iCnt=esam_write(bBuf, 5)) != 5)
	{
		DTRACE(DB_FAPROTO,("GetResponse write failed %d\n",iCnt));
		return SENDTIMEOUT;
	}
	TraceBuf(DB_FAPROTO, "GetResponse write",bBuf,iCnt);

	dwTick = GetTick();
	do
	{
		Sleep(100);
		iCnt = esam_read(bBuf, sizeof(bBuf));
		if ((iSW=CheckSW2(iCnt,bBuf, 0x61, 0x04)) >= 0) 
			break;
		if ((iSW=CheckSW2(iCnt,bBuf, 0x61, 0x06)) >= 0) 
			break;
		if ((iSW=CheckSW2(iCnt,bBuf, 0x61, 0xfa)) >= 0) 
			break;
		if ((iSW=CheckSW2(iCnt,bBuf, 0x61, 0x1a)) >= 0) 
			break;
		if ((iSW=CheckSW2(iCnt,bBuf, 0x90, 0x00)) >= 0) 
			break;
	}while (GetTick()-dwTick < dwTimeout);

	if(iSW == NOSW)
	{
		ResetESAM();
	}

	TraceBuf(DB_FAPROTO, "GetResponse read",bBuf,iCnt);
	memcpy(pbBuf,bBuf,iCnt);
	return bLen;
}

int VerifyMac(BYTE bAFN, BYTE bA3, BYTE bGrpAddr, BYTE *pbData, BYTE *pbMacData, WORD wDataLen, DWORD dwTimeout)
{
	BYTE bBuf[300];
	int iCnt = 0;
	int i=0;
	//int j=0;
	//BYTE bNum=0;
	WORD wLen = 0;
	WORD wLenAll =0;
	DWORD dwTick;
	bool fSW = false;//正确响应
	bool fLastOne = false;
//  TraceBuf(DB_FAPROTO, "VerifyMac data",wDataLen,pbData);
//  TraceBuf(DB_FAPROTO, "VerifyMac Macdata",4,pbMacData);

	DTRACE(DB_FAPROTO,("VerifyMac--->AFN=%0x,A3=%x,bGrpAddr=%x.\n\n",bAFN,bA3,bGrpAddr));
	DTRACE(DB_FAPROTO,("Get MACData: %02x %02x %02x %02x \n",pbMacData[0],pbMacData[1],pbMacData[2],pbMacData[3]));
//	if(((bAFN==0x05) || (bAFN==0x04)) && (bA3==0) && (bGrpAddr==0))
	{
	//	DTRACE(DB_FAPROTO,("MAC Check OK\n"));
		//return 1;
	}

	//check para
	if ((bA3&0x01) == 0) //使用单地址
	{
		if (bGrpAddr != 0) 
		{
			DTRACE(DB_FAPROTO,("data too long1\n"));
			return -1;
		}
	}
	else//使用组地址
	{
		if ((bGrpAddr<1)||(bGrpAddr>8)) 
		{
			DTRACE(DB_FAPROTO,("data too long2,%d\n",bGrpAddr));
			return -1;
		}
	}

	if ((bAFN!=0x01)&&(bAFN!=0x04)&&(bAFN!=0x05)&&(bAFN!=0x0f)&&(bAFN!=0x10)) 
	{
		DTRACE(DB_FAPROTO,("data too long3 AFN%d\n",bAFN));
		return 1;//此AFN无需校验，直接返回OK
	}
	WaitSemaphore(g_ESAM.semEsam, SYS_TO_INFINITE);

	if (wDataLen > 240) 
	{
		while(1) 
		{
			if (wDataLen == wLenAll) 
			{
				DTRACE(DB_FAPROTO,("link stopped!\n"));
				break;
			}
			if(wDataLen<wLenAll+240)//最后一条
			{
				bBuf[0] = 0x80;//todo:CLA 级联
				bBuf[1] = 0xe8;//INS
				bBuf[2] = GetKID(GetMacTable(bAFN),bGrpAddr,P1);
				bBuf[3] = (GetKID(GetMacTable(bAFN),bGrpAddr,P2)<<2)|0x00;//todo:级联
				bBuf[4] = wDataLen-240*i;
				wLen = wDataLen-240*i;
				wLenAll += wLen;
				fLastOne = true;
				memcpy(&bBuf[5],pbData+240*i,wLen);
				DTRACE(DB_FAPROTO,("link last one! %d,bBuf[4]==%d\n",wLenAll,bBuf[4]));
			}
			else if (i == 0)//第一条 
			{
				bBuf[0] = 0x90;//todo:CLA 级联
				bBuf[1] = 0xe8;//INS
				bBuf[2] = GetKID(GetMacTable(bAFN),bGrpAddr,P1);
				bBuf[3] = (GetKID(GetMacTable(bAFN),bGrpAddr,P2)<<2)|0x01;//todo:级联
				bBuf[4] = 240;
				wLen = 240;
				wLenAll += 240;
				memcpy(&bBuf[5],pbData,240);
				DTRACE(DB_FAPROTO,("link first one! %d\n",wLenAll));
//              TraceBuf(DB_FAPROTO, "first one",wLen+5,bBuf);
			}
			else 
			{
				bBuf[0] = 0x90;//todo:CLA 级联
				bBuf[1] = 0xe8;//INS
				bBuf[2] = GetKID(GetMacTable(bAFN),bGrpAddr,P1);
				bBuf[3] = (GetKID(GetMacTable(bAFN),bGrpAddr,P2)<<2)|0x02;//todo:级联
				bBuf[4] = 240;
				wLen = 240;
				wLenAll += 240;
				memcpy(&bBuf[5],pbData+240*i,240);
			}

			DTRACE(DB_FAPROTO,("p1 %02x,p2 %02x,DataLen %d\n",bBuf[2],bBuf[3],wLen));
            dwTick = GetTick();
            iCnt = 0;
            do 
            {
                Sleep(100);
                iCnt += esam_write(bBuf+iCnt, wLen+5-iCnt);
                if(iCnt == wLen+5) 
                    break;
            }while (GetTick()-dwTick < 5*1000);
			TraceBuf(DB_FAPROTO, "VerifyMac write",bBuf,wLen+5);

			dwTick = GetTick();
			do 
			{
				Sleep(100);
				iCnt = esam_read(bBuf, sizeof(bBuf));
				TraceBuf(DB_FAPROTO, "VerifyMac read1",bBuf,iCnt);
				if (fLastOne) 
				{
					if ((CheckSW2(iCnt,bBuf,0x61,0x04)) >= 0) 
					{
						DTRACE(DB_FAPROTO,("VerifyMacLast ok\n"));
						fSW = true;
						break;
					}
				}
				if ((CheckSW2(iCnt,bBuf,0x90,0x00)) >= 0) 
				{
					DTRACE(DB_FAPROTO,("VerifyMac1 ok\n"));
					fSW = true;
					break;
				}
			}while (GetTick()-dwTick < dwTimeout+2*1000);

			if(fSW) 
			{
				DTRACE(DB_FAPROTO,("VerifyMac~~~~ ok\n"));
				fSW = false;
			}
			else
			{
				DTRACE(DB_FAPROTO,("VerifyMac1 failed\n"));
				SignalSemaphore(g_ESAM.semEsam);
				return -1;
			}
			i++;
			DTRACE(DB_FAPROTO,("wLen %d,wLenAll %d\n",wLen,wLenAll));
		}
	}

	if (wDataLen <= 240) 
	{
		bBuf[0] = 0x90;
		bBuf[1] = 0xe8;//INS
		bBuf[2] = GetKID(GetMacTable(bAFN),bGrpAddr,P1);
		bBuf[3] = (GetKID(GetMacTable(bAFN),bGrpAddr,P2)<<2)|0x03;
		bBuf[4] = wDataLen;
		wLen = wDataLen;

		DTRACE(DB_FAPROTO,("p1 %02x,p2 %02x,DataLen %d\n",bBuf[2],bBuf[3],wLen));
		memcpy(&bBuf[5],pbData,wDataLen);
        dwTick = GetTick();
        iCnt = 0;
        do 
        {
            Sleep(100);
            iCnt += esam_write(bBuf+iCnt, wDataLen+5-iCnt);
            if(iCnt == wDataLen+5) 
                break;
        }while (GetTick()-dwTick < 5*1000);
		TraceBuf(DB_FAPROTO, "VerifyMac write", bBuf, wDataLen+5);

		dwTick = GetTick();
		do 
		{
			Sleep(100);
			iCnt = esam_read(bBuf, sizeof(bBuf));
			if ((CheckSW2(iCnt,bBuf,0x61,0x04)) >= 0) 
			{
				DTRACE(DB_FAPROTO,("VerifyMac1 ok\n"));
				break;
			}
		}while (GetTick()-dwTick < dwTimeout+2*1000);

		TraceBuf(DB_FAPROTO, "VerifyMac read", bBuf, iCnt);
	}

	if(GetResponse(0x04,bBuf, 2*1000) != 0x04)
	{
		DTRACE(DB_FAPROTO,("VerifyMac GetResponse1 failed~~~\n"));
		ResetESAM();
		SignalSemaphore(g_ESAM.semEsam);
		return -1;
	}

	if (bBuf[0]==0xc0) 
	{
		if ((bBuf[5]==0x90) && (bBuf[6]==0x00))
		{
			TraceBuf(DB_FAPROTO, "Get MAC",bBuf+1,4);
			if ((pbMacData[3] == bBuf[1])
				&&(pbMacData[2] == bBuf[2]) 
				&&(pbMacData[1] == bBuf[3])
				&&(pbMacData[0] == bBuf[4]))
			{
				DTRACE(DB_FAPROTO,("MAC Check OK\n"));
				SignalSemaphore(g_ESAM.semEsam);
				return 1;
			}
		}
	}
	else
	{
		DTRACE(DB_FAPROTO,("VerifyMac no ins\n"));
		if ((bBuf[4]==0x90) && (bBuf[5]==0x00))
		{
			TraceBuf(DB_FAPROTO, "Get MAC",bBuf,4);
			if ((pbMacData[3] == bBuf[0])
				&&(pbMacData[2] == bBuf[1]) 
				&&(pbMacData[1] == bBuf[2])
				&&(pbMacData[0] == bBuf[3]))
			{
				SignalSemaphore(g_ESAM.semEsam);
				return 1;
			}
		}
	}

	DTRACE(DB_FAPROTO,("MAC Check Fail\n"));
	SignalSemaphore(g_ESAM.semEsam);
	return -1; 
}
int VerifyPublicKey(BYTE bP2,BYTE *pbSign,BYTE *pbMastRandom,DWORD dwTimeout)
{
	BYTE bBuf[300];
	int iCnt = 0;
	int iSW = 0;
	DWORD dwTick;
	bBuf[0] = 0x80;
	bBuf[1] = 0x42;
	bBuf[2] = 0x00;
	bBuf[3] = bP2;
	bBuf[4] = 0x8c;
	bBuf[5] = 0x81;
	bBuf[6] = 0x08;
	memrcpy(&bBuf[7],pbMastRandom,8);//长度固定
	bBuf[15] = 0x60;
	bBuf[16] = 0x80;
	memrcpy(&bBuf[17],pbSign,128);//长度固定
	WaitSemaphore(g_ESAM.semEsam, SYS_TO_INFINITE);
    dwTick = GetTick();
    iCnt = 0;
    do 
    {
        Sleep(100);
        iCnt += esam_write(bBuf+iCnt, 145-iCnt);
        if(iCnt == 145) 
            break;
 	}while (GetTick()-dwTick < 5*1000);
	TraceBuf(DB_FAPROTO, "VerifyPublicKey write",bBuf,iCnt);

	dwTick = GetTick();
	do 
	{
		Sleep(100);
		iCnt = esam_read(bBuf, sizeof(bBuf));
//      TraceBuf(DB_FAPROTO, "VerifyMac read1",iCnt,bBuf);
		if ((iSW=CheckSW2(iCnt,bBuf,0x90,0x00)) >= 0) 
		{
			DTRACE(DB_FAPROTO,("VerifyPublicKey1 ok\n"));
			break;
		}
	}while (GetTick()-dwTick < dwTimeout);

	TraceBuf(DB_FAPROTO, "VerifyPublicKey read",bBuf,iCnt);
	if((iSW=CheckSW1(iCnt,bBuf)) != 0)
	{
		DTRACE(DB_FAPROTO,("%s failed %d~~~\n",__FUNCTION__,iSW));
		ResetESAM();
		SignalSemaphore(g_ESAM.semEsam);
		return -iSW;
	}
	SignalSemaphore(g_ESAM.semEsam);

	return NOERROR;
}

int UpdateMKeyLocal(BYTE *pbKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	BYTE bBuf[300];
	int iCnt = 0;
	int iSW = 0;
	DWORD dwTick;
	bBuf[0] = 0x90;
	bBuf[1] = 0x34;
	bBuf[2] = 0x00;
	bBuf[3] = 0x00;
	bBuf[4] = 0x92;
	bBuf[5] = 0x62;
	bBuf[6] = 0x90;
	memrcpy(&bBuf[7],pbKey,144);//长度固定
	WaitSemaphore(g_ESAM.semEsam, SYS_TO_INFINITE);
    dwTick = GetTick();
    iCnt = 0;
    do 
    {
        Sleep(100);
        iCnt += esam_write(bBuf+iCnt, 151-iCnt);
        if(iCnt == 151) 
            break;
  	}while (GetTick()-dwTick < 5*1000);
	TraceBuf(DB_FAPROTO, "UpdateMKeyLocal write1",bBuf,iCnt);

	dwTick = GetTick();
	do 
	{
		Sleep(100);
		iCnt = esam_read(bBuf, sizeof(bBuf));
//      TraceBuf(DB_FAPROTO, "VerifyMac read1",iCnt,bBuf);
		if ((iSW=CheckSW2(iCnt,bBuf,0x90,0x00)) >= 0) 
		{
			DTRACE(DB_FAPROTO,("UpdateMKeyLocal1 ok\n"));
			break;
		}
	}while (GetTick()-dwTick < dwTimeout+2*1000);

	TraceBuf(DB_FAPROTO, "UpdateMKeyLocal1 read",bBuf,iCnt);
	bBuf[0] = 0x80;
	bBuf[1] = 0x34;
	bBuf[2] = 0x00;
	bBuf[3] = 0x00;
	bBuf[4] = 0x82;
	bBuf[5] = 0x60;
	bBuf[6] = 0x80;
	memrcpy(&bBuf[7],pbSign,128);//长度固定
    dwTick = GetTick();
    iCnt = 0;
    do 
    {
        Sleep(100);
        iCnt += esam_write(bBuf+iCnt, 135-iCnt);
        if(iCnt == 135) 
            break;
	}while (GetTick()-dwTick < 5*1000);
	TraceBuf(DB_FAPROTO, "UpdateMKeyLocal write", bBuf, iCnt);

	dwTick = GetTick();
	do 
	{
		Sleep(100);
		iCnt = esam_read(bBuf, sizeof(bBuf));
		if ((iSW=CheckSW2(iCnt,bBuf,0x90,0x00)) >= 0) 
		{
			DTRACE(DB_FAPROTO,("UpdateMKeyLocal2 ok\n"));
			break;
		}
	}while (GetTick()-dwTick < dwTimeout+2*1000);
	TraceBuf(DB_FAPROTO, "UpdateMKeyLocal2 read",bBuf,iCnt);

	if((iSW=CheckSW1(iCnt,bBuf)) != 0)
	{
		ResetESAM();
		DTRACE(DB_FAPROTO,("%s failed %d~~~\n",__FUNCTION__,iSW));
		SignalSemaphore(g_ESAM.semEsam);
		return -iSW;
	}

	SignalSemaphore(g_ESAM.semEsam);
	return NOERROR;
}

int UpdateMKeyFar(BYTE *pbSessionKey,BYTE *pbMKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	BYTE bBuf[300];
	BYTE bBuf2[300];
	memrcpy(bBuf2,pbMKey,144);
	int iSW = 0;
	int iCnt = 0;
	DWORD dwTick = 0;

	bBuf[0] = 0x90;
	bBuf[1] = 0x3c;
	bBuf[2] = 0x00;
	bBuf[3] = 0x00;
	bBuf[4] = 0xfa;
	bBuf[5] = 0x63;
	bBuf[6] = 0x80;
	memrcpy(&bBuf[7],pbSessionKey,128);//长度固定
	bBuf[135] = 0x62;
	bBuf[136] = 0x90;
	memcpy(&bBuf[137],bBuf2,118);//长度固定

	WaitSemaphore(g_ESAM.semEsam, SYS_TO_INFINITE);
    dwTick = GetTick();
    iCnt = 0;
    do 
    {
        Sleep(100);
        iCnt += esam_write(bBuf+iCnt, 255-iCnt);
        if(iCnt == 255) break;
  	}while (GetTick()-dwTick < 5*1000);
	TraceBuf(DB_FAPROTO, "UpdateMKeyFar write1",bBuf,iCnt);

	dwTick = GetTick();
	do 
	{
		Sleep(100);
		iCnt = esam_read(bBuf, sizeof(bBuf));
		if ((iSW=CheckSW2(iCnt,bBuf,0x90,0x00)) >= 0) 
		{
			DTRACE(DB_FAPROTO,("UpdateMKeyFar1 ok\n"));
			break;
		}
	}while (GetTick()-dwTick < dwTimeout+2*1000);
	TraceBuf(DB_FAPROTO, "UpdateMKeyFar1 read",bBuf,iCnt);

	bBuf[0] = 0x80;
	bBuf[1] = 0x3c;
	bBuf[2] = 0x00;
	bBuf[3] = 0x00;
	bBuf[4] = 0x9c;
	memcpy(&bBuf[5],bBuf2+118,26);
	bBuf[31] = 0x60;
	bBuf[32] = 0x80;
	memrcpy(&bBuf[33],pbSign,128);

    dwTick = GetTick();
    iCnt = 0;
    do 
    {
        Sleep(100);
        iCnt += esam_write(bBuf+iCnt, 161-iCnt);
        if(iCnt == 161) break;
    }while (GetTick()-dwTick < 5*1000);
	TraceBuf(DB_FAPROTO, "UpdateMKeyFar write2",bBuf,iCnt);

	dwTick = GetTick();
	do 
	{
		Sleep(100);
		iCnt = esam_read(bBuf, sizeof(bBuf));
//      TraceBuf(DB_FAPROTO, "UpdateMKeyFar read",bBuf,iCnt);
		if ((iSW=CheckSW2(iCnt,bBuf,0x90,0x00)) >= 0) 
		{
			DTRACE(DB_FAPROTO,("UpdateMKeyFar ok\n"));
			break;
		}
	}while (GetTick()-dwTick < dwTimeout+2*1000);
	TraceBuf(DB_FAPROTO, "UpdateMKeyFar read",bBuf,iCnt);

	if((iSW=CheckSW1(iCnt,bBuf)) != 0)
	{
		ResetESAM();
		DTRACE(DB_FAPROTO,("%s failed %d~~~\n",__FUNCTION__,iSW));
		SignalSemaphore(g_ESAM.semEsam);
		return -iSW;
	}
	SignalSemaphore(g_ESAM.semEsam);
	return 0;
}

int UpdateSymKey(BYTE bNum,BYTE *pbSessionKey,BYTE *pbTermKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	BYTE bBuf[300];
	int iCnt = 0;
	int iSW = 0;
	DWORD dwTick;
	if (bNum<=0) 
		return NOSW;

	bBuf[0] = 0x90;
	bBuf[1] = 0x3a;
	bBuf[2] = 0x00;
	bBuf[3] = 0x00;
	bBuf[4] = 0xa5;
	bBuf[5] = 0x63;
	bBuf[6] = 0x80;
	memrcpy(&bBuf[7],pbSessionKey,128);//长度固定
	bBuf[135] = 0x64;
	bBuf[136] = 0x00;
	bBuf[137] = 0x20*bNum;
	memrcpy(&bBuf[138], pbTermKey, 32*bNum);//长度固定
	WaitSemaphore(g_ESAM.semEsam, SYS_TO_INFINITE);
    dwTick = GetTick();
    iCnt = 0;
    while(1)
    {
        iCnt += esam_write(bBuf+iCnt, 170-iCnt);
        Sleep(100);
        if(iCnt == 170) 
            break;
    }
	TraceBuf(DB_FAPROTO, "UpdateSymKey write1",bBuf,iCnt);

    dwTick = GetTick();
    do 
    {
        Sleep(100);
        iCnt = esam_read(bBuf, sizeof(bBuf));
        if ((iSW=CheckSW2(iCnt,bBuf,0x90,0x00)) >= 0) 
        {
            DTRACE(DB_FAPROTO,("UpdateSymKey ok1\n"));
            break;
        }
    }while (GetTick()-dwTick < dwTimeout+1*1000);
    TraceBuf(DB_FAPROTO, "UpdateSymKey read1",bBuf,iCnt);

	bBuf[0] = 0x80;
	bBuf[1] = 0x3a;
	bBuf[2] = 0x00;
	bBuf[3] = 0x00;
	bBuf[4] = 0x82+0x20*(bNum-1);

	memrcpy(&bBuf[5], pbTermKey, 32*(bNum-1));//长度固定	

	bBuf[32*(bNum-1)+5] = 0x60;
	bBuf[32*(bNum-1)+6] = 0x80;
	
	memrcpy(&bBuf[32*(bNum-1)+7],pbSign,128);
	iCnt = 0;
	while(1) 
	{
		iCnt += esam_write(bBuf+iCnt, 32*(bNum-1)+135-iCnt);
		if (iCnt == (32*(bNum-1)+135)) 
		{
			break;
		}
	}
	TraceBuf(DB_FAPROTO, "UpdateSymKey write2",bBuf,iCnt);

	dwTick = GetTick();
	do 
	{
		Sleep(100);
		iCnt = esam_read(bBuf, sizeof(bBuf));
		if ((iSW=CheckSW2(iCnt,bBuf,0x90,0x00)) >= 0) 
		{
			DTRACE(DB_FAPROTO,("UpdateSymKey ok2\n"));
			break;
		}
	}while (GetTick()-dwTick < dwTimeout+2*1000);
	TraceBuf(DB_FAPROTO, "UpdateSymKey read2",bBuf,iCnt);

	if((iSW=CheckSW1(iCnt,bBuf)) != 0)
	{
		ResetESAM();//更新失败 
		DTRACE(DB_FAPROTO,("%s failed %d,%d~~~\n",__FUNCTION__,iSW,-iSW));
		SignalSemaphore(g_ESAM.semEsam);
		return -iSW;
	}
	SignalSemaphore(g_ESAM.semEsam);

	return NOERROR;
}

int RegisterNonSymKey(BYTE bP1,BYTE *pbMastRandom,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout,BYTE *pbBuf)
{
	BYTE bBuf[300];
	BYTE bBuf2[300];
	int iCnt = 0;
	int iSW = 0;
	DWORD dwTick = 0;
	bBuf[0] = 0x80;
	bBuf[1] = 0x36;
	bBuf[2] = bP1;
	bBuf[3] = 0x00;
	bBuf[4] = 0x8c;
	bBuf[5] = 0x81;
	bBuf[6] = 0x08;
	memrcpy(&bBuf[7],pbMastRandom,8);//长度固定
	bBuf[15] = 0x60;
	bBuf[16] = 0x80;
	memrcpy(&bBuf[17],pbSign,128);//长度固定

	WaitSemaphore(g_ESAM.semEsam, SYS_TO_INFINITE);
    dwTick = GetTick();
    iCnt = 0;
    do 
    {
        Sleep(100);
        iCnt += esam_write(bBuf+iCnt, 145-iCnt);
        if(iCnt == 145) 
            break;
    }while (GetTick()-dwTick < 5*1000);
	TraceBuf(DB_FAPROTO, "RegisterNonSymKey write",bBuf,iCnt);

	dwTick = GetTick();
	do 
	{
		Sleep(100);
		iCnt = esam_read(bBuf, sizeof(bBuf));
		if ((iSW=CheckSW2(iCnt,bBuf,0x61,0xfa)) >= 0) 
		{
			DTRACE(DB_FAPROTO,("RegisterNonSymKey ok\n"));
			break;
		}
	}while (GetTick()-dwTick < dwTimeout+5*1000);
	TraceBuf(DB_FAPROTO, "UpdateMKeyLocal2 read",bBuf,iCnt);

	if(iSW == NOSW)//没收到61fa
	{
		ResetESAM();
	}

	if((iSW = GetResponse(0xfa, bBuf, 2*1000)) != 0xfa)
	{
		DTRACE(DB_FAPROTO,("RegisterNonSymKey GetResponse1 failed~~~\n"));
		SignalSemaphore(g_ESAM.semEsam);
		return iSW;
	}
	if (bBuf[0] == 0xc0) 
	{
		memcpy(bBuf2,bBuf+1,250);
	}
	else
	{//应该不会进到这来
		memcpy(bBuf2,bBuf,250);
	}
	
	if(iSW=(GetResponse(0x06, bBuf, 2*1000)) != 0x06)
	{
		DTRACE(DB_FAPROTO,("RegisterNonSymKey GetResponse2 failed~~~\n"));
		SignalSemaphore(g_ESAM.semEsam);
		return iSW;
	}
	if (bBuf[0] == 0xc0) 
	{
		memcpy(bBuf2+250,bBuf+1,6);
	}
	else
	{
		memcpy(bBuf2+250,bBuf,6);
	}

	for (int k=0;k<256;k++) 
	{
		pbBuf[k] = bBuf2[255-k];
	}

	TraceBuf(DB_FAPROTO, "RegisterNonSymKey Get KEY",pbBuf,256);
	SignalSemaphore(g_ESAM.semEsam);
	return 256;
}

int UpdateNonSymKey(BYTE bP1,BYTE *pbSessionKey,BYTE *pbMastRandom,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout,BYTE *pbBuf)
{
	BYTE bBuf[300];
	BYTE bBuf2[300];
	int iCnt = 0;
	int iSW = 0;
	DWORD dwTick;

	bBuf[0] = 0x90;
	bBuf[1] = 0x38;
	bBuf[2] = bP1;
	bBuf[3] = 0x00;
	bBuf[4] = 0x8c;
	bBuf[5] = 0x63;
	bBuf[6] = 0x80;
	memrcpy(&bBuf[7],pbSessionKey,128);//长度固定
	bBuf[135] = 0x81;
	bBuf[136] = 0x08;
	memrcpy(&bBuf[137],pbMastRandom,8);//长度固定
	WaitSemaphore(g_ESAM.semEsam, SYS_TO_INFINITE);
    dwTick = GetTick();
    iCnt = 0;
    do 
    {
        Sleep(100);
        iCnt += esam_write(bBuf+iCnt, 145-iCnt);
        if(iCnt == 145) 
            break;
    }while (GetTick()-dwTick < 5*1000);
	TraceBuf(DB_FAPROTO, "UpdateNonSymKey write1",bBuf,iCnt);

	dwTick = GetTick();
	do 
	{
		iCnt = esam_read(bBuf, sizeof(bBuf));
		if ((iSW=CheckSW2(iCnt,bBuf,0x90,0x00)) >= 0) 
		{
			DTRACE(DB_FAPROTO,("UpdateNonSymKey ok\n"));
			break;
		}
	}while (GetTick()-dwTick < dwTimeout+3*1000);

	TraceBuf(DB_FAPROTO, "UpdateNonSymKey read1",bBuf,iCnt);
	bBuf[0] = 0x80;
	bBuf[1] = 0x38;
	bBuf[2] = bP1;
	bBuf[3] = 0x00;
	bBuf[4] = 0x82;
	bBuf[5] = 0x60;
	bBuf[6] = 0x80;
	memrcpy(&bBuf[7],pbSign,128);//长度固定
    dwTick = GetTick();
    iCnt = 0;
    do 
    {
        Sleep(100);
        iCnt += esam_write(bBuf+iCnt, 135-iCnt);
        if(iCnt == 135) 
            break;
    }while (GetTick()-dwTick < 5*1000);
	TraceBuf(DB_FAPROTO, "UpdateNonSymKey write",bBuf,iCnt);

	dwTick = GetTick();
	do 
	{
		iCnt = esam_read(bBuf, sizeof(bBuf));
		if ((iSW=CheckSW2(iCnt,bBuf,0x61,0xfa)) >= 0) 
		{
			DTRACE(DB_FAPROTO,("UpdateNonSymKey ok \n"));
			break;
		}
	}while (GetTick()-dwTick < dwTimeout+4*1000);

//  if(iSW == NOSW)
//  	Reset();
	TraceBuf(DB_FAPROTO, "UpdateNonSymKey read",bBuf,iCnt);

	if((iSW=GetResponse(0xfa, bBuf, 2*1000)) != 0xfa)
	{
		DTRACE(DB_FAPROTO,("UpdateNonSymKey GetResponse1 failed~~~\n"));
		SignalSemaphore(g_ESAM.semEsam);
		return iSW;
	}
	if (bBuf[0] == 0xc0) 
	{
		memcpy(bBuf2,bBuf+1,250);
	}
	else
	{
		memcpy(bBuf2,bBuf,250);
	}
	TraceBuf(DB_FAPROTO, "UpdateNonSymKey readpart1",bBuf2,250);

	if((iSW=GetResponse(0x1a, bBuf, 2*1000)) != 0x1a)
	{
		DTRACE(DB_FAPROTO,("UpdateNonSymKey GetResponse2 failed~~~\n"));
		SignalSemaphore(g_ESAM.semEsam);
		return iSW;
	}
	if (bBuf[0] == 0xc0) 
	{
		memcpy(bBuf2+250,bBuf+1,0x1a);
	}
	else
	{
		memcpy(bBuf2+250,bBuf,0x1a);
	}

	//数据A＋数据B = ”6190”+144字节终端公钥＋”6080”＋128字节数据签名组成的数据
	memcpy(pbBuf, pbMastRandom, 8);
	memrcpy(pbBuf+8, bBuf2, 146);//
	memrcpy(&pbBuf[144+8], &bBuf2[148], 128);	
	
	TraceBuf(DB_FAPROTO, "UpdateNonSymKey got", pbBuf, 280);
		
	SignalSemaphore(g_ESAM.semEsam);
	return NOERROR;
}
