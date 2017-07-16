/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�LibAcConst.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֶԽ������������Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��2��
 * ��    ע: 
 *********************************************************************************************************/
#ifndef LIBACCONST_H
#define LIBACCONST_H

#define RATE_NUM           4

#define FREQ_UNIT    256      //256              ������㾫��,�ȳ��ϣ�����ٳ���
#define FREQ_SHIFT   8        
#define FFT_NUM	     32

//#define SCN_NUM           1    //���֧�ֵĲ�����ͨ����,�ı�ú궨��ʱע��ͬʱ�ı����ݿ���У׼����0x2060�ĳ���!!!!!
#define SCN_NUM      1   //������������û�н�������,���㵼�����������쳣  Changed By Wing 2014/07/29


#define NUM_PER_CYC       FFT_NUM  	//64 ÿ�����ڲɼ���������
#define CYC_NUM		      1    //ÿ��ͨ��������ٸ����ڵ�����
#define SBUF_SIZE         (NUM_PER_CYC*CYC_NUM)  //ÿ��ͨ�������������

#define FREQ_CYC_NUM      256 //200     //Ƶ�ʼ����������

#define NUM_PER_CYC_45HZ  (NUM_PER_CYC*50/45)  //ÿ�����ڲɼ���������
#define NUM_PER_CYC_55HZ  (NUM_PER_CYC*50/55 + 1)  //ÿ�����ڲɼ���������

//FREQ_CYC_NUM���ܲ��������С�Ĳɼ��������������ڵ����˲������ڵ��µġ�
#define NUM_PER_MAX_PN    (FREQ_CYC_NUM*NUM_PER_CYC*55/45 + 1)
#define NUM_PER_MIN_PN    (FREQ_CYC_NUM*NUM_PER_CYC*45/55)


//#define E_CONST        6000
						//  (�� *��) / ���峣��

#define  E_PER_PULSE  0x1C9C3800 //(10L*1000L*3600L*1000L*10*8/6000)    //ÿ��������ڶ��ٸ� ��/10 * ����/8
  					 //  (�� *�� *����*��/10*����/8) / ���峣��
												
//#define E_PER_PULSE    0x1C9C3800  //(1000*3600*1000*10*8/E_CONST)*10    //ÿ��������ڶ��ٸ� ��/10 * ����/8
  						  //  (�� *�� *����*��/10*����/8) / ���峣��

//#define E_PER_PULSE       0xABA9500 //0x112A880 //(1000*3600*1000*10*8/16000)    //ÿ��������ڶ��ٸ� ��/10 * ����/8
  						 //  (�� *�� *����*��/10*����/8) / ���峣��



#define QUAD_POS_P        0x00
#define QUAD_NEG_P        0x01 
#define QUAD_POS_Q        0x00
#define QUAD_NEG_Q        0x02

//�ն��ڼ���ĵ��ܵ����
#define EP_POS_A	0
#define EP_POS_B	1
#define EP_POS_C	2
#define EP_POS_ABC	3

#define EP_NEG_A	4
#define EP_NEG_B	5
#define EP_NEG_C	6
#define EP_NEG_ABC	7

#define EQ_POS_A	8
#define EQ_POS_B	9
#define EQ_POS_C	10
#define EQ_POS_ABC	11

#define EQ_NEG_A	12
#define EQ_NEG_B	13
#define EQ_NEG_C	14
#define EQ_NEG_ABC	15

#define EQ_Q1		16
#define EQ_Q2		17
#define EQ_Q3		18
#define EQ_Q4		19	


//�����ڲ����ݵ�����
#define AC_VAL_UA	0
#define AC_VAL_UB	1
#define AC_VAL_UC	2
#define AC_VAL_IA	3
#define AC_VAL_IB	4
#define AC_VAL_IC	5

#define AC_VAL_P	6
#define AC_VAL_PA	7
#define AC_VAL_PB	8
#define AC_VAL_PC	9
#define AC_VAL_Q	10
#define AC_VAL_QA	11
#define AC_VAL_QB	12
#define AC_VAL_QC	13

#define AC_VAL_COS	14
#define AC_VAL_COSA	15
#define AC_VAL_COSB	16
#define AC_VAL_COSC	17

#define AC_VAL_ANG_UA	18
#define AC_VAL_ANG_UB	19
#define AC_VAL_ANG_UC	20
#define AC_VAL_ANG_IA	21
#define AC_VAL_ANG_IB	22
#define AC_VAL_ANG_IC	23

#define AC_VAL_PRO_IA	24		//A�򱣻�����
#define AC_VAL_PRO_IB	25		//B�򱣻�����
#define AC_VAL_PRO_IC	26		//C�򱣻�����
#define AC_VAL_PRO_I0	27		//���򱣻�����
#define AC_VAL_I0		28		//�������
#define AC_VAL_U0		29		//�����ѹ
#define AC_VAL_F		30		//Ƶ��

#define AC_VAL_PHASESTATUS	25	//����״̬
#define AC_VAL_MTRSTATUS	27	//���״̬��

#define AC_VAL_S			28
#define AC_VAL_SA			29	
#define AC_VAL_SB			30
#define AC_VAL_SC			31
							 
#define AC_VAL_NUM			(AC_VAL_F+1)


//�޹������ۼӱ�־����ʱ�õ��ĳ�������
#define QADD	3
#define QSUB	2
#define Q1ADD	QADD
#define Q1SUB	QSUB
#define Q2ADD	(QADD<<2)
#define Q2SUB	(QSUB<<2)
#define Q3ADD	(QADD<<4)
#define Q3SUB	(QSUB<<4)
#define Q4ADD	(QADD<<6)
#define Q4SUB	(QSUB<<6)


//����״̬��־
#define DISORDER_U      0x01
#define DISORDER_I      0x02

#define	ID_TO_RATENUM(id)	( (id&0x000f)==0x000f ? RATE_NUM+1 : 1 )

/////////////////////////////////////////////////////////////////////////////
//����
#define ACLOG_ENABLE	1 

#define AD_CHK_DELAY      (NUM_PER_CYC*50*2)
						
#define CONNECT_1P    	1	//�����
#define CONNECT_3P3W    3
#define CONNECT_3P4W    4

//һ����������������
#define HARM_NUM_MAX	15	//г������������
#define ENERGY_NUM_MAX	50	//��������������
#define DEMAND_NUM_MAX 	50	//��������������

#endif //LIBACCONST_H
