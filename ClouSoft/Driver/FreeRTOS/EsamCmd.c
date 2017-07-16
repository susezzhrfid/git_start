//#include "stdafx.h"
#include "sysapi.h"
#include "EsamCmd.h"
#include "sysarch.h"
#include "Trace.h"
#include "sysdebug.h"
#include "ComAPI.h"
#include "TypeDef.h"
//#include "sysfs.h"
#include "FaCfg.h"
#ifndef SYS_WIN
#include "Esam.h"
#endif

// IEC7816操作命令
#define WARMACTIVATE            0

#define CASE1  1
#define CASE2  2
#define CASE3  3
#define CASE4  4

#define P1     1
#define P2     2

#define CLA_OFF 1

//CESAM g_ESAM;
bool g_fEsamOpen = false;
static BYTE g_bCertState = 0;	//证书状态：00为测试证书；01为正式证书
TSem g_semEsam;   //保护g_bEsamTxRxBuf，还有每一个加密流程
BYTE g_bEsamTxRxBuf[1800];		//发送接收共用
BYTE g_bEsamCmpDataBuf[20];
WORD g_wEsamCmpDataLen = 0;
WORD g_wEsamLastRxDataLen = 0;

static const TSWTab g_tSwTable[]=
{
	{0x90,0x00,"OK"},//指令执行成功
	{0x63,0xcf,"Cert Or Swt failed"},//认证失败
	{0x64,0x00,"InExcu wrong"},//内部执行出错
	{0x65,0x81,"Card locked"},//卡被锁死
	{0x67,0x00,"Len wrong"},//Lc或Le长度错
	{0x69,0x01,"Cntzero Or Cmd wrong"},//离线计数器为0或命令不接受
	{0x69,0x82,"No Safe sta"},//不满足安全状态
	{0x69,0x83,"Kut zero"},//Kut使用次数为0
	{0x69,0x84,"Use Data invalid"},//引用数据无效
	{0x69,0x85,"Condition Incomplete"},//使用条件不满足
	{0x69,0x86,"Online Cnt zero"},//在线计数器为0
	{0x69,0x88,"MAC wrong"},//MAC错误
	{0x6a,0x80,"Para wrong"},//参数错误
	{0x6a,0x86,"P1P2 wrong"},//参数错误
	{0x6a,0x88,"No Find data"},//未找到引用数据
	{0x6d,0x00,"Cmd No exist"},//命令不存在
	{0x6e,0x00,"Cmd Or CLA wrong "},//命令或CLA错
	{0x6f,0x00,"Data invalid"},//数据无效
	{0x90,0x86,"CheckSign wrong"},//验签错误
	{0x9e,0x2f,"File wrong"},//文件错误
	{0x9e,0x3f,"Calc wrong"},//算法计算错误
	{0x9e,0x57,"Cert wrong"},//认证错误
	{0x9e,0x60,"Session wrong"},//建立会话错误
	{0x9e,0x5e,"CA wrong"},//CA证书错误
};

BYTE dayofweek(const TTime* time)
{
	BYTE bWeek = DayOfWeek(time); //DayOfWeek()的返回 1 = Sunday, 2 = Monday, ..., 7 = Saturday
	if (bWeek != 0)
		bWeek--;

	return bWeek;
}

void EsamClose()
{
    if (g_fEsamOpen)
    {
		g_fEsamOpen = false;
    }
}

bool EsamInit()
{
    int iDataLen;
	BYTE bcont = 0;
	const BYTE bCmdGetEsamStatus[] = {0x80, 0x0E, 0x00, 0x05, 0x00, 0x00};  
	
	g_fEsamOpen = true;
	g_semEsam = NewSemaphore(1,1);
/*	g_ifd = open("/dev/esam", O_RDWR|O_NONBLOCK|O_NOCTTY|O_NDELAY, 0);    
	if (g_ifd < 0)
	{
		DTRACE(DB_CRITICAL,("Open esam dev failed\r\n"));
		return false;
	}
*/    
	//证书状态：00为测试证书；01为正式证书
	//获取芯片状态信息1字节
	//发送：800E00050000
	//返回：9000+LEN+芯片状态信息
	do
	{
		bcont++;
		iDataLen = EsamTxRxCmd(bCmdGetEsamStatus, &g_bCertState, 7);
	} while(iDataLen==-2 && bcont<2);
	if (iDataLen <= 0)
	{
		DTRACE(DB_CRITICAL,("Get esam certsta failed\n"));
		return false;
	}

	DTRACE(DB_CRITICAL, ("Esam driver init OK，g_bCertState=%d\n", g_bCertState));
	return true; 
}

bool EsamReset()
{
	esam_warm_reset();
	return true;
}

//参数：@pbBuf 接收缓冲区
//		@wExpLen 期待的接收长度
int EsamRead(BYTE* pbBuf, WORD wExpLen)
{
	int iLen = 0; 
	if (wExpLen > 0) 
	{
        iLen = esam_read(pbBuf, wExpLen);
	}

	return iLen;
}

int EsamRead2(BYTE* pbBuf, WORD wExpLen)
{
	return esam_read2(pbBuf, wExpLen);
}

int EsamWrite(BYTE* pbBuf, WORD wExpLen)
{
  return esam_write(pbBuf, wExpLen);
}

