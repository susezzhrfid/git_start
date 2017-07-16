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

//8���ֽڰ����¶���
//���λ��7λ��ʾ֡�Ƿ��д����
//��1λ����6λĿǰ��������ʹ��
//��0λ��ʾ��½֡,1��ʾ����ɹ���0��ʾ����ʧ��
extern BYTE g_bFrmHandle;




//���ӷ�ʽ
#define CONNECTTYPE_LOCAL               0      //�������ӷ�ʽ
#define CONNECTTYPE_GPRS                1      //GPRS���ӷ�ʽ
#define CONNECTTYPE_UPLINK             	2      //UPLINK���ӷ�ʽ
#define CONNECTTYPE_MEGA16            	4      //�͵�Ƭ��ͨѶ

//���޸���
//#define GBPRO_MAXSUMGROUP		(GB_MAXSUMGROUP-GB_MAXOFF)	//�ܼ������
#define GBPRO_MAXPOINT			(GB_MAXMETER-GB_MAXOFF)		//���������

//����ռ��С����
#define GB_FRM_SIZE   					2048				//��Э��֧��һ֡�Ļ����С
//#define GB_FRMSIZE_TX					GB_FRM_SIZE			//����֡�����С
//#define GB_FRMSIZE_RX					GB_FRM_SIZE			//����֡�����С
#define GB_MAXDATASIZE					(GB_FRM_SIZE>>1)	//ʵ����֡ʱ���б�Ŀǰ���Ϊ1K
#define GB_MTRFWDSIZE					2048				//ת���Ļ����С
#define GB_C1DATASIZE					10000				//��Ϊ1�����ݶ����ӿ��Ƕ����ģ���˻��浥������Ϊ���漫�޴�С

//ͨ��ƽ̨ͳһʹ�õ�֡���ȶ���
#define FAP_FRM_SIZE        			GB_FRM_SIZE

//��Ϊ���ٶ����������պͷ���
#define GB_PRO_RX						0
#define GB_PRO_TX						1

#define SG_MTRFWDSIZE					2048				//ת���Ļ����С
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
#define SG_LFUN_ASKN1DATA				10 //1������
#define SG_LFUN_ASKN2DATA				11 //2������
//PRM = 0
#define SG_LFUN_CONFIRM					0
#define SG_LFUN_DATAREPLY				8
#define SG_LFUN_NOASKDATA				9
#define SG_LFUN_LINKSTATUS				11

/*******************defination of App layer's FUN command code**********************/
#define SG_AFUN_CONFIRM					0x00	//ȷ�ϨM��
#define SG_AFUN_CHECKLINK				0x02	//��·�ӿڼ��
#define SG_AFUN_SETPARA					0x04	//д����
#define SG_AFUN_VERIFYPWD				0x06	//�����֤����ԿЭ��
#define SG_AFUN_ASKPARA					0x0a	//������
#define SG_AFUN_ASKCLASS1				0x0c	//����ǰ����
#define SG_AFUN_ASKCLASS2				0x0d	//����ʷ����
#define SG_AFUN_ASKCLASS3				0x0e	//���¼���¼
#define SG_AFUN_TRANSFILE				0x0f	//�ļ�����
#define SG_AFUN_MTRFWD					0x10	//�м�ת��
#define SG_AFUN_ASKTASK					0x12	//����������
#define SG_AFUN_ASKALARM				0x13	//���澯����
#define SG_AFUN_LINKCMD					0x14	//��������

#define SG_AFUN_USERDEF					0x15	//�Զ��岿��
#define SG_AFUN_ADDON					0xff	//�Զ��岿��
/*******************ȷ��/���ϻش�����**********************/
#define SG_RCQ_FRM_OK					0x00	//ȷ��
#define SG_RCQ_FRM_NONE				0x01	//����
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

//��·�ӿڼ��
#define SG_LNKTST_LOGIN					1
#define SG_LNKTST_LOGOUT				2
#define SG_LNKTST_BEAT					3

//ȷ��/���ϻش�
#define AFN00_ALLOK						0x01
#define AFN00_ALLERR					0x02
#define AFN00_EVERYITEM					0x04
#define AFN00_EASMERR					0x08

//Ӳ����ȫ��������
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

#define FWR_DATA_OFFSET				8		//�м�ת����������ʼƫ��

