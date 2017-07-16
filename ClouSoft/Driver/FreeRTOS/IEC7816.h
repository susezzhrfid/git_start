#ifndef _IEC7816_H_
#define _IEC7816_H_

#include "Typedef.h"
#include "SysArch.h"

//������
#define NOERROR                                         0//�ɹ�
#define SENDTIMEOUT                                    -0xfe//���ͳ�ʱ
#define NOSW                                           -0xff//������Ӧ����
#define LENWRONG                                       -1//���ݳ��ȴ���
#define	RANDOMNOGET                                    -2//û�п��������
#define PUBKEYFILEWRITEWRONG                           -3//д��Կ�ļ�����
#define	COMMANDWRONG                                   -4//ָ��ṹ����
#define SM1KEYWRONG                                    -5//SM1��Կ����  
#define CHECKFILETYPENOMATCH                           -6//��ǩ�ļ����Ͳ�ƥ��
#define CHECKFILENOFIND                                -7//��ǩ�ļ�δ�ҵ�
#define PRIVATEKEYFILENOFIND                           -8//����RSA��Կ��ʱ˽Կ�ļ�δ�ҵ�
#define PUBLICKEYFILEFORCIPHERNOMATCH                  -9//�������ܵĹ�Կ�ļ���ƥ��
#define PUBLICKEYFILEFORCIPHERNOFIND                   -10//�������ܵĹ�Կ�ļ�û�ҵ�
#define PUBLICKEYFORDECIPHERNOMATCH                    -11//�������ܵĹ�Կ�ļ���ƥ��
#define PUBLICKEYFILEFORDECIPHERNOFIND                 -12//�������ܵĹ�Կ�ļ�û�ҵ�
#define RSACIPHERWRONG                                 -13//RSA���ܴ���
#define RSADECIPHERWRONG                               -14//RSA���ܴ���
#define RSACHECKSIGNWRONG                              -15//RSA��ǩ����
#define RSAKEYPAIRWRONG                                -16//RSA������Կ�Դ���
#define RSASIGNWRONG                                   -17//RSAǩ������
#define SM1DECIPHERDATAWRONG                           -18//SM1�������ݴ���
#define LINEPROTECTIONWRONG                            -19//SM1�������ݴ���
#define NOTHINGTOGET                                   -20//���������ݿɷ���
#define CLANOVALID                                     -21//��Ч��CLA
#define STATUSNOVALID                                  -22//��Ч��״̬
#define P1NOPUBFILE                                    -23 //P1��P2��ָ�ı�ʶ��������Ӧ�Ĺ�Կ�ļ�/��֧�ִ˹���
#define RIGHTSNOMEET                                   -24//���ӻ��޸�Ȩ�޲�����
#define KEYLOCKED                                      -25//��Կ������
#define KEYFILESPACEFULL                               -26//KEY�ļ��ռ�����
#define KEYNOFIND                                      -27//��Կδ�ҵ�
#define KEYFILENOFIND                                  -28//KEY�ļ�δ�ҵ�

struct TIEC7816_TPDUHead//����ͷ
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
	char *str;//״̬˵��������
}TSWTable;