int EsamCheckRxFrm(BYTE* pbFrm, WORD wLen, BYTE bSW1, BYTE bSW2, int* piStart)
{
	//接收数据的结构为：SW1 SW2 Len1 Len2 DATA LRC2
	//LRC2的计算方法：对Len1 Len2 DATA数据，每个字节的异或值，再取反。
	//Len1代表长度的高字节，Len2代表长度的低字节
	//Len1 Len2代表DATA域的长度，不包括LRC1或LRC2
	//if (pbFrm[0]!=bSW1 || pbFrm[1]!=bSW2)
	//	return -1;

	int iStart = -1;
	int iCmdHead = -1;
	WORD i, wDataLen;
	BYTE bLRC2;

	*piStart = -1;
	for (i=0; i<wLen-1; i++)
	{
		if (pbFrm[i]==0x55 && iCmdHead==-1)
			iCmdHead = i;
		if (pbFrm[i]==bSW1 && pbFrm[i+1]==bSW2)
		{
			iStart = i;
			break;
		}
	}

	if (iStart == -1)
	{
		if (iCmdHead >= 0)
		{
			for (i=0; i<sizeof(g_tSwTable); i++) 
			{
				if ((pbFrm[iCmdHead+1]==g_tSwTable[i].bSW1)&&(pbFrm[iCmdHead+2]==g_tSwTable[i].bSW2))
				{
					DTRACE(DB_FAPROTO,("SW: %s\r\n", g_tSwTable[i].str));
					break;
				}
			}
		}
		else
			DTRACE(DB_FAPROTO,("SW: failed\r\n"));
		return -1;
	}

	wDataLen = ((WORD )pbFrm[iStart+2]<<8) + pbFrm[iStart+3];
	if ((int )wLen-iStart < wDataLen+5)
		return -1;

	bLRC2 = pbFrm[iStart];
	for (i=0; i<wDataLen+3; i++)
	{
		bLRC2 ^= pbFrm[iStart+1+i];
	}

	bLRC2 = ~bLRC2;

	if (pbFrm[iStart+4+wDataLen] != bLRC2)
	{
		DTRACE(DB_FAPROTO,("EsamCheckRxFrm: CheckRxFrm fail \r\n"));
		return -1;
	}

	*piStart = iStart;
	return wDataLen;
}

//备注：CLA INS P1 P2 Len1 Len2 DATA都已经放好，补充剩余的部分
WORD EsamMakeTxFrm(BYTE* pbFrm, WORD wDataLen)
{
	//发送数据的结构为：55 CLA INS P1 P2 Len1 Len2 DATA LRC1
	//Len1 Len2是后续DATA的长度，不包含LRC1，由两字节表示
	//LRC1的计算方法：对CLA INS P1 P2 Len1 Len2 DATA数据，每个字节的异或值，再取反。
	WORD i;
	BYTE bLRC1;
	pbFrm[0] = 0x55;
	bLRC1 = pbFrm[1];
	for (i=0; i<wDataLen+5; i++)
	{
		bLRC1 ^= pbFrm[2+i];
	}
	
	bLRC1 = ~bLRC1;

	pbFrm[7+wDataLen] = bLRC1;

	return wDataLen+8;
}


//描述：发送接收一个帧，直接使用pbTx的缓冲，不另外定义缓冲区
//参数	@wExpLen 期待的接收长度
int EsamTxRxFrm(BYTE* pbTx, WORD wDataLen, BYTE* pbRx, WORD wExpLen)
{
	int iLen, iDataLen, iStart = -1;
	WORD wTxLen;
//	BYTE  bReadBuf[60];//写之前读，不能用 g_bEsamTxRxBuf,因为和指针pbTx重复了  xzz   
	if (!g_fEsamOpen)
	{
		DTRACE(DB_FAPROTO,("EsamTxRxFrm: failed due to device not exit\r\n"));
		return -1;
	}

	wTxLen = EsamMakeTxFrm(pbTx, wDataLen);

	/************/ //写之前不读的话收不到数据，有点奇怪，留个疑问后面待查
	/*iLen = EsamRead2(bReadBuf, 60);
	if(iLen>0)
		TraceBuf(DB_FAFRM, "EsamTxRxFrm: EsamRead2", bReadBuf, iLen);
		
	Sleep(100);*/
	//memset(bBuf, 0x00, sizeof(bBuf)); 
	/***********/

    /*if (bFirst >= 2)
    {
        for (iLen=wTxLen-1; iLen>=0; iLen--)
        {
            pbTx[iLen+1] = pbTx[iLen];
        }
        pbTx[0] = 0x00;
        //pbTx[1] = 0xff;
        wTxLen += 1;    //长度加一个
    }*/
        
	if (EsamWrite(pbTx, wTxLen) != wTxLen)
	{
		DTRACE(DB_FAPROTO,("EsamTxRxFrm: write failed %d\r\n", wTxLen));
		return -1;
	}
    
//#ifdef SYS_WIN
	TraceBuf(DB_FAFRM, "EsamTxRxFrm: write", pbTx, wTxLen);
//#endif

    /*if (wTxLen == 1764)//会话初始化  1764
        Sleep(900);
    else if (wTxLen == 1692)//证书更新1692  
        Sleep(1300);
    else if (wTxLen == 286)//会话协商 286 
        Sleep(600);
	else if (wTxLen > 1000)     
		Sleep(800);	
    else if (wTxLen > 200)
        Sleep(200);
    */
	//Sleep(1300);
	wExpLen += 32;		//返回有多余数据，多接收一些字节
	if (wExpLen >= sizeof(g_bEsamTxRxBuf))
		wExpLen = sizeof(g_bEsamTxRxBuf);

	iLen = EsamRead(g_bEsamTxRxBuf, wExpLen);
    //iLen = EsamRead2(g_bEsamTxRxBuf, 100);
	if (iLen <= 0)
	{
		DTRACE(DB_FAPROTO,("EsamTxRxFrm: read failed\r\n"));
		EsamReset();
		return -1;
	}
//#ifdef SYS_WIN
	TraceBuf(DB_FAFRM, "EsamTxRxFrm: read", g_bEsamTxRxBuf, iLen);
//#endif
    
    /*Sleep(1300);
    iLen = EsamRead2(bReadBuf, 60);
	if(iLen>0)
		TraceBuf(DB_FAFRM, "EsamTxRxFrm: EsamRead2", bReadBuf, iLen);*/
    
	if ((iDataLen=EsamCheckRxFrm(g_bEsamTxRxBuf, iLen, 0x90, 0x00, &iStart)) < 0)
	{
        g_wEsamCmpDataLen = g_wEsamLastRxDataLen = 0;
		return -1;
	}
	///////////////////////////////////////////
	//wExpLen -= 38; //32+6
	//if(iDataLen!=wExpLen)
	//{
	//  	return -2;////返回该值说明有问题，需要重新来过
	//}
	/////////////////////////////////////////////
	if (pbRx != NULL)
		memcpy(pbRx, &g_bEsamTxRxBuf[iStart+4], iDataLen);
    
	if (g_wEsamLastRxDataLen!=0 && iDataLen==g_wEsamLastRxDataLen)
	{
		if (memcmp(g_bEsamCmpDataBuf, &g_bEsamTxRxBuf[iStart+4], g_wEsamCmpDataLen) == 0)
        {    
            g_wEsamCmpDataLen = g_wEsamLastRxDataLen = 0;   //只捕获一次错误，重发一次，再发生错误就不管了
            DTRACE(DB_FAPROTO, ("EsamTxRxFrm:Esam retry!\r\n"));
            return -2;
        }
	}
	g_wEsamLastRxDataLen = iDataLen;
	if (iDataLen < 20)
		g_wEsamCmpDataLen = iDataLen;
	else
		g_wEsamCmpDataLen = 20;
	memcpy(g_bEsamCmpDataBuf, &g_bEsamTxRxBuf[iStart+4], g_wEsamCmpDataLen);	

	return iDataLen;
}

