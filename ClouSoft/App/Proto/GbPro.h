#ifndef GBPRO_H
#define GBPRO_H

#include "Comm.h"
#include "sysarch.h"
#include "stdio.h"
#include <stdarg.h> 
#include "FaConst.h"
//#include "FaStruct.h"
#include "Pro.h"
//#include "sftp.h"
#include "DbConst.h"
#include "ProStruct.h"
#include "AutoSendTask.h"

//8个字节按如下定义
//最高位第7位表示帧是否有处理过
//第1位到第6位目前保留，不使用
//第0位表示登陆帧,1表示处理成功，0表示处理失败
extern BYTE g_bFrmHandle;




//连接方式
#define CONNECTTYPE_LOCAL               0      //本地连接方式
#define CONNECTTYPE_GPRS                1      //GPRS连接方式
#define CONNECTTYPE_UPLINK             	2      //UPLINK连接方式
#define CONNECTTYPE_MEGA16            	4      //和单片机通讯

//极限个数
//#define GBPRO_MAXSUMGROUP		(GB_MAXSUMGROUP-GB_MAXOFF)	//总加组个数
#define GBPRO_MAXPOINT			(GB_MAXMETER-GB_MAXOFF)		//测量点个数

//缓存空间大小定义
#define GB_FRM_SIZE   					2048				//本协议支持一帧的缓存大小
//#define GB_FRMSIZE_TX					GB_FRM_SIZE			//发送帧缓存大小
//#define GB_FRMSIZE_RX					GB_FRM_SIZE			//接收帧缓存大小
#define GB_MAXDATASIZE					(GB_FRM_SIZE>>1)	//实际组帧时的判别，目前设计为1K
#define GB_MTRFWDSIZE					2048				//转发的缓存大小
#define GB_C1DATASIZE					10000				//因为1类数据读出接口是独立的，因此缓存单开，此为缓存极限大小

//通用平台统一使用的帧长度定义
#define FAP_FRM_SIZE        			GB_FRM_SIZE

//此为跟踪定义来区分收和发的
#define GB_PRO_RX						0
#define GB_PRO_TX						1

#define SG_MTRFWDSIZE					2048				//转发的缓存大小
#define SG_PRO_FRM_SIZE 				2048
/*******************Location of one frame**********************/
//FIX FRAME HEADER
#define SG_LOC_START1					0
#define SG_LOC_LLEN1					1
#define SG_LOC_HLEN1					2
#define SG_LOC_LLEN2					3
#define SG_LOC_HLEN2					4
#define SG_LOC_START2					5

#define SG_LOC_CONTROL					6

#define SG_LOC_ADDA1					7
#define SG_LOC_ADDA2					8
#define SG_LOC_ADDA3					9

#define SG_LOC_ADDB1					10
#define SG_LOC_ADDB2					11
#define SG_LOC_ADDB3					12
#define SG_LOC_ADDB4					13

#define SG_LOC_AFN						14
#define SG_LOC_SEQ						15
#define SG_LOC_DA1						16
#define SG_LOC_DA2						17

#define SG_LOC_DI1						18
#define SG_LOC_DI2						19
#define SG_LOC_DI3						20
#define SG_LOC_DI4						21

#define SG_LOC_DATA						22

#define SG_FORWARD_TYPE					22
#define SG_FORWARD_PORT					23
#define SG_FORWARD_BAUDRATE				24
#define SG_FORWARD_VERIFY				25
#define SG_FORWARD_BYTE_DATA			26
#define SG_FORWARD_BYTE_STOP			27
#define SG_FORWARD_WAIT_TIME			28
#define SG_FORWARD_CMD_LEN				29
#define SG_FORWARD_DATA					30

/*******************defination of CONTROL BYTE**********************/
#define SG_CTL_FUN						0x0f
#define SG_CTL_FCV						0x10
#define SG_CTL_FCB						0x20
#define SG_CTL_ACD						0x20
#define SG_CTL_PRM						0x40
#define SG_CTL_DIR						0x80

/*******************defination of Linker layer's FUN code**********************/
//PRM = 1
#define SG_LFUN_RESET					1
#define SG_LFUN_ASKNOREPLY				4
#define SG_LFUN_ASKLINKSTATUS			9
#define SG_LFUN_ASKN1DATA				10 //1级数据
#define SG_LFUN_ASKN2DATA				11 //2级数据
//PRM = 0
#define SG_LFUN_CONFIRM					0
#define SG_LFUN_DATAREPLY				8
#define SG_LFUN_NOASKDATA				9
#define SG_LFUN_LINKSTATUS				11

