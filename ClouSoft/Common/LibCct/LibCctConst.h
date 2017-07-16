/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�LibCctConst.h
 * ժ    Ҫ�����ļ���Ҫʵ�ּ��������������弰���ó�������
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��1��
 *********************************************************************************************************/
#ifndef LIBCCTCONST_H
#define LIBCCTCONST_H

#define PLC_MODULE_RT	1	//��½�Զ�·���㷨(��������)
#define PLC_MODULE_ES	2	//����

//�ز���λ��Ϣ
#define PLC_PHASE_UNK		0	
#define PLC_PHASE_A			1
#define PLC_PHASE_B			2
#define PLC_PHASE_C			3

//֡��ʽ�е��ֶ�ƫ��
#define FRM_HEAD			        0
#define FRM_LEN				        1
#define FRM_C			            3  
#define FRM_INF		        		4 	  //��Ϣ��R

#define FRM_ADDR					10 
#define FRM_AFN			        	10    //������ַ��AFNƫ��
#define FRM_AFN_A			        22 	  //����ַ��AFNƫ��
#define FRM_TRAN_645ADDR		    28 	  //ת������645֡�ĵ�ַƫ��


//����ģʽ����
#define WM_RDMTR	0	//����
#define WM_STUDY	1	//ѧϰ
//AFN
#define AFN_CON		            	0x00	//ȷ�ϨM����
#define AFN_INIT			        0x01	//��ʼ��
#define AFN_TRAN	    	    	0x02	//����ת��
#define AFN_QRYDATA			    	0x03	//��ѯ����
#define FRM_LNKTEST			    	0x04	//��·�ӿڼ��
#define AFN_CTRL			    	0x05	//��������
#define AFN_REP			    		0x06	//�����ϱ�
#define AFN_QRYRT					0x10	//·�ɲ�ѯ
#define AFN_SETRT		    		0x11	//·������
#define AFN_CTRLRT					0x12	//·�ɿ���
#define AFN_RTFWD 		    		0x13	//·������ת��
#define AFN_RTRD 		    		0x14	//·�����ݳ���
#define AFN_TRSFILE   				0x15    //�ļ�����


#define AFN_FCNETOP					0x09	//�������
#define AFN_FCROUTE					0x08	//��Ѷ������������
#define AFN_FCNETINFO				0x07	//��Ѷ��������Ϣ��
#define AFN_RPT						0x0a	//��Ѷ�������ϱ�

//FN����
#define F1		1
#define F2		2
#define F3		3
#define F4		4
#define F5		5
#define F6		6
#define F7		7
#define F8		8
#define F9		9
#define F10		10
#define F11		11

//·�����󳭶����ݵĳ�����־
#define RD_FLG_FAIL		0 	//����ʧ��
#define RD_FLG_SUCC 	1	//�����ɹ�
#define RD_FLG_TORD		2	//���Գ���


#define AFN_DEBUG					0xcc    //��Ѷ�������Ϣ�����AFN
//�м����
#define PLC_FWD_DEPTH_0		0	//�м����0
#define FWD_DEPTH_MAX		7	//���֧�ֵ��м����

//�ز��������ĵײ�ͨ����·����
#define AR_LNK_485  	  0x00		  //485����
#define AR_LNK_ES		  0x02		  //�����ز�(ģ��·��)
#define AR_LNK_TC		  0x04		  //�ൺ����(ģ��·��)
#define AR_LNK_STD_TC	  0x05		  //�ൺ����698��׼�ӿ�
#define AR_LNK_STD_XC	  0x06		  //��������698��׼�ӿ�
#define AR_LNK_STD_RC	  0x07		  //��˹��698��׼�ӿ�
#define AR_LNK_STD_ES	  0x08		  //����698��׼�ӿ�
#define AR_LNK_RF		  0x09		  //���߷���
#define AR_LNK_ES_EX	  0x0a		  //�����ز�(����Э��)
#define AR_LNK_STD_FC	  0x0b		  //��Ѷ�����߱�׼�ӿ�
#define AR_LNK_STD_MI	  0x0c		  //����΢
#define AR_LNK_STD_SR	  0x0d		  //ɣ��
#define AR_LNK_STD_LM	  0x0e		  //����΢
#define AR_LNK_STD_GD     0x0f        //��������  
#define AR_LNK_STD_XC_A   0x10        //�������������� 
#define AR_LNK_STD_LK	  0x11		  //�ӱ����Ժ�汾
#define AR_LNK_STD_LM_N	  0x12		  //����΢��,Ϊ�����ϵ�����΢ģ��,�¼������.
#define AR_LNK_STD_RZ     0x13        //�����º��
#define AR_LNK_STD_ZC     0x14        //������巺��
#define AR_LNK_STD_NT     0x15        //������
#define AR_LNK_STD_HR     0x16        //���ݺ��
#define AR_LNK_STD_MX     0x17       //������ϣС����
#define AR_LNK_STD_NT_HB  0x18       //�ӱ�������

#define AR_LNK_FXR	0x81		//��������(��½�Զ�·���㷨)
#define AR_LNK_ESR	0x82		//�����ز�(��½�Զ�·���㷨)
#define AR_LNK_ATR	0x83		//��̩�ɼ�������(��½�Զ�·���㷨)
#define AR_LNK_TCR	0x84		//�ൺ����(��½�Զ�·���㷨)

#define AR_LNK_NUM	AR_LNK_TCR	//�Զ�����ϵͳ�ײ�ͨ����·������

#define RT_INFO_LEN			28
#define RT_SUBST_LEN		12

#define	RD_DONE			0x01	//����
#define RD_UNDONE		0x00	//δ����

#define	LenStateW		0x10	//״̬�ֽڳ���

//Ŀǰ����֧�ֵĵ��Э��,�����698.42����һ��
#define	CCT_MTRPRO_97	1	//97��645
#define	CCT_MTRPRO_07	2	//07��645
#define	CCT_MTRPRO_NJSL	3	//�Ͼ����ְ�645

//�����㲥�����ֶ���,���ָ�698.42����,��չ0x10Ϊ�㲥Уʱ
#define	BC_CTL_TRANS	0x00	//͸������
#define	BC_CTL_97		0x01	//DLT/645-1997 
#define	BC_CTL_07		0x02 	//DLT/645-2007
#define	BC_CTL_PHASE	0x03    //��λʶ����; 04H FFH������
#define	BC_CTL_ADJTIME	0x10	//�㲥Уʱ
#define	BC_CTL_FRZ		0x11	//�㲥����

//��������������
#define	CCT_RD_ERR_OK		0		//�޴���
#define	CCT_RD_ERR_ITEM		1		//����������ݴ���,�����ն���ʱ������
#define	CCT_RD_ERR_RX		2		//����ʧ��
#define	CCT_RD_ERR_ADDR		3		//���յ�ַû�ҵ�
#define	CCT_RD_ERR_REPLY 	4		//�쳣Ӧ��֡,������Ϣ�ַ������ݻ���ĵ�һ���ֽ�
#define	CCT_RD_ERR_UNSUP 	5		//��֧��������

#define CCT_SPD_NUM         5       //376.2-2013֧�ֵ��ز�ͨ����������
#endif