//描述：发送接收一个命令，这种命令的数据长度为0
int EsamTxRxCmd(const BYTE* pbTx, BYTE* pbRx, WORD wExpLen)
{
	BYTE bBuf[16];
	memcpy(&bBuf[CLA_OFF], pbTx, 6);
	return EsamTxRxFrm(bBuf, 0, pbRx, wExpLen);
}

//新加程序
//描述：F11	获取终端信息
//返回：如果正确返回数据长度，错误返回负数
int EsamGetTermInfo(BYTE* pbRx)
{
    int iDataLen;
	BYTE bcont = 0;
	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);

	//BYTE bBuf[16];
	const BYTE bCmdGetOfflineCnt[] = {0x80, 0x0E, 0x00, 0x03, 0x00, 0x00};
	const BYTE bCmdGetEsamStatus[] = {0x80, 0x0E, 0x00, 0x05, 0x00, 0x00};
	const BYTE bCmdGetKeyVer[] = {0x80, 0x0E, 0x00, 0x06, 0x00, 0x00};
	BYTE bCmdGetCertSN[] = {0x80, 0x32, 0x00, 0x02, 0x00, 0x00};
	//获取芯片序列号8字节
	//发送：800E00020000
	//返回：9000+LEN+ESAM序列号
	const BYTE bCmdGetSN[] = {0x80, 0x0E, 0x00, 0x02, 0x00, 0x00};
	do
	{
		bcont++;
		iDataLen = EsamTxRxCmd(bCmdGetSN, pbRx, 14);
	}while(iDataLen==-2 && bcont<2);
	if (iDataLen <= 0)	//
	{
		DTRACE(DB_FAPROTO,("GetTermInfo: fail to GetSN \r\n"));
		goto GetTermInfo_fail_ret;
	}
    
	Swap(pbRx, 8);	
	
	//获取离线计数器4字节
	//发送：800E00030000
	//返回：9000+LEN+离线计数器信息
	//const BYTE bCmdGetOfflineCnt[] = {0x80, 0x0E, 0x00, 0x03, 0x00, 0x00};
	bcont = 0;
	do
	{
		bcont++;
		iDataLen = EsamTxRxCmd(bCmdGetOfflineCnt, pbRx+24, 10);
	}while(iDataLen==-2 && bcont<2);
	if (iDataLen <= 0)		//
	{
		DTRACE(DB_FAPROTO,("GetTermInfo: fail to GetOfflineCnt \r\n"));
		goto GetTermInfo_fail_ret;
	}

	Swap(pbRx+24, 4);

	//获取芯片状态信息1字节
	//发送：800E00050000
	//返回：9000+LEN+芯片状态信息
	//const BYTE bCmdGetEsamStatus[] = {0x80, 0x0E, 0x00, 0x05, 0x00, 0x00};
	bcont = 0;
	do
	{
		bcont++;
		iDataLen = EsamTxRxCmd(bCmdGetEsamStatus, pbRx+28, 7);
	}while(iDataLen==-2 && bcont<2);
	if (iDataLen <= 0)  //
	{
		DTRACE(DB_FAPROTO,("GetTermInfo: fail to GetEsamStatus \r\n"));
		goto GetTermInfo_fail_ret;
	}

	//获取密钥版本：8字节
	//发送：800E00060000
	//返回：9000+LEN+密钥版本信息
	//const BYTE bCmdGetKeyVer[] = {0x80, 0x0E, 0x00, 0x06, 0x00, 0x00};
	bcont = 0;
	do
	{
		bcont++;
		iDataLen = EsamTxRxCmd(bCmdGetKeyVer, pbRx+29, 14);
	}while(iDataLen==-2 && bcont<2);
	if (iDataLen <= 0)  //
	{
		DTRACE(DB_FAPROTO,("GetTermInfo: fail to GetKeyVer \r\n"));
		goto GetTermInfo_fail_ret;
	}
	
	Swap(pbRx+29, 8);
	//证书序列号：16字节
	//发送：8032 + P1 + 02 +0000
	//返回：9000+LEN+证书序列号
	//BYTE bCmdGetCertSN[] = {0x80, 0x32, 0x00, 0x02, 0x00, 0x00};
	bcont = 0;
	do
	{
		bCmdGetCertSN[2] = g_bCertState;
		iDataLen = EsamTxRxCmd(bCmdGetCertSN, pbRx+8, 22);
	}while(iDataLen==-2 && bcont<2);
	if (iDataLen <= 0)  //
	{
		DTRACE(DB_FAPROTO,("GetTermInfo: fail to GetCertSN \r\n"));
		goto GetTermInfo_fail_ret;
	}

	Swap(pbRx+8, 16);
	SignalSemaphore(g_semEsam);
	return 37;

GetTermInfo_fail_ret:
	SignalSemaphore(g_semEsam);
	return -1;
}


