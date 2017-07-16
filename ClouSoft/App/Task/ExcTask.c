 /*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ExcTask.cpp
 * ժ    Ҫ���澯�¼��Ļ��ࣻ�澯�¼��洢�Ľӿں������澯�¼�����ʾ�Ľӿں���
 *
 * ��    ��: 1.0 1
 * ��    �ߣ�����
 * ������ڣ�2008-04-25
 *
 * ȡ���汾��1.0 0
 * ԭ �� ��:
 * ������ڣ�
 * ��    ע: �澯�¼���Ϊ���ࣺ###����Ҫ�ύ����ID,���罻�ɣ�ֱ��ģ������г���������¼�����CUnMtrExcTask��ʵ�֣�
 *						       ###��Ҫ�ύ����ID���¼�������CMtrExcTask��ʵ�֣�
 *
 * ����CMtrExcTask�У���Ϊ���ࣺ###Խ���¼���ͨ���Ƚϵ�ǰ���ڵĵ��������趨����ֵ���Ӷ�ȷ�����쳣�¼���
 *							    ###����һ������ͨ���Ƚ���һ���ںʹ����ڵĵ�����������CUnExdExcTask��ʵ�֡�
************************************************************************************************************/
#include "ExcTask.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "sysarch.h"
#include "DbAPI.h"
#include "ComAPI.h"
#include "FlashMgr.h"
#include "SysDebug.h"

#ifdef PRO_698
static const BYTE bAlrLen[] = {9, 1, 0, 2, 5, 9, 11, 2, 22, 22, //1~10
								18, 1, 2, 5, 43, 4, 21, 0, 26, 17,//11~20
								1, 0, 11, 8, 11, 8, 11, 12, 12, 7,//21~30
								16, 8, 29, 2, 0, 0, 64, 64, 16, 10,//31~40
								14, 0, 0, 0, 0, 0, 0, 0, 0, 0,//41~50
								0, 0, 0, 0, 0, 0, 0, 0, 0, 4};//51~60
#else
static const BYTE bAlrLen[] = {9, 0, 0, 2, 5, 9, 11, 2, 19, 19, //1~10
                   						18, 1, 2, 5, 43, 4, 18, 0, 26, 17,//11~20
								1, 0, 0, 8, 8, 8, 11, 12, 12, 7};//21~30
#endif

#define ALR_LEN_NUM	(sizeof(bAlrLen)/sizeof(BYTE))

/*
//���������ش������¼������Լ�Ͷ���Ƿ�䶯��
//���أ����쳣�������Ч�����Ч,�򷵻�true;
//		���쳣�����ޱ䶯������Ҫ�¼���һ���¼����ɣ��򷵻�false;
bool IsAlrAttriChg(BYTE bERC, BYTE& bAlrType)
{
	bAlrType = GetErcType(bERC);
	if (m_bAlrType[bERC] != bAlrType)
	{
		if (m_bAlrType[bERC]>0 && bAlrType>0) //����Ҫ�¼���һ���¼����ɣ���Ϊ�ޱ仯��
		{
			m_bAlrType[bERC] = bAlrType;
			return false;
		}
		
		m_bAlrType[bERC] = bAlrType;	//����Ч�����Ч�����Ч�����Ч
		return true;	
	}
	else	//��Ч��û�����仯
	{
		return false;
	}
}
*/
/*
//���������ش����ڲ���������Լ������Ƿ��б䶯
bool IsPnPropChg(BYTE bPn, BYTE& rbPnProp)
{
	//����:0��ʾ��Ч��1��ʾ���ɣ�2��ʾ���3��ʾ���壬4��ʾģ����
	rbPnProp = GetPnProp(bPn);

	//DTRACE(DB_METER_EXC, ("IsPnPropChg::bPn=%d oldbPnProp =%d newbPnProp=%d.\r\n", bPn, m_bPnProp[bPn], rbPnProp));

	if (m_bPnProp[bPn] != rbPnProp)		
		return true;	
	else	
		return false;	
}
*/

//��ʼ���澯����
//���ն��������ô˺���
bool InitExcTask()
{
	return true;
}

