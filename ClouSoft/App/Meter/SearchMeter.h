#ifndef SEARCHMETER_H
#define SEARCHMETER_H

#include "Typedef.h"

#define STACK_SIZE  7
#define MTR_PORT_NUM	3

#define PRO97METORNOT    0
#define PRO97            1
#define PRO07METORNOT    2
#define PRO07            3
#define PRONJSLMETORNOT  4
#define PRONJSL          5
#define SEARCHOVER       6
#define SEARCHWAIT       7

#define DISABLE_DATA    0xff

#define M97MET   0x01
#define M07MET   0x02
#define MNJSLMET 0x04

#define SEARCH_UNDOEN   0
#define SEARCH_OVER     1
#define SEARCH_ERROR    2

typedef struct
{
    WORD wBaud;
    BYTE bProto;
}TMeterPro;

typedef struct
{
    //BYTE bPn;      //测量点号
    BYTE bEn;      //测量点有效
    BYTE bAddr[6]; //测量点地址  
    BYTE bBaud;
    BYTE bProto;
	BYTE bPort;
//    DWORD dwTime;  //
}TMeterAddrTab;     

typedef struct
{
    BYTE bData[STACK_SIZE];
    int iTop;  //先压栈后移动，先移动后出栈。
}TStack;

typedef struct
{
	BYTE bFinish;
	BYTE bCurTry; 
	BYTE bCurTryLevel;

	BYTE bSearchState;
	BYTE bAddrPatten[6];

	TStack tStack ;
}TMtrSchInf;//搜表的记录变量

extern TMtrSchInf g_tMtrRdSchInf[MTR_PORT_NUM];
void SaveSchMtrStaInfo();
void ReinitSearch(BYTE bPort);
void InitSearch(BYTE bPort, BYTE bStartSer);
void DoSearch(BYTE bPort);
void StartSearch(BYTE bPort);
void StopSearch(BYTE bPort);

#endif