//描述：F12	会话初始化/恢复
//备注：会话初始化和会话恢复主站下发的命令和ESAM的命令格式完全一样
int EsamInitSession(BYTE* pbTx, WORD wTxLen, BYTE* pbRx)
{
	//发送：84100000+LC+报文1+Time
	//返回：9000+LEN+报文2
	int iDataLen;
	BYTE bcont = 0;
	WORD wExpLen = 0;
	TTime now;
	//BYTE bBuf[1800];
    WaitSemaphore(g_semEsam,SYS_TO_INFINITE);
	Swap(pbTx+2, wTxLen-0x56);//主站证书
	Swap(pbTx+(wTxLen-0x54), 16);//Eks1（R1）
	Swap(pbTx+(wTxLen-0x44),4);//MAC1
	Swap(pbTx+(wTxLen-0x40),64);//签名数据S1
	BYTE* p;
	do
	{
	    bcont++;
		memset(g_bEsamTxRxBuf, 0x00, sizeof(g_bEsamTxRxBuf));
		p = &g_bEsamTxRxBuf[CLA_OFF];
		
		*p++ = 0x84;
		*p++ = 0x10;
		*p++ = 0x00;
		*p++ = 0x00;
		
		*p++ = (wTxLen+7)>>8;	//Len1代表长度的高字节，
		*p++ = wTxLen+7;		//Len2代表长度的低字节
		memcpy(p, pbTx, wTxLen);
		if (*(p+1) == 1)//会话恢复
			wExpLen = 44;
		else
			wExpLen = 1778;
		p += wTxLen;
		
		GetCurTime(&now);
		*p++ = ByteToBcd(now.nYear/100);
		*p++ = ByteToBcd(now.nYear%100);
		*p++ = ByteToBcd(now.nMonth);
		*p++ = ByteToBcd(now.nDay);
		*p++ = ByteToBcd(now.nHour);
		*p++ = ByteToBcd(now.nMinute);
		*p++ = ByteToBcd(now.nSecond);
		 iDataLen=EsamTxRxFrm(g_bEsamTxRxBuf, wTxLen+7, pbRx, wExpLen);//1782]
	}while(iDataLen==-2 && bcont<2);
	
	//if ((iDataLen=EsamTxRxFrm(g_bEsamTxRxBuf, wTxLen+7, pbRx, 1782)) <= 0)	//114+128+5 下发指令1756+7，接收指令数据1777+5
	if(iDataLen <= 0)
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("InitSession: fail\r\n"));		
		return -1;
	}
	
	
	if(*(pbRx+1) == 0x00)//会话ID为0
	{
	  Swap(pbRx+2,16);
	  Swap(pbRx+18,16);
	  Swap(pbRx+34,(iDataLen-0x72));
	  Swap(pbRx+(iDataLen-80), 16);
	  Swap(pbRx+(iDataLen-64), 64);
	}
	else
	{
	  Swap(pbRx+2,16);
	  Swap(pbRx+18,16);
	  Swap(pbRx+34,4);
	}

	SignalSemaphore(g_semEsam);
	return iDataLen;
}

//描述：F13	会话协商
//备注：跟 5.3 会话密钥协商 有点区别
int EsamNegotiateKey(BYTE* pbTx, WORD wTxLen, BYTE* pbRx)
{
	//发送：84120000+LC+报文3
	//返回：9000+LEN+报文4
	BYTE* p;
	int iDataLen;
	BYTE bcont = 0;
	//BYTE bBuf[512];
	
	
	Swap(pbTx, 113);//会话密钥密文
	Swap(pbTx+113, wTxLen-181);//主站证书验证码
	Swap(pbTx+(wTxLen-68), 4);//MAC2
	Swap(pbTx+(wTxLen-64), 64);//签名数据S3


    WaitSemaphore(g_semEsam,SYS_TO_INFINITE);
	do
	{
	  	bcont++;
		memset(g_bEsamTxRxBuf, 0x00, sizeof(g_bEsamTxRxBuf));
		p = &g_bEsamTxRxBuf[CLA_OFF];
	
		*p++ = 0x84;
		*p++ = 0x12;
		*p++ = 0x00;
		*p++ = 0x00;
	
		*p++ = wTxLen>>8;	//Len1代表长度的高字节，
		*p++ = wTxLen;		//Len2代表长度的低字节
		memcpy(p, pbTx, wTxLen);
		iDataLen=EsamTxRxFrm(g_bEsamTxRxBuf, wTxLen, pbRx, 26);
	}while(iDataLen==-2 && bcont<2);
	
	//if ((iDataLen=EsamTxRxFrm(g_bEsamTxRxBuf, wTxLen, pbRx, 25)) <= 0) //下发指令278+7，接收指令数据20+5
	if(iDataLen <= 0) 
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("NegotiateKey: fail\r\n"));		
		return -1;
	}

	Swap(pbRx, 16);//R3
	Swap(pbRx+16, 4);//MAC3

	SignalSemaphore(g_semEsam);
	return iDataLen;
}

//描述：F14	对称密钥更新
//返回：如果正确返回true，错误返回false
bool EsamUpdSymKey(BYTE bKeyNum, BYTE* pbKey)
{
	//发送：84240100+LC+密钥密文数据
	//返回：9000+0000
	BYTE* p;
	BYTE bcont = 0;
	BYTE i;
	int iDataLen;
	//BYTE bBuf[1500]; //46条全部密钥更新下发指令1473+7
	if (bKeyNum > 46)
		bKeyNum = 46;

	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);
	do
	{
		bcont++;
		memset(g_bEsamTxRxBuf, 0x00, sizeof(g_bEsamTxRxBuf));
		//for(i=0; i<bKeyNum; i++)
		p = &g_bEsamTxRxBuf[CLA_OFF];

		*p++ = 0x84;
		*p++ = 0x24;
		*p++ = 0x01;
		*p++ = 0x00;

		*p++ = ((WORD)(bKeyNum*32+1)) >> 8;	//Len1代表长度的高字节，
		*p++ = (WORD)(bKeyNum*32+1);	//Len2代表长度的低字节
		*p++ = bKeyNum;
		//memcpy(p, pbKey, bKeyNum*32);
		for(i=0; i<bKeyNum; i++)
		{
			memrcpy(p+i*32, pbKey+i*32, 32);//密钥密文
			//memcpy(p, pbKey, bKeyNum*32);
		}
		

		pbKey += bKeyNum*32;
		iDataLen = EsamTxRxFrm(g_bEsamTxRxBuf, bKeyNum*32+1, NULL, 6);
	}while(iDataLen==-2 && bcont<2);

	if (iDataLen < 0)  //
	{
		SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("UpdSymKey: fail at %d\r\n", i));			
		return false;
	}
		
		Sleep(100);

	SignalSemaphore(g_semEsam);
	return true;
}

