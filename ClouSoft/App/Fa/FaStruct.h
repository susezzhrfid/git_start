#ifndef FASTRUCT_H
#define FASTRUCT_H

#include "ComStruct.h"
//#include "DbStruct.h"
#include "FaConst.h"
#include "ThreadMonitor.h"
#include "StatMgr.h"
#include "DbCfg.h"

typedef struct{
	DWORD time;			//����ʱ��
	BYTE bVerInfo[9];	//�汾��Ϣ
}TSoftVerChg;		//����İ汾����¼�
/*
typedef struct {
	WORD wYxFlag;	//��Ч��־λ,ĳλ��1��ʾ��λ��Ч
	WORD wYxPolar;
}TYxPara;	//ң�Ų���
*/
typedef struct{
	BYTE bVer;              //�汾,���BYTE�ֽڣ����ⷢ���������� 
	bool fTmpValid;         //�����ݴ������Ч��־
	bool fGPRSConected;     //GPRS������
	BYTE bRemoteDownIP[8];  //Զ����������ķ�����IP��ַ
    WORD wLocalPort;        //�ն˱��ض˿ںţ�ÿ��ʹ��������4097-9200
	bool fAlrPowerOff;		//����ǰ�ϱ���ͣ��澯
	WORD wRstNum;			//��λ����
	WORD wMonitorRstNum;	//�̼߳�ظ�λ����
	char szMonitorRstThrd[THRD_NAME_LEN];	//�̼߳�ظ�λ���һ�θ�λ���߳�����
	short iRemoteDownPort;
	TTime tPowerOn;        //�ϴ�����ʱ��
	TTime tPoweroff;        //�ϴ�ͣ��ʱ��
	BYTE  bParaEvtType;		//��¼����ǰ������ʼ���¼������ͣ�0��Ч 1��Ҫ 2һ��
	TSoftVerChg ParaInit;	//������ʼ���¼�
    BYTE bAcAvrVolt[30];  //����ƽ����ѹ����,ÿ�����ֽ�Ϊһ����BCD��
    TTermStat tTermStat;	//�ն�ͳ�ƣ��������¹���ʱ�䡢��λ����������
	TVoltStat tDayVoltStat;	//�յ�ѹͳ��
    TVoltStat tMonVoltStat; //�µ�ѹͳ��
	BYTE bSect2Data[SECT2_DATA_SZ];	//ϵͳ����Ҫ����ʱ����ı�����
									//��Щ����ƽ��д�ñȽ϶࣬����ÿ�ζ�ֱ�ӱ��浽FLASH
	BYTE bBatTaskInfo[5]; // ���籣��Ķ�ʱ�������Ϣ    
}TPowerOffTmp;   //�����ݴ����

#endif //FASTRUCT_H

