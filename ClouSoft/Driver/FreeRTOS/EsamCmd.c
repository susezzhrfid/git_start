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

// IEC7816��������
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
static BYTE g_bCertState = 0;	//֤��״̬��00Ϊ����֤�飻01Ϊ��ʽ֤��
TSem g_semEsam;   //����g_bEsamTxRxBuf������ÿһ����������
BYTE g_bEsamTxRxBuf[1800];		//���ͽ��չ���
BYTE g_bEsamCmpDataBuf[20];
WORD g_wEsamCmpDataLen = 0;
WORD g_wEsamLastRxDataLen = 0;

static const TSWTab g_tSwTable[]=
{
	{0x90,0x00,"OK"},//ָ��ִ�гɹ�
	{0x63,0xcf,"Cert Or Swt failed"},//��֤ʧ��
	{0x64,0x00,"InExcu wrong"},//�ڲ�ִ�г���
	{0x65,0x81,"Card locked"},//��������
	{0x67,0x00,"Len wrong"},//Lc��Le���ȴ�
	{0x69,0x01,"Cntzero Or Cmd wrong"},//���߼�����Ϊ0���������
	{0x69,0x82,"No Safe sta"},//�����㰲ȫ״̬
	{0x69,0x83,"Kut zero"},//Kutʹ�ô���Ϊ0
	{0x69,0x84,"Use Data invalid"},//����������Ч
	{0x69,0x85,"Condition Incomplete"},//ʹ������������
	{0x69,0x86,"Online Cnt zero"},//���߼�����Ϊ0
	{0x69,0x88,"MAC wrong"},//MAC����
	{0x6a,0x80,"Para wrong"},//��������
	{0x6a,0x86,"P1P2 wrong"},//��������
	{0x6a,0x88,"No Find data"},//δ�ҵ���������
	{0x6d,0x00,"Cmd No exist"},//�������
	{0x6e,0x00,"Cmd Or CLA wrong "},//�����CLA��
	{0x6f,0x00,"Data invalid"},//������Ч
	{0x90,0x86,"CheckSign wrong"},//��ǩ����
	{0x9e,0x2f,"File wrong"},//�ļ�����
	{0x9e,0x3f,"Calc wrong"},//�㷨�������
	{0x9e,0x57,"Cert wrong"},//��֤����
	{0x9e,0x60,"Session wrong"},//�����Ự����
	{0x9e,0x5e,"CA wrong"},//CA֤�����
};

BYTE dayofweek(const TTime* time)
{
	BYTE bWeek = DayOfWeek(time); //DayOfWeek()�ķ��� 1 = Sunday, 2 = Monday, ..., 7 = Saturday
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
	//֤��״̬��00Ϊ����֤�飻01Ϊ��ʽ֤��
	//��ȡоƬ״̬��Ϣ1�ֽ�
	//���ͣ�800E00050000
	//���أ�9000+LEN+оƬ״̬��Ϣ
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

	DTRACE(DB_CRITICAL, ("Esam driver init OK��g_bCertState=%d\n", g_bCertState));
	return true; 
}

bool EsamReset()
{
	esam_warm_reset();
	return true;
}

//������@pbBuf ���ջ�����
//		@wExpLen �ڴ��Ľ��ճ���
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
	//�������ݵĽṹΪ��SW1 SW2 Len1 Len2 DATA LRC2
	//LRC2�ļ��㷽������Len1 Len2 DATA���ݣ�ÿ���ֽڵ����ֵ����ȡ����
	//Len1�����ȵĸ��ֽڣ�Len2�����ȵĵ��ֽ�
	//Len1 Len2����DATA��ĳ��ȣ�������LRC1��LRC2
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

