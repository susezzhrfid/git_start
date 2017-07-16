#include "DoTest.h"
#include "ComAPI.h"
#include "FlashIf.h"
#include "SysCfg.h"

//长度是指数据内容的长度，不包括ID
BYTE MakeAnsFrmEx(BYTE *pbBuf, BYTE bBufSize, BYTE bDataLen)
{
    BYTE *p = pbBuf;
    *p++ = 0x68;
    memset(p, 0xaa, 6);
    p += 6;
    *p++ = 0x68;
    *p++ = 0x84;
    //*p++ = bDataLen+2;
    *p++ = bDataLen;
    //p += 2;    //ID
    p += bDataLen;    
    *p++ = CheckSum(pbBuf, pbBuf[9] + 10);
    *p++ = 0x16;
    
    return p-pbBuf;
}

void DoTest(struct TPro* pPro, BYTE *pbBuf, WORD wLen)
{    
    BYTE bBuf[128];
    short sRet;
    BYTE bLen;
    DWORD dwTestFlag;
    sRet = Mtr645RcvBlock(pbBuf, wLen, bBuf, &bLen, sizeof(bBuf));
    if (sRet > 0)
    {
        if ((bBuf[8]==0x04) && (bBuf[9]==0x10) && (bBuf[10]==0xf8) && (bBuf[11]==0xff))//启动测试命令帧
        {            
            memset(&bBuf[10], 0, 4);
            MakeAnsFrmEx(bBuf, sizeof(bBuf), 0);
		#ifndef SYS_WIN
			//写一个测试标志
            dwTestFlag = TEST_FLAG;
            Program((BYTE *)&dwTestFlag, BL_USE_LEN+8, 4);
            pPro->pfnSend(pPro, bBuf, bLen);
            
            //重启进入BL进行自检
            SetInfo(INFO_APP_RST);
		#endif
        }
    }
}