/*
//ִ�в�����Pn�����и澯����
bool DoExcTask(BYTE bPn)
{
	for (BYTE bTask=0; bTask<EXC_TASK_NUM; bTask++)
	{
		if ((bTask>=1 && bTask <=4) || bTask==14 || bTask==21 || bTask==31 || bTask==32)	//�ն˸澯����
			DoTermnTxc(bPn, bTask);
		else if ((bTask>=8 && bTask<=13) || bTask==17 || bTask==33)
			DoMtrExc(bPn, bTask);
	}
}
*/


const BYTE g_bChgAlrID[]={8,9,10,11,12,13,15,17,18,24,25,26,27,28,29,30,33,34,37,38,0};
//�����������澯���ݵ��澯��¼�����ݶ�,
// 		pbBuf�е������ǰ���05��Լ��д��,���Զ���05��Լ��˵���õ���
// 		����698Э��,Ҫ��05��Լ�ĵ���¼�ת����698��Լ�ĵ���¼�,�������ֽ����䳤һ���ֽ�
//������@bSaveBuf - �������;
//      @pbBuf -�������ݣ�
//���أ��ޡ�
//��ע���������ֽ����䳤һ���ֽ�
BYTE CopyAlrData(BYTE *bSaveBuf, BYTE *pbBuf, BYTE bLen)
{
	//�澯����ת��Ϊ698��ʽ
#ifdef PRO_698
	BYTE bNum = 0;
	bool bFind = false;
	while (g_bChgAlrID[bNum])
	{
		if (bSaveBuf[0] == g_bChgAlrID[bNum])
		{
			bFind = true;
			break;
		}

		if(bSaveBuf[0] < g_bChgAlrID[bNum])
			break;
		bNum++;
	}

	if(bFind) 
	{
		bSaveBuf[1] = bSaveBuf[1]+1;	//��������һ���ֽ�
		bSaveBuf[7]= (pbBuf[0]&0x7F);	//�������
		pbBuf[0]= (pbBuf[0]&0x80);		//��/ֹ��־
		memcpy(&bSaveBuf[8], pbBuf, bLen);
		return bLen+8;
	}
	else
	{
		memcpy(&bSaveBuf[7], pbBuf, bLen); 
	}
#else
	memcpy(&bSaveBuf[7], pbBuf, bLen); //05��Լֱ�ӿ���
#endif
	return bLen+7;
}

//�������澯�¼����档
//������@bAlrID - �¼�ID;
//      @time - �澯�¼�������ʱ�䣻
//      @pbBuf - �澯�¼����ݣ�
//      @bLen - �����¼�Ĭ��Ϊ0���䳤�¼�Ϊ�¼����ݳ��ȣ��澯ʱ�������ݳ���,Ŀǰֻ��Erc3��Erc22Ϊ�䳤�¼���
//		@bErcType - ͬGetErcType�����Ĺ��ܣ���Ҫ���ڲ�����ʼ���¼��õ�,�����¼�Ŀǰ������
bool SaveAlrData(DWORD dwAlrID, WORD wPn, BYTE* pbBuf, DWORD dwLen)
{
	if (IsDbLocked())
		return false;

	if ((dwLen+6)>ALRSECT_SIZE)
		return false;

	WordToByte(wPn, pbBuf+1);
	DWordToByte(dwAlrID, pbBuf+3);
	dwLen += 6;

	//���������ݿ�洢�쳣�¼����ݣ�
	if(dwAlrID&0x00010000)
    {
		WriteAlrRec(FADDR_EVTREC, pbBuf, dwLen);//���¼�
    }
	else
    {
		WriteAlrRec(FADDR_ALRREC, pbBuf, dwLen);//��澯
       	SetLedCtrlMode(LED_ALARM, LED_MODE_TOGGLE);   //�澯��
    }

	return true;
}

