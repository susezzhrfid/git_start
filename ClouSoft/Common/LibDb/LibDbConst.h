/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�LibDbConst.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֱ�׼ϵͳ��ĳ�������
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2011��3��
 *********************************************************************************************************/
#ifndef LIBDBCONST_H
#define LIBDBCONST_H
#include "LibDbCfg.h"


//#define AUTO_COMPUTE	1
#define INVALID_TIME 	0	//��Ч��ʱ��

#define BANK0        0
#define BANK1        1
#define BANK2        2
#define BANK3		 3
#define BANK4        4
#define BANK5		 5
#define BANK6		 6
#define BANK7        7
#define BANK8		 8
#define BANK9		 9
#define BANK10       10
#define BANK11		 11
#define BANK12		 12
#define BANK13		 13
#define BANK14		 14
#define BANK15		 15
#define BANK16		 16
#define BANK17		 17
#define BANK18		 18
#define BANK19		 19
#define BANK20		 20
#define BANK21		 21
#define BANK22		 22
#define BANK23		 23
#define BANK24		 24
#define BANK25		 25

#define BN0     0
#define BN1     1
#define BN2     2
#define BN3		3
#define BN4     4
#define BN5		5
#define BN6		6
#define BN7     7
#define BN8		8
#define BN9		9
#define BN10    10
#define BN11	11
#define BN12	12
#define BN13	13
#define BN14	14
#define BN15	15
#define BN16	16
#define BN17	17
#define BN18	18
#define BN19	19
#define BN20	20
#define BN21	21
#define BN22	22
#define BN23	23
#define BN24	24
#define BN25	25
#define BN26	26
#define BN27	27
#define BN28	28
#define BN29	29
#define BN30	30

#define POINT0       0
#define POINT1       1

#define PN0       0
#define PN1       1
#define PN2       2
#define PN3       3
#define PN4       4
#define PN5       5
#define PN6       6
#define PN7       7
#define PN8       8
#define PN9       9
#define PN10      10
#define PN11      11
#define PN12      12
#define PN13      13
#define PN14      14
#define PN15      15
#define PN16      16
#define PN17      17
#define PN18      18
#define PN19      19
#define PN20      20
#define PN21      21

#define IMG0		0

#define SECT0	0
#define SECT1	1
#define SECT2	2
#define SECT3	3
#define SECT4	4
#define SECT5	5
#define SECT6	6
#define SECT7	7
#define SECT8	8
#define SECT9	9
#define SECT10	10

#define BN_VER_LEN		20	//BANK���ݰ汾���Ʊ�����ֽڳ���

//�����㶯̬ӳ��
#define PNUNMAP			0	//�����㲻����ӳ��
#define PNMAP1			1	//������ӳ�䷽��1

//#define PNMAP_PATHNAME	USER_PARA_PATH"PNMAP%d.cfg"
#define UPG_LOG_LEN		32

//һЩ�������,һ������²��ø�
#define SECT_MAX		    32
#define BANK_MAX		    32
#define BANK_FILE_MAX		200							//һ��BANK���֧�ֵ��ļ�����128
#define BANK_FILE_FLG_SIZE	((BANK_FILE_MAX+7)/8)		//һ��BANK�ļ���־λ����Ĵ�С
#define PNMAP_MAX			32							//���֧�ֵ�ӳ�䷽����

#define DBBUF_SIZE			4096	//1024

#define DYN_PN_NUM			2		//��̬�洢������ĸ���
#define INVALID_DYN_PN		0xff	//��Ч�Ķ�̬�������
#endif //LIBDBCONST_H
