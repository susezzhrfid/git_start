//#include "stdafx.h"
#include "SearchMeter.h"
#include "ComAPI.h"
#include "MtrAPI.h"
#include "FaAPI.h"
#include "FlashMgr.h"

TMtrSchInf g_tMtrRdSchInf[MTR_PORT_NUM];
const WORD g_wTestID[] = {0x901f, 0x9010};

const TMeterPro g_tMeterPro[] = 
{//---Baud-------Proto---
    {CBR_1200, CCT_MTRPRO_97},
    {CBR_2400, CCT_MTRPRO_07},  
};


//�������Ӵ��ڻ�������������645�ı���
//������@pbBlock - ���ܵĻ���
//		@dwLen - ���յĳ���
//		@pbRxBuf - �����������֡�Ļ���
//		@pbRxLen - ��������֡���ĳ���
//		@dwBufSize - ���ջ������ĳ���
//���أ�0-�����ݣ�����-���յ�������֡���ȣ�����-��Ч���ݳ���
int Mtr645RcvBlock(BYTE* pbBlock, DWORD dwLen, BYTE* pbRxBuf, BYTE* pbRxLen, DWORD dwBufSize)
{
	WORD i;
	BYTE bRxPtr = 0;
	BYTE bRxCnt = 0;
	BYTE bRxStep = 0;
	short sFrmHead = -1;

	for (i=0; i<dwLen; i++)
	{
		BYTE b = *pbBlock++;

		switch (bRxStep) 
		{
		case 0:   //0x68
			if (b == 0x68)
			{
				pbRxBuf[0] = 0x68;
				bRxPtr = 1;
				bRxCnt = 9;       
				bRxStep = 1;
				sFrmHead = i;//��֮ǰ�����ݶ�����Ч��
			}
			break;
		case 1:    //������ǰ������
			pbRxBuf[bRxPtr++] = b;
			bRxCnt --;
			if (bRxCnt == 0)   //�����꣬����У��
			{
				if (pbRxBuf[7] == 0x68) // && (p[FAPDL645_CMD]&FAPDL645_CMD_DIR)==FAPDL645_CMD_DOWN //��ֹ���յ����ⷵ�ص��Լ�����ȥ��֡
				{
					bRxCnt = pbRxBuf[9] + 2;  //0xfe+2
					sFrmHead++;
					if (bRxCnt+10>dwBufSize || pbRxBuf[9]>=dwBufSize)   //��֡�Ļ���������
					{
						bRxStep = 0;					
						break;
						//return -sFrmHead;                 //���ﷵ��0������������Զ�޷��ͷ�
					}	
					bRxStep = 2;
				}
				else
				{					
					bRxStep = 0;
					sFrmHead++;
				}		
			}
			break;
		case 2:     //���� + ������ + ������
			pbRxBuf[bRxPtr++] = b;
			//DTRACE(DB_DL645V07, ("CDL645V07::RcvBlock : m_wRxCnt=%d.m_wRxPtr=%d,m_bRxBuf=%x\r\n", m_wRxCnt, m_wRxPtr, m_bRxBuf[m_wRxPtr-1])); 	
			bRxCnt -- ;
			if (bRxCnt == 0)   //�����꣬����У��
			{
				bRxStep = 0;

				if (pbRxBuf[bRxPtr-1]==0x16 && pbRxBuf[bRxPtr-2]==CheckSum(pbRxBuf, pbRxBuf[9]+10))
				{
					*pbRxLen = pbRxBuf[9] + 12;
					//return i+1;//���յ�������һ֡		
					return dwLen;//���յ�������һ֡	����ȫ�� �Ա�ʾ��������ȫ���������
				}
				else
				{
					sFrmHead++;
				}
			}
			break;
		default:
			bRxStep = 0;
			break;
		} //switch (m_nRxStep) 
	}

	if 	(sFrmHead != -1)
		return -sFrmHead;

	return -(int)dwLen;
}	


static void InitStack(TStack *ptStack)
{
    ptStack->iTop = 0;                   //��ѹջ���ƶ������ƶ����ջ��
}

static BYTE IsStackEmpty(TStack *ptStack)
{
    if (ptStack->iTop == 0)
        return 1;             //��
    return 0;
}

static BYTE PushStack(TStack *ptStack, BYTE bData)
{
    if ((ptStack->iTop+1) == STACK_SIZE)          //ջ���� ������ջ
    {
        return 0;
    }
    else
    {
        ptStack->bData[ptStack->iTop] = bData;
        ptStack->iTop++;
        return 1;
    }
}

