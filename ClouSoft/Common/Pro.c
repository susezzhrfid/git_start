/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Pro.c
 * ժ    Ҫ�����ļ�ʵ����ͨ��Э�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
 * ��    ע��
 *********************************************************************************************************/
#include "Pro.h"
#include "FaCfg.h"
#include "FaAPI.h"
#include "SysDebug.h"

#define PRO_FRM_SIZE	128		//64

int ProRcvBlock(struct TPro* pPro, BYTE* pbBlock, int nLen)
{
	return 0;
}

bool ProHandleFrm(struct TPro* pPro)
{
	return true;
}

bool ProLogin(struct TPro* pPro)
{
	return true;
}

bool ProLogoff(struct TPro* pPro)
{
	return true;
}

bool ProBeat(struct TPro* pPro)
{
	return true;
}

bool ProAutoSend(struct TPro* pPro)
{
	return true;
}

bool ProIsNeedAutoSend(struct TPro* pPro)
{
	return false;
}

void ProLoadUnrstPara(struct TPro* pPro)	//װ�طǸ�λ����
{
}

void ProDoProRelated(struct TPro* pPro)	//��һЩЭ����صķǱ�׼������
{
}

bool ProSend(struct TPro* pPro, BYTE* pbTxBuf, WORD wLen)
{
	struct TProIf* pIf = pPro->pIf;		//ͨ�Žӿ�
#ifndef SYS_WIN
    TraceBuf(DB_FAFRM, "tx<---", pbTxBuf, wLen);
#endif
        
    DTRACE(DB_FAPROTO, ("ProSend: tx from %s, len=%d, click=%d\n", pIf->pszName, wLen, GetClick())); 
        
	return pIf->pfnSend(pIf, pbTxBuf, wLen);
}

int ProReceive(struct TPro* pPro, BYTE* pbRxBuf, WORD wBufSize)
{
	struct TProIf* pIf = pPro->pIf;		//ͨ�Žӿ�
	return pIf->pfnReceive(pIf, pbRxBuf, wBufSize);
}
	
//�������������ղ�����֡
int ProRcvFrm(struct TPro* pPro)
{
	struct TProIf* pIf = pPro->pIf;		//ͨ�Žӿ�
	int iRet = 0;
	int len = 0;
    WORD wOffset;
	BYTE bBuf[PRO_FRM_SIZE];
    BYTE i=0;
	BYTE bMode = 0;
    int nScanLen = 0;
	ReadItemEx(BN2, PN0, 0x2040, &bMode);  //todo

    for (i=0; i<128; i++)
    {
        len = pPro->pfnReceive(pPro, bBuf, PRO_FRM_SIZE-1);
        if (len > 0)
        {
            DTRACE(DB_FAPROTO, ("RcvFrm: rx from %s, len=%d\n", pIf->pszName, len)); 
#ifndef SYS_WIN
            TraceBuf(DB_FAFRM, "rx--->", bBuf, len);
#endif
            //����������֡����������֡��һ��645֡��
			if (bMode != 0)
				DoTest(pPro, bBuf, len);
        }
    
        if (len <= 0)
        {
            #ifdef SYS_WIN
                Sleep(300); //�������,��VC socket��ʽ�²�˯�߻ᵼ��CPU�����ʸߴ�99%,ԭ�����
            #endif //SYS_WIN
            if (len < 0)
                return -1; //����
            else
                return 0;  //û������
        }
        
        wOffset=0;
        while (len > 0)
        {
            nScanLen = pPro->pfnRcvBlock(pPro, bBuf+wOffset, len);	//��֡
            if (nScanLen > 0)   //�ɹ����һ֡
            {
                pPro->pfnHandleFrm(pPro);   //֡����
                
                len = len - nScanLen;
                wOffset += nScanLen;
                pIf->pfnOnRcvFrm(pIf);	//��ͨ��Э���յ���ȷ֡ʱ����,��Ҫ������·״̬,����������
                iRet = 1;                
            }
            else if (nScanLen < 0)   //��ȫ�ı���
            {
                break;
            }
        }
        if (iRet > 0)
            return iRet;        
    }
	
	return 0; //û����ȷ��֡
}

//�������ⲿ����������ݲ�����֡
bool ProRxFrm(struct TPro* pPro, BYTE* pbRxBuf, int iLen)
{
	struct TProIf* pIf = pPro->pIf;		//ͨ�Žӿ�
	bool fRet = false;
	while (iLen > 0)
	{
		int nScanLen = pPro->pfnRcvBlock(pPro, pbRxBuf, iLen);	//��֡
		if (nScanLen > 0)   //�ɹ����һ֡
		{
			pPro->pfnHandleFrm(pPro);   //֡����

			iLen = iLen - nScanLen;
			pIf->pfnOnRcvFrm(pIf);	//��ͨ��Э���յ���ȷ֡ʱ����,��Ҫ������·״̬,����������
			fRet = true;
		}
		else if (nScanLen < 0)   //��ȫ�ı���
		{
			break;
		}
	}

	return fRet;
}

void ProInit(struct TPro* pPro)
{
	//�麯������Ҫʵ����Ϊ����Э��Ķ�Ӧ����
	pPro->pfnRcvFrm = ProRcvFrm;		//����֡
	pPro->pfnRxFrm = ProRxFrm;			//�ⲿ����������ݲ�����֡
	pPro->pfnRcvBlock = ProRcvBlock;	//��֡
	pPro->pfnHandleFrm = ProHandleFrm;	//����֡

	pPro->pfnLogin = ProLogin;			//��½
	pPro->pfnLogoff = ProLogoff;			//��½�˳�
	pPro->pfnBeat = ProBeat;			//����
	pPro->pfnAutoSend = ProAutoSend;		//��������
	pPro->pfnIsNeedAutoSend = ProIsNeedAutoSend;	//�Ƿ���Ҫ��������
	pPro->pfnLoadUnrstPara = ProLoadUnrstPara;	//װ�طǸ�λ����
	pPro->pfnDoProRelated = ProDoProRelated;	//��һЩЭ����صķǱ�׼������
	pPro->pfnSend = ProSend;	//��pIf->pIfpfnSend()�Ķ��η�װ�������س����⺯��
	pPro->pfnReceive = ProReceive; //��pIf->pfnReceive()�Ķ��η�װ�������س����⺯��
}
