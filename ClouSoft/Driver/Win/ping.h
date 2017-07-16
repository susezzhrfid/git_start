#ifndef __PING_H__
#define __PING_H__
#include "TypeDef.h"

void CheckEthernet(void);
bool GetEthPingResult(void);
BYTE GetEthPingFailCnt(void);
void UpdPingTime(void);
bool DhcpGetIpOver(void);

#endif /* __PING_H__ */