static BYTE PopStack(TStack *ptStack, BYTE *pbData)
{
    if (IsStackEmpty(ptStack))           //��ջ
    {
        return 0;
    }
    else
    {
        ptStack->iTop--;
        *pbData = ptStack->bData[ptStack->iTop];
        return 1;
    }
}

static int DepthStack(TStack *ptStack)
{
    return ptStack->iTop;
}

//ȡ��ջ��Ԫ�ص����ǵ���
static BYTE GetStackTop(TStack *ptStack, BYTE *pbData)
{
    if (IsStackEmpty(ptStack))           //��ջ
    {
        return 0;
    }
    else
    {
        *pbData = ptStack->bData[ptStack->iTop-1];
        return 1;
    }
}

void GetSchMtrStaInfo()
{
	BYTE i;

	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);

	memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
	if(!readfile(FILE_SCHMTR_STATUS, 0, g_ExFlashBuf, -1))
	{
		SignalSemaphore(g_semExFlashBuf);
		return ;
	}

	if(!CheckFile(FILE_SCHMTR_STATUS, g_ExFlashBuf, 0))
	{
		SignalSemaphore(g_semExFlashBuf);
		return ;
	} 
	memcpy(g_tMtrRdSchInf, g_ExFlashBuf, sizeof(TMtrSchInf)*MTR_PORT_NUM);

	SignalSemaphore(g_semExFlashBuf);
	//����
	for (i=0; i<MTR_PORT_NUM; i++)
	{
		if (g_tMtrRdSchInf[i].bSearchState>SEARCHWAIT || g_tMtrRdSchInf[i].bCurTryLevel>6 || DepthStack(&g_tMtrRdSchInf[i].tStack)<0 || DepthStack(&g_tMtrRdSchInf[i].tStack)>=STACK_SIZE)
		{
			g_tMtrRdSchInf[i].bSearchState = PRO97METORNOT;
			ReinitSearch(i);
		}
	}
}

void SaveSchMtrStaInfo()
{
	WaitSemaphore(g_semExFlashBuf, SYS_TO_INFINITE);

	memset(g_ExFlashBuf, 0x00, EXSECT_SIZE);
	memcpy(g_ExFlashBuf, g_tMtrRdSchInf, sizeof(TMtrSchInf)*MTR_PORT_NUM);
	MakeFile(FILE_SCHMTR_STATUS, g_ExFlashBuf);
	writefile(FILE_BATTASK_STATUS, 0, g_ExFlashBuf);

	SignalSemaphore(g_semExFlashBuf);
}

void InitSearch(BYTE bPort, BYTE bStartSer)
{    
	if (bPort >= MTR_PORT_NUM)
		return;

	memset(g_tMtrRdSchInf[bPort].bAddrPatten, 0xAA, sizeof(g_tMtrRdSchInf[bPort].bAddrPatten));
    
	if (bStartSer)
		g_tMtrRdSchInf[bPort].bSearchState = PRO97METORNOT;
	else
		g_tMtrRdSchInf[bPort].bSearchState = SEARCHWAIT;                  //��ʼ����һ���ѱ�
    
	InitStack(&g_tMtrRdSchInf[bPort].tStack);

	g_tMtrRdSchInf[bPort].bFinish = 0;
	g_tMtrRdSchInf[bPort].bCurTry = 0;
	g_tMtrRdSchInf[bPort].bCurTryLevel = 0;

	//if (bStartSer > 1)
		//GetSchMtrStaInfo();
}

void ReinitSearch(BYTE bPort)
{
	if (bPort >= MTR_PORT_NUM)
		return;

    memset(g_tMtrRdSchInf[bPort].bAddrPatten, 0xAA, sizeof(g_tMtrRdSchInf[bPort].bAddrPatten));
    InitStack(&g_tMtrRdSchInf[bPort].tStack);
    g_tMtrRdSchInf[bPort].bFinish = 0;
    g_tMtrRdSchInf[bPort].bCurTry = 0;
    g_tMtrRdSchInf[bPort].bCurTryLevel = 0;
}

void StartSearch(BYTE bPort)
{
    if (bPort >= MTR_PORT_NUM)
		return;
    
    ReinitSearch(bPort);
    g_tMtrRdSchInf[bPort].bSearchState = PRO97METORNOT;
}

