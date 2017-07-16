/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbFmt.h
 * 摘    要：本文件主要实现数据库中数据项的值到格式、格式到值的转换接口
 * 当前版本：1.0
 * 作    者：岑坚宇、杨凡
 * 完成日期：2008年3月
 *********************************************************************************************************/
#include "TypeDef.h"
#include "FaCfg.h"
#include "ComStruct.h"

/////////////////////////////////////////////////////////////////////////
//对外提供的接口函数
//int ValToFmt(int iVal, BYTE* pbBuf, BYTE bFmt, WORD wLen);
int Val64ToFmt(int64 iVal, BYTE* pbBuf, BYTE bFmt, WORD wLen);

//WORD ValToFmt(int* piVal, BYTE* pbBuf, const BYTE* pbFmtStr, WORD* pwValNum);
//WORD Val64ToFmt(int64 *piVal64, BYTE *pbBuf, const BYTE *pbFmtStr, WORD* pwValNum);

//int FmtToVal(BYTE* pbBuf, BYTE bFmt, WORD wLen);
//int64 FmtToVal64(BYTE* pbBuf, BYTE bFmt, WORD wLen);

//WORD FmtToVal(BYTE* pbBuf, int* piVal, const BYTE* pbFmtStr, WORD* pwValNum);
//WORD FmtToVal64(BYTE* pbBuf, int64* piVal64, const BYTE* pbFmtStr, WORD* pwValNum);

int TimeToBcd(TTime* pTm, BYTE* pbBuf);
void BcdToTime(BYTE* pbBuf, TTime* pTm);
int TimeToFmt(TTime* pTm, BYTE* pbBuf, BYTE bFmt);
int MinToFmt(DWORD min, BYTE* pbBuf, BYTE bFmt);

/////////////////////////////////////////////////////////////////////////
//内部使用的函数
int BinToVal32(BYTE* pbBuf, WORD  wLen);
int64 Fmt2ToVal64(BYTE* pbBuf, WORD wLen);
int Fmt2ToVal(BYTE* pbBuf, WORD wLen);
int64 Fmt3ToVal64(BYTE* pbBuf, WORD wLen);
int Fmt3ToVal(BYTE* pbBuf, WORD wLen);
int64 Fmt4ToVal64(BYTE* pbBuf, WORD wLen);
int Fmt4ToVal(BYTE* pbBuf, WORD wLen);
int Fmt5ToVal(BYTE* pbBuf, WORD wLen);
int64 Fmt5ToVal64(BYTE* pbBuf, WORD wLen);
int Fmt6ToVal(BYTE* pbBuf, WORD wLen);
int64 Fmt6ToVal64(BYTE* pbBuf, WORD wLen);
int Fmt7ToVal(BYTE* pbBuf, WORD wLen);
int64 Fmt7ToVal64(BYTE* pbBuf, WORD wLen);
int Fmt8ToVal(BYTE* pbBuf, WORD wLen);
int64 Fmt8ToVal64(BYTE* pbBuf,WORD wLen);
int Fmt9ToVal(BYTE* pbBuf, WORD wLen);
int64 Fmt9ToVal64(BYTE* pbBuf, WORD wLen);
int Fmt10ToVal(BYTE* pbBuf, WORD wLen);
int64 Fmt10ToVal64(BYTE* pbBuf, WORD wLen);
int Fmt11ToVal(BYTE* pbBuf, WORD wLen);
int64 Fmt11ToVal64(BYTE* pbBuf, WORD wLen);
int Fmt12ToVal(BYTE* pbBuf, WORD wLen);
int64 Fmt12ToVal64(BYTE* pbBuf, WORD wLen);
int Fmt13ToVal(BYTE* pbBuf, WORD wLen);
int64 Fmt13ToVal64(BYTE* pbBuf, WORD wLen);
int Fmt14ToVal(BYTE* pbBuf, WORD wLen);
int64 Fmt14ToVal64(BYTE* pbBuf, WORD wLen);
int Fmt22ToVal(BYTE* pbBuf,WORD wLen);
int64 Fmt22ToVal64(BYTE* pbBuf, WORD wLen);
int Fmt23ToVal(BYTE* pbBuf, WORD wLen);
int64 Fmt23ToVal64(BYTE* pbBuf, WORD wLen);
int Fmt25ToVal(BYTE* pbBuf, WORD wLen);
int64 Fmt25ToVal64(BYTE* pbBuf, WORD wLen);

