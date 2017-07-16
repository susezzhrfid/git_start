#ifndef _IEC7816_H_
#define _IEC7816_H_

#include "Typedef.h"
#include "SysArch.h"

//错误码
#define NOERROR                                         0//成功
#define SENDTIMEOUT                                    -0xfe//发送超时
#define NOSW                                           -0xff//不明响应数据
#define LENWRONG                                       -1//数据长度错误
#define	RANDOMNOGET                                    -2//没有可用随机数
#define PUBKEYFILEWRITEWRONG                           -3//写公钥文件错误
#define	COMMANDWRONG                                   -4//指令结构错误
#define SM1KEYWRONG                                    -5//SM1密钥错误  
#define CHECKFILETYPENOMATCH                           -6//验签文件类型不匹配
#define CHECKFILENOFIND                                -7//验签文件未找到
#define PRIVATEKEYFILENOFIND                           -8//产生RSA密钥对时私钥文件未找到
#define PUBLICKEYFILEFORCIPHERNOMATCH                  -9//用来加密的公钥文件不匹配
#define PUBLICKEYFILEFORCIPHERNOFIND                   -10//用来加密的公钥文件没找到
#define PUBLICKEYFORDECIPHERNOMATCH                    -11//用来解密的公钥文件不匹配
#define PUBLICKEYFILEFORDECIPHERNOFIND                 -12//用来解密的公钥文件没找到
#define RSACIPHERWRONG                                 -13//RSA加密错误
#define RSADECIPHERWRONG                               -14//RSA解密错误
#define RSACHECKSIGNWRONG                              -15//RSA验签错误
#define RSAKEYPAIRWRONG                                -16//RSA产生密钥对错误
#define RSASIGNWRONG                                   -17//RSA签名错误
#define SM1DECIPHERDATAWRONG                           -18//SM1解密数据错误
#define LINEPROTECTIONWRONG                            -19//SM1解密数据错误
#define NOTHINGTOGET                                   -20//卡中无数据可返回
#define CLANOVALID                                     -21//无效的CLA
#define STATUSNOVALID                                  -22//无效的状态
#define P1NOPUBFILE                                    -23 //P1、P2所指的标识符不是响应的公钥文件/不支持此功能
#define RIGHTSNOMEET                                   -24//增加或修改权限不满足
#define KEYLOCKED                                      -25//密钥被锁死
#define KEYFILESPACEFULL                               -26//KEY文件空间已满
#define KEYNOFIND                                      -27//密钥未找到
#define KEYFILENOFIND                                  -28//KEY文件未找到

struct TIEC7816_TPDUHead//命令头
{
	BYTE CLA;
	BYTE INS;
	BYTE P1;
	BYTE P2;
	BYTE P3;
};

struct TIEC7816_TAPDU
{
	BYTE CLA;
	BYTE INS;
	BYTE P1;
	BYTE P2;
	BYTE Lc;
	BYTE Le;
	BYTE *pData;
};

struct TIEC7816_RAPDU
{
	union
	{
		WORD word;
		BYTE byte[2];
	} SW;
	BYTE *pResponse;
	BYTE Le;
};

typedef struct SWTable
{
	BYTE bSW1;
	BYTE bSW2;
	char *str;//状态说明性文字
}TSWTable;

typedef struct CIEC7816
{	
//	BYTE bSeris[8];//序列号 
	BYTE bRandomNumber[8];//当前随机数 
	bool fReseted;

    bool fGetChallenge;
	TSem  semEsam;
}TIEC7816;

//BYTE SendChar(BYTE bCharToSend);
//BYTE GetChar(BYTE *pCharToReceive);

//void GetATR(BYTE* pAtr, BYTE* pLength);
//void Decode_ATR(BYTE* pAtr);
//BYTE APDU(const struct TIEC7816_TAPDU *pCAPDU, struct TIEC7816_RAPDU *pRAPDU);
//BYTE TPDU(const unsigned char *pAPDU, unsigned char *pMessage, unsigned short wLength );
int CheckSW2(int iCnt,BYTE *pbBuf,BYTE bSW1,BYTE bSW2);
int CheckSW1(int iCnt,BYTE *pbBuf);