//////////////////////////////////////////////////////////////
//�㽭Э�鶨��
//�����εĳ���
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
#define FAP_CMD_LINK        			0x18  //��������
#define FAP_CMD_LOGIN       			0x21
#define FAP_CMD_LOGOUT      			0x22
#define FAP_CMD_BEAT        			0x24
#define FAP_CMD_LINK_TRANS  			0x28
#define FAP_CMD_RX_SMS      			0x29
//�㽭Э�鶨�����
//////////////////////////////////////////////////////////////

#define SG_ERR_RIGHT					0	//��ȷ  
#define SG_ERR_TIMEOUT					1	//�м�����û�з���
#define SG_ERR_DATASETVALID			2	//�������ݷǷ�
#define SG_ERR_PASSWORDERR			3	//����Ȩ�޲���
#define SG_ERR_NODATA					4	//�޴�������
#define SG_ERR_VALIDTIME				5	//����ʱ��ʧЧ


#define SCHED_07_FRM_PERMANENT_BYTE	12	//07���֡����ǰ���ֽ���
#define SCHED_07_FRM_LEN_BIT			9	//07�����վ�������еĳ���λ

#define SCHED_07_CMD_ALARM	0x1C			//����բ,�澯,��������
#define SCHED_07_BYTE_STATIC	0x08			//���� (4)+�����ߴ���(4)
#define SCHED_07_REQUEST_FRAME_LENGTH		16	//����֡���ݳ���
#define SCHED_07_REQUEST_FIX_LENGTH		12	//����֡��������ĳ���

#define SCHED_07_RESPONSE_BIT			8		//Ӧ��λ
#define SCHED_07_RESPONSE_OK			0x9C	//����Ӧ��
#define SCHED_07_RESPONSE_ABNORMITY	0xDC	//�쳣Ӧ��

#define SCHED_07_ERR_BIT					9		//����λ
#define SCHED_07_RESPONSE_VALIDTIME		0x70	//����ʱ��ʧЧ
#define SCHED_07_RESPONSE_DATASETVALID	0x08	//�������ݷǷ�
#define SCHED_07_RESPONSE_PASSERR			0x04	//����Ȩ�޲���

#define SCHED_07_RESPONSE_OK_TIME			0x94	//����Ӧ��
#define SCHED_07_RESPONSE_ABNORMITY_TIME	0xD4	//�쳣Ӧ��

#define SG_FAP_DATA_EX   6			//�Զ���������ļ���ƫ�� (2�ֽڿ�½ʶ���� + 1��չ������ + 3���ֽ��Զ�������)

#define FRM_TYPE_NO_DATA        0                   //�����������ݵ�֡				  �統ǰ����
#define FRM_TYPE_TIME_ONLY      1                   //����ʼʱ��ͽ���ʱ���֡        ���¼�����,�澯����
#define FRM_TYPE_TIME_DENSITY   2                   //����ʼʱ�䡢����ʱ����ܶȵ�֡  ����ʷ����,��������

#define DATA_DENSITY_LEN        1                   //�����ܶȳ���
#define DADI_LEN                6                   //DADI����
#define TP_LEN					5					//ʱ���ǩ����

////////////////////////////////////////////////////////////////////////////////////////////
//GbPro��������
#define SG_FNPN_GRP_SZ			8		//һ��DADI���ֻ��8������DAΪFFFF��ȫ����Ч������ʱ���ŵ�Э���ѭ������
#define SG_DADT_GRP_SZ			65

//���յ�֡��־λ
#define FRM_CONFIRM				0x01	//ȷ��֡
#define DATA_TIME_LEN			6		//����ʱ�곤��

#define DAY_FRZ_TYPE			1
#define MONTH_FRZ_TYPE			2
#define CURVE_FRZ_TYPE			3

typedef struct{
	DWORD	dwClick;			//������Ϣʱ��ʱ��

	TCommPara CommPara;				//���ڲ���

	bool 	fFixTO;				//==trueʱ���̶�ʹ��wFrmTimeoutָ����ʱ��,��ֹ�Զ���ʱʱ�����(�����Զ�����Ϊ��С5��)��
	WORD	wFrmTimeout;			//���յȴ����ĳ�ʱʱ��(ms)��0��Ч
	WORD	wByteTimeout;			//���յȴ��ֽڳ�ʱʱ��(ms)��0��Ч

	WORD  	wTxLen;					//����֡����
	//BYTE*  	pbTxBuf;				//����֡������

	WORD  	wRxBufSize;				//���ջ�������С
	BYTE*  	pbRxBuf;				//����֡������

	WORD  	wRxLen;				//�������ؽ��ճ���
	int 	iRet;				//����ֵ,==0:��û����,1:��ȷ,<0:����
}TMtrFwdMsg; //һ��͸��������Ϣ�ṹ