//��ע��CLA INS P1 P2 Len1 Len2 DATA���Ѿ��źã�����ʣ��Ĳ���
WORD EsamMakeTxFrm(BYTE* pbFrm, WORD wDataLen)
{
	//�������ݵĽṹΪ��55 CLA INS P1 P2 Len1 Len2 DATA LRC1
	//Len1 Len2�Ǻ���DATA�ĳ��ȣ�������LRC1�������ֽڱ�ʾ
	//LRC1�ļ��㷽������CLA INS P1 P2 Len1 Len2 DATA���ݣ�ÿ���ֽڵ����ֵ����ȡ����
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


//���������ͽ���һ��֡��ֱ��ʹ��pbTx�Ļ��壬�����ⶨ�建����
//����	@wExpLen �ڴ��Ľ��ճ���
int EsamTxRxFrm(BYTE* pbTx, WORD wDataLen, BYTE* pbRx, WORD wExpLen)
{
	int iLen, iDataLen, iStart = -1;
	WORD wTxLen;
//	BYTE  bReadBuf[60];//д֮ǰ���������� g_bEsamTxRxBuf,��Ϊ��ָ��pbTx�ظ���  xzz   
	if (!g_fEsamOpen)
	{
		DTRACE(DB_FAPROTO,("EsamTxRxFrm: failed due to device not exit\r\n"));
		return -1;
	}

	wTxLen = EsamMakeTxFrm(pbTx, wDataLen);

	/************/ //д֮ǰ�����Ļ��ղ������ݣ��е���֣��������ʺ������
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
        wTxLen += 1;    //���ȼ�һ��
    }*/
        
	if (EsamWrite(pbTx, wTxLen) != wTxLen)
	{
		DTRACE(DB_FAPROTO,("EsamTxRxFrm: write failed %d\r\n", wTxLen));
		return -1;
	}
    
//#ifdef SYS_WIN
	TraceBuf(DB_FAFRM, "EsamTxRxFrm: write", pbTx, wTxLen);
//#endif

    /*if (wTxLen == 1764)//�Ự��ʼ��  1764
        Sleep(900);
    else if (wTxLen == 1692)//֤�����1692  
        Sleep(1300);
    else if (wTxLen == 286)//�ỰЭ�� 286 
        Sleep(600);
	else if (wTxLen > 1000)     
		Sleep(800);	
    else if (wTxLen > 200)
        Sleep(200);
    */
	//Sleep(1300);
	wExpLen += 32;		//�����ж������ݣ������һЩ�ֽ�
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
	//  	return -2;////���ظ�ֵ˵�������⣬��Ҫ��������
	//}
	/////////////////////////////////////////////
	if (pbRx != NULL)
		memcpy(pbRx, &g_bEsamTxRxBuf[iStart+4], iDataLen);
    
	if (g_wEsamLastRxDataLen!=0 && iDataLen==g_wEsamLastRxDataLen)
	{
		if (memcmp(g_bEsamCmpDataBuf, &g_bEsamTxRxBuf[iStart+4], g_wEsamCmpDataLen) == 0)
        {    
            g_wEsamCmpDataLen = g_wEsamLastRxDataLen = 0;   //ֻ����һ�δ����ط�һ�Σ��ٷ�������Ͳ�����
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

//���������ͽ���һ�����������������ݳ���Ϊ0
int EsamTxRxCmd(const BYTE* pbTx, BYTE* pbRx, WORD wExpLen)
{
	BYTE bBuf[16];
	memcpy(&bBuf[CLA_OFF], pbTx, 6);
	return EsamTxRxFrm(bBuf, 0, pbRx, wExpLen);
}

//�¼ӳ���
//������F11	��ȡ�ն���Ϣ
//���أ������ȷ�������ݳ��ȣ����󷵻ظ���
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
	//��ȡоƬ���к�8�ֽ�
	//���ͣ�800E00020000
	//���أ�9000+LEN+ESAM���к�
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
	
	//��ȡ���߼�����4�ֽ�
	//���ͣ�800E00030000
	//���أ�9000+LEN+���߼�������Ϣ
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

	//��ȡоƬ״̬��Ϣ1�ֽ�
	//���ͣ�800E00050000
	//���أ�9000+LEN+оƬ״̬��Ϣ
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

	//��ȡ��Կ�汾��8�ֽ�
	//���ͣ�800E00060000
	//���أ�9000+LEN+��Կ�汾��Ϣ
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
	//֤�����кţ�16�ֽ�
	//���ͣ�8032 + P1 + 02 +0000
	//���أ�9000+LEN+֤�����к�
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


//������F12	�Ự��ʼ��/�ָ�
//��ע���Ự��ʼ���ͻỰ�ָ���վ�·��������ESAM�������ʽ��ȫһ��
int EsamInitSession(BYTE* pbTx, WORD wTxLen, BYTE* pbRx)
{
	//���ͣ�84100000+LC+����1+Time
	//���أ�9000+LEN+����2
	int iDataLen;
	BYTE bcont = 0;
	WORD wExpLen = 0;
	TTime now;
	//BYTE bBuf[1800];
    WaitSemaphore(g_semEsam,SYS_TO_INFINITE);
	Swap(pbTx+2, wTxLen-0x56);//��վ֤��
	Swap(pbTx+(wTxLen-0x54), 16);//Eks1��R1��
	Swap(pbTx+(wTxLen-0x44),4);//MAC1
	Swap(pbTx+(wTxLen-0x40),64);//ǩ������S1
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
		
		*p++ = (wTxLen+7)>>8;	//Len1�����ȵĸ��ֽڣ�
		*p++ = wTxLen+7;		//Len2�����ȵĵ��ֽ�
		memcpy(p, pbTx, wTxLen);
		if (*(p+1) == 1)//�Ự�ָ�
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
	
	//if ((iDataLen=EsamTxRxFrm(g_bEsamTxRxBuf, wTxLen+7, pbRx, 1782)) <= 0)	//114+128+5 �·�ָ��1756+7������ָ������1777+5
	if(iDataLen <= 0)
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("InitSession: fail\r\n"));		
		return -1;
	}
	
	
	if(*(pbRx+1) == 0x00)//�ỰIDΪ0
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

//������F13	�ỰЭ��
//��ע���� 5.3 �Ự��ԿЭ�� �е�����
int EsamNegotiateKey(BYTE* pbTx, WORD wTxLen, BYTE* pbRx)
{
	//���ͣ�84120000+LC+����3
	//���أ�9000+LEN+����4
	BYTE* p;
	int iDataLen;
	BYTE bcont = 0;
	//BYTE bBuf[512];
	
	
	Swap(pbTx, 113);//�Ự��Կ����
	Swap(pbTx+113, wTxLen-181);//��վ֤����֤��
	Swap(pbTx+(wTxLen-68), 4);//MAC2
	Swap(pbTx+(wTxLen-64), 64);//ǩ������S3


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
	
		*p++ = wTxLen>>8;	//Len1�����ȵĸ��ֽڣ�
		*p++ = wTxLen;		//Len2�����ȵĵ��ֽ�
		memcpy(p, pbTx, wTxLen);
		iDataLen=EsamTxRxFrm(g_bEsamTxRxBuf, wTxLen, pbRx, 26);
	}while(iDataLen==-2 && bcont<2);
	
	//if ((iDataLen=EsamTxRxFrm(g_bEsamTxRxBuf, wTxLen, pbRx, 25)) <= 0) //�·�ָ��278+7������ָ������20+5
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

//������F14	�Գ���Կ����
//���أ������ȷ����true�����󷵻�false
bool EsamUpdSymKey(BYTE bKeyNum, BYTE* pbKey)
{
	//���ͣ�84240100+LC+��Կ��������
	//���أ�9000+0000
	BYTE* p;
	BYTE bcont = 0;
	BYTE i;
	int iDataLen;
	//BYTE bBuf[1500]; //46��ȫ����Կ�����·�ָ��1473+7
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

		*p++ = ((WORD)(bKeyNum*32+1)) >> 8;	//Len1�����ȵĸ��ֽڣ�
		*p++ = (WORD)(bKeyNum*32+1);	//Len2�����ȵĵ��ֽ�
		*p++ = bKeyNum;
		//memcpy(p, pbKey, bKeyNum*32);
		for(i=0; i<bKeyNum; i++)
		{
			memrcpy(p+i*32, pbKey+i*32, 32);//��Կ����
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

//������F15	�ն�֤�����
//���أ������ȷ�������ݳ��ȣ����󷵻ظ���
//��ע����Ӧ5.7 �ն�֤���������
int EsamUpdCert(BYTE* pbRx)
{
    int iDataLen;
	BYTE bcont = 0;
	const BYTE bCmdGetPublic[] = {0x80, 0x2c, 0x00, 0x01, 0x00, 0x00};
	BYTE bCmdGetCertSN[] = {0x80, 0x32, 0x00, 0x02, 0x00, 0x00};

	//֤�����кţ�16�ֽ�
	//���ͣ�8032+P1+020000
	//���أ�9000+LEN+�ն�֤�����к�
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

	//���ͣ�802C00010000
	//���أ�9000+LEN+�ն˹�Կ
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

//������F16	CA֤�����
//���أ������ȷ����ture�����󷵻�false
//��ע����Ӧ5.8 ֤�����
bool EsamUpdCA(BYTE* pbTx, WORD wTxLen)
{
	//���ͣ�8430+֤���������
	//���أ�9000+0000
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

    if (iDataLen < 0) //�·�ָ��1684+7������ָ������5
	{
		DTRACE(DB_FAPROTO,("UpdCA: fail\r\n"));
		SignalSemaphore(g_semEsam);
		return false;
	}

	SignalSemaphore(g_semEsam);
	return true;
}

//������F17	�ڲ���֤
//���أ������ȷ�������ݳ��ȣ����󷵻ظ���
//��ע��5.9 �ڡ��ⲿ��֤
int EsamIntCert(BYTE* pbTx, WORD wTxLen, BYTE* pbRx)
{
	BYTE* p;
	int iDataLen;
	BYTE bcont = 0;

	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);

	//���ͣ�842600000010+R4
	//���أ�9000+LEN+ER4
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
	
		*p++ = 0x84;
		*p++ = 0x26;
		*p++ = 0x00;
		*p++ = 0x00;
	
		*p++ = 0;		//Len1�����ȵĸ��ֽڣ�
		*p++ = 0x10;	//Len2�����ȵĵ��ֽ�
		memrcpy(p, pbTx, 16);
		iDataLen = EsamTxRxFrm(g_bEsamTxRxBuf, 16, pbRx, 22);
	}while(iDataLen==-2 && bcont<2);

	if (iDataLen <= 0) //
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("IntCert: fail\r\n"));		
		return -1;
	}

	//���ͣ�801A10000000
	//���أ�9000+LEN+R5
	bcont = 0;
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
		
		*p++ = 0x80;
		*p++ = 0x1a;
		*p++ = 0x10;
		*p++ = 0x00;
		
		*p++ = 0;	//Len1�����ȵĸ��ֽڣ�
		*p++ = 0;	//Len2�����ȵĵ��ֽ�
		iDataLen = EsamTxRxFrm(g_bEsamTxRxBuf, 0, pbRx+16, 22);
	}while(iDataLen==-2 && bcont<2);

	if (iDataLen <= 0) //
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("IntCert: fail\r\n"));		
		return -1;
	}
	
	Swap(pbRx, 16);//R4����
	Swap(pbRx+16, 16);//R5

	SignalSemaphore(g_semEsam);
	return 32;
}