void StopSearch(BYTE bPort)
{
    if (bPort >= MTR_PORT_NUM)
		return;
    
    g_tMtrRdSchInf[bPort].bSearchState = SEARCHWAIT;
}

//��ͨ����е�AA����FF
void TransAAToFF(BYTE bD, BYTE bS, BYTE *pbAddr, BYTE bLen)
{
    BYTE i;
    for (i=0; i<bLen; i++)
    {
        if (pbAddr[i] == bS)
            pbAddr[i] = bD;
    }
}

//����±��ַ�Ƿ���Ч
//����1-��Ч��0-��Ч
BYTE CheckMetAddr(BYTE *pbMAC)
{
    BYTE i;
    for (i=0; i<6; i++)
    {
        if (pbMAC[i] > 0x99)
            return 0;
    }
    return 1;
}

void AddMetToTab(TMtrSchInf* ptMtrSch, BYTE *pbMAC, BYTE bPro, BYTE bBaud)
{
    BYTE bGbPro;
    if ((bPro==CCT_MTRPRO_97) || (bPro==CCT_MTRPRO_07))
    {
        if (!CheckMetAddr(pbMAC))
            return;
    }

	if (bPro == CCT_MTRPRO_97)
		bGbPro = PROTOCOLNO_DLT645;
	if (bPro == CCT_MTRPRO_07)
		bGbPro = PROTOCOLNO_DLT645_V07;

	SaveSearchPnToPointSect(pbMAC, bGbPro, bBaud);

    //for (i=0; i<sizeof(ptMtrSch->tMeterAddrTab)/sizeof(TMeterAddrTab); i++)  
    //{
    //    if (ptMtrSch->tMeterAddrTab[i].bEn)
    //    {
    //        if ((memcmp(ptMtrSch->tMeterAddrTab[i].bAddr, pbMAC, 6) == 0) && (ptMtrSch->tMeterAddrTab[i].bProto == bPro))
    //            return;      //�ñ��ַ�Ѿ�����
    //        continue;
    //    }
    //    memcpy(ptMtrSch->tMeterAddrTab[i].bAddr, pbMAC, 6);
    //    ptMtrSch->tMeterAddrTab[i].bProto = bPro;
    //    ptMtrSch->tMeterAddrTab[i].bBaud = bBaud;
    //    ptMtrSch->tMeterAddrTab[i].bEn = 1;
    //    break;
    //}
}

//68 AA AA AA AA AA AA 68 01 02 43 C3 D5 16 //�Թ㲥��ַ��43 C3
//68 AA AA AA AA AA AA 68 81 06 43 C3 94 A5 35 33 FA 16 //��Щ97�ı�صĻ���ͨ���ַ
//���ֱ����º����Ѳ������ַ
//���� 0-û�б�
//     1- 1����
//     2- �����
BYTE IsMetOrNot(BYTE bPort, BYTE bMetType)
{    
	BYTE i, j;
	BYTE bMAC[6]; 
    BYTE bRxBuf[128];
	BYTE bRxFrm[100], bLen;   
    BYTE bMetNum = 0;
    short sRet = -1;
	int	iLen;
	WORD wPortNum, wPortMin, wPortMax;

	if (bMetType >= sizeof(g_tMeterPro)/sizeof(TMeterPro))
		return 0;   

	GetLogicPortNum(&wPortNum, &wPortMin, &wPortMax);

    /*if (g_tMeterPro[bMetType].bProto == CCT_MTRPRO_NJSL)
        memset(bMAC, 0xff, 6);
	else */
		memset(bMAC, 0xaa, 6);

	for (i=0; i<sizeof(g_wTestID)/sizeof(WORD); i++)  //�������Ե�ID���ID
	{
	    for (j=0; j<2; j++)           //���������α������
	    {         
			//memset(bRxBuf, 0, sizeof(bRxBuf));
			//memset(bRxFrm, 0, sizeof(bRxFrm));
	
			iLen = DoMtrFwdFunc(bPort+wPortMin, g_wTestID[i], g_tMeterPro[bMetType].bProto, bMAC, bRxBuf, sizeof(bRxBuf));
			if (iLen == -1) //�˿ڲ�Ϊ�����
				break;
			else if (iLen <= 0) //��������   
				continue;			
			
			sRet =  Mtr645RcvBlock(bRxBuf, (DWORD)iLen, bRxFrm, &bLen, sizeof(bRxFrm));
	        if (sRet > 0)
	        {       
	            if (sRet > bLen+4) //�����Ϊ֡ͷ���4���ֽ�  //��һ�³��ȣ���ѭ����������֡���滹��û�����ݣ�������
	            {
	                bMetNum |= 2;
	            }
	            else   //���ʱ���ﲻ��ӣ���Ϊ��ỹ����ң�������ظ�
	            {
	                AddMetToTab(&g_tMtrRdSchInf[bPort], &bRxFrm[1], g_tMeterPro[bMetType].bProto, ((bPort+wPortMin)&0x1f)+ (GbBaudrateToVal(g_tMeterPro[bMetType].wBaud)<<5));//todo:ע��һ����Ȼأ�����������ص����
	                bMetNum |= 1;
	            }
	        }
	        else if (sRet < 0)//û����֡             ֡ͷ����
	        {   
	            bMetNum |= 2;
	        }	        	        	
	        //sRet == 0  û�б�      
            
            if (bMetNum >= 2) //��ǰID����Ӧ,��������һ��ID        
	     	    break;		
	    }
	    
	    if (bMetNum >= 2) //��ǰID����Ӧ,��������һ��ID        
	     	break;		
	}

    if (bMetNum >= 2) //����
        return 2;
        
    return bMetNum;
}

