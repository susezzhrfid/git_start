/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbStruct.h
 * ժ    Ҫ�����ļ���Ҫʵ�����ݿ�ĳ�������
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
 *********************************************************************************************************/
#ifndef DBCONST_H
#define DBCONST_H
#include "FaCfg.h"
#include "LibDbConst.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//���ݿ��׼��������
#define POINT_NUM     	  97			//������ר��
#define CCT_POINT_NUM     32            //�ز�������
#define PN_MASK_SIZE  ((POINT_NUM+7)/8)			//����������λ����Ĵ�С
//#define PN_MASK_SIZE  ((PN_NUM+7)/8)	//����������λ����Ĵ�С
#define PN_NUM     	  	 97		//����,���,�����,�������ɼ���ʹ��
#define REAL_PN_NUM	  	 96		//����,���,�����,�������ɼ���ʹ��
#define REAL_PN_MASK_SIZE  ((REAL_PN_NUM+7)/8)		//������Ҫ�Ĳ���������λ����Ĵ�С
#define PN_VALID_NUM	 96			//��Ч������ĸ���
#define METER_NUM		 96			//��ͨ��������,�������ص㻧

#define BANK_NUM		 26

#define MAX_PN_NUM       1       //todo:�����ǽ���Ҫ�õ��ġ�
#define MAX_P0_YX	     1	

//�������������ݷ�BANK�洢ʹ�õ���BANK�Ŷ���
#define CCT_BN_SPM		21			//���������BANK
#define CCT_BN_MFM		21			//�๦�ܱ�����BANK

//���ݿ�BANK0��SECT����
#define SECT_KEEP_PARA			0
#define SECT_TERM_PARA			1
#define SECT_TERM_DATA			2
#define SECT_PN_DATA			3
#define SECT_PN_PARA			4
#define SECT_EXT_TERM_PARA		5

#define SECT_NUM	 			6	//7

#define IMG_NUM     	 		0

#define INVALID_POINT     0

//�汾��Ϣ�ĳ��ȶ���
#ifdef PRO_698
#define SOFT_VER_LEN	36	//����汾���ֽڳ���
#else
#define SOFT_VER_LEN	30	//����汾���ֽڳ���
#endif
#define INN_SOFT_VER_LEN	7	//�ڲ�����汾���ֽڳ���

//�����㶯̬ӳ��
#define PNMAP_NUM		1		//�����㶯̬ӳ�䷽������
								 
//�����㶯̬ӳ�䷽��,������������1��ʼ��,0��ʾ��ӳ��
#define MTRPNMAP		PNUNMAP		//1 PNUNMAP/PMMAP1������궨�����л�����֧�ֵĲ������費��Ҫ��̬ӳ��
#define PNMAP_CCTMFM	PNUNMAP	//�����๦�ܱ�ӳ�䷽��,
#define PNMAP_VIP		PNUNMAP	//�����ص㻧ӳ�䷽��,
					 
/////////////////////////////////////////////////////////////////////////////////////////////
//����泣������

//ϵͳ���ļ�����
#define DB_FILE

//��ʼ�����ŵĶ���
#define MTR_START_PN	1	//�����ʼ������
#define GRP_START_PN	1	//�ܼ�����ʼ������
#define TURN_START_PN	1	//�ִ���ʼ������

//��������
#define GRP_NUM			8	//�ܼ������
#define TURN_NUM		4	//�ִθ���


//������Ķ���
#define GB_DATACLASS1	1	//˲ʱ����
#define GB_DATACLASS2	2	//��������
#define GB_DATACLASS3	3	//�¼�
#define GB_DATACLASS4	4	//����
#define GB_DATACLASS5	5	//����
#define GB_DATACLASS6	6	//�����ļ�
#define GB_MAXDATACLASS			7//	6+1
#define GB_DATACLASS8	8	//���󱻼����ն������ϱ�
#define GB_DATACLASS9	9	//�����ն�����

