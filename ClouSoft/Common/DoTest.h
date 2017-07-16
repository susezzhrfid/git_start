#ifndef DOTEST_H
#define DOTEST_H

#include "Typedef.h"
#include "Pro.h"

#define TEST_FLAG       0x14253647

void DoTest(struct TPro* pPro, BYTE *pbBuf, WORD wLen);

#endif