typedef struct CIEC7816
{	
//	BYTE bSeris[8];//���к� 
	BYTE bRandomNumber[8];//��ǰ����� 
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

//��ȡ�ն����к�
//�ɹ� �������кŵĳ��� ʧ�ܴ�����
int GetESAMSeris(BYTE *pbBuf,DWORD dwTimeout);
//��ȡ�ն˵�ǰ�����  
//�ɹ� ����������ĳ��� ʧ��-1
int GetESAMRandomNumber(BYTE *pbBuf,DWORD dwTimeout);

//��ȡ�ն��µ������  ���·�������������
//bLen ���������������
// pbBuf ESAM���ص�������
//�ɹ� ����������ĳ��� ʧ��-1
int GetChallenge(BYTE bLen,BYTE *pbBuf,DWORD dwTimeout);
//���ع�Կ����
// ���ع�Կ�ͳ��� pbKey bKeyLen 144�ֽ�
// ����ǩ���ͳ���pbSign bSignLen 128�ֽ�
// �ɹ�  ����0 ���򷵻���Ӧ�Ĵ�����
int UpdateESAMKey(BYTE *pbKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout);
//��ȡ�ն˵���Ӧ����
//bLen ��������Ӧ���ݳ���
// pbBuf ESAM���ص���Ӧ����
// �ɹ�  ������Ӧ���ݵĳ��� ���򷵻���Ӧ�Ĵ�����
int GetResponse(BYTE bLen,BYTE *pbBuf,DWORD dwTimeout);
//MACУ��  
//bAFN--������
// bA3--���ַ��־
// pbTermAddr--�ն˵�ַ
// pbGrpAddr--8�����ַ,ÿ��2�ֽ�
// pbData--�ն��յ�����������
// pbMacData--�ն��յ���MAC��Ϣ
// wDataLen--�ն��������ݳ���
// �ɹ�  ����MAC�ĳ��� ���򷵻���Ӧ�Ĵ����븺��
int VerifyMac(BYTE bAFN, BYTE bA3, BYTE bGrpAddr, BYTE *pbData, BYTE *pbMacData, WORD wDataLen, DWORD dwTimeout);

//��Կ��֤
//pbRandom ��վ�����8
// bP2 Ҫ��֤��Կ�ļ����ļ���ʶ
// pbSign ����ǩ��128
// �ɹ�  ����0 ���򷵻���Ӧ�Ĵ�����
int VerifyPublicKey(BYTE bP2,BYTE *pbSign,BYTE *pbMastRandom,DWORD dwTimeout);
//��վ��Կ���ظ���
// ��վ��Կ�ͳ��� pbKey bKeyLen 144�ֽ�
// ����ǩ���ͳ���pbSign bSignLen 128�ֽ�
// pbRandom �ն������
// �ɹ�  ����0 ���򷵻���Ӧ�Ĵ�����
int UpdateMKeyLocal(BYTE *pbKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout);
//��վ��ԿԶ�̸���
// pbSessionKey  �Ự��Կ 128
// ��վ��Կ�ͳ��� pbMKey bKeyLen 144�ֽ�
// ����ǩ���ͳ���pbSign bSignLen 128�ֽ�
// pbRandom �ն������
// �ɹ�  ����0 ���򷵻���Ӧ�Ĵ�����
int UpdateMKeyFar(BYTE *pbSessionKey,BYTE *pbMKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout);
//�Գ���Կ�ĸ���  ����0--ע��ɹ� ����--������
//bNum:������Կ����
// pbSign:����ǩ��128
// pbSessionKey �Ự��Կ128
// �ɹ�  ����0 ���򷵻���Ӧ�Ĵ�����
int UpdateSymKey(BYTE bNum,BYTE *pbSessionKey,BYTE *pbTermKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout);
//�ǶԳ���Կ��ע��  ����0--ע��ɹ� ����--������
//bP1:01/02 
// pbRandom: ��վ�����
// bRamdomLen: ��վ��������� 8�ֽ�
// pbSign:����ǩ��
// bSignLen:����ǩ������ 128�ֽ�
// pbBuf:ע��ɹ��󷵻صķǶԳ���Կ��Կ 256�ֽ�
// �ɹ�  ������Կ�ĳ��� ���򷵻���Ӧ�Ĵ�����
int RegisterNonSymKey(BYTE bP1,BYTE *pbMastRandom,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout,BYTE *pbBuf);

//�ǶԳ���Կ�ĸ���  ����0--ע��ɹ� ����--������
//bP1:01/02
// pbRandom: ��վ�����8
// pbSign:����ǩ��128
// pbSessionKey �Ự��Կ128
// pbBuf:ע��ɹ��󷵻صķǶԳ���Կ��Կ 256�ֽ�
// �ɹ�  ������Կ�ĳ��� ���򷵻���Ӧ�Ĵ�����
int UpdateNonSymKey(BYTE bP1,BYTE *pbSessionKey,BYTE *pbMastRandom,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout,BYTE *pbBuf);

bool InitESAM(void);
void ResetESAM(void);

extern TIEC7816 g_ESAM;
/////////////////////////////////

#endif /* _IEC7816_H_ */