//���ݸ�ʽ����
#define FMT_UNK	0	//δ֪���ݸ�ʽ
#define FMT1	(1)
#define FMT2	(2)
#define FMT3	(3)
#define FMT4	(4)
#define FMT5	(5)
#define FMT6	(6)
#define FMT7	(7)
#define FMT8	(8)
#define FMT9	(9)
#define FMT10	(10)
#define FMT11	(11)
#define FMT12	(12)
#define FMT13	(13)
#define FMT14	(14)
#define FMT15	(15)
#define FMT16	(16)
#define FMT17	(17)
#define FMT18	(18)
#define FMT19	(19)
#define FMT20	(20)
#define FMT21	(21)
#define FMT22	(22)
#define FMT23	(23)
#define FMT24	(24)
#define FMT25	(25)
#define FMT26	(26)
#define FMT27	(27)
#define FMT28	(28)
#define FMT29	(29)
#define FMT30	(30)

#define FMT_NUM		31
#define FMTEX_NUM	1

//�Ǹ�¼����չ��ʽ����80��ʼ
#define FMTEX_START 80
#define FMT_BIN		80


#define FMT_BCD	(24*0x100)
#define   FMT_ROUND   (1)
#define   FMT_NROUND  (0)

//��ͬ�汾��E,U,I,P,Q,cos�ĸ�ʽ������ܲ�һ��,��ʹ�����µĶ���
//��Щ���嶼�����645ID��,��645ID�ĸ�ʽ��Э��Ϊ׼
#define EFMT		FMT14			//����
#define REFMT		FMT11			//����
#define UFMT		FMT7			//��ѹ
#define PFMT		FMT9			//�й�����
#define QFMT		FMT9			//�й�����
#define COSFMT		FMT5			//��������			
#define DMFMT		FMT23			//����
#define DMTFMT		FMT17			//����ʱ��
#define ANGFMT		FMT5			//�Ƕ�
#define ANGFMT		FMT5
#define VBRKCOUNTSFMT		FMT8	//�������
#define VBRKACCUMTFMT		FMT10	//�����ۼ�ʱ��
#define VBRKTIMEFMT			FMT17	//������ʼ/����ʱ��
#define PROGTIMEFMT			FMT17	//���ʱ��
#define DMCLEANTIMEFMT		FMT17	//��������ʱ��
#define PROGCOUNTSFMT		FMT8	//��̴���
#define DMCLEANCOUNTSFMT	FMT8	//�����������
#define BATTWORKTFMT		FMT10	//��ع���ʱ��

#define ELEN		4
#define RELEN		4
#define ULEN		2
#define PLEN		3
#define QLEN		3
#define COSLEN		2
#define DMLEN		3
#define DMTLEN		5
#define ANGLEN		2

#ifdef PRO_GB2005
	#define IFMT		FMT6			//����
	#define ILEN		2
#else	//PRO_698
	#define IFMT		FMT25			//����治ͬ
	#define ILEN		3
#endif

#define VBRKCOUNTSLEN		3	//�������
#define VBRKACCUMTLEN		3	//�����ۼ�ʱ��
#define VBRKTIMELEN			4	//������ʼ/����ʱ��
#define PROGTIMELEN			4	//���ʱ��
#define DMCLEANTIMELEN		4	//��������ʱ��
#define PROGCOUNTSLEN		2	//��̴���
#define DMCLEANCOUNTSLEN	2	//�����������
#define BATTWORKTLEN		3	//��ع���ʱ��

#define EMPTY_DATA 	 0
#define INVALID_DATA 0xff

#define INVALID_VAL 	(-0x7ffffff0)
#define INVALID_VAL64 	(-0x7ffffffffffffff0LL)

#define INVALID_TIME 	0	//��Ч��ʱ��


//������1-5��ϸ�ֶ���,��4�����ȫ��������ͨ�� 
#define CLASS_NULL				0	//����                          
#define CLASS_P0				1	//PN�޶���                      
#define CLASS_METER				2	//������                        
#define CLASS_SUMGROUP			3	//�ܼ���                        
#define CLASS_MEASURE			4	//ֱ��ģ����                    
#define CLASS_CONTROLTURN		5	//�����ִ�                      
#define CLASS_TASK				8	//�����                        
                           
