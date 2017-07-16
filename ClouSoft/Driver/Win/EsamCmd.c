#include "EsamCmd.h"
#include "Sysarch.h"


BYTE g_bEsamTxRxBuf[1800];		//发送接收共用
TSem g_semEsam;

int esam_read(BYTE *buf, int count)
{
	return 0;//返回本次收到的字节数
}

//接口设备发送完数据Len2后，需要加100μs时间间隔，再发送DATA
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

//描述：通过ESAM加密'身份认证任务中的电表密钥密文',得到 ER0：16字节密文
//参数：@bP2 对应为命令中的P2，P2设置为01时，下发任务命令数据中的表号为实际表号；P2设置为02时下发任务命令数据中的表号为8字节AA时
//		@pbMtrAddr 实际表号，不能是8字节AA
//		@pbMtrCiph 身份认证任务中的电表密钥密文
//返回：如果正确返回数据长度，错误返回负数
bool EsamGetMtrAuth(BYTE bP2, BYTE* pbMtrCiph, BYTE* pbMtrAddr, BYTE* pbTskData, BYTE* pbR1, BYTE* pbER1)
{
	return true;
}

//描述：通过ESAM加密'电表对时任务密文'
//参数：@pbMtrCiph 身份认证任务中的电表密钥密文
//返回：如果正确返回数据长度，错误返回负数
//备注：在电表身份认证中返回
//		随机数2（R2）：38A45D72（在对时任务举例中使用）
//		ESAM序列号：100112BB4798D2FD
int EsamGetAdjTmCiph(BYTE* pbTskFmt, BYTE* pbTskData, BYTE bTskLen, BYTE* pbMtrKeyCiph, BYTE* pbR2, BYTE* pbRx)
{
	return 0;
}
