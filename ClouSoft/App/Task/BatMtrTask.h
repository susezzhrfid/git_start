#ifndef BATMTRTASK_H
#define BATMTRTASK_H

typedef struct{
	BYTE	bAddr[8];
	BYTE	bMtrKeyCiph[32];
	WORD	wTaskDataLen;
	BYTE	bTaskData[8];//0�ֽڻ���2���ֽ�  �ڴ˶࿪Ϊ8���ֽ�
}TBatTask0;//�����֤

//����������Ϣ�ṹ��
typedef struct{
	BYTE	bAddr[8];
	BYTE	bMtrKeyCiph[32];
	WORD	wTaskDataLen;
	BYTE	bTaskData[256];
}TBatTask1;//��ʱ����
bool GetBatMtrTask1Data(TBatTask1* pBatTask1, WORD* wMtrIdx, BYTE* pwSect);//ȡ������ʱ��������
bool GetBatMtrTask0Data(TBatTask0* pBatTask0, BYTE* bP2);//ȡ���������֤�������ݺ���
BYTE Trans645BatTaskFrm(BYTE* pbAddr, BYTE bTaskID, BYTE bFrmLen, BYTE* pbFrm, BYTE* pbRetFrm);
bool DoBatMtrCmd();

#endif //BATMTRTASK_H