#ifndef SYSDEBUG_H
#define SYSDEBUG_H
//#include "Typedef.h"
#include "trace.h"

extern bool IsDebugOn(BYTE bType);

//��������ʼ�������������
//��������
//���أ�true-�ɹ�,false-ʧ��
bool SysDebugInit();
int UsrSprintf(BYTE* pbBuf, const char *fmt, ...);
//���������������Ϣ��Ӧ�ó���ֱ��ʹ�ñ��ӿڣ�
//�������������
//���أ���
//void DebugPrintf(const char *fmt, ...);
//void DebugPrintf(char *fmt, ...);
void DebugPrintf(const char *fmt, ...);
//���������������Ϣ
//������@debug - ���Կ���
//      @x - ������Ϣ����
//���أ��� �������������Ҫ����866�ֽڵĴ���ռ�
#define DTRACE(debug, x) do { if (IsDebugOn(debug)){ DebugPrintf x; }} while(0)

//���������������Ϣ
//������@debug - ���Կ���
//      @s - ������Ϣ����
//      @len - ������Ϣ����
//���أ���
#define STRACE(debug, s, len) do { if (IsDebugOn(debug)){ DebugPrintf(s, len); }} while(0)

WORD PrintBuf(BYTE* out, WORD wOutLen, BYTE* in, WORD wInLen);
void TraceBuf(WORD wSwitch, char* szHeadStr, BYTE* p, WORD wLen);
void TraceFrm(char* pszHeader, BYTE* pbBuf, WORD wLen);

#endif