/*******************defination of App layer's FUN command code**********************/
#define SG_AFUN_CONFIRM					0x00	//确认∕否定
#define SG_AFUN_CHECKLINK				0x02	//链路接口检测
#define SG_AFUN_SETPARA					0x04	//写参数
#define SG_AFUN_VERIFYPWD				0x06	//身份认证及密钥协商
#define SG_AFUN_ASKPARA					0x0a	//读参数
#define SG_AFUN_ASKCLASS1				0x0c	//读当前数据
#define SG_AFUN_ASKCLASS2				0x0d	//读历史数据
#define SG_AFUN_ASKCLASS3				0x0e	//读事件记录
#define SG_AFUN_TRANSFILE				0x0f	//文件传输
#define SG_AFUN_MTRFWD					0x10	//中继转发
#define SG_AFUN_ASKTASK					0x12	//读任务数据
#define SG_AFUN_ASKALARM				0x13	//读告警数据
#define SG_AFUN_LINKCMD					0x14	//级联命令

#define SG_AFUN_USERDEF					0x15	//自定义部分
#define SG_AFUN_ADDON					0xff	//自定义部分
/*******************确认/否认回答内容**********************/
#define SG_RCQ_FRM_OK					0x00	//确认
#define SG_RCQ_FRM_NONE				0x01	//否认
/*******************defination of SEQUENCE BYTE**********************/
#define SG_SEQ_SEQ						0x0f
#define SG_SEQ_CON						0x10
#define SG_SEQ_FIN						0x20
#define SG_SEQ_FIR						0x40
#define SG_SEQ_TVP						0x80
#define SG_SEQ_TVP0						0x00
#define SG_SEQ_TVP1						0x80
/*******************defination of ERR BYTE**********************/
#define SG_ERR_OK						0
#define SG_ERR_DATAERR					1
#define SG_ERR_PASSERR					2
#define SG_ERR_NOVALIDDATA				3

//链路接口检测
#define SG_LNKTST_LOGIN					1
#define SG_LNKTST_LOGOUT				2
#define SG_LNKTST_BEAT					3

//确认/否认回答
#define AFN00_ALLOK						0x01
#define AFN00_ALLERR					0x02
#define AFN00_EVERYITEM					0x04
#define AFN00_EASMERR					0x08

//硬件安全错误类型
#define ESAM_ERR_SIGN					1
#define ESAM_ERR_KEY					2
#define ESAM_ERR_MAC					3

/*******************  defination of others  **********************/
#define SGADDR_BROADCAST				0xffffff
#define SGADDR_INVALID					0
                                		
#define GBADDR_SINGLE					0
#define GBADDR_GROUP					1
                                		
#define GBLNKFLG_PtoP					0
#define GBLNKFLG_BROAD					1
#define GBLNKFLG_GROUP					2

#define FWR_DATA_OFFSET				8		//中继转发数据域起始偏移

//////////////////////////////////////////////////////////////
//浙江协议定义
//各个段的长度
#define LEN_OF_HEAD   14
#define LEN_OF_AFN    1
#define LEN_OF_SEQ    1
#define LEN_OF_DA     2
#define LEN_OF_DT     4
#define LEN_OF_CC     2
#define LEN_OF_LEN    2
#define LEN_OF_CL     2
#define LEN_OF_CON    1
#define LEN_OF_PWD    3

#define LEN_TO_LEN    (LEN_OF_AFN+LEN_OF_SEQ+LEN_OF_DA+LEN_OF_DT+LEN_OF_CC)
#define LEN_TO_DATA   (LEN_TO_LEN+LEN_OF_LEN+LEN_OF_CL+LEN_OF_CON+LEN_OF_PWD)



#define FAP_MSTA_SEQ  5
#define FAP_CMD       0   
#define FAP_LEN       1	  
#define FAP_DATA      3   
#define FAP_DATA_EX   7   

#define FAP_CMD_GET     0x3f
#define FAP_CMD_UP      0x80
#define FAP_CMD_DIR 	0x80
#define FAP_CMD_DOWN    0x00
#define FAP_FIX_LEN     (SG_LOC_DATA-SG_LOC_CONTROL)

#define FAP_CMD_READ_DATA   0x01
#define FAP_CMD_READ_TASK   0x02
#define FAP_CMD_WRITE_DATA  0x08
#define FAP_CMD_USERDEF     0x15