//������F18	�ⲿ��֤
//���أ������ȷ�������ݳ��ȣ����󷵻ظ���
//��ע��5.9 �ڡ��ⲿ��֤
int EsamExtCert(BYTE* pbTx, WORD wTxLen, BYTE* pbRx)
{
	BYTE* p;	
    int iDataLen;
	BYTE bcont = 0;

	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);

	//���ͣ�842800000010+ER5
	//���أ�9000+0000	
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
		
		*p++ = 0x84;
		*p++ = 0x28;
		*p++ = 0x00;
		*p++ = 0x00;
		
		*p++ = 0;		//Len1�����ȵĸ��ֽڣ�
		*p++ = 0x10;	//Len2�����ȵĵ��ֽ�
		memrcpy(p, pbTx, 16);
		iDataLen=EsamTxRxFrm(g_bEsamTxRxBuf, 16, NULL, 6);
	}while(iDataLen==-2 && bcont<2);

	if (iDataLen < 0) //
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("ExtCert: fail\r\n"));		
		return -1;
	}

	//���ͣ�801A10000000
	//���أ�9000+LEN+R6
	bcont = 0;
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
	
		*p++ = 0x80;
		*p++ = 0x1a;
		*p++ = 0x10;
		*p++ = 0x00;
	
		*p++ = 0;		//Len1�����ȵĸ��ֽڣ�
		*p++ = 0;		//Len2�����ȵĵ��ֽ�
		iDataLen = EsamTxRxFrm(g_bEsamTxRxBuf, 0, pbRx, 22);
	}while(iDataLen==-2 && bcont<2);

	if (iDataLen <= 0) //
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("ExtCert: fail\r\n"));		
		return -1;
	}

	Swap(pbRx, 0x10);//R6 ��λ

	SignalSemaphore(g_semEsam);
	return iDataLen;
}