typedef struct{
	BYTE bCopCode[ 4 ];//���̴���
	BYTE bDevCode[ 8 ];//�豸���
	BYTE bVerNo[4];	//����汾��
	BYTE bVerDate[3];    //�汾����
	BYTE bTerminalFlashInfo[11]; //������Ϣ
} TERMINAL_VER_INFO;

typedef struct{
    char    chFileName[32];                  //�ļ���
	WORD   wFileProp;        //�ļ�����
	WORD   wFileTotalSec;    //�ļ��ܶ��� N
	DWORD  dwFileTotalLen;   //�ļ��ܳ���
	WORD   wFileCurSec;      //�ļ���ǰ�κ� 0 ~ N-1
	WORD   wFileCurSecLen;	 //��ǰ�γ���
	WORD   wFileCRC;         //�ļ���CRCУ��
	DWORD  dwFileOffset;     //д�ļ�ʱ��ƫ����
	BYTE    bFileFlg[256];
} TRANSMIT_FILE_INFO;                                     
//extern TRANSMIT_FILE_INFO g_TransmitFileInfo;  
                                            
typedef struct{
	DWORD	dwId;
	BYTE	bPn;	//Ϊ�˽�ʡ�ڴ棬�����㶨��ΪBYTE
	//BYTE    bFn;
}TGbFnPn;	//һ����Ϣ��ʶ�ṹ

typedef struct{
	BYTE	bDA1;
	BYTE	bDA2;
	BYTE	bDI1;
	BYTE	bDI2;
	BYTE	bDI3;
	BYTE	bDI4;
	BYTE	bErr;
	//bool	fItemOK;
}TSgDaDt;	//DADT�������Ϣ
typedef struct{
	DWORD	dwID;
	WORD	wPN;
}TSgIdPn;//һ����Ϣ��ʶ�ṹ

#define SMS_ADDR_INTER    				0x91

//����Э���ඨ��
typedef struct{
	//����
	BYTE* pbRxBuf;	//������֡�Ļ�����������ʹ��ڿ��Դ��벻ͬ��С��ָ���Դﵽ��ʡ�ڴ��Ŀ��
	BYTE* pbTxBuf;	//������֡�Ļ�����������ʹ��ڿ��Դ��벻ͬ��С��ָ���Դﵽ��ʡ�ڴ��Ŀ��
	DWORD dwRxClick; //���յ��ֽڵ�ʱ��
	DWORD dwFrmStartClick;  //���յ�֡��ʼʱ��
	bool fLocal;	//�Ƿ��Ǳ���ά����
	bool fAutoSend;			//�Ƿ�����������͵Ĺ���
	DWORD dwAddr1;	//����������A1
	DWORD dwAddr2;	//�ն˵�ַA2
	
	//����
	WORD wRxStep;
    WORD wRxPtr;
	WORD wRxCnt;
	WORD wRxFrmLen;
	WORD wRxtry;

	WORD wTxPtr;

	BYTE bEC1;
	BYTE bHisSEQ;
	BYTE bMySEQ;
	BYTE bRxFrmFlg;	//���յ�֡��־λ
	bool fRptState;	//1��2��������������״̬

	//����ͳ��
	DWORD dwDayFlux;
	DWORD dwMonFlux;
}TSgPro;	//GPRSͨ�Žӿڻ���ṹ

typedef struct{
	WORD wYkPn;
	BYTE bYkMtrAddr[6];
	BYTE bYkOptPwd[4];
	BYTE bYKOptCode[4];
	BYTE bYKValDly;//������Ч��ʱ
	BYTE bYKCtrlType;	//����������
}TYkCtrl;			//YK���ƽṹ

////////////////////////////////////////////////////////////////////////////////////////////
//GbPro������������
void SgInit(struct TPro* pPro, BYTE* pbRxBuf, BYTE* pbTxBuf, bool fLocal);

////////////////////////////////////////////////////////////////////////////////////////////
//GbPro˽�к�������
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

