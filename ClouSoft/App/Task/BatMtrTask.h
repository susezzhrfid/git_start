#ifndef BATMTRTASK_H
#define BATMTRTASK_H

typedef struct{
	BYTE	bAddr[8];
	BYTE	bMtrKeyCiph[32];
	WORD	wTaskDataLen;
	BYTE	bTaskData[8];//0字节或者2个字节  在此多开为8个字节
}TBatTask0;//身份认证

//批量数据信息结构体
typedef struct{
	BYTE	bAddr[8];
	BYTE	bMtrKeyCiph[32];
	WORD	wTaskDataLen;
	BYTE	bTaskData[256];
}TBatTask1;//对时任务
bool GetBatMtrTask1Data(TBatTask1* pBatTask1, WORD* wMtrIdx, BYTE* pwSect);//取批量对时任务数据
bool GetBatMtrTask0Data(TBatTask0* pBatTask0, BYTE* bP2);//取批量身份认证任务数据函数
BYTE Trans645BatTaskFrm(BYTE* pbAddr, BYTE bTaskID, BYTE bFrmLen, BYTE* pbFrm, BYTE* pbRetFrm);
bool DoBatMtrCmd();

#endif //BATMTRTASK_H