//描述：F15	终端证书更新
//返回：如果正确返回数据长度，错误返回负数
//备注：对应5.7 终端证书更新请求
int EsamUpdCert(BYTE* pbRx)
{
    int iDataLen;
	BYTE bcont = 0;
	const BYTE bCmdGetPublic[] = {0x80, 0x2c, 0x00, 0x01, 0x00, 0x00};
	BYTE bCmdGetCertSN[] = {0x80, 0x32, 0x00, 0x02, 0x00, 0x00};

	//证书序列号：16字节
	//发送：8032+P1+020000
	//返回：9000+LEN+终端证书序列号
	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);

	do
	{
		bcont++;
		bCmdGetCertSN[2] = g_bCertState;
		iDataLen = EsamTxRxCmd(bCmdGetCertSN, pbRx, 22);
		
	}while(iDataLen==-2 && bcont<2);

	if ( iDataLen <= 0)  //
	{
		DTRACE(DB_FAPROTO,("UpdCert: fail to GetCertSN \r\n"));
		goto UpdCert_fail_ret;
	}

	//发送：802C00010000
	//返回：9000+LEN+终端公钥
	//const BYTE bCmdGetPublic[] = {0x80, 0x2c, 0x00, 0x01, 0x00, 0x00};
	bcont = 0;
	do
	{
		bcont++;
		iDataLen = EsamTxRxCmd(bCmdGetPublic, pbRx+16, 70);
	}while(iDataLen==-2 && bcont<2);
	if (iDataLen <= 0)  //
	{
		DTRACE(DB_FAPROTO,("UpdCert: fail to GetPublic \r\n"));
		goto UpdCert_fail_ret;
	}

	Swap(pbRx, 16);
	Swap(pbRx+16, 64);


	SignalSemaphore(g_semEsam);
	return 80;

UpdCert_fail_ret:
	SignalSemaphore(g_semEsam);
	return -1;
}

//描述：F16	CA证书更新
//返回：如果正确返回ture，错误返回false
//备注：对应5.8 证书更新
bool EsamUpdCA(BYTE* pbTx, WORD wTxLen)
{
	//发送：8430+证书更新密文
	//返回：9000+0000
	BYTE* p;
	BYTE bcont = 0;
	int iDataLen;
	//BYTE bBuf[1800];
	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);
	do
	{
		bcont++;
		memset(g_bEsamTxRxBuf, 0x00, sizeof(g_bEsamTxRxBuf));
		p = &g_bEsamTxRxBuf[CLA_OFF];
		
		*p++ = 0x84;
		*p++ = 0x30;
		memrcpy(p, pbTx, wTxLen);
		iDataLen = EsamTxRxFrm(g_bEsamTxRxBuf, wTxLen-4, NULL, 6);
	}while(iDataLen==-2 && bcont<2);

    if (iDataLen < 0) //下发指令1684+7，接收指令数据5
	{
		DTRACE(DB_FAPROTO,("UpdCA: fail\r\n"));
		SignalSemaphore(g_semEsam);
		return false;
	}

	SignalSemaphore(g_semEsam);
	return true;
}

//描述：F17	内部认证
//返回：如果正确返回数据长度，错误返回负数
//备注：5.9 内、外部认证
int EsamIntCert(BYTE* pbTx, WORD wTxLen, BYTE* pbRx)
{
	BYTE* p;
	int iDataLen;
	BYTE bcont = 0;

	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);

	//发送：842600000010+R4
	//返回：9000+LEN+ER4
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
	
		*p++ = 0x84;
		*p++ = 0x26;
		*p++ = 0x00;
		*p++ = 0x00;
	
		*p++ = 0;		//Len1代表长度的高字节，
		*p++ = 0x10;	//Len2代表长度的低字节
		memrcpy(p, pbTx, 16);
		iDataLen = EsamTxRxFrm(g_bEsamTxRxBuf, 16, pbRx, 22);
	}while(iDataLen==-2 && bcont<2);

	if (iDataLen <= 0) //
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("IntCert: fail\r\n"));		
		return -1;
	}

	//发送：801A10000000
	//返回：9000+LEN+R5
	bcont = 0;
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
		
		*p++ = 0x80;
		*p++ = 0x1a;
		*p++ = 0x10;
		*p++ = 0x00;
		
		*p++ = 0;	//Len1代表长度的高字节，
		*p++ = 0;	//Len2代表长度的低字节
		iDataLen = EsamTxRxFrm(g_bEsamTxRxBuf, 0, pbRx+16, 22);
	}while(iDataLen==-2 && bcont<2);

	if (iDataLen <= 0) //
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("IntCert: fail\r\n"));		
		return -1;
	}
	
	Swap(pbRx, 16);//R4密文
	Swap(pbRx+16, 16);//R5

	SignalSemaphore(g_semEsam);
	return 32;
}