//������F19	״̬�л�
//���أ������ȷ����ture�����󷵻�false
//��ע��5.10 ֤��״̬�л�
bool EsamSwitchState(BYTE bP1, BYTE* pbTx, WORD wTxLen)
{
	BYTE* p;
	int iDataLen;
	BYTE bcont = 0;
    
	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);

	//���ͣ�842A+P1+000014+ER6+MAC6
	//���أ�9000+0000	
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
	
		*p++ = 0x84;
		*p++ = 0x2a;
		*p++ = bP1;
		*p++ = 0x00;
	
		*p++ = 0;		//Len1�����ȵĸ��ֽڣ�
		*p++ = 0x14;	//Len2�����ȵĵ��ֽ�
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
	//��֤��״̬�����ϵͳ��
	SignalSemaphore(g_semEsam);
	return true;
}

//������F20	�����߼�����
//���أ������ȷ����ture�����󷵻�false
//��ע��5.11 �����߼�����
bool EsamSetOfflineCnt(BYTE* pbTx, WORD wTxLen)
{
	BYTE* p;	
	int iDataLen;
	BYTE bcont = 0;

	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);

	//���ͣ�842000000014+���߼���������+MAC
	//���أ�9000+0000
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
	
		*p++ = 0x84;
		*p++ = 0x20;
		*p++ = 0x00;
		*p++ = 0x00;
	
		*p++ = 0;		//Len1�����ȵĸ��ֽڣ�
		*p++ = 0x14;	//Len2�����ȵĵ��ֽ�
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

