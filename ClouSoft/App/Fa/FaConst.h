/*******************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�FaConst.h
 * ժ    Ҫ�����ļ���Ҫ����ϵͳӦ����ʹ�õ���һЩ����
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2011��3��
 ******************************************************************************/
#ifndef FACONST_H
#define FACONST_H

#define  PWROFF_VER       0x01

#define ERR_APP_OK       0x0
#define ERR_FORWARD  0x1
#define ERR_INVALID  0x2    //�������ݷǷ�
#define ERR_PERM     0x3
#define ERR_ITEM     0x4	//���ݿⲻ֧�ֵ�������
#define ERR_TIME     0x5    //ʱ��ʧЧ

#define ERR_UNSUP	 0x06	//���֧�ֵ��������
#define ERR_FAIL	 0x07	//����ʧ��,���糭��ʧ�ܵ�

#define ERR_ADDR     0x11
#define ERR_SEND     0x12
#define ERR_SMS      0x13 
#define ERR_PNUNMAP	 0x14	//������δӳ��
#define ERR_DEV	 	 0x15	//�豸������
#define ERR_NOTEXIST 0x16	//������

#define ERR_APP_TIMEOUT  0x20
#define ERR_SYS      0x30

#define ERR_CALIB_OVER   0x40  //У׼���
#define ERR_PARA         0x41  //��������ȷ
#define ERR_OUT_CALIB    0x42  //����У׼״̬

//Ӧ��֪ͨ:
//������ʱ֪ͨ��,��Ҫ2���Ӳ��ύ��֪ͨ��Ϣ�ŵ�ǰ��,
//����1���ӿ����ύ�Ͽ��ID�ŵ�INFO_LONG_DELAY_END�涨�ĺ����
#define INFO_NONE	   			0
#define INFO_APP_RST			1
#define INFO_SYS_RST   			2
#define INFO_WK_PARA			3	//GPRS�����̲߳�������
#define INFO_FAP_PARA			4	//��վͨ��Э���������
#define INFO_COMM_ADDR			5	//�ն˵�ַ�����������
#define INFO_COMM_RLD			6	//����վͨ����Ҫ��װ����
//����ʱ
#define INFO_230M_PARA			7	//��̨��������
#define INFO_EXC_PARA			8	//�쳣����������
//����ʱ
#define INFO_ACTIVE	   			9	//���Ż������弤����Ϣ
#define INFO_METER_PARA			10   //����������
#define INFO_RST_PARA			11  //������λ
#define INFO_RST_DATA			12	//���ݸ�λ
#define	INFO_CLR_DEMAND			13	//������
#define INFO_AC_PARA			14	//���ɲ������
#define INFO_TASK_PARA			15	//����������
#define INFO_TASK_ENABLE		16	//����ʹ�ܱ�־���
#define INFO_YX_PARA			17	//ң�Ų����仯
#define INFO_STAT_PARA 	   		18	//����ͳ�Ʋ������
#define INFO_PULSE	   			19	//�����������

#define INFO_CTRL				20	//������Ϣ���
#define INFO_TEMPCTRL			21	//��ʱ�޵��
#define INFO_WTCURVE			22	//д��������
#define INFO_POINTPARA_CTRL		23	//д���������
#define INFO_REMOTEDOWN			24	//����վͨ����Ҫ��װ����

#define INFO_GPRS_OFFLINE		25	//Ҫ��GPRS����

#define INFO_DC_SAMPLE			26


