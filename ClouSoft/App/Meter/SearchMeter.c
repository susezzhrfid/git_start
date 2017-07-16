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


//描述：从串口缓冲区中找满足645的报文
//参数：@pbBlock - 接受的缓存
//		@dwLen - 接收的长度
//		@pbRxBuf - 存放完整数据帧的缓存
//		@pbRxLen - 完整数据帧报文长度
//		@dwBufSize - 接收缓冲区的长度
//返回：0-无数据，正数-接收到的数据帧长度，负数-无效数据长度
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
				sFrmHead = i;//这之前的数据都是无效的
			}
			break;
		case 1:    //数据域前的数据
			pbRxBuf[bRxPtr++] = b;
			bRxCnt --;
			if (bRxCnt == 0)   //接收完，进行校验
			{
				if (pbRxBuf[7] == 0x68) // && (p[FAPDL645_CMD]&FAPDL645_CMD_DIR)==FAPDL645_CMD_DOWN //防止接收到红外返回的自己发出去的帧
				{
					bRxCnt = pbRxBuf[9] + 2;  //0xfe+2
					sFrmHead++;
					if (bRxCnt+10>dwBufSize || pbRxBuf[9]>=dwBufSize)   //剪帧的缓存区不够
					{
						bRxStep = 0;					
						break;
						//return -sFrmHead;                 //这里返回0，缓存区将永远无法释放
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
		case 2:     //数据 + 检验码 + 结束码
			pbRxBuf[bRxPtr++] = b;
			//DTRACE(DB_DL645V07, ("CDL645V07::RcvBlock : m_wRxCnt=%d.m_wRxPtr=%d,m_bRxBuf=%x\r\n", m_wRxCnt, m_wRxPtr, m_bRxBuf[m_wRxPtr-1])); 	
			bRxCnt -- ;
			if (bRxCnt == 0)   //接收完，进行校验
			{
				bRxStep = 0;

				if (pbRxBuf[bRxPtr-1]==0x16 && pbRxBuf[bRxPtr-2]==CheckSum(pbRxBuf, pbRxBuf[9]+10))
				{
					*pbRxLen = pbRxBuf[9] + 12;
					//return i+1;//接收到完整的一帧		
					return dwLen;//接收到完整的一帧	返回全长 以表示本轮收码全部处理完毕
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
    ptStack->iTop = 0;                   //先压栈后移动，先移动后出栈。
}

static BYTE IsStackEmpty(TStack *ptStack)
{
    if (ptStack->iTop == 0)
        return 1;             //空
    return 0;
}

static BYTE PushStack(TStack *ptStack, BYTE bData)
{
    if ((ptStack->iTop+1) == STACK_SIZE)          //栈满， 不能入栈
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
    if (IsStackEmpty(ptStack))           //空栈
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

//取得栈顶元素但不是弹出
static BYTE GetStackTop(TStack *ptStack, BYTE *pbData)
{
    if (IsStackEmpty(ptStack))           //空栈
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
	//防错
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
		g_tMtrRdSchInf[bPort].bSearchState = SEARCHWAIT;                  //初始化起动一次搜表
    
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

//将通配符中的AA换成FF
void TransAAToFF(BYTE bD, BYTE bS, BYTE *pbAddr, BYTE bLen)
{
    BYTE i;
    for (i=0; i<bLen; i++)
    {
        if (pbAddr[i] == bS)
            pbAddr[i] = bD;
    }
}

//检查下表地址是否有效
//返回1-有效，0-无效
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
    //            return;      //该表地址已经存在
    //        continue;
    //    }
    //    memcpy(ptMtrSch->tMeterAddrTab[i].bAddr, pbMAC, 6);
    //    ptMtrSch->tMeterAddrTab[i].bProto = bPro;
    //    ptMtrSch->tMeterAddrTab[i].bBaud = bBaud;
    //    ptMtrSch->tMeterAddrTab[i].bEn = 1;
    //    break;
    //}
}

//68 AA AA AA AA AA AA 68 01 02 43 C3 D5 16 //以广播地址读43 C3
//68 AA AA AA AA AA AA 68 81 06 43 C3 94 A5 35 33 FA 16 //有些97的表回的还是通配地址
//这种表以下函数搜不出表地址
//返回 0-没有表，
//     1- 1块电表，
//     2- 多块电表，
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

	for (i=0; i<sizeof(g_wTestID)/sizeof(WORD); i++)  //连续尝试单ID与块ID
	{
	    for (j=0; j<2; j++)           //连续发二次避免出错
	    {         
			//memset(bRxBuf, 0, sizeof(bRxBuf));
			//memset(bRxFrm, 0, sizeof(bRxFrm));
	
			iLen = DoMtrFwdFunc(bPort+wPortMin, g_wTestID[i], g_tMeterPro[bMetType].bProto, bMAC, bRxBuf, sizeof(bRxBuf));
			if (iLen == -1) //端口不为抄表口
				break;
			else if (iLen <= 0) //其他错误   
				continue;			
			
			sRet =  Mtr645RcvBlock(bRxBuf, (DWORD)iLen, bRxFrm, &bLen, sizeof(bRxFrm));
	        if (sRet > 0)
	        {       
	            if (sRet > bLen+4) //多表，因为帧头最多4个字节  //查一下长度，看循环缓存区的帧后面还有没有数据，有则多表
	            {
	                bMetNum |= 2;
	            }
	            else   //多表时这里不添加，因为后会还会查找，否则会重复
	            {
	                AddMetToTab(&g_tMtrRdSchInf[bPort], &bRxFrm[1], g_tMeterPro[bMetType].bProto, ((bPort+wPortMin)&0x1f)+ (GbBaudrateToVal(g_tMeterPro[bMetType].wBaud)<<5));//todo:注意一块表先回，其它几块表后回的情况
	                bMetNum |= 1;
	            }
	        }
	        else if (sRet < 0)//没剪到帧             帧头处理
	        {   
	            bMetNum |= 2;
	        }	        	        	
	        //sRet == 0  没有表      
            
            if (bMetNum >= 2) //当前ID有响应,不用试下一个ID        
	     	    break;		
	    }
	    
	    if (bMetNum >= 2) //当前ID有响应,不用试下一个ID        
	     	break;		
	}

    if (bMetNum >= 2) //多块表
        return 2;
        
    return bMetNum;
}

//搜表从地址低字节向高尝试，第一个尝试的地址为AA AA AA AA AA 00，如果碰到多块表应该将00压入栈中，
//然后尝试AA AA AA AA 00 00，AA AA AA AA 01 00，...
//6个字节的地址叫5级，最低字节叫0级，从0级开始尝试。每个字节的地址值只能是0-99.
//栈空的时候尝试的0级，栈深度为1时尝试的为1级，因此栈的深度可以知道当前应该尝试哪一级。
//栈深度加1时，地址值应该重新由0开始直到99.到了99表示该级尝试完可以弹出一个栈顶元素，栈顶元素的大小加1就是
//该级应该开始尝试的地址值的起始值。
//变量解释
//g_tMtrRdSchInf[bPort].bCurTry：当前尝试级的地址值，变化范围是0-99
//g_tMtrRdSchInf[bPort].bCurTryLevel：记录级别。变化范围0-5，用于判断是否有压栈
//g_tMtrRdSchInf[bPort].bAddrPatten：将要尝试的表址计算好，放入g_tMtrRdSchInf[bPort].bAddrPatten中。比如栈里有2个字节02，01。Ln表示栈的对应位但没有元素， L7 L6 L5 L4 L3 L2 02 01，
//那么当前尝试位为L2，L2是的值从0开始到99来试。g_tMtrRdSchInf[bPort].bAddrPatten中为值为：AA AA AA g_tMtrRdSchInf[bPort].bCurTry 02 01
//获取一个尝试抄读的电表地址，返回当前是在哪一层（也就是电表地址的第几个字节）尝试
BYTE GetTryAddr(TMtrSchInf* pMtrSch, BYTE *pbMtrAddr)
{        
    BYTE bNode;
    BYTE bLevel = 0;
    if (IsStackEmpty(&pMtrSch->tStack))
    {
    	memset(&pMtrSch->bAddrPatten[bLevel], 0xaa, 6-bLevel); 
        pMtrSch->bAddrPatten[0] = ByteToBcd(pMtrSch->bCurTry++);
        if (pMtrSch->bCurTry > 100)//////     所有表地址都已找完。
            pMtrSch->bFinish = 1; 
    }
    else
    {
        GetStackTop(&pMtrSch->tStack, &bNode);     
        bLevel = (BYTE)DepthStack(&pMtrSch->tStack);	
        if (bLevel > pMtrSch->bCurTryLevel)      //压过栈               //bCurTryLevel
        {       //BYTE SearchMeter()压入了新的字节，pMtrSch->bCurTryLevel前进一级进行搜索
            if (bLevel >= 6)//说明该地址，至少有两块表地址一模一样
            {
                //将该地址从栈中取出，但不要移动栈           todo:可以设置告警事件
                PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry);     //跳过该表。
                bLevel--;
                pMtrSch->bCurTry++;     
                while (pMtrSch->bCurTry > 99)
                {
                    if (!PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry))//棧空,      全99为广播地址，电表不会回
                    {
                        pMtrSch->bFinish = 1; 
                        break;
                    }
                    pMtrSch->bCurTry++;        //在弹出来的值基础上加1继续试
                    pMtrSch->bCurTryLevel--;
                    bLevel--;
                }                     
            }
            else
            {
                pMtrSch->bCurTry = 0;
                pMtrSch->bAddrPatten[bLevel-1] = ByteToBcd(bNode); //栈指针为1的时候，栈中只有一个元素
                pMtrSch->bCurTryLevel++;
            }
        }
        else if (bLevel == pMtrSch->bCurTryLevel)    //当前尝试级别的字节，0~99地递增
        {
            while (pMtrSch->bCurTry > 99)  //在这个字节上，没冲突的表可能已经搜出来了，后者之前有冲突的都已经压栈且搜完
            {
                if (!PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry))//棧空,      全99为广播地址，电表不会回
                {
                    pMtrSch->bFinish = 1; 
                    break;
                }
                pMtrSch->bCurTry++;        //在弹出来的值基础上加1继续试
                pMtrSch->bCurTryLevel--;
                bLevel--;
            }        
        }
        else         //
        {            
        }

        //填入：当前尝试级别的字节(0~99)、及后面的广播字节
        memset(&pMtrSch->bAddrPatten[bLevel], 0xaa, 6-bLevel);         //退栈时要将退出的位补上0xAA
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
  
    GetTryAddr(&g_tMtrRdSchInf[bPort], bMtrAddr);            //todotodo:分协议
    if (g_tMtrRdSchInf[bPort].bFinish)
        return SEARCH_OVER;    
   
    //if (g_tMeterPro[bMetType].bProto == CCT_MTRPRO_NJSL)
        //TransAAToFF(0xFF, 0xAA, bMtrAddr, 6);           
   
	for (i=0; i<1; i++)  //重复二次
	{
		for (j=0; j<2; j++)  //重复二次，块ID与单ID
		{
			//memset(bRxBuf, 0, sizeof(bRxBuf));
			//memset(bRxFrm, 0, sizeof(bRxFrm));

			iLen = DoMtrFwdFunc(bPort+wPortMin, g_wTestID[j], g_tMeterPro[bMetType].bProto, bMtrAddr, bRxBuf, sizeof(bRxBuf)); 			
			if (iLen == -1) //端口不为抄表口
				return SEARCH_UNDOEN;//break;
			else if (iLen <= 0) //其他错误   
				continue;			

			sRet =  Mtr645RcvBlock(bRxBuf, (DWORD)iLen, bRxFrm, &bLen, sizeof(bRxFrm));
			if (sRet > 0)
			{       
				if (sRet > bLen+4) //多表，因为帧头最多4个字节  //查一下长度，看循环缓存区的帧后面还有没有数据，有则多表
				{			
					if (bMutiMet == 0)   //同一表地址重复三次不能每次都压栈，而只能压一次。
					{
						if (!PushStack(&g_tMtrRdSchInf[bPort].tStack, g_tMtrRdSchInf[bPort].bCurTry-1))          //将有冲突的压入栈中
							return SEARCH_ERROR;
						bMutiMet = 1;
					}
				}			
				AddMetToTab(&g_tMtrRdSchInf[bPort], &bRxFrm[1], g_tMeterPro[bMetType].bProto, ((bPort+wPortMin)&0x1f) + (GbBaudrateToVal(g_tMeterPro[bMetType].wBaud)<<5));//todo:注意一块表先回，其它几块表后回的情况
				if (GetMtrNum() >= 192)      //已经找到192块表。则不再寻找。
					return SEARCH_OVER;
			}      
			else if (sRet < 0)//没剪到帧             帧头处理
			{            
				if (bMutiMet == 0)
				{
					if (!PushStack(&g_tMtrRdSchInf[bPort].tStack, g_tMtrRdSchInf[bPort].bCurTry-1))          //将有冲突的压入栈中
						return SEARCH_ERROR;
					bMutiMet = 1;
				}
			} 
			//sRet == 0  没有表
            
            if (bMutiMet != 0) //当前ID有响应,不用试下一个ID, 已经压栈了,再试是多余的        
			    break;		
		}

		if (bMutiMet != 0) //当前ID有响应,不用试下一个ID        
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
        if (IsMetOrNot(bPort, 0) >= 2)//是否有多块97的表
            g_tMtrRdSchInf[bPort].bSearchState = PRO97;
        else
            g_tMtrRdSchInf[bPort].bSearchState = PRO07METORNOT;
		SaveSchMtrStaInfo();
        break;
    case PRO97:              //97协议搜表
        bSerState = SearchMeter(bPort, 0);
        if (bSerState == SEARCH_OVER) 
        {
            if (g_tMtrRdSchInf[bPort].bFinish)//97搜完
            {
                ReinitSearch(bPort);
                g_tMtrRdSchInf[bPort].bSearchState = PRO07METORNOT;
            }
            else //超过32块表
                g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
			SaveSchMtrStaInfo();
        }        
        break;
    case PRO07METORNOT:        
        if (IsMetOrNot(bPort, 1) >= 2)//是否有多块07的表
            g_tMtrRdSchInf[bPort].bSearchState = PRO07;
        else 
            g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
		SaveSchMtrStaInfo();
        break;
    case PRO07:              //07协议搜表 
        bSerState = SearchMeter(bPort, 1);
        if (bSerState == SEARCH_OVER) 
        {
            if (g_tMtrRdSchInf[bPort].bFinish)//07搜完
            {
                ReinitSearch(bPort);
                g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
            }
			SaveSchMtrStaInfo();
        }
        break;
    /*case PRONJSLMETORNOT:        
        if (IsMetOrNot(bPort, 2) >= 2)//是否有多块南京松林的表
            g_tMtrRdSchInf[bPort].bSearchState = PRONJSL;
        else
            g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
        break;
    case PRONJSL: 
        bSerState = SearchMeter(bPort, 2);
        if (bSerState == SEARCH_OVER) 
        {
            if (g_tMtrRdSchInf[bPort].bFinish)//南京松林搜表完成
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
    case SEARCHWAIT:                //搜表结束       //由初始化来起动一次重新搜表。        
        g_tMtrRdSchInf[bPort].bSearchState = SEARCHWAIT;
        break;
    default:
        g_tMtrRdSchInf[bPort].bSearchState = SEARCHWAIT;
        break;
    }    
}