//读取终端序列号
//成功 返回序列号的长度 失败错误码
int GetESAMSeris(BYTE *pbBuf,DWORD dwTimeout);
//读取终端当前随机数  
//成功 返回随机数的长度 失败-1
int GetESAMRandomNumber(BYTE *pbBuf,DWORD dwTimeout);

//读取终端新的随机数  重新发起读随机数命令
//bLen 期望的随机数长度
// pbBuf ESAM返回的数据数
//成功 返回随机数的长度 失败-1
int GetChallenge(BYTE bLen,BYTE *pbBuf,DWORD dwTimeout);
//主控公钥更新
// 主控公钥和长度 pbKey bKeyLen 144字节
// 数据签名和长度pbSign bSignLen 128字节
// 成功  返回0 否则返回相应的错误码
int UpdateESAMKey(BYTE *pbKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout);
//读取终端的响应数据
//bLen 期望的响应数据长度
// pbBuf ESAM返回的响应数据
// 成功  返回响应数据的长度 否则返回相应的错误码
int GetResponse(BYTE bLen,BYTE *pbBuf,DWORD dwTimeout);
//MAC校验  
//bAFN--功能码
// bA3--组地址标志
// pbTermAddr--终端地址
// pbGrpAddr--8个组地址,每个2字节
// pbData--终端收到的明文数据
// pbMacData--终端收到的MAC信息
// wDataLen--终端明文数据长度
// 成功  返回MAC的长度 否则返回相应的错误码负数
int VerifyMac(BYTE bAFN, BYTE bA3, BYTE bGrpAddr, BYTE *pbData, BYTE *pbMacData, WORD wDataLen, DWORD dwTimeout);

//公钥验证
//pbRandom 主站随机数8
// bP2 要验证公钥文件的文件标识
// pbSign 数据签名128
// 成功  返回0 否则返回相应的错误码
int VerifyPublicKey(BYTE bP2,BYTE *pbSign,BYTE *pbMastRandom,DWORD dwTimeout);
//主站公钥本地更新
// 主站公钥和长度 pbKey bKeyLen 144字节
// 数据签名和长度pbSign bSignLen 128字节
// pbRandom 终端随机数
// 成功  返回0 否则返回相应的错误码
int UpdateMKeyLocal(BYTE *pbKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout);
//主站公钥远程更新
// pbSessionKey  会话密钥 128
// 主站公钥和长度 pbMKey bKeyLen 144字节
// 数据签名和长度pbSign bSignLen 128字节
// pbRandom 终端随机数
// 成功  返回0 否则返回相应的错误码
int UpdateMKeyFar(BYTE *pbSessionKey,BYTE *pbMKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout);
//对称密钥的更新  返回0--注册成功 其他--错误码
//bNum:更新密钥条数
// pbSign:数据签名128
// pbSessionKey 会话密钥128
// 成功  返回0 否则返回相应的错误码
int UpdateSymKey(BYTE bNum,BYTE *pbSessionKey,BYTE *pbTermKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout);
//非对称密钥的注册  返回0--注册成功 其他--错误码
//bP1:01/02 
// pbRandom: 主站随机数
// bRamdomLen: 主站随机数长度 8字节
// pbSign:数据签名
// bSignLen:数据签名长度 128字节
// pbBuf:注册成功后返回的非对称密钥公钥 256字节
// 成功  返回密钥的长度 否则返回相应的错误码
int RegisterNonSymKey(BYTE bP1,BYTE *pbMastRandom,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout,BYTE *pbBuf);

//非对称密钥的更新  返回0--注册成功 其他--错误码
//bP1:01/02
// pbRandom: 主站随机数8
// pbSign:数据签名128
// pbSessionKey 会话密钥128
// pbBuf:注册成功后返回的非对称密钥公钥 256字节
// 成功  返回密钥的长度 否则返回相应的错误码
int UpdateNonSymKey(BYTE bP1,BYTE *pbSessionKey,BYTE *pbMastRandom,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout,BYTE *pbBuf);

bool InitESAM(void);
void ResetESAM(void);

extern TIEC7816 g_ESAM;
/////////////////////////////////

#endif /* _IEC7816_H_ */
