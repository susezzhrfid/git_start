/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MeterPro.h
 * ժ    Ҫ�����ļ���Ҫ��������Э��Ļ���API������ȫ�ֱ����Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�������
 * ������ڣ�2011��3��
 * ��    ע��
 *********************************************************************************************************/

#include "MtrProAPI.h"
#include "FaConst.h"


struct TMtrPro g_MtrPro[DYN_PN_NUM];

//����:ͨ���������ȡ����Ӧ������Ĳ����ͱ������
//����:	@wPn:�������
//		@pMtrPara:���ڷ��ظõ�������ָ��
bool LoadMtrPara(WORD wPn, TMtrPara* pMtrPara)
{
	if (pMtrPara == NULL)
		return false;

	///////////////////////
	return true;
}

//����:ͨ���������д���Ӧ�����㱣�����
//����:	@wPn:�������
//		@pSaveInf:���ڴ���õ���������ָ��
//		@pbUnsupIdFlg:���ڴ���õ�����ID�Ƿ�֧�ֱ�־�Ļ����ָ��
bool SaveMtrInfo(WORD wPn, TMtrSaveInf* pSaveInf)
{
	if (pSaveInf == NULL)
		return false;

	///////////////////////
	return true;
}

//����:���������Э��������
//����:	@wPn:�������
//		@pMtrPara:���������������ָ��
//		@pSaveInf:�����������������ָ��
//����ֵ:�ɹ��򷵻ؾ�����Э��ṹ��ָ��,ʧ���򷵻ؿ�ָ��
struct TMtrPro*  CreateMtrPro(WORD wPn, TMtrPara* pMtrPara, TMtrSaveInf* pSaveInf, bool fInit, BYTE bThrId)
{
	//�ж�Э������ĺ�����
	//if (pMtrPara==NULL || pSaveInf==NULL || pbUnsupIdFlg==NULL)
		//return NULL;

	//if ( !LoadMtrInfo(wPn, pMtrPara, pSaveInf) )
//		return NULL;

	g_MtrPro[bThrId].pMtrPara = pMtrPara;	

//	if ( fInit ) //���״γ�ʼ��ֱ�ӷ���
//		ResetMtrUnsupIdFlg(pSaveInf->pbUnsupIdFlg, wPn);
	
	switch (pMtrPara->bProId)
	{
	case PROTOCOLNO_DLT645:			
		g_MtrPro[bThrId].pvMtrPro = &pSaveInf->tMtrPriv.t645Priv;			
		if ( Mtr645Init(&g_MtrPro[bThrId], fInit, bThrId) )
			return &g_MtrPro[bThrId];
		break;
	case PROTOCOLNO_DLT645_V07:
		g_MtrPro[bThrId].pvMtrPro = &pSaveInf->tMtrPriv.tV07Priv;	
		if ( Mtr645V07Init(&g_MtrPro[bThrId], fInit, bThrId) )
			return &g_MtrPro[bThrId];	
		break;		
	default:
		break;
	}

	return NULL;
}


//����:���������Э�����ݽӿ�
//����:	@pMtrPro: �������������Э��ṹ��ָ��
//		@wPn:�������
//		@wID:����ID
//		@pbBuf:���ڷ��س������ݵ�ָ��

int AskMtrItem(struct TMtrPro* pMtrPro, WORD wPn, WORD wID, BYTE* pbBuf)
{
	const WORD wRateBlkId[] ={0xc31f, 0xc32f, 0xc33f}; //Ŀǰֻ��3������ID	
	const WORD wRateBlkLen[] = {5, 42, 42};
	BYTE bBuf[50], bNum, i;	
	BYTE bPrintPro = pMtrPro->pMtrPara->bProId;
	char szProName[20];
	int iRv, iRet=0;
	BYTE* p = pbBuf;


	pMtrPro->pfnGetProPrintType(&bPrintPro, szProName);	
	DTRACE(bPrintPro, ("MtrPro=%s::Point=%d,read id=0x%x.\r\n", szProName,pMtrPro->pMtrPara->wPn, wID)); 

	//���ʱ�η���,��ָ����ȡ
	if (wID == 0xc60f)
	{
		bNum = sizeof(wRateBlkId)/sizeof(WORD);
		memset(pbBuf, m_bInvdData, (bNum-1)*42+5);
        //memset(pbBuf, 0, (bNum-1)*42+5);

		for (i=0; i<bNum; i++)
		{	
			//iRv = pMtrPro->pfnAskItem(pMtrPro, wRateBlkId[i], bBuf);
            if (i == 2)//̨�����ֻ��0xc33f
				iRv = pMtrPro->pfnAskItem(pMtrPro, wRateBlkId[i], bBuf);
			else
				iRv = -1;

			if (iRv > 0)
			{
				memcpy(p, bBuf, iRv);
			    p += iRv;
			}
			else if (i < 2)
			{//��֧�ֵķ���ID��������Ϊ0
				iRv = wRateBlkLen[i];
				memset(bBuf, 0, iRv);
				memcpy(p, bBuf, iRv);
				p += iRv;
			}
			//else Ϊ����̨��ģ���07Э���c31f��c32f��֧�ֵ�����ʱ��֧��,��ͨ���쳣Ҳ���˳�������
		}	
		iRet = (bNum-1)*42+5;
	}
	else
		iRet = pMtrPro->pfnAskItem(pMtrPro, wID, pbBuf);

	return  iRet;
}