#define FAP_CMD_RD_FORWARD  			0x00
#define FAP_CMD_READ_DATA   			0x01
#define FAP_CMD_READ_TASK   			0x02
#define FAP_CMD_READ_LOG    			0x04
#define FAP_CMD_RTWR_DATA   			0x07
#define FAP_CMD_WRITE_DATA  			0x08
#define FAP_CMD_ALR	        			0x09
#define FAP_CMD_ALR_RE      			0x0a
//#define FAP_CMD_USERDEF     			0x0f
#define FAP_CMD_LINK        			0x18  //级联命令
#define FAP_CMD_LOGIN       			0x21
#define FAP_CMD_LOGOUT      			0x22
#define FAP_CMD_BEAT        			0x24
#define FAP_CMD_LINK_TRANS  			0x28
#define FAP_CMD_RX_SMS      			0x29
//浙江协议定义结束
//////////////////////////////////////////////////////////////

#define SG_ERR_RIGHT					0	//正确  
#define SG_ERR_TIMEOUT					1	//中继命令没有返回
#define SG_ERR_DATASETVALID			2	//设置内容非法
#define SG_ERR_PASSWORDERR			3	//密码权限不足
#define SG_ERR_NODATA					4	//无此数据项
#define SG_ERR_VALIDTIME				5	//命令时间失效


#define SCHED_07_FRM_PERMANENT_BYTE	12	//07电表帧内容前的字节数
#define SCHED_07_FRM_LEN_BIT			9	//07电表主站请求帖中的长度位

#define SCHED_07_CMD_ALARM	0x1C			//跳合闸,告警,保电命令
#define SCHED_07_BYTE_STATIC	0x08			//密码 (4)+操作者代码(4)
#define SCHED_07_REQUEST_FRAME_LENGTH		16	//请求帧内容长度
#define SCHED_07_REQUEST_FIX_LENGTH		12	//请求帧除内容外的长度

#define SCHED_07_RESPONSE_BIT			8		//应答位
#define SCHED_07_RESPONSE_OK			0x9C	//正常应答
#define SCHED_07_RESPONSE_ABNORMITY	0xDC	//异常应答

#define SCHED_07_ERR_BIT					9		//错误位
#define SCHED_07_RESPONSE_VALIDTIME		0x70	//命令时间失效
#define SCHED_07_RESPONSE_DATASETVALID	0x08	//设置内容非法
#define SCHED_07_RESPONSE_PASSERR			0x04	//密码权限不足

#define SCHED_07_RESPONSE_OK_TIME			0x94	//正常应答
#define SCHED_07_RESPONSE_ABNORMITY_TIME	0xD4	//异常应答

#define SG_FAP_DATA_EX   6			//自定义命令报文文件名偏移 (2字节科陆识别码 + 1扩展控制码 + 3个字节自定义密码)

#define FRM_TYPE_NO_DATA        0                   //不带数据内容的帧				  如当前数据
#define FRM_TYPE_TIME_ONLY      1                   //带起始时间和结束时间的帧        如事件数据,告警数据
#define FRM_TYPE_TIME_DENSITY   2                   //带起始时间、结束时间和密度的帧  如历史数据,任务数据

#define DATA_DENSITY_LEN        1                   //数据密度长度
#define DADI_LEN                6                   //DADI长度
#define TP_LEN					5					//时间标签长度

////////////////////////////////////////////////////////////////////////////////////////////
//GbPro常量定义
#define SG_FNPN_GRP_SZ			8		//一组DADI最大只有8个，（DA为FFFF即全部有效测量点时，放到协议层循环处理）
#define SG_DADT_GRP_SZ			65

//接收的帧标志位
#define FRM_CONFIRM				0x01	//确认帧
#define DATA_TIME_LEN			6		//数据时标长度

#define DAY_FRZ_TYPE			1
#define MONTH_FRZ_TYPE			2
#define CURVE_FRZ_TYPE			3

typedef struct{
	DWORD	dwClick;			//发送消息时的时间

	TCommPara CommPara;				//串口参数

	bool 	fFixTO;				//==true时，固定使用wFrmTimeout指定的时间,禁止自动超时时间调整(比如自动调整为最小5秒)，
	WORD	wFrmTimeout;			//接收等待报文超时时间(ms)，0无效
	WORD	wByteTimeout;			//接收等待字节超时时间(ms)，0无效

	WORD  	wTxLen;					//发送帧长度
	//BYTE*  	pbTxBuf;				//接收帧缓冲区

	WORD  	wRxBufSize;				//接收缓冲区大小
	BYTE*  	pbRxBuf;				//接收帧缓冲区

	WORD  	wRxLen;				//用来返回接收长度
	int 	iRet;				//返回值,==0:还没处理,1:正确,<0:错误
}TMtrFwdMsg; //一个透明传输消息结构