//描述：F18	外部认证
//返回：如果正确返回数据长度，错误返回负数
//备注：5.9 内、外部认证
int EsamExtCert(BYTE* pbTx, WORD wTxLen, BYTE* pbRx)
{
	BYTE* p;	
    int iDataLen;
	BYTE bcont = 0;

	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);

	//发送：842800000010+ER5
	//返回：9000+0000	
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
		
		*p++ = 0x84;
		*p++ = 0x28;
		*p++ = 0x00;
		*p++ = 0x00;
		
		*p++ = 0;		//Len1代表长度的高字节，
		*p++ = 0x10;	//Len2代表长度的低字节
		memrcpy(p, pbTx, 16);
		iDataLen=EsamTxRxFrm(g_bEsamTxRxBuf, 16, NULL, 6);
	}while(iDataLen==-2 && bcont<2);

	if (iDataLen < 0) //
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("ExtCert: fail\r\n"));		
		return -1;
	}

	//发送：801A10000000
	//返回：9000+LEN+R6
	bcont = 0;
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
	
		*p++ = 0x80;
		*p++ = 0x1a;
		*p++ = 0x10;
		*p++ = 0x00;
	
		*p++ = 0;		//Len1代表长度的高字节，
		*p++ = 0;		//Len2代表长度的低字节
		iDataLen = EsamTxRxFrm(g_bEsamTxRxBuf, 0, pbRx, 22);
	}while(iDataLen==-2 && bcont<2);

	if (iDataLen <= 0) //
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("ExtCert: fail\r\n"));		
		return -1;
	}

	Swap(pbRx, 0x10);//R6 倒位

	SignalSemaphore(g_semEsam);
	return iDataLen;
}

//描述：F19	状态切换
//返回：如果正确返回ture，错误返回false
//备注：5.10 证书状态切换
bool EsamSwitchState(BYTE bP1, BYTE* pbTx, WORD wTxLen)
{
	BYTE* p;
	int iDataLen;
	BYTE bcont = 0;
    
	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);

	//发送：842A+P1+000014+ER6+MAC6
	//返回：9000+0000	
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
	
		*p++ = 0x84;
		*p++ = 0x2a;
		*p++ = bP1;
		*p++ = 0x00;
	
		*p++ = 0;		//Len1代表长度的高字节，
		*p++ = 0x14;	//Len2代表长度的低字节
		//memcpy(p, pbTx, 0x14);
		
		memrcpy(p, pbTx, 0x10);//R6
		memrcpy(p+0x10, pbTx+0x10, 4);//MAC6
		iDataLen = EsamTxRxFrm(g_bEsamTxRxBuf, 0x14, NULL, 6);
	}while(iDataLen==-2 && bcont<2);


	if (iDataLen < 0)  //
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("SwitchState: fail\r\n"));		
		return false;
	}
	
	
	g_bCertState = bP1==0 ? 1 : 0;
	//把证书状态保存的系统库
	SignalSemaphore(g_semEsam);
	return true;
}

//描述：F20	置离线计数器
//返回：如果正确返回ture，错误返回false
//备注：5.11 置离线计数器
bool EsamSetOfflineCnt(BYTE* pbTx, WORD wTxLen)
{
	BYTE* p;	
	int iDataLen;
	BYTE bcont = 0;

	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);

	//发送：842000000014+离线计数器密文+MAC
	//返回：9000+0000
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
	
		*p++ = 0x84;
		*p++ = 0x20;
		*p++ = 0x00;
		*p++ = 0x00;
	
		*p++ = 0;		//Len1代表长度的高字节，
		*p++ = 0x14;	//Len2代表长度的低字节
		memrcpy(p, pbTx, 0x14);
  		iDataLen = EsamTxRxFrm(g_bEsamTxRxBuf, 0x14, NULL, 6);
	}while(iDataLen==-2 && bcont<2);

	if (iDataLen < 0) //
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("SetOfflineCnt: fail\r\n"));		
		return false;
	}

	SignalSemaphore(g_semEsam);
	return true;
}

//描述：F21	转加密授权
//返回：如果正确返回ture，错误返回false
//备注：5.12 转加密授权
bool EsamTransEncrAuth(BYTE* pbTx, WORD wTxLen)
{
	BYTE* p;
	int iDataLen;
	BYTE bcont;
	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);

	//发送：801600000020+转加密数据
	//返回：9000+0000
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
	
		*p++ = 0x80;
		*p++ = 0x16;
		*p++ = 0x00;
		*p++ = 0x00;
	
		*p++ = 0;		//Len1代表长度的高字节，
		*p++ = 0x20;	//Len2代表长度的低字节
		memrcpy(p, pbTx, 0x20);
		iDataLen = EsamTxRxFrm(g_bEsamTxRxBuf, 0x20, NULL, 6);
	}while(iDataLen==-2 && bcont<2);

	if (iDataLen < 0)  //
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("TransEncrAuth: fail\r\n"));		
		return false;
	}

	SignalSemaphore(g_semEsam);
	return true;
}

