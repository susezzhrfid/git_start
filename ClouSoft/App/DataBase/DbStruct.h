/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbStruct.h
 * ժ    Ҫ�����ļ���Ҫ����������汾��������ݽṹ
 * ��ǰ�汾��1.1
 * ��    �ߣ�᯼���
 * ������ڣ�2009��2��
 *********************************************************************************************************/
#ifndef DBSTRUCT_H
#define DBSTRUCT_H
#include "SysArch.h"
#include "LibDbStruct.h"
#include "TypeDef.h"

typedef struct{
	BYTE  bValid:1;        				//��Ч�ı�־  
	BYTE  bNotFix:7;					//�����Ƿ�̶��ı�־
	BYTE  bBank;        				//��ӦBank��ʶ    
    WORD  wID;         					//��ӦID��ʶ  
	BYTE  bType;						//PN����  
	WORD  wPNMax;						//PN����  
}TConvertFNIDDesc;


#endif //DBSTRUCT_H