typedef struct{
	BYTE bCopCode[ 4 ];//厂商代码
	BYTE bDevCode[ 8 ];//设备编号
	BYTE bVerNo[4];	//软件版本号
	BYTE bVerDate[3];    //版本日期
	BYTE bTerminalFlashInfo[11]; //容量信息
} TERMINAL_VER_INFO;

typedef struct{
    char    chFileName[32];                  //文件名
	WORD   wFileProp;        //文件性质
	WORD   wFileTotalSec;    //文件总段数 N
	DWORD  dwFileTotalLen;   //文件总长度
	WORD   wFileCurSec;      //文件当前段号 0 ~ N-1
	WORD   wFileCurSecLen;	 //当前段长度
	WORD   wFileCRC;         //文件总CRC校验
	DWORD  dwFileOffset;     //写文件时的偏移量
	BYTE    bFileFlg[256];
} TRANSMIT_FILE_INFO;                                     
//extern TRANSMIT_FILE_INFO g_TransmitFileInfo;  
                                            
typedef struct{
	DWORD	dwId;
	BYTE	bPn;	//为了节省内存，测量点定义为BYTE
	//BYTE    bFn;
}TGbFnPn;	//一个信息标识结构

typedef struct{
	BYTE	bDA1;
	BYTE	bDA2;
	BYTE	bDI1;
	BYTE	bDI2;
	BYTE	bDI3;
	BYTE	bDI4;
	BYTE	bErr;
	//bool	fItemOK;
}TSgDaDt;	//DADT及结果信息
typedef struct{
	DWORD	dwID;
	WORD	wPN;
}TSgIdPn;//一个信息标识结构

#define SMS_ADDR_INTER    				0x91

//国标协议类定义
typedef struct{
	//参数
	BYTE* pbRxBuf;	//接收组帧的缓冲区，网络和串口可以传入不同大小的指针以达到节省内存的目的
	BYTE* pbTxBuf;	//发送组帧的缓冲区，网络和串口可以传入不同大小的指针以达到节省内存的目的
	DWORD dwRxClick; //接收到字节的时标
	DWORD dwFrmStartClick;  //接收到帧起始时标
	bool fLocal;	//是否是本地维护口
	bool fAutoSend;			//是否具有主动上送的功能
	DWORD dwAddr1;	//行政区划码A1
	DWORD dwAddr2;	//终端地址A2
	
	//数据
	WORD wRxStep;
    WORD wRxPtr;
	WORD wRxCnt;
	WORD wRxFrmLen;
	WORD wRxtry;

	WORD wTxPtr;

	BYTE bEC1;
	BYTE bHisSEQ;
	BYTE bMySEQ;
	BYTE bRxFrmFlg;	//接收的帧标志位
	bool fRptState;	//1类2类数据主动上送状态

	//流量统计
	DWORD dwDayFlux;
	DWORD dwMonFlux;
}TSgPro;	//GPRS通信接口基类结构

typedef struct{
	WORD wYkPn;
	BYTE bYkMtrAddr[6];
	BYTE bYkOptPwd[4];
	BYTE bYKOptCode[4];
	BYTE bYKValDly;//控制生效延时
	BYTE bYKCtrlType;	//本控制类型
}TYkCtrl;			//YK控制结构

////////////////////////////////////////////////////////////////////////////////////////////
//GbPro公共函数定义
void SgInit(struct TPro* pPro, BYTE* pbRxBuf, BYTE* pbTxBuf, bool fLocal);

////////////////////////////////////////////////////////////////////////////////////////////
//GbPro私有函数定义
int SgRcvBlock(struct TPro* pPro, BYTE* pbBlock, int nLen);
bool SgHandleFrm(struct TPro* pPro);
bool SgLogin(struct TPro* pPro);
bool SgBeat(struct TPro* pPro);
bool SgLogoff(struct TPro* pPro);
void SgLoadUnrstPara(struct TPro* pPro);
void SgDoProRelated(struct TPro* pPro);
bool SgIsNeedAutoSend(struct TPro* pPro);