//�ѱ�ӵ�ַ���ֽ���߳��ԣ���һ�����Եĵ�ַΪAA AA AA AA AA 00�������������Ӧ�ý�00ѹ��ջ�У�
//Ȼ����AA AA AA AA 00 00��AA AA AA AA 01 00��...
//6���ֽڵĵ�ַ��5��������ֽڽ�0������0����ʼ���ԡ�ÿ���ֽڵĵ�ֵַֻ����0-99.
//ջ�յ�ʱ���Ե�0����ջ���Ϊ1ʱ���Ե�Ϊ1�������ջ����ȿ���֪����ǰӦ�ó�����һ����
//ջ��ȼ�1ʱ����ֵַӦ��������0��ʼֱ��99.����99��ʾ�ü���������Ե���һ��ջ��Ԫ�أ�ջ��Ԫ�صĴ�С��1����
//�ü�Ӧ�ÿ�ʼ���Եĵ�ֵַ����ʼֵ��
//��������
//g_tMtrRdSchInf[bPort].bCurTry����ǰ���Լ��ĵ�ֵַ���仯��Χ��0-99
//g_tMtrRdSchInf[bPort].bCurTryLevel����¼���𡣱仯��Χ0-5�������ж��Ƿ���ѹջ
//g_tMtrRdSchInf[bPort].bAddrPatten����Ҫ���Եı�ַ����ã�����g_tMtrRdSchInf[bPort].bAddrPatten�С�����ջ����2���ֽ�02��01��Ln��ʾջ�Ķ�Ӧλ��û��Ԫ�أ� L7 L6 L5 L4 L3 L2 02 01��
//��ô��ǰ����λΪL2��L2�ǵ�ֵ��0��ʼ��99���ԡ�g_tMtrRdSchInf[bPort].bAddrPatten��ΪֵΪ��AA AA AA g_tMtrRdSchInf[bPort].bCurTry 02 01
//��ȡһ�����Գ����ĵ���ַ�����ص�ǰ������һ�㣨Ҳ���ǵ���ַ�ĵڼ����ֽڣ�����
BYTE GetTryAddr(TMtrSchInf* pMtrSch, BYTE *pbMtrAddr)
{        
    BYTE bNode;
    BYTE bLevel = 0;
    if (IsStackEmpty(&pMtrSch->tStack))
    {
    	memset(&pMtrSch->bAddrPatten[bLevel], 0xaa, 6-bLevel); 
        pMtrSch->bAddrPatten[0] = ByteToBcd(pMtrSch->bCurTry++);
        if (pMtrSch->bCurTry > 100)//////     ���б��ַ�������ꡣ
            pMtrSch->bFinish = 1; 
    }
    else
    {
        GetStackTop(&pMtrSch->tStack, &bNode);     
        bLevel = (BYTE)DepthStack(&pMtrSch->tStack);	
        if (bLevel > pMtrSch->bCurTryLevel)      //ѹ��ջ               //bCurTryLevel
        {       //BYTE SearchMeter()ѹ�����µ��ֽڣ�pMtrSch->bCurTryLevelǰ��һ����������
            if (bLevel >= 6)//˵���õ�ַ��������������ַһģһ��
            {
                //���õ�ַ��ջ��ȡ��������Ҫ�ƶ�ջ           todo:�������ø澯�¼�
                PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry);     //�����ñ�
                bLevel--;
                pMtrSch->bCurTry++;     
                while (pMtrSch->bCurTry > 99)
                {
                    if (!PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry))//����,      ȫ99Ϊ�㲥��ַ��������
                    {
                        pMtrSch->bFinish = 1; 
                        break;
                    }
                    pMtrSch->bCurTry++;        //�ڵ�������ֵ�����ϼ�1������
                    pMtrSch->bCurTryLevel--;
                    bLevel--;
                }                     
            }
            else
            {
                pMtrSch->bCurTry = 0;
                pMtrSch->bAddrPatten[bLevel-1] = ByteToBcd(bNode); //ջָ��Ϊ1��ʱ��ջ��ֻ��һ��Ԫ��
                pMtrSch->bCurTryLevel++;
            }
        }
        else if (bLevel == pMtrSch->bCurTryLevel)    //��ǰ���Լ�����ֽڣ�0~99�ص���
        {
            while (pMtrSch->bCurTry > 99)  //������ֽ��ϣ�û��ͻ�ı�����Ѿ��ѳ����ˣ�����֮ǰ�г�ͻ�Ķ��Ѿ�ѹջ������
            {
                if (!PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry))//����,      ȫ99Ϊ�㲥��ַ��������
                {
                    pMtrSch->bFinish = 1; 
                    break;
                }
                pMtrSch->bCurTry++;        //�ڵ�������ֵ�����ϼ�1������
                pMtrSch->bCurTryLevel--;
                bLevel--;
            }        
        }
        else         //
        {            
        }

        //���룺��ǰ���Լ�����ֽ�(0~99)��������Ĺ㲥�ֽ�
        memset(&pMtrSch->bAddrPatten[bLevel], 0xaa, 6-bLevel);         //��ջʱҪ���˳���λ����0xAA
        pMtrSch->bAddrPatten[bLevel] = ByteToBcd(pMtrSch->bCurTry++);        
    }    
    memcpy(pbMtrAddr, pMtrSch->bAddrPatten, 6);   
    return bLevel;
}