void ValToFmt2(int val, BYTE* pbBuf, WORD wLen);
void ValToFmt3(int val, BYTE* pbBuf, WORD wLen);
void ValToFmt4(int val, BYTE* pbBuf, WORD wLen);
void ValToFmt5(int val, BYTE* pbBuf, WORD wLen);
void ValToFmt6(int val, BYTE* pbBuf, WORD wLen);
void ValToFmt7(int val, BYTE* pbBuf, WORD wLen);
void ValToFmt8(int val, BYTE* pbBuf, WORD wLen);
void ValToFmt9(int val, BYTE* pbBuf, WORD wLen);
void ValToFmt10(int val, BYTE* pbBuf, WORD wLen);
void ValToFmt11(int val, BYTE* pbBuf, WORD wLen);
void ValToFmt12(int val, BYTE* pbBuf, WORD wLen);
void ValToFmt13(int val, BYTE* pbBuf, WORD wLen);
void ValToFmt14(int val, BYTE* pbBuf, WORD wLen);
void ValToFmt22(int val, BYTE* pbBuf, WORD wLen);
void ValToFmt25(int val, BYTE* pbBuf, WORD wLen);
void ValToUnknownFmt(int val, BYTE* pbBuf, WORD wLen);
void ValToBin(int val, BYTE* pbBuf, WORD wLen);
void Val64ToINT64(int64 val64, BYTE* pbBuf, WORD wLen);
void Val64ToBin4(int64 val64, BYTE* pbBuf, WORD wLen);
void ValToBin2(int val, BYTE* pbBuf, WORD wLen);

void Val64ToFmt2(int64 val, BYTE* pbBuf, WORD wLen);
void Val64ToFmt3(int64 val, BYTE* pbBuf, WORD wLen);
void Val64ToFmt4(int64 val, BYTE* pbBuf, WORD wLen);
void Val64ToFmt5(int64 val, BYTE* pbBuf, WORD wLen);
void Val64ToFmt6(int64 val, BYTE* pbBuf, WORD wLen);
void Val64ToFmt7(int64 val, BYTE* pbBuf, WORD wLen);
void Val64ToFmt8(int64 val, BYTE* pbBuf, WORD wLen);
void Val64ToFmt9(int64 val, BYTE* pbBuf, WORD wLen);
void Val64ToFmt10(int64 val, BYTE* pbBuf, WORD wLen);
void Val64ToFmt11(int64 val, BYTE* pbBuf, WORD wLen);
void Val64ToFmt12(int64 val, BYTE* pbBuf, WORD wLen);
void Val64ToFmt13(int64 val, BYTE* pbBuf, WORD wLen);
void Val64ToFmt14(int64 val, BYTE* pbBuf, WORD wLen);
void Val64ToFmt22(int64 val, BYTE* pbBuf, WORD wLen);
void Val64ToFmt23(int64 val, BYTE* pbBuf, WORD wLen);

int TimeToFmt1(TTime* pTm,BYTE* pbBuf);
int TimeToFmt15(TTime* pTm, BYTE* pbBuf);
int TimeToFmt16(TTime* pTm, BYTE* pbBuf);
int TimeToFmt17(TTime* pTm, BYTE* pbBuf);
int TimeToFmt18(TTime* pTm, BYTE* pbBuf);
int TimeToFmt19(TTime* pTm, BYTE* pbBuf);
int TimeToFmt20(TTime* pTm, BYTE* pbBuf);
int TimeToFmt21(TTime* pTm, BYTE* pbBuf);

int Fmt1ToTime(BYTE* pbBuf, TTime* pTm);
int Fmt15ToTime(BYTE* pbBuf, TTime* pTm);
int Fmt17ToTime(BYTE* pbBuf, TTime* pTm);
int Fmt20ToTime(BYTE* pbBuf, TTime* pTm);
int Fmt21ToTime(BYTE* pbBuf, TTime* pTm);
void Val64ToBCD(int64 val, BYTE* bcd, WORD len);

int TimeTo6Bcd(TTime* pTm, BYTE* pbBuf);
int TimeTo4Bcd(TTime* pTm, BYTE* pbBuf);
int TimeTo3Bcd(TTime* pTm, BYTE* pbBuf);
int TimeTo2Bcd(TTime* pTm, BYTE* pbBuf);

#ifdef PRO_GB2005
	#define FmtCurToVal		Fmt6ToVal
#else	//PRO_698
	#define FmtCurToVal		Fmt25ToVal
#endif

void Val64ToFmt30(int64 val, BYTE* pbBuf, WORD wLen);
int64 Fmt30ToVal64(BYTE* pbBuf, WORD wLen);