//�洢�澯���ݵĽӿں���
//                   �澯ID        ������   ����ʱҪ������������   ���������    ����ʱ��    �澯����ʱ��  �澯����ǰ��������  ����ǰ���ݳ���
bool HandleAlr(DWORD dwAlrID, WORD wPn, TBankItem* pBankItem, WORD wIdNum, DWORD dwCurMin, TTime tmNow, BYTE* bPreData, WORD wDataLen)
{
	BYTE bRecBuf[ALRSECT_SIZE];
	BYTE *pbRec = bRecBuf+7;	//1���ֽڵ�CRC + 2���ֽڵ�Pn + 4���ֽڵ�ID
	WORD wTotalLen = 0;//�˲�����������Э���У�û��������
	int nRead = 0;
	WORD i;

	memset(bRecBuf, INVALID_DATA, sizeof(bRecBuf));

	//�澯״̬
	*pbRec++ = 1;
	//�澯����ʱ��
	pbRec += TimeTo6Bcd(&tmNow,pbRec);

	wTotalLen += 7;//���ȼ��������7���ֽ�

	if(bPreData!=NULL && wDataLen>0)
	{
		memcpy(pbRec, bPreData, wDataLen);
		wTotalLen += wDataLen;
		pbRec += wDataLen;
	}
	for (i=0; i<3; i++)
	{
		if (dwAlrID == 0xE200003D)
		{
			memcpy(pbRec, g_bRdMtrAlrStatus, sizeof(g_bRdMtrAlrStatus));
			memset(pbRec+sizeof(g_bRdMtrAlrStatus), 0, 256-sizeof(g_bRdMtrAlrStatus));
			wTotalLen += 256;
			break;
		}
		nRead = ReadItemMbi(pBankItem, wIdNum, pbRec, dwCurMin*60, INVALID_TIME);
		if (nRead >= 0)
			break;
	}
	if(nRead<0)
		nRead = 0;
	wTotalLen += nRead;

	return SaveAlrData(dwAlrID, wPn, bRecBuf, wTotalLen);
}

/*
//���������쳣�¼�������ж�ȡĳ���澯�¼������n����¼��
//������@bAlrType - �澯�¼�����
//      @bAlrID - �澯�¼�ID��
//      @bRecNum - ȡ�澯�¼���������
//���أ�����������򷵻ظ澯�¼�������pbBuf��,�ҷ��ض�ȡ�ļ�¼����������֮���򷵻�0��
//��ע���澯�¼�������¼��ʽ���澯�¼����ͣ�1-��Ҫ��2-һ�㣩+ �澯�¼�ID + �澯�¼����ȣ�ʱ��+����֮�ͣ�+ �澯ʱ�� + �澯���ݣ�
BYTE ReadAlrItem(BYTE bAlrType, BYTE bAlrID, BYTE bRecNum, BYTE* pbBuf, BYTE* pbLen)
{
	//BYTE bType = GetErcType(bAlrID);
	//if (bAlrType != bType)
	//	return 0;
	BYTE bAlrItem = 0;//�����ĸ澯�¼���¼������������<=�����ϱ�������
	BYTE bLen = 0;
	WORD i=0;
	BYTE bBuf[37];

	if (bAlrID == 22) //���澯�¼�ÿ����¼���Ȳ��̶������õıȽ��٣��ʲ��ڴ˺������жϣ����Ժ��������¼���ȡ�������򽫲��澯��¼��ʽ��Ϊ��¼���ȹ̶�����
		return 0;

	memset(bBuf, 0, sizeof(bBuf));

	*pbLen = 0;
	//while (ReadAlrRec(bAlrType, j++, bBuf, MAX_ALR_LEN)>0)
	while (ReadAlrRec(bAlrType, i++, bBuf, 37)>0)
	{

		if (bBuf[0]==bAlrID)
		{
			bLen = bBuf[1]+2;
			memcpy(pbBuf++, &bAlrType, 1);
			memcpy(pbBuf, bBuf, bLen);
			pbBuf += bLen;
			
			if (++bAlrItem > bRecNum)
				break;
			
			*pbLen += bLen;
			memset(bBuf, 0, sizeof(bBuf));
			
		}
		else if (bBuf[0] == 0)
			break;

		if (i>255)
			break;

	}
	return bAlrItem;
}*/