BYTE SearchMeter(BYTE bPort, BYTE bMetType)
{
	BYTE bMtrAddr[6]; 
	BYTE bRxBuf[128];
	BYTE bRxFrm[100], bLen;	
	BYTE bMutiMet = 0;
	BYTE i, j;
	short sRet = -1;
	int	iLen;     

	WORD wPortNum, wPortMin, wPortMax;
	GetLogicPortNum(&wPortNum, &wPortMin, &wPortMax);

	if (bMetType >= sizeof(g_tMeterPro)/sizeof(TMeterPro))
		return SEARCH_UNDOEN;
  
    GetTryAddr(&g_tMtrRdSchInf[bPort], bMtrAddr);            //todotodo:��Э��
    if (g_tMtrRdSchInf[bPort].bFinish)
        return SEARCH_OVER;    
   
    //if (g_tMeterPro[bMetType].bProto == CCT_MTRPRO_NJSL)
        //TransAAToFF(0xFF, 0xAA, bMtrAddr, 6);           
   
	for (i=0; i<1; i++)  //�ظ�����
	{
		for (j=0; j<2; j++)  //�ظ����Σ���ID�뵥ID
		{
			//memset(bRxBuf, 0, sizeof(bRxBuf));
			//memset(bRxFrm, 0, sizeof(bRxFrm));

			iLen = DoMtrFwdFunc(bPort+wPortMin, g_wTestID[j], g_tMeterPro[bMetType].bProto, bMtrAddr, bRxBuf, sizeof(bRxBuf)); 			
			if (iLen == -1) //�˿ڲ�Ϊ�����
				return SEARCH_UNDOEN;//break;
			else if (iLen <= 0) //��������   
				continue;			

			sRet =  Mtr645RcvBlock(bRxBuf, (DWORD)iLen, bRxFrm, &bLen, sizeof(bRxFrm));
			if (sRet > 0)
			{       
				if (sRet > bLen+4) //�����Ϊ֡ͷ���4���ֽ�  //��һ�³��ȣ���ѭ����������֡���滹��û�����ݣ�������
				{			
					if (bMutiMet == 0)   //ͬһ���ַ�ظ����β���ÿ�ζ�ѹջ����ֻ��ѹһ�Ρ�
					{
						if (!PushStack(&g_tMtrRdSchInf[bPort].tStack, g_tMtrRdSchInf[bPort].bCurTry-1))          //���г�ͻ��ѹ��ջ��
							return SEARCH_ERROR;
						bMutiMet = 1;
					}
				}			
				AddMetToTab(&g_tMtrRdSchInf[bPort], &bRxFrm[1], g_tMeterPro[bMetType].bProto, ((bPort+wPortMin)&0x1f) + (GbBaudrateToVal(g_tMeterPro[bMetType].wBaud)<<5));//todo:ע��һ����Ȼأ�����������ص����
				if (GetMtrNum() >= 192)      //�Ѿ��ҵ�192�������Ѱ�ҡ�
					return SEARCH_OVER;
			}      
			else if (sRet < 0)//û����֡             ֡ͷ����
			{            
				if (bMutiMet == 0)
				{
					if (!PushStack(&g_tMtrRdSchInf[bPort].tStack, g_tMtrRdSchInf[bPort].bCurTry-1))          //���г�ͻ��ѹ��ջ��
						return SEARCH_ERROR;
					bMutiMet = 1;
				}
			} 
			//sRet == 0  û�б�
            
            if (bMutiMet != 0) //��ǰID����Ӧ,��������һ��ID, �Ѿ�ѹջ��,�����Ƕ����        
			    break;		
		}

		if (bMutiMet != 0) //��ǰID����Ӧ,��������һ��ID        
			break;		
	}
    return SEARCH_UNDOEN;
}