//������F21	ת������Ȩ
//���أ������ȷ����ture�����󷵻�false
//��ע��5.12 ת������Ȩ
bool EsamTransEncrAuth(BYTE* pbTx, WORD wTxLen)
{
	BYTE* p;
	int iDataLen;
	BYTE bcont;
	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);

	//���ͣ�801600000020+ת��������
	//���أ�9000+0000
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
	
		*p++ = 0x80;
		*p++ = 0x16;
		*p++ = 0x00;
		*p++ = 0x00;
	
		*p++ = 0;		//Len1�����ȵĸ��ֽڣ�
		*p++ = 0x20;	//Len2�����ȵĵ��ֽ�
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

//������MACУ��
//���أ������ȷ����0�����󷵻�Random���ݳ���,�������󷵻ظ���
//��ע��5.5 MACУ��
int EsamVerifyMac(BYTE bAFN, BYTE* pbData, WORD wDataLen, BYTE* pbMac, BYTE* pbRx)
{
	BYTE* p;
	BYTE bMac[16];
	int iDataLen;
	BYTE bcont = 0;
	//BYTE bBuf[1024];

	//KeyID�����ϵ��
	BYTE bKeyID;
	switch (bAFN)
	{
	case 0x01: //��λ��AFN=01H��
		bKeyID = 0x20;
		break;

	case 0x04: //��Σ�AFN=04H��
		bKeyID = 0x21;
		break;

	case 0x05: //���ƣ�AFN=05H��
		bKeyID = 0x22;
		break;

	case 0x10: //����ת����AFN=10H��
		bKeyID = 0x23;
		break;
	
	default: return -1;
	}

	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);

	//���ͣ�80FA+03+KeyID+0000
	//���أ�9000+0000
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

	//���ͣ�80FA+0200+LC+Data
	//���أ�9000+LEN+MAC1
	bcont = 0;
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
		*p++ = 0x80;
		*p++ = 0xfa;
		*p++ = 0x02;
		*p++ = 0x00;
		*p++ = wDataLen>>8;	//Len1�����ȵĸ��ֽڣ�
		*p++ = wDataLen;	//Len2�����ȵĵ��ֽ�
		memcpy(p, pbData, wDataLen);
		iDataLen = EsamTxRxFrm(g_bEsamTxRxBuf, wDataLen, bMac, 10);
	}while(iDataLen==-2 && bcont<2);

	if (iDataLen < 0)  //
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("TransEncrAuth: fail\r\n"));		
		return -1;
	}
	
	Swap(bMac, 4);//MAC ��λ xzz

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