#define INFO_PLC_MTRSCHCMD		27		//Plc������������
#define INFO_PLC_STATUPDATA		28		//Plcͳ�Ƹ���
#define INFO_PLC_RDALLRTINFO 	29  //��ȡ���нڵ���м�·����Ϣ
#define INFO_PLC_PARA	   		30   //�ز���������
#define INFO_PLC_CLRRT	   		31	//��·��
#define INFO_PLC_STOPRD			32	 //ֹͣ����
#define INFO_PLC_RESUMERD		33	 //�ָ�����	
#define INFO_PLC_UPDATE_ROUTE   34  //�ز�·��������
#define INFO_PLC_WIRLESS_CHANGE 35  //�����ŵ����
#define INFO_PLC_RADIO_PARA     36  //���߲������
#define INFO_PLC_TIME_SET		37  //������ʱ�䱻����
#define INFO_PLC_MOD_CHANGED    38  //�����ز�ģ��

#define INFO_START_REREAD       39
#define INFO_PRO_REGINFO 		40  //�����·�ע�����

#define INFO_FRZ_TASK_PARA      41  //��������������    Jason add. 20130719
#define INFO_STAT_TERM			42	//�ն�ͳ������
#define INFO_CLR_STAT_TERM		43	//�ն�ͳ������

#define INFO_RST_ALLPARA		44   //������в�������

#define INFO_AC_REPOWERON       45  //���������ϵ�	//20140517-1

#define INFO_STOP_FEED_WDG		46	//ֹͣι���Ź�
#define INFO_DISCONNECT			47		//DISCONNECT
#define INFO_RESTART_RDMTR		48	//ָ���˿����³���
#define INFO_START_485I_MTRSCH	49  //����4851�ѱ�����
#define INFO_STOP_485I_MTRSCH	50  //ֹͣ4851�ѱ�����
#define INFO_START_485II_MTRSCH	51  //����4852�ѱ�����
#define INFO_STOP_485II_MTRSCH	52  //ֹͣ4852�ѱ�����
#define INFO_START_485III_MTRSCH	53  //����4853�ѱ�����
#define INFO_STOP_485III_MTRSCH	54  //ֹͣ4853�ѱ�����
#define INFO_UPDATE_BOOTLOADER  55  //Bootloader����
#define INFO_MAC_RST            56  //��̫������㸴λ
#define INFO_PORT_FUN            57  //�˿ڹ��ܸı���Ϣ
#define INFO_RST_TERM_STAT		58	//��λ�ն�ͳ������
#define INFO_FWDTASK_PARA       59  //�м��������
#define INFO_COMM_RST			60	//����վͨ����Ҫ��λ
#define INFO_TASK_INIT			61  //���������ʼ��
#define INFO_END	   			62	//����Ϣ,��Ϊ������Ϣ�Ľ���
									//�ѱ�֪ͨ�㶨��Ϊ���һ��

#define INFO_NUM	   	    		(INFO_END+1)
#define INFO_SHORT_DELAY_START	 	INFO_230M_PARA
#define INFO_NO_DELAY_START	 		INFO_ACTIVE

//��Ϣ����ʱ�����ʱ�Ķ���,��λ��
#define INFO_SHORT_DELAY	6
#define INFO_LONG_DELAY		30


//���Э���ڲ�����
#define PROTOCOLNO_NULL			2
#define PROTOCOLNO_DLT645		0	//DL645
//#define PROTOCOLNO_AC			2	//����װ��
#define PROTOCOLNO_ABB			3	//ABB����
#define PROTOCOLNO_FUJ			4	//������չ
#define PROTOCOLNO_EDMI			5	//����
//#define PROTOCOLNO_ABB2			6	//ABBԲ��	
#define PROTOCOLNO_WS			7	//��ʢI��
#define PROTOCOLNO_HND			8	//������
#define PROTOCOLNO_LANDIS  		9	//������1107
#define PROTOCOLNO_OSTAR 		10	//���
//#define PROTOCOLNO_DLMS  		11	//������	
//#define PROTOCOLNO_HL645  		12	//��¡�ϱ�	
//#define PROTOCOLNO_AH645		13	//����645	
#define PROTOCOLNO_1107			14	//A1700
#define PROTOCOLNO_HT3A			15	//��ͨ
#define PROTOCOLNO_LANDIS_DLMS	16  //������DLMS
//#define PROTOCOLNO_TJ645		17	//���645
#define PROTOCOLNO_HB645		18	//����645
#define PROTOCOLNO_EMAIL		19	//EMAIL��	
#define PROTOCOLNO_MODBUS		20	//�¹�GMC A2000��(MODBUSЭ��)
//#define PROTOCOLNO_LANDIS_ZMC	22	//������ZMC��Э��
//#define PROTOCOLNO_BJT645		23  //����97��645
#define PROTOCOLNO_DL645_Q		24  //97��645���޹���
#define PROTOCOLNO_DLT645_SID	28	//97��645�ĵ�ID����
//#define PROTOCOLNO_NMG645		29  //���ɹű�
#define PROTOCOLNO_DLT645_V07	1	//2007��645��
//#define PROTOCOLNO_TEMPERATURE	35	//��ͨ���¶Ȳ�����
//#define	PROTOCOLNO_DTM645		36  //�������Ե�����ת��ģ��

//#ifdef VER_CL195N4
#define PROTOCOLNO_DLT645_11	11	//DL645
#define PROTOCOLNO_DLT645_12	12	//DL645
#define PROTOCOLNO_DLMS  		25	//������	
#define PROTOCOLNO_HL645  		26	//��¡�ϱ�	
//#endif

#define	CCT_MTRPRO_97	1	//97��645
#define	CCT_MTRPRO_07	2	//07��645

#define PROTOCOLNO_MAXNO		40	//���ĵ��Э��ţ�Ŀǰ������40

#define FLG_FORMAT_DISK   		0x34aebd24
#define FLG_DEFAULT_CFG   		0x8a5bc4be
#define FLG_REMOTE_DOWN   		0xbe7245cd
#define FLG_HARD_RST   	  		0x4ab8ce90
#define FLG_DISABLE_METEREXC    0xce7821bd
#define FLG_ENERGY_CLR    		0xee6ad23f
#define FLG_APP_RST             1

#define TYPE_FRZ_TASK		    0		//������������
#define TYPE_COMM_TASK			1		//��ͨ��������
#define TYPE_FWD_TASK			2		//�м���������
#define TYPE_ALR_EVENT		    3		//�澯�¼�

//485�ڹ���
#define PORT_FUN_RDMTR		0		//�����
#define PORT_FUN_INMTR		1		//������
#define PORT_FUN_LINK		2		//������
#define PORT_FUN_VARCPS		3		//���޹�����װ��
#define PORT_FUN_ACQ		4		//�ɼ���
#define PORT_FUN_DEBUG		0xFF	//debug���(ֻ��3����Ч)

		//ע��:������Ķ���Ͳ�Ҫ�ٸ���,�����������,
		// 	   ���Ǿ͹̶���Ϊ�߼��˿ڸ����ϵ�ϰ��,�Ǵ���->���˳��ʼ���1,2...
#define LOGIC_PORT_NUM	2		//(sizeof(g_iInSnToPhyPort)/sizeof(int))

#define LOGIC_PORT_MIN	0									//��С���߼��˿ڶ���
#define LOGIC_PORT_MAX	(LOGIC_PORT_MIN+LOGIC_PORT_NUM-1)	//�����߼��˿ڶ���
#define INMTR_BAUD			CBR_2400

#define TRANSMIT_TIME_OUT   50      //�ļ������е�ת����ʱʱ�� ��λs

#define COM_FUNC_METER  0 //485���ڹ��ܶ���
#define COM_FUNC_LINK   1
#define COM_FUNC_DL645  2
#define COM_FUNC_UPLOAD  3

#define COM_FUNC_ACQ		4		//�ɼ���
#define COM_FUNC_JC485		5		//����485��.

#define SG_FAP_DATA_EX   6			//�Զ���������ļ���ƫ�� (2�ֽڿ�½ʶ���� + 1��չ������ + 3���ֽ��Զ�������)

#endif