#define GB_MAXOFF				1           //ע��PN�͵Ĵ�1��ʼ��0����                                
#define GB_MAXCONTROLTURN		(4+GB_MAXOFF)	//�����ִ�                      
#define GB_MAXMETER				PN_NUM	//�����                        
#define GB_MAXPULSE				PN_NUM	//�������                      
#define GB_MAXMEASURE			PN_NUM//(4+GB_MAXOFF)	//ֱ����������                  
#define GB_MAXSTATE  			(4+GB_MAXOFF)	//״̬����                      
#define GB_MAXBRANCH			(8+GB_MAXOFF)	//��·��                        
#define GB_MAXTASK				(64+GB_MAXOFF)  //������                        
#define GB_MAXSUMGROUP			(8+GB_MAXOFF)   //�ܼ����	 
#define GB_MAXCOMCHNNOTE		(5+GB_MAXOFF)	//��ͨ������Ϣ����
#define GB_MAXIMPCHNNOTE		(5+GB_MAXOFF)	//��Ҫ������Ϣ����

#define GB_MAXERCODE			31  //�¼�����      
#define GB_MAXCOMMTHREAD		4	//ͨ���̸߳��� 
                                                                    
//���������������Ϊ���޿��ܵ�����                                  
#define GBC4_MAXMETER			GB_MAXMETER				//�����            
#define GBC4_MAXSUMGROUP		GB_MAXSUMGROUP	//�ܼ���            
#define GBC4_MAXMEASURE			GB_MAXMEASURE//ֱ����������      
#define GBC4_MAXTASK			GB_MAXTASK		//������            
#define GBC4_MAXCONTROLTURN		GB_MAXCONTROLTURN//�����ִ�      
#define GBC4_MAXPULSE			GB_MAXPULSE     //������    
#define GBC4_MTRPORTNUM			4				//32����F33ʱ��ͨ�Ŷ˿���

//��������������ּ����䳤�����ĳ��ȿռ䶨�� 
#define GBC4IDLEN_F15			(256)
#define GBC4IDLEN_F27			(256)//(512)
#define GBC4IDLEN_F41			(137)
#define GBC4IDLEN_F65_OLD		(256)//���ֻ��֧��61����Ϣ��
#define GBC4IDLEN_F66_OLD		GBC4IDLEN_F65_OLD
#define GBC4IDLEN_F65			(1029)//�����֧��255����Ϣ�� 4*255+9
#define GBC4IDLEN_F66			GBC4IDLEN_F65

#define COM_TASK_IDLEN			(512)	//ÿ����ͨ����������19+1+2*1+1+4*255=1043���ݿ���512
#define FWD_TASK_IDLEN			(281)	//ÿ���м�����������26+255=281
#define MAX_COM_TASK			 254
#define MAX_FWD_TASK			 254

#define MAX_MTR_CHANNEL			 32		//��Ч����ͨ������
#define MTR_CHANNEL_IDLEN		 7		//��Ч����ͨ����������
#define MTR_PARA_CFGLEN			 712	//����ͨ���������ñ���

#ifdef PRO_698
#define USR_MAIN_CLASS_NUM		7				//16�û�������
#define USR_SUB_CLASS_NUM		16				//16�û�С����
												 
//�������弸���䳤�����ĳ��ȿռ䶨�� 
//#define GBC1IDLEN_F16			(((PN_NUM+7)>>3)+1) 
#define GBC1IDLEN_F169			(130)	//(6*7+1)*3+1

//�������弸���䳤�����ĳ��ȿռ䶨�� 
#define GBC9IDLEN_F2			(((GBC4_MTRPORTNUM-1)*12)+17)	
#define GBC9IDLEN_F6			((USR_MAIN_CLASS_NUM*(31+1))+2)
#endif

#define ADDONS_NULL		0
#define ADDONS_TIME		1
#define ADDONS_CURVE	2
#define ADDONS_DAYFRZ	3
#define ADDONS_MONFRZ	4
#define ADDONS_COPYDAY	5
#define ADDONS_EC		6

//������
#define RATE_NUM		4
#define TOTAL_RATE_NUM  (RATE_NUM+1)	//��+�ַ��ʵĸ���

//г������
#define HARMONIC_NUM		19

//�˿�����
#define	PN_PORT_INVALID	0	//��Ч�˿�( ���������Ч���ã�����ΪPN_PROP_INVALID ʱ���˴β����������޸Ĳ�����Ч)

//����������

#define PN_PROP_METER	1	//���
#define PN_PROP_DC		2	//ֱ��ģ����
#define PN_PROP_PULSE	3	//����
#define PN_PROP_CALCU	4	//����ֵ
#define	PN_PROP_AC		5	//����
#define PN_PROP_CCT		6	//����������(����������·,�Ժ�����Ҫ������չΪPN_PROP_PLC,PN_PROP_CCT485��,����Ϊ�˱�����չ̫��,�����Ƽ�ʹ��PN_PROP_CCT)
#define PN_PROP_EXTAC   7	//��ӽ���װ��
#define PN_PROP_UNSUP	0xff	//��ʱ��֧�ֵĲ���������

