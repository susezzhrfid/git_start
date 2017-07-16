#ifndef LCD_H
#define LCD_H

#ifdef EN_LCD
#include "Typedef.h"

typedef struct
{
    WORD wAscii;     
    BYTE bDat[32];   //��ģ
}ThzItem;

typedef struct
{
    BYTE xpos;  //�ַ���ǰx����
    BYTE ypos;
    
    BYTE xp;    //��ͼ��ǰx����
    BYTE yp;
    
    BYTE lcdmap[128][2];   //todo:ע��Ϊ�˽�Լ�ڴ棬ֻ������ǰ��������ʾ���ݣ���Ϊ��������Ҫ��ͼ���ַ������
}TLcd;

void InitLcd(void);
void WriteCmd(BYTE bCmd);
void WriteData(BYTE bData);
void WriteDat(BYTE bDat, BYTE x, BYTE y, int WriteToMap);

void LcdOpen(void);
void LcdClose(void);
void LcdReset(void);
void LcdBlightOn(bool fOn);
void Refresh(void);
void LcdAllPointOn(bool fOn);

//void tinsharptest(BYTE k, BYTE l);
//void LcmDisplayBmp(const BYTE *puts );

void WriteChar(const BYTE *dat, BYTE x, BYTE y, bool reverse, int WriteToMap);
void WriteChar2(const BYTE *dat, BYTE x, BYTE y, bool reverse, int WriteToMap);
void WriteChchr(const BYTE *dat, BYTE x, BYTE y, bool reverse, int WriteToMap);

void Print(char *str,short x,short y,bool reverse, int WriteToMap);
void Show(short x, short y, bool reverse, const char *fmt, ...);

void ClearAll(bool reverse, bool map);
void Clear(BYTE x, BYTE y, bool reverse, bool map);
void ClearAChr(BYTE x, BYTE y, bool reverse, bool map);
void ClearAChChr(BYTE x, BYTE y, bool reverse, bool map);
void ClearLine(BYTE line, bool reverse, bool map);

bool Setx(BYTE x);
bool Sety(BYTE y);
bool Setxy(BYTE x,BYTE y);
bool Point(BYTE x,BYTE y,BYTE color);
bool Line(BYTE x1,BYTE y1,BYTE x2,BYTE y2,BYTE color);
void Rectangle(BYTE x1,BYTE y1,BYTE x2,BYTE y2,BYTE color);
void Circle(BYTE x,BYTE y,BYTE r,BYTE color);

#endif
#endif