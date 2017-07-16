/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�TaskConst.h
 * ժ    Ҫ�����ļ���Ҫ��������������
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2009��9��
 *********************************************************************************************************/
#ifndef TASKCONST_H
#define TASKCONST_H


#define SURVEY_INTERV 		15

#define COMTASK_REC_HEAD_LEN	7

//��ͨ����Ĳ�������������
#define TASK_PN_TYPE_P0		0x01
#define TASK_PN_TYPE_AC		0x02
#define TASK_PN_TYPE_MTR	0x04
#define TASK_PN_TYPE_PULSE	0x08
#define TASK_PN_TYPE_DC		0x10
#define TASK_PN_TYPE_GRP	0x20

//��ͨ����Ķ�����������
#define TASK_FRZ_PROP_MTR	0x01	//���������
#define TASK_FRZ_PROP_TEM	0x02	//�ն�������

#define TASK_PN_TYPE_MSR	(TASK_PN_TYPE_AC|TASK_PN_TYPE_MTR|TASK_PN_TYPE_PULSE) //������

#define REC_TIME_LEN   5

#define CCT_COMTASK_NUM_MAX		64		//�������֧��������
#define CCT_COMTASK_NUM			32		//�����������

//��������ID����
#define CCT_TASK_VIP		0		//�ص㻧����
#define CCT_TASK_DAYFRZ		4		//������׼�ն��������

#define PNCHAR_MAX			16		//���֧�ֵĲ����������ָ���
#define PNCHAR_ALL			0		//���в�����������,���ڲ����������ָ����ֽ�,������ʾ�����������
#define PNCHAR_VIP			0xff	//�ص㻧������������,���ڲ����������ָ����ֽ�,������ʾ�����������
			
//�����궨��
#define PN_TIMEOUT	-2
#define PN_FAIL		-1
#define PN_BUSY		0
#define PN_SUCCESS	1
#endif //TASKCONST_H