void SgInitECVal(TSgPro* pGbPro);
bool SgVeryRxFrm(TSgPro* pGbPro);
int SgRxReset(struct TPro* pPro);
bool SgRxSetPara(struct TPro* pPro);
int SgRxAskFaCfg(struct TPro* pPro);
bool SgRxAskPara(struct TPro* pPro);
int SgRxControlCmd(struct TPro* pPro);
int SgRxCallClass1(struct TPro* pPro);
bool SgRxCallClass2(struct TPro* pPro);
bool SgRxCallClass3(struct TPro* pPro, BYTE bAFn);
void TransFileInit();
void GetFileInfo(TRANSMIT_FILE_INFO *ptTransFInfo, BYTE *pbBuf, WORD wFrmLen);
bool AnsTransFile(struct TPro* pPro, BYTE bFD);
int  SgRxCmd_TransFile(struct TPro* pPro);
int SgRxMtrFwdCmd(struct TPro* pPro);
bool ReadTask(struct TPro* pPro);
bool UserDefData(struct TPro* pPro);
//bool SgGetRxSeq(TSgPro* pGbPro);
bool SgGetRxSeq(TSgPro* pGbPro);
void SgGetAuthPara(BYTE* pbAuthType, WORD* pwAuthPara);
int SgVeryPsw(struct TPro* pPro);
DWORD SgMakeDirPsw(TSgPro* pGbPro);
WORD SgMakeCrcPsw(BYTE *pdata);
bool SgCheckTVP(TSgPro* pGbPro);
bool GetACD(TSgPro* pGbPro);
WORD GetLeftBufSize(WORD wTxPtr);
bool SgAnsConfirm(struct TPro* pPro, TSgDaDt* pDaDt, BYTE bDaDtNum);
WORD SgMakeTxFrm(struct TPro* pPro, bool fPRM, BYTE bCmd, BYTE bAFN, WORD wTxPtr);
void SgPrintCmd(BYTE bCmd, bool fTx);
void SgSaveAlrParaChg(TSgDaDt* pDaDt, BYTE bDaDtNum, BYTE bRxMstAddr);
void SgSaveAlrPswErr(BYTE* pbPass, BYTE bRxMstAddr);
int SgRxConfirm(TSgPro* pGbPro);
WORD SgMakeMasterReqFrm(struct TPro* pPro, bool fPRM, BYTE bCmd, BYTE bAFN, WORD wTxPtr);

int ZJHandleFrm(BYTE* pbRxBuf, BYTE* pbTxBuf);
int ZJUserDef(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf);
int ZJMakeFrm(WORD wDataLen, BYTE* pbRxBuf, BYTE* pbTxBuf, bool fErr);
int ZJReplyErr(BYTE bErrCode, BYTE* pbRxBuf, BYTE* pbTxBuf);
int ZJReExtCmd(BYTE bErrCode, BYTE* pbRxBuf, BYTE* pbTxBuf);
int ZJReadDataEx(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf, bool fWordPn);
int ZJWriteDataEx(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf, bool fWordPn);
int ZJSftpDataEx(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf);
int ZJLoadParaFile(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf);
int ZJDealTestCmd(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf);
bool ZJRunCmd(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf);

int RxCmd_VerifyPwd(struct TPro* pPro);
int AnsCmd_Reset(struct TPro* pPro, BYTE bErr, BYTE* pbData, BYTE bDataLen);
bool PutToDaDt(BYTE bIdx, BYTE* p, TSgDaDt* pDaDt, BYTE bSize);
void PutReturnValue(TSgDaDt* pDaDt, BYTE bErr);
WORD Rx_GetIDPN(BYTE *p);
bool ReExtCmd1(struct TPro* pPro, BYTE *pbSftp, WORD len);
bool ReExtCmd(struct TPro* pPro, BYTE bErrCode);
bool RunCmd(struct TPro* pPro);
//int TrigerAdj(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf);
bool ClearConfigFile(struct TPro* pPro);
bool UserDef(struct TPro* pPro);
bool ReadDataEx(struct TPro* pPro);
bool WriteDataEx(struct TPro* pPro);
bool ReExtErr(struct TPro* pPro, BYTE bErrCode, BYTE* pbRxBuf, BYTE* pbTxBuf);
DWORD ValToBaudrate(BYTE val);
BYTE BaudrateToVal(DWORD val);
BYTE ValToParity(BYTE val);
BYTE ParityToVal(BYTE val);
BYTE ValToStopBits(BYTE val);
BYTE StopBitsToVal(BYTE val);
BYTE ValToByteSize(BYTE val);
BYTE ByteSizeToVal(BYTE val);
extern void PnToBytes(WORD wPn, BYTE *pbBuf);
extern WORD GetIdPnFromDiDa(BYTE bDA,BYTE bDAG,BYTE bDI1,BYTE bDI2,BYTE bDI3,BYTE bDI4,TSgIdPn *pIDPNGrp,WORD bIDPNGrpNum);
extern void IdPnToBytes(DWORD dwID, WORD wPn, BYTE *pbBuf);
#endif //GBPRO_H