//描述：MAC校验
//返回：如果正确返回0，错误返回Random数据长度,其它错误返回负数
//备注：5.5 MAC校验
int EsamVerifyMac(BYTE bAFN, BYTE* pbData, WORD wDataLen, BYTE* pbMac, BYTE* pbRx)
{
	BYTE* p;
	BYTE bMac[16];
	int iDataLen;
	BYTE bcont = 0;
	//BYTE bBuf[1024];

	//KeyID分配关系表
	BYTE bKeyID;
	switch (bAFN)
	{
	case 0x01: //复位（AFN=01H）
		bKeyID = 0x20;
		break;

	case 0x04: //设参（AFN=04H）
		bKeyID = 0x21;
		break;

	case 0x05: //控制（AFN=05H）
		bKeyID = 0x22;
		break;

	case 0x10: //数据转发（AFN=10H）
		bKeyID = 0x23;
		break;
	
	default: return -1;
	}

	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);

	//发送：80FA+03+KeyID+0000
	//返回：9000+0000
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
		*p++ = 0x80;
		*p++ = 0xfa;
		*p++ = 0x03;
		*p++ = bKeyID;
		*p++ = 0x00;
		*p++ = 0x00;
		 iDataLen = EsamTxRxFrm(g_bEsamTxRxBuf, 0, NULL, 6);
	}while(iDataLen==-2 && bcont<2);

	if (iDataLen < 0)  //
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("VerifyMac: fail\r\n"));		
		return -1;
	}

	//发送：80FA+0200+LC+Data
	//返回：9000+LEN+MAC1
	bcont = 0;
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
		*p++ = 0x80;
		*p++ = 0xfa;
		*p++ = 0x02;
		*p++ = 0x00;
		*p++ = wDataLen>>8;	//Len1代表长度的高字节，
		*p++ = wDataLen;	//Len2代表长度的低字节
		memcpy(p, pbData, wDataLen);
		iDataLen = EsamTxRxFrm(g_bEsamTxRxBuf, wDataLen, bMac, 10);
	}while(iDataLen==-2 && bcont<2);

	if (iDataLen < 0)  //
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("TransEncrAuth: fail\r\n"));		
		return -1;
	}
	
	Swap(bMac, 4);//MAC 倒位 xzz

	if (memcmp(bMac, pbMac, 4) != 0)
	{
		TraceBuf(DB_FAPROTO, "VerifyMac: verify wrong, pro mac is", pbMac, 4);
		if (EsamGetRandom(pbRx, 8) <= 0)
		{	
			SignalSemaphore(g_semEsam);
			return -1;
		}

		SignalSemaphore(g_semEsam);
		return 8;
	}
	else
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("VerifyMac: verify ok\r\n"));		
		return 0;
	}
}

//描述：组广播MAC校验
//返回：如果正确返回true，错误返回 false
//备注：5.14 组广播
bool EsamGrpBroadcastVerifyMac(BYTE bAFN, WORD wGrpAddr, BYTE* pbData, WORD wDataLen, BYTE* pbRx)
{
	BYTE* p;
	int iDataLen;
	BYTE bcont = 0;
	WORD wExpLen =0;
    
	//KeyID分配关系表
	BYTE bKeyID1, bKeyID2;
	switch (bAFN)
	{
	case 0x01: //复位（AFN=01H）
		if(wGrpAddr == 0xffff)
		{
			bKeyID1 = 0x10;//12
			bKeyID2 = 0x1A;
		}
		else
		{
			bKeyID1 = 0x0C;
			bKeyID2 = 0x16;//12
		}
		wExpLen = 12;
		break;

	case 0x04: //设参（AFN=04H）
		if(wGrpAddr == 0xffff)
		{
			bKeyID1 = 0x11;
			bKeyID2 = 0x1B;//28
		}
		else
		{
			bKeyID1 = 0x0D;
			bKeyID2 = 0x17;//28
		}	
		wExpLen = 28;
		break;

	case 0x05: //控制（AFN=05H）
		if(wGrpAddr == 0xffff)
		{
			bKeyID1 = 0x12;
			bKeyID2 = 0x1C;//18

		}
		else
		{
			bKeyID1 = 0x0E;
			bKeyID2 = 0x18;//18
		}
		wExpLen = 18;
		break;

	case 0x10: //数据转发（AFN=10H）
		if(wGrpAddr == 0xffff)
		{
			bKeyID1 = 0x13;
			bKeyID2 = 0x1D;//47
		}
		else
		{
			bKeyID1 = 0x0F;
			bKeyID2 = 0x19;//47
		}
		wExpLen = 47;
		break;
	
	default: return false;
	}

	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);

	//发送：8022+KeyID1+KeyID2+LC+00+TotalTN(组地址)+130202224622+Data(明文+MAC)+20130202224622+05
	//返回：9000+LEN+明文
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
		*p++ = 0x80;
		*p++ = 0x22;
		*p++ = bKeyID1;
		*p++ = bKeyID2;
		*p++ = (wDataLen+17)>>8;	//Len1代表长度的高字节，
		*p++ = wDataLen+17;	//Len2代表长度的低字节
		*p++ = 0x00;
		*p++ = wGrpAddr>>8;
		*p++ = wGrpAddr;
		*p++ = 0x13;
		*p++ = 0x02;
		*p++ = 0x02;
		*p++ = 0x22;
		*p++ = 0x46;
		*p++ = 0x22;
		memcpy(p, pbData, wDataLen);
		p += wDataLen;
		Swap(p-4, 4);//MAC
		*p++ = 0x20;
		*p++ = 0x13;
		*p++ = 0x02;
		*p++ = 0x02;
		*p++ = 0x22;
		*p++ = 0x46;
		*p++ = 0x22;
		*p++ = 0x05;
		iDataLen = EsamTxRxFrm(g_bEsamTxRxBuf, wDataLen+17, NULL, wExpLen);
	}while(iDataLen==-2 && bcont<2);

	if (iDataLen < 0)  //
	{
		DTRACE(DB_FAPROTO,("EsamGrpBroadcastVerifyMac: fail\r\n"));
		EsamGetRandom(pbRx, 8);
		SignalSemaphore(g_semEsam);
		return false;
	}

    SignalSemaphore(g_semEsam);
	DTRACE(DB_FAPROTO,("EsamGrpBroadcastVerifyMac: verify ok\r\n"));	
	return true;
}

//描述：取随机数
//返回：如果正确返回数据长度，错误返回负数
int EsamGetRandom(BYTE* pbRx, BYTE bRandomLen)
{
//，则取随机数
//发送：801A+08+00 0000
//返回：9000+LEN+Random

//发送：801A 1000 0000
//返回：9000+LEN+R0
	int iDataLen;
	BYTE bcont = 0;
	//WaitSemaphore(g_semEsam);	//供内部函数调用，不要信号量

	BYTE bCmdGetRandom[] = {0x80, 0x1a, 0x00, 0x00, 0x00, 0x00};
	do
	{
		bcont++;
	    bCmdGetRandom[2] = bRandomLen;
		iDataLen = EsamTxRxCmd(bCmdGetRandom, pbRx, bRandomLen+6);
	}while(iDataLen==-2 && bcont<2);
	  
	if (iDataLen != bRandomLen) //
	{
		DTRACE(DB_FAPROTO,("GetRandom: fail\r\n"));
		//SignalSemaphore(g_semEsam);
		return -1;
	}
	
	//SignalSemaphore(g_semEsam);
	return bRandomLen;
}