//����ڲ��������Ե�����һ�ֲ��������Ͷ���,ÿ������ռһλ
#define PN_TYPE_P0		0x01	//������0
#define PN_TYPE_AC		0x02	//����
#define PN_TYPE_MTR		0x04	//���
#define PN_TYPE_PULSE	0x08	//����
#define PN_TYPE_DC		0x10	//ֱ��ģ����
#define PN_TYPE_GRP		0x20	//�ܼ���

#define PN_TYPE_MSR		(PN_TYPE_AC|PN_TYPE_MTR|PN_TYPE_PULSE) //������

//////////////////////////////////////////////////////////////////////////////////////
//GB2005��698�����ϵͳ��������Ĳ�ͬ����

#define FMT22TOCUR_SCALE	100

#define F25_LEN				11
#define F25_CONN_OFFSET		10

#define F26_VOLUPPER_OFFSET	6
#define F26_VOLLOWER_OFFSET 11
#define F26_CURUPPER_OFFSET	16
#define F26_CURUP_OFFSET	22	
#define F26_ZCURUP_OFFSET	28	
#define F26_SUPER_OFFSET	34
#define F26_SUP_OFFSET		40
#define F26_VUNB_OFFSET		46
#define F26_IUNB_OFFSET		51

#define MTR_PARA_LEN		27		//8902
#define MTR_PORT_OFFSET		4		//8902��ͨ�����ʺͶ˿�ƫ��
#define MTR_BAUD_OFFSET		4		//8902��ͨ�����ʺͶ˿�ƫ��
#define MTR_PRO_OFFSET		5		//8902�й�Լ����ƫ��
#define MTR_ADDR_OFFSET		6		//8902�е���ַƫ��
#define MTR_PSW_OFFSET		12		//8902�е������ƫ��
#define MTR_RATE_NUM_OFFSET 18		//8902�з��ʸ���ƫ��
#define MTR_DOT_NUM_OFFSET 19		//8902��С�������ƫ��
#define MTR_USR_TYPE_OFFSET	26		//8902�д�С���ƫ��


#define MTRPRO_TO_IFMT_SCALE 1		//���Э���ĵ�����ʽ����վͨ��Э���ʽ������ת��

#define NO_CUR				50		//�޵����Ĺ̶���ֵ
#define STD_UN				2200	//��׼���ѹ
#define STD_IN				5000	//��׼�����
									 
#define F10_LEN_PER_PN		27		//F10��ÿ������������ĳ���
#define F10_MTRNUM_LEN		2		//F10�б��ε��ܱ�/��������װ����������n�ĳ���
#define F10_SN_LEN			2		//F10�е��ܱ�/��������װ����ŵĳ���
#define F10_PN_LEN			2		//F10������������ŵĳ���							

#define C1_CFG_LEN	(1+1+(1+1+31)*USR_SUB_CLASS_NUM) 	//(�����+С�������+(�û�С���+��Ϣ������n+31)*USR_SUB_CLASS_NUM)
#define C2_CFG_LEN	(1+1+(1+1+31)*USR_SUB_CLASS_NUM)	//(�����+С�������+(�û�С���+��Ϣ������n+31)*USR_SUB_CLASS_NUM)

#define RATE_SIZE	24		//���ʲ����ĳ���		//ע�⣬��ֵ��ȷ��������//140809





#define ARDFMT_NUM	22
//�澯��ʽ
#define ARD			0
#define	ARD1		1
#define	ARD2		2
#define	ARD3		3
#define	ARD4		4
#define	ARD5		5
#define	ARD6		6
#define	ARD7		7
#define	ARD8		8
#define	ARD9		9
#define	ARD10		10
#define	ARD11		11
#define	ARD12		12
#define	ARD13		13
#define	ARD14		14
#define	ARD15		15
#define	ARD16		16
#define	ARD17		17
#define	ARD18		18
#define	ARD19		19
#define	ARD20		20
#define	ARD21		21
#define	ARD22		22
#define ALR_NUM		25

#endif //DBCONST_H

