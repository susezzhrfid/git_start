#ifndef ESAM_H
#define ESAM_H

#include "Typedef.h"

void esam_init(void);
void esam_warm_reset(void);
int esam_write(BYTE * buffer, int count);
int esam_read(BYTE *buf, int count);
int esam_read2(BYTE *buf, int count);

#endif