//描述：通过ESAM加密'身份认证任务中的电表密钥密文',得到 ER0：16字节密文
//参数：@bP2 对应为命令中的P2，P2设置为01时，下发任务命令数据中的表号为实际表号；P2设置为02时下发任务命令数据中的表号为8字节AA时
//		@pbMtrAddr 实际表号，不能是8字节AA
//		@pbMtrCiph 身份认证任务中的电表密钥密文
//返回：如果正确返回数据长度，错误返回负数
bool EsamGetMtrAuth(BYTE bP2, BYTE* pbMtrCiph, BYTE* pbMtrAddr, BYTE* pbTskData, BYTE* pbR1, BYTE* pbER1)
{
	BYTE* p;
	WORD wLen;
	BYTE i;
	BYTE bTmp[32];
	int iDataLen;
	BYTE bcont = 0;
	//BYTE bBuf[128];

	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);
	
	if (EsamGetRandom(g_bEsamTxRxBuf, 16) != 16)
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("GetER1: fail to GetRandom\r\n"));		
		return false;
	}

	for (i=0; i<8; i++)
	{
		pbR1[i] = g_bEsamTxRxBuf[i] ^ g_bEsamTxRxBuf[8+i];
	}

	//发送：841C00P2+LC+身份认证任务中的电表密钥密文+分散因子+R1
	//返回：9000+LEN+ER0
	//if (IsAllAByte(pbMtrAddr, 0xaa, 8))
		//bP2 = 2;

	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
	
		*p++ = 0x84;
		*p++ = 0x1c;
		*p++ = 0x00;
		*p++ = bP2;
	
		if (bP2 == 1)	//第一种情况，指定表号的身份认证任务:P2设置为01，分散因子为0000+6字节表号；
		{
			*p++ = 0x00;	//Len1代表长度的高字节，
			*p++ = 0x30;	//Len2代表长度的低字节
	
			memcpy(p, pbMtrCiph, 32);
			p += 32;
			memcpy(p, pbMtrAddr, 8);
			p += 8;
			memcpy(p, pbR1, 8);
			p += 8;
	
			wLen = 0x30;
		}
		else	//第二种情况，通用表号的身份认证任务
		{
			*p++ = 0x00;	//Len1代表长度的高字节，
			*p++ = 0x38;	//Len2代表长度的低字节
	
			memcpy(p, pbMtrCiph, 32);
			p += 32;
			memcpy(p, pbTskData, 2);
			p += 2;
			memcpy(p, pbMtrAddr+2, 6);
			p += 6;
			memset(p, 0x00, 2);
			p += 2;
			memcpy(p, pbMtrAddr+2, 6);
			p += 6;
			memcpy(p, pbR1, 8);
			p += 8;
	
			wLen = 0x38;
		}
		iDataLen = EsamTxRxFrm(g_bEsamTxRxBuf, wLen, bTmp, 22);
	}while(iDataLen==-2 && bcont<2);

	if (iDataLen < 16)  //
	{
		DTRACE(DB_FAPROTO,("GetER1: fail\r\n"));
		SignalSemaphore(g_semEsam);
		return false;
	}
    
    SignalSemaphore(g_semEsam);

	for (i=0; i<8; i++)
	{
		pbER1[i] = bTmp[i] ^ bTmp[8+i];
	}
	
	return true;
}

//描述：通过ESAM加密'电表对时任务密文'
//参数：@pbMtrCiph 身份认证任务中的电表密钥密文
//返回：如果正确返回数据长度，错误返回负数
//备注：在电表身份认证中返回
//		随机数2（R2）：38A45D72（在对时任务举例中使用）
//		ESAM序列号：100112BB4798D2FD
int EsamGetAdjTmCiph(const BYTE* pbTskFmt, BYTE* pbTskData, BYTE bTskLen, BYTE* pbMtrKeyCiph, BYTE* pbR2, BYTE* pbRx)
{
	BYTE* p;
	TTime now;
	int iDataLen;
	BYTE bcont = 0;
	WORD wLen;
	//BYTE bBuf[128];

	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);

	//发送：8418+任务格式（0404）+005A+任务类型(01)+05+0032+0005+07+
	//		YYMMDDWWhhmmss+任务数据（含数据标识）+对时任务中电表密钥密文+ R2
	//返回：9000+LEN+数据标识（4字节）+电表对时任务密文+4字节MAC

	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
	
		*p++ = 0x84;
		*p++ = 0x18;
		memcpy(p, pbTskFmt, 2);
		p += 2;
	
		*p++ = 0x00;	//Len1代表长度的高字节，
		*p++ = 0x5a;	//Len2代表长度的低字节
	
		*p++ = 0x01; 	//任务类型
		*p++ = 0x05;
		*p++ = 0x00;
		*p++ = 0x32;
		*p++ = 0x00;
		*p++ = 0x05;
		*p++ = 0x07;
	
		GetCurTime(&now);
		*p++ = now.nYear;
		*p++ = now.nMonth;
		*p++ = now.nDay;
		*p++ = dayofweek(&now);
		*p++ = now.nHour;
		*p++ = now.nMinute;
		*p++ = now.nSecond;
	
		memcpy(p, pbTskData, bTskLen);
		p += bTskLen;
	
		memcpy(p, pbMtrKeyCiph, 32);
		p += 32;
	
		memcpy(p, pbR2, 4);
		p += 4;
	
		wLen = p - &g_bEsamTxRxBuf[CLA_OFF+6];
		iDataLen=EsamTxRxFrm(g_bEsamTxRxBuf, wLen, pbRx, 30);
	}while(iDataLen==-2 && bcont<2);

	if (iDataLen < 0)	//举例中返回24个字节的长度，为了稳妥起见，多接收点字节
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("GetAdjTmCiph: fail\r\n"));		
		return -1;
	}

	SignalSemaphore(g_semEsam);
	return iDataLen;
}
