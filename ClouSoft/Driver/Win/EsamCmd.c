#include "EsamCmd.h"
#include "Sysarch.h"


BYTE g_bEsamTxRxBuf[1800];		//���ͽ��չ���
TSem g_semEsam;

int esam_read(BYTE *buf, int count)
{
	return 0;//���ر����յ����ֽ���
}

//�ӿ��豸����������Len2����Ҫ��100��sʱ�������ٷ���DATA
int esam_write(BYTE * buffer, int count)
{
	return count;
}

void esam_warm_reset(void)
{
}

void esam_init(void)
{
	g_semEsam = NewSemaphore(1,1);
}

bool EsamInit()
{
	esam_init();
	return true; 
}

//������ͨ��ESAM����'�����֤�����еĵ����Կ����',�õ� ER0��16�ֽ�����
//������@bP2 ��ӦΪ�����е�P2��P2����Ϊ01ʱ���·��������������еı��Ϊʵ�ʱ�ţ�P2����Ϊ02ʱ�·��������������еı��Ϊ8�ֽ�AAʱ
//		@pbMtrAddr ʵ�ʱ�ţ�������8�ֽ�AA
//		@pbMtrCiph �����֤�����еĵ����Կ����
//���أ������ȷ�������ݳ��ȣ����󷵻ظ���
bool EsamGetMtrAuth(BYTE bP2, BYTE* pbMtrCiph, BYTE* pbMtrAddr, BYTE* pbTskData, BYTE* pbR1, BYTE* pbER1)
{
	return true;
}

//������ͨ��ESAM����'����ʱ��������'
//������@pbMtrCiph �����֤�����еĵ����Կ����
//���أ������ȷ�������ݳ��ȣ����󷵻ظ���
//��ע���ڵ�������֤�з���
//		�����2��R2����38A45D72���ڶ�ʱ���������ʹ�ã�
//		ESAM���кţ�100112BB4798D2FD
int EsamGetAdjTmCiph(BYTE* pbTskFmt, BYTE* pbTskData, BYTE bTskLen, BYTE* pbMtrKeyCiph, BYTE* pbR2, BYTE* pbRx)
{
	return 0;
}