//��������㲥MACУ��
//���أ������ȷ����true�����󷵻� false
//��ע��5.14 ��㲥
bool EsamGrpBroadcastVerifyMac(BYTE bAFN, WORD wGrpAddr, BYTE* pbData, WORD wDataLen, BYTE* pbRx)
{
	BYTE* p;
	int iDataLen;
	BYTE bcont = 0;
	WORD wExpLen =0;
    
	//KeyID�����ϵ��
	BYTE bKeyID1, bKeyID2;
	switch (bAFN)
	{
	case 0x01: //��λ��AFN=01H��
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

	case 0x04: //��Σ�AFN=04H��
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

	case 0x05: //���ƣ�AFN=05H��
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

	case 0x10: //����ת����AFN=10H��
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

	//���ͣ�8022+KeyID1+KeyID2+LC+00+TotalTN(���ַ)+130202224622+Data(����+MAC)+20130202224622+05
	//���أ�9000+LEN+����
	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
		*p++ = 0x80;
		*p++ = 0x22;
		*p++ = bKeyID1;
		*p++ = bKeyID2;
		*p++ = (wDataLen+17)>>8;	//Len1�����ȵĸ��ֽڣ�
		*p++ = wDataLen+17;	//Len2�����ȵĵ��ֽ�
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

//������ȡ�����
//���أ������ȷ�������ݳ��ȣ����󷵻ظ���
int EsamGetRandom(BYTE* pbRx, BYTE bRandomLen)
{
//����ȡ�����
//���ͣ�801A+08+00 0000
//���أ�9000+LEN+Random

//���ͣ�801A 1000 0000
//���أ�9000+LEN+R0
	int iDataLen;
	BYTE bcont = 0;
	//WaitSemaphore(g_semEsam);	//���ڲ��������ã���Ҫ�ź���

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

//������ͨ��ESAM����'�����֤�����еĵ����Կ����',�õ� ER0��16�ֽ�����
//������@bP2 ��ӦΪ�����е�P2��P2����Ϊ01ʱ���·��������������еı��Ϊʵ�ʱ�ţ�P2����Ϊ02ʱ�·��������������еı��Ϊ8�ֽ�AAʱ
//		@pbMtrAddr ʵ�ʱ�ţ�������8�ֽ�AA
//		@pbMtrCiph �����֤�����еĵ����Կ����
//���أ������ȷ�������ݳ��ȣ����󷵻ظ���
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

	//���ͣ�841C00P2+LC+�����֤�����еĵ����Կ����+��ɢ����+R1
	//���أ�9000+LEN+ER0
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
	
		if (bP2 == 1)	//��һ�������ָ����ŵ������֤����:P2����Ϊ01����ɢ����Ϊ0000+6�ֽڱ�ţ�
		{
			*p++ = 0x00;	//Len1�����ȵĸ��ֽڣ�
			*p++ = 0x30;	//Len2�����ȵĵ��ֽ�
	
			memcpy(p, pbMtrCiph, 32);
			p += 32;
			memcpy(p, pbMtrAddr, 8);
			p += 8;
			memcpy(p, pbR1, 8);
			p += 8;
	
			wLen = 0x30;
		}
		else	//�ڶ��������ͨ�ñ�ŵ������֤����
		{
			*p++ = 0x00;	//Len1�����ȵĸ��ֽڣ�
			*p++ = 0x38;	//Len2�����ȵĵ��ֽ�
	
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

//������ͨ��ESAM����'����ʱ��������'
//������@pbMtrCiph �����֤�����еĵ����Կ����
//���أ������ȷ�������ݳ��ȣ����󷵻ظ���
//��ע���ڵ�������֤�з���
//		�����2��R2����38A45D72���ڶ�ʱ���������ʹ�ã�
//		ESAM���кţ�100112BB4798D2FD
int EsamGetAdjTmCiph(const BYTE* pbTskFmt, BYTE* pbTskData, BYTE bTskLen, BYTE* pbMtrKeyCiph, BYTE* pbR2, BYTE* pbRx)
{
	BYTE* p;
	TTime now;
	int iDataLen;
	BYTE bcont = 0;
	WORD wLen;
	//BYTE bBuf[128];

	WaitSemaphore(g_semEsam,SYS_TO_INFINITE);

	//���ͣ�8418+�����ʽ��0404��+005A+��������(01)+05+0032+0005+07+
	//		YYMMDDWWhhmmss+�������ݣ������ݱ�ʶ��+��ʱ�����е����Կ����+ R2
	//���أ�9000+LEN+���ݱ�ʶ��4�ֽڣ�+����ʱ��������+4�ֽ�MAC

	do
	{
		bcont++;
		p = &g_bEsamTxRxBuf[CLA_OFF];
	
		*p++ = 0x84;
		*p++ = 0x18;
		memcpy(p, pbTskFmt, 2);
		p += 2;
	
		*p++ = 0x00;	//Len1�����ȵĸ��ֽڣ�
		*p++ = 0x5a;	//Len2�����ȵĵ��ֽ�
	
		*p++ = 0x01; 	//��������
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

	if (iDataLen < 0)	//�����з���24���ֽڵĳ��ȣ�Ϊ���������������յ��ֽ�
	{
        SignalSemaphore(g_semEsam);
		DTRACE(DB_FAPROTO,("GetAdjTmCiph: fail\r\n"));		
		return -1;
	}

	SignalSemaphore(g_semEsam);
	return iDataLen;
}
