#ifndef DBSGAPI_H
#define DBSGAPI_H

#include "TypeDef.h"
#include "sysarch.h"
#include "sysapi.h"
#include "LibDbAPI.h"
#include "DbHook.h"
#include "DbFmt.h"


#define SUB_ID_MAX		15


#define T_SG			0x00			//单独数据项,没有相对应的子ID的数据ID
#define T_FEE_BLK		0x01	 		//简单的块数据项,表示当前ID的数据为块数据ID，有相对应的子ID的数据ID,带费率个数
#define T_BLK			0x02	 		//简单的块数据项,表示当前ID的数据为块数据ID，有相对应的子ID的数据ID,不带费率个数

#define C_01			0x00				//表示块ID对应的编码方式,这种编码表示起始第一个子ID最低位00开始,第二个子ID的最低位从01开始
#define C_02			0x01				//表示块ID对应的编码方式,这种编码表示起始第一个子ID最低位00开始,第二个子ID的最低位从02开始
#define C_01R			0x02				//表示块ID对应的编码方式,这种编码表示第一个字节的内容为费率数，并起始第一个子ID最低位00开始,第二个子ID的最低位从02开始
#define C_03			0x03			//表示块ID对应的编码方式,这种编码表示起始第一个子ID最低位01开始,第二个子ID的最低位从02开始

#define N_64		64					//块ID对应的子ID个数(总+费率)
#define N_21		21					//块ID对应的子ID个数(总+费率)
#define N_16		16					//块ID对应的子ID个数() 如本日电压偏差最大值、最小值、平均值
#define N_5			5					//块ID对应的子ID个数(总+费率) 数据库暂用这个
#define N_4			4					//块ID对应的子ID个数(如功率因数等数据块)
#define N_6			6					//块ID对应的子ID个数(如电流数据块)
#define N_7			7					//块ID对应的子ID个数(如状态字)
#define N_9			9					//块ID对应的子ID个数(如电压数据块)


typedef struct  
{
	DWORD	dwID;					//协议ID
	DWORD dwBlkID;			//所属块ID，不属于某个块ID的赋值为0
	WORD	wIdType;					//标志位
	BYTE	bSubNum;				//块ID包括的子ID个数
	BYTE	bCode;					//块ID的子ID编排方式
	WORD	wSubId[SUB_ID_MAX];		//内部对应子ID
}TProId2Sub;						//协议ID到子ID的映射

typedef struct
{
	WORD	wPn;					//测量点号
	WORD	wBn;					//BANK号
	DWORD	dwID;					//数据项ID
}TProItem;							//主要用来进行数据项的时间查询

typedef struct
{
	DWORD dwId;             //子ID

	BYTE  bIdType;          //ID的类型          	
	DWORD dwProId;       	//子ID对应的配置表上的协议ID号，如果配置表上有完全相等的ID则等于dwId
	WORD  wProLen;          //块ID对应的数据长度
	BYTE	bCode;			//块ID的子ID编排方式
	
	WORD  wOffset;          //子ID数据在ID数据中的偏移位置，如果是ID本身则等于0
	WORD  wDataLen;         //子ID对应的数据长度，如果是ID本身则等于wProLen
	BYTE  bIdx;             //子ID在块ID中的索引，如果是ID本身则等于0
}TProIdInfo;	//协议ID的信息

int WriteItemDw(WORD wBank, WORD wPn, DWORD dwID, BYTE* pbBuf, DWORD dwTime);

//单个ID按缓冲区的读
int ReadItemDw(WORD wBank, WORD wPn, DWORD dwID, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime); //指定时间读

//多个ID的读接口,目前只支持按缓冲区的读
int ReadItemDwMid(WORD wBank, WORD wPn, DWORD *pdwID, WORD wNum, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime);//按相同测量点时间读
int ReadItemDwMbi(TProItem* pProItem, WORD wNum, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime);//按相同时间读

int GetItemLenDw(WORD wBn, DWORD dwID);
int GetItemsLenDw(DWORD *pdwItemID, WORD wNum);

//数据项时间的查询,看数据项是否再指定时间内更新了
int QueryItemTimeDwMbi(DWORD dwStartTime, DWORD dwEndTime, TProItem* pProItem, WORD wNum, BYTE* pbBuf, WORD* pwValidNum);  //按照TBankItem的通用做法
int QueryItemTimeDwMid(DWORD dwStartTime, DWORD dwEndTime, WORD wBn, WORD wPn, DWORD* pdwID, WORD wNum, BYTE* pbBuf, WORD* pwValidNum); //同一个BANK同一个测量点不同ID的查询
int QueryItemTimeDwMt(DWORD* pdwStartTime, DWORD* pdwEndTime, TProItem* pProItem, WORD wNum, BYTE* pbBuf, WORD* pwValidNum); //按照不同数据项不同时间的查询

WORD* ProId2SubId(DWORD dwID,WORD* pwSubNum);
WORD* ProId2RdId(DWORD dwID,WORD* pwSubNum);
bool InitProId2Sub();

//取得协议ID的信息
bool GetProIdInfo(DWORD dwId, TProIdInfo* pInfo, bool fBlk);
WORD GetOnePnFromDA(BYTE bDA,BYTE bDAG);

#endif