void DoSearch(BYTE bPort)
{
    BYTE bSerState;   

	if (bPort >= MTR_PORT_NUM)
		return;
   
    switch(g_tMtrRdSchInf[bPort].bSearchState)
    {
    case PRO97METORNOT:        
        if (IsMetOrNot(bPort, 0) >= 2)//�Ƿ��ж��97�ı�
            g_tMtrRdSchInf[bPort].bSearchState = PRO97;
        else
            g_tMtrRdSchInf[bPort].bSearchState = PRO07METORNOT;
		SaveSchMtrStaInfo();
        break;
    case PRO97:              //97Э���ѱ�
        bSerState = SearchMeter(bPort, 0);
        if (bSerState == SEARCH_OVER) 
        {
            if (g_tMtrRdSchInf[bPort].bFinish)//97����
            {
                ReinitSearch(bPort);
                g_tMtrRdSchInf[bPort].bSearchState = PRO07METORNOT;
            }
            else //����32���
                g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
			SaveSchMtrStaInfo();
        }        
        break;
    case PRO07METORNOT:        
        if (IsMetOrNot(bPort, 1) >= 2)//�Ƿ��ж��07�ı�
            g_tMtrRdSchInf[bPort].bSearchState = PRO07;
        else 
            g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
		SaveSchMtrStaInfo();
        break;
    case PRO07:              //07Э���ѱ� 
        bSerState = SearchMeter(bPort, 1);
        if (bSerState == SEARCH_OVER) 
        {
            if (g_tMtrRdSchInf[bPort].bFinish)//07����
            {
                ReinitSearch(bPort);
                g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
            }
			SaveSchMtrStaInfo();
        }
        break;
    /*case PRONJSLMETORNOT:        
        if (IsMetOrNot(bPort, 2) >= 2)//�Ƿ��ж���Ͼ����ֵı�
            g_tMtrRdSchInf[bPort].bSearchState = PRONJSL;
        else
            g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
        break;
    case PRONJSL: 
        bSerState = SearchMeter(bPort, 2);
        if (bSerState == SEARCH_OVER) 
        {
            if (g_tMtrRdSchInf[bPort].bFinish)//�Ͼ������ѱ����
            {
                ReinitSearch(bPort);
                g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
            }
        }
        break;*/
    case SEARCHOVER:        
        g_tMtrRdSchInf[bPort].bSearchState = SEARCHWAIT;
		SaveSchMtrStaInfo();
        break;
    case SEARCHWAIT:                //�ѱ����       //�ɳ�ʼ������һ�������ѱ�        
        g_tMtrRdSchInf[bPort].bSearchState = SEARCHWAIT;
        break;
    default:
        g_tMtrRdSchInf[bPort].bSearchState = SEARCHWAIT;
        break;
    }    
}
