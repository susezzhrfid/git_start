#ifndef SYSDEBUG_H
#define SYSDEBUG_H
//#include "Typedef.h"
#include "trace.h"

extern bool IsDebugOn(BYTE bType);

//描述：初始化调试输出功能
//参数：无
//返回：true-成功,false-失败
bool SysDebugInit();
int UsrSprintf(BYTE* pbBuf, const char *fmt, ...);
//描述：输出调试信息（应用程序不直接使用本接口）
//参数：输出内容
//返回：无
//void DebugPrintf(const char *fmt, ...);
//void DebugPrintf(char *fmt, ...);
void DebugPrintf(const char *fmt, ...);
//描述：输出调试信息
//参数：@debug - 调试开关
//      @x - 调试信息内容
//返回：无 加入这个功能需要耗用866字节的代码空间
#define DTRACE(debug, x) do { if (IsDebugOn(debug)){ DebugPrintf x; }} while(0)

//描述：输出调试信息
//参数：@debug - 调试开关
//      @s - 调试信息内容
//      @len - 调试信息长度
//返回：无
#define STRACE(debug, s, len) do { if (IsDebugOn(debug)){ DebugPrintf(s, len); }} while(0)

WORD PrintBuf(BYTE* out, WORD wOutLen, BYTE* in, WORD wInLen);
void TraceBuf(WORD wSwitch, char* szHeadStr, BYTE* p, WORD wLen);
void TraceFrm(char* pszHeader, BYTE* pbBuf, WORD wLen);

#endif
