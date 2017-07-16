/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DL645V07.c
 * 摘    要：本文件给出07版645抄表协议的功能实现
 * 当前版本：1.0
 * 作    者：潘香玲
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
#include "DL645V07.h"
#include "DbAPI.h"
#include "MtrCtrl.h"
#include "DrvCfg.h"

#define	DL645V07_CMD	      8	
#define	DL645V07_LEN	      9	
#define	DL645V07_DATA	      10

//#define	DL645V07_CMD_RESERVE	0x00	
#define	DL645V07_CMD_BC_TIME	0x08
#define	DL645V07_CMD_ASK_DATA	0x11	
#define	DL645V07_CMD_ASK_NEXT	0x12	
//#define	DL645V07_CMD_ASK_ADDR	0x13	
//#define	DL645V07_CMD_WRITE_DATA	0x14	
//#define	DL645V07_CMD_WRITE_ADDR	0x15	
//#define	DL645V07_CMD_FRZ		0x16	
//#define	DL645V07_CMD_CHG_BR		0x17	
//#define	DL645V07_CMD_CHG_PSW	0x18	
//#define	DL645V07_CMD_DMD_RESET	0x19	
//#define	DL645V07_CMD_ENG_RESET	0x1A
//#define	DL645V07_CMD_EVENT_RESET 0x1B
#define	DL645V07_CMD_CTRL		0x1C	//跳合闸、报警、保电

//#define	DL645V07CMD_MAX			DL645V07CMD_EVENT_RESET
#define	DL645V07_CMD_GET	0x1f

/*******************************************************
********************************************************/ 

const TItemList DL645_to_DL645V07[]=
{ 
			{0x1280, 0x020a01ff, 21*2, 21*2},//当前A相电压1～21次谐波含有率
			{0x1281, 0x020a02ff, 21*2, 21*2},//当前B相电压1～21次谐波含有率
			{0x1282, 0x020a03ff, 21*2, 21*2},//当前C相电压1～21次谐波含有率
			{0x1283, 0x020b01ff, 21*2, 21*2},//当前A相电流1～21次谐波含有率
			{0x1284, 0x020b02ff, 21*2, 21*2},//当前B相电流1～21次谐波含有率
			{0x1285, 0x020b03ff, 21*2, 21*2},//当前C相电流1～21次谐波含有率

			//以下为按2007版645协议扩展的曲线ID数据包
			{0x3681, 0x06020001, 3, 1},			
			{0x3689, 0x06010001, 2, 1},		
			{0x3701, 0x06040001, 4, 1},		
			{0x3705, 0x06030001, 2, 1},
			{0x3745, 0x06050001, 4, 1},			

			//以下只列出总（电能、需量、需量时间），对应的分费率及块ID经处理同样支持
			{0x9000, 0x00000000, 4, 4},		//（当前）组合有功电能总	

			{0x9010, 0x00010000, 4, 4},		//（当前）正向有功电能总					
			{0x9020, 0x00020000, 4, 4},		//（当前）反向有功电能总 		

			//以下只列出分相，对应的块ID经处理同样支持
			{0x9070, 0x00150000, 4, 4},	   //（当前）A相正向有功电能		
			{0x9071, 0x00290000, 4, 4},	   //（当前）B相正向有功电能		
			{0x9072, 0x003D0000, 4, 4},	   //（当前）C相正向有功电能			

			{0x9080, 0x00160000, 4, 4},	   //（当前）A相反向有功电能		
			{0x9081, 0x002A0000, 4, 4},	   //（当前）B相反向有功电能		
			{0x9082, 0x003E0000, 4, 4},	   //（当前）C相反向有功电能	
		
			{0x9110, 0x00030000, 4, 4},		//（当前）正向（感性）无功电能总 					
			{0x9120, 0x00040000, 4, 4},		//（当前）反向（容性）无功电能总 

			{0x9130, 0x00050000, 4,	4},		//（当前）I象限（感性）无功电能总 					
			{0x9140, 0x00080000, 4, 4},		//（当前）IV象限（容性）无功电能总 		    
			{0x9150, 0x00060000, 4, 4},		//（当前）II象限无功电能总 		    
			{0x9160, 0x00070000, 4, 4},		//（当前）III象限无功电能总 

			{0x9170, 0x00170000, 4, 4},	   //（当前）A相（感性）无功电能		
			{0x9171, 0x002B0000, 4, 4},	   //（当前）B相（感性）无功电能		
			{0x9172, 0x003F0000, 4, 4},	   //（当前）C相（感性）无功电能			

			{0x9180, 0x00180000, 4, 4},	   //（当前）A相（容性）无功电能		
			{0x9181, 0x002C0000, 4, 4},	   //（当前）B相（容性）无功电能		
			{0x9182, 0x00400000, 4, 4},	   //（当前）C相（容性）无功电能		

			{0x9400, 0x00000001, 4, 4},		//（上月）组合有功电能总	
			{0x9410, 0x00010001, 4, 4},		//（上月）正向有功电能总					
			{0x9420, 0x00020001, 4, 4},		//（上月）反向有功电能总 	

			{0x9470, 0x00150001, 4, 4},	   //（上月）A相正向有功电能		
			{0x9471, 0x00290001, 4, 4},	   //（上月）B相正向有功电能		
			{0x9472, 0x003D0001, 4, 4},	   //（上月）C相正向有功电能		

			{0x9480, 0x00160001, 4, 4},	   //（上月）A相反向有功电能		
			{0x9481, 0x002A0001, 4, 4},	   //（上月）B相反向有功电能		
			{0x9482, 0x003E0001, 4, 4},	   //（上月）C相反向有功电能		

			{0x9510, 0x00030001, 4, 4},		//（上月）正向（感性）无功电能总 					
			{0x9520, 0x00040001, 4, 4},		//（上月）反向（容性）无功电能总 

			{0x9530, 0x00050001, 4,	4},		//（上月）I象限（感性）无功电能总 					
			{0x9540, 0x00080001, 4, 4},		//（上月）IV象限（容性）无功电能总 		    
			{0x9550, 0x00060001, 4, 4},		//（上月）II象限无功电能总 		    
			{0x9560, 0x00070001, 4, 4},		//（上月）III象限无功电能总 

			{0x9570, 0x00170001, 4, 4},	   //（上月）A相（感性）无功电能		
			{0x9571, 0x002B0001, 4, 4},	   //（上月）B相（感性）无功电能		
			{0x9572, 0x003F0001, 4, 4},	   //（上月）C相（感性）无功电能			

			{0x9580, 0x00180001, 4, 4},	   //（上月）A相（容性）无功电能		
			{0x9581, 0x002C0001, 4, 4},	   //（上月）B相（容性）无功电能		
			{0x9582, 0x00400001, 4, 4},	   //（上月）C相（容性）无功电能	

			{0x9a00, 0x05060001, 5, 5},		//（日冻结）上一次日冻结时间	

			{0x9a1f, 0x05060101, 20, 256},		//（日冻结）正向有功电能总及费率					
			{0x9a2f, 0x05060201, 20, 256},		//（日冻结）反向有功电能总及费率						
			{0x9b1f, 0x05060301, 20, 256},		//（日冻结）正向（组合一）无功电能总及费率 					
			{0x9b2f, 0x05060401, 20, 256},		//（日冻结）反向（组合二）无功电能总及费率 
			{0x9b3f, 0x05060501, 20, 256},		//（日冻结）一象限无功电能总及费率					
			{0x9b4f, 0x05060801, 20, 256},		//（日冻结）四象限无功电能总及费率						
			{0x9b5f, 0x05060601, 20, 256},		//（日冻结）二象限无功电能总及费率 					
			{0x9b6f, 0x05060701, 20, 256},		//（日冻结）三象限无功电能总及费率 

			{0x9c0f, 0x05060901, 15, 512},		//（日冻结）正向有功需量及发生时间总及费率		
			{0x9c1f, 0x0103ff00, 15, 512},		//（日冻结）正向无功需量及发生时间总及费率－因电表没有日冻结的无功需量，故取当前值
			{0x9c2f, 0x05060a01, 15, 512},		//（日冻结）反向有功需量及发生时间总及费率		
			{0x9c3f, 0x0104ff00, 15, 512},		//（日冻结）反向无功需量及发生时间总及费率－因电表没有日冻结的无功需量，故取当前值
			{0x9c8f, 0x05060901, 20, 512},		//（日冻结）正向有功需量及发生时间总及费率	
			{0x9c9f, 0x0103ff00, 20, 512},		//（日冻结）正向无功需量及发生时间总及费率－因电表没有日冻结的无功需量，故取当前值
			{0x9caf, 0x05060a01, 20, 512},		//（日冻结）反向有功需量及发生时间总及费率			
			{0x9cbf, 0x0104ff00, 20, 512},		//（日冻结）反向无功需量及发生时间总及费率－因电表没有日冻结的无功需量，故取当前值

			{0xA010, 0x01010000, 3, 8},	   //（当前）正向有功总最大需量	
			{0xA020, 0x01020000, 3, 8},	   //（当前）反向有功总最大需量	
			{0xA110, 0x01030000, 3, 8},	   //（当前）正向（感性）无功总最大需量	
			{0xA120, 0x01040000, 3, 8},	   //（当前）反向（容性）无功总最大需量	

			{0xA130, 0x01050000, 3, 8},	   //（当前）I象限无功总最大需量	
			{0xA140, 0x01080000, 3, 8},	   //（当前）IV象限无功总最大需量
			{0xA150, 0x01060000, 3, 8},	   //（当前）II象限无功总最大需量
			{0xA160, 0x01070000, 3, 8},	   //（当前）III象限无功总最大需量

			{0xA410, 0x01010001, 3, 8},	   //（上月）正向有功总最大需量	
			{0xA420, 0x01020001, 3, 8},	   //（上月）反向有功总最大需量	
			{0xA510, 0x01030001, 3, 8},	   //（上月）正向（感性）无功总最大需量	
			{0xA520, 0x01040001, 3, 8},	   //（上月）反向（容性）无功总最大需量	

			{0xA530, 0x01050001, 3, 8},	   //（上月）I象限无功总最大需量	
			{0xA540, 0x01080001, 3, 8},	   //（上月）IV象限无功总最大需量
			{0xA550, 0x01060001, 3, 8},	   //（上月）II象限无功总最大需量
			{0xA560, 0x01070001, 3, 8},	   //（上月）III象限无功总最大需量

			{0xB010, 0x01010000, 5, 8},	   //（当前）正向有功总最大需量发生时间		
			{0xB020, 0x01020000, 5, 8},	   //（当前）反向有功总最大需量发生时间			
			{0xB110, 0x01030000, 5, 8},	   //（当前）正向（感性）无功总最大需量发生时间			
			{0xB120, 0x01040000, 5, 8},	   //（当前）反向（容性）无功总最大需量发生时间	

			{0xB130, 0x01050000, 5, 8},	   //（当前）I象限无功总最大需量发生时间	
			{0xB140, 0x01080000, 5, 8},	   //（当前）IV象限无功总最大需量发生时间
			{0xB150, 0x01060000, 5, 8},	   //（当前）II象限无功总最大需量发生时间
			{0xB160, 0x01070000, 5, 8},	   //（当前）III象限无功总最大需量发生时间

			{0xB210, 0x03300001, 4, 50},	//最近一次编程时间	
			{0xB211, 0x03300201, 4, 202},	//最近一次清需量时间	
			{0xB212, 0x03300000, 2, 3},		//编程总次数	
			{0xB213, 0x03300200, 2, 3},		//需量清零总次数	
			{0xB214, 0x0280000A, 3, 4},		//电池工作时间			

			//{0xB310, 0x03040000, 3, 18},//DL645总断相次数
			{0xB311, 0x13010001, 3, 3},//DL645A相断相次数
			{0xB312, 0x13020001, 3, 3},//DL645B相断相次数
			{0xB313, 0x13030001, 3, 3},//DL645C相断相次数
			//{0xB31f, 0x13010001, 3*4, 9},//DL645C相断相次数

			//{0xB320, 0x03040000, 3, 18},//DL645总断相时间累计值
			{0xB321, 0x13010002, 3, 3},//DL645A相断相时间累计值
			{0xB322, 0x13020002, 3, 3},//DL645B相断相时间累计值
			{0xB323, 0x13030002, 3, 3},//DL645C相断相时间累计值
			//{0xB32f, 0x13010002, 3*4, 9},//DL645C相断相次数

			//以下只列出分相，对应的块ID经处理同样支持
			//{0xB330, 0x03040101, 4, 115},//DL645总断相起始时刻
			{0xB331, 0x13010101, 4, 6},//DL645A相断相起始时刻
			{0xB332, 0x13020101, 4, 6},//DL645B相断相起始时刻
			{0xB333, 0x13030101, 4, 6},//DL645C相断相起始时刻		
			//{0xB33f, 0x03040101, 4*4, 115},//DL645C相断相起始时刻	

			//{0xB340, 0x03040101, 4, 115},//DL645总断相结束时刻
			{0xB341, 0x13012501, 4, 6},//DL645A相断相结束时刻
			{0xB342, 0x13022501, 4, 6},//DL645B相断相结束时刻
			{0xB343, 0x13032501, 4, 6},//DL645C相断相结束时刻		
			//{0xB34f, 0x03040101, 4*4, 115},//DL645C相断相起始时刻	

			{0xB410, 0x01010001, 5, 8},	   //（上月）正向有功总最大需量发生时间		
			{0xB420, 0x01020001, 5, 8},	   //（上月）反向有功总最大需量发生时间			
			{0xB510, 0x01030001, 5, 8},	   //（上月）正向（感性）无功总最大需量发生时间			
			{0xB520, 0x01040001, 5, 8},	   //（上月）反向（容性）无功总最大需量发生时间	

			{0xB530, 0x01050001, 5, 8},	   //（上月）I象限无功总最大需量发生时间	
			{0xB540, 0x01080001, 5, 8},	   //（上月）IV象限无功总最大需量发生时间
			{0xB550, 0x01060001, 5, 8},	   //（上月）II象限无功总最大需量发生时间
			{0xB560, 0x01070001, 5, 8},	   //（上月）III象限无功总最大需量发生时间			
			
			{0xB611, 0x02010100, 2, 2},		//A相电压
			{0xB612, 0x02010200, 2, 2},		//B相电压
			{0xB613, 0x02010300, 2, 2},		//C相电压
			{0xB61F, 0x0201FF00, 2*3, 2*3},	//A、B、C相电压

			{0xB621, 0x02020100, 3, 3},		//A相电流
			{0xB622, 0x02020200, 3, 3},		//B相电流
			{0xB623, 0x02020300, 3, 3},		//C相电流		
			{0xB62F, 0x0202FF00, 3*3, 3*3},	//A、B、C相电流

			{0xB630, 0x02030000, 3, 3},		//瞬时有功功率
			{0xB631, 0x02030100, 3, 3},		//瞬时A相有功功率
			{0xB632, 0x02030200, 3, 3},		//瞬时B相有功功率
			{0xB633, 0x02030300, 3, 3},		//瞬时C相有功功率
			{0xB63f, 0x0203FF00, 3*4, 3*4},	//瞬时有功功率块	

			{0xB640, 0x02040000, 3, 3},		//瞬时无功功率
			{0xB641, 0x02040100, 3, 3},		//瞬时A相无功功率
			{0xB642, 0x02040200, 3, 3},		//瞬时B相无功功率
			{0xB643, 0x02040300, 3, 3},		//瞬时C相无功功率
			{0xB64f, 0x0204FF00, 3*4, 3*4},	//瞬时无功功率块	

			{0xB650, 0x02060000, 2, 2},		//瞬时功率因素
			{0xB651, 0x02060100, 2, 2},		//瞬时A相功率因素
			{0xB652, 0x02060200, 2, 2},		//瞬时B相功率因素
			{0xB653, 0x02060300, 2, 2},		//瞬时C相功率因素
			{0xB65f, 0x0206FF00, 2*4, 2*4},	//瞬时功率因素块			

			{0xB660, 0x02070100, 2, 2},		//瞬时A相相角(电流与电压夹角)
			{0xB661, 0x02070200, 2, 2},		//瞬时B相相角(电流与电压夹角)
			{0xB662, 0x02070300, 2, 2},		//瞬时C相相角(电流与电压夹角)
			{0xB66f, 0x0207FF00, 2*3, 2*3},//瞬时相角块			

			{0xB670, 0x02050000, 3, 3},		//瞬时视在功率
			{0xB671, 0x02050100, 3, 3},		//瞬时A相视在功率
			{0xB672, 0x02050200, 3, 3},		//瞬时B相视在功率
			{0xB673, 0x02050300, 3, 3},		//瞬时C相视在功率
			{0xB67f, 0x0205FF00, 3*4, 3*4},	//瞬时视在功率块

			{0xB680, 0x02800002, 2, 2},		//电网频率

			{0xB6A0, 0x02800001, 3, 3},		//零线电流		

			{0xB901, 0x03300d00, 3, 3},//电能表尾盖打开次数
			{0xB902, 0x03300d01, 16, 60},//最近一次尾盖打开和关闭时间

			{0xC010, 0x04000101, 4, 4},//日期及周次
			{0xC011, 0x04000102, 3, 3},//时间
			
			{0xC020, 0x04000501, 1, 2},//电表状态字
			//{0xC021, 0x04000102, 1, 1},//电网状态字
			{0xC022, 0x04000801, 1, 1},//周休日状态字
			
			{0xC030, 0x04000409, 3, 3},//电表有功常数
			{0xC031, 0x0400040A, 3, 3},//电表无功常数			

			{0xC111, 0x04000103, 1, 1},//最大需量周期
			{0xC112, 0x04000104, 1, 1},//滑差时间

			{0xC117, 0x04000B01, 2, 2},//自动抄表日（结算日）

			//以下只列出分项，对应的块ID经处理同样支持
			{0xC310, 0x04000201, 1, 1},//年时区数
			{0xC311, 0x04000202, 1, 1},//日时段表叔
			{0xC312, 0x04000203, 1, 1},//日时段
			{0xC313, 0x04000204, 1, 1},//费率数
			{0xC314, 0x04000205, 1, 2},//公共假日数		

			//以下只列出块ID，对应的分项ID经处理同样支持
			{0xC32f, 0x04010000, 42, 42},//第一套第时区表起始日期及时段表号
			{0xC33f, 0x04010001, 42, 42},//第一套第一日时段表第1时段起始时间及费率号	
			{0xC34f, 0x04010002, 42, 42},//第一套第二日时段表第1时段起始时间及费率号	
			{0xC35f, 0x04010003, 42, 42},//第一套第三日时段表第1时段起始时间及费率号	
			{0xC36f, 0x04010004, 42, 42},//第一套第四日时段表第1时段起始时间及费率号	
			{0xC37f, 0x04010005, 42, 42},//第一套第五日时段表第1时段起始时间及费率号	
			{0xC38f, 0x04010006, 42, 42},//第一套第六日时段表第1时段起始时间及费率号	
			{0xC39f, 0x04010007, 42, 42},//第一套第七日时段表第1时段起始时间及费率号	
			{0xC3Af, 0x04010008, 42, 42},//第一套第八日时段表第1时段起始时间及费率号

			////////////////////////////////////////////////////////////////////
			//以下为按2007版645协议扩展的数据ID
			{0xc810, 0x03300000, 3, 3},//电表编程总次数
			{0xc811, 0x03300001, 6, 50},//电表编程时间		
			{0xc812, 0x03300d00, 3, 3},//电能表尾盖打开次数
			{0xc813, 0x03300d01, 6, 60},//最近一次尾盖打开时间
			{0xc814, 0x03300e00, 3, 3},//电能表端钮盒打开次数
			{0xc815, 0x03300e01, 6, 60},//最近一次端钮盒打开时间
			{0xc818, 0x03300d01, 60, 60},//（上1次）开表盖记录
			{0xc819, 0x03300e01, 60, 60},//（上1次）开端钮盒记录(同开表盖)
			{0xc81a, 0x03110000, 3, 3},//掉电总次数

			{0xc820, 0x03300100, 3, 3},//电表清零总次数
			{0xc821, 0x03300101, 6, 106},//电表清零时间
			{0xc830, 0x03300200, 3, 3},//需量清零总次数
			{0xc831, 0x03300201, 6, 202},//需量清零时间
			{0xc840, 0x03300300, 3, 3},//电表事件清零总次数
			{0xc841, 0x03300301, 6, 14},//电表事件清零时间

			{0xc850, 0x03300400, 3, 3},//电表校时总次数
			{0xc851, 0x03300401, 6, 16},//电表校时时间
			{0xc852, 0x03300500, 3, 3},//电能表时段表编程总次数
			{0xc853, 0x03300501, 6, 682},//最近一次电能表时段表编程时间
			{0xc854, 0x03300401, 6, 16},//电表校时后时间

			{0xc860, 0x04000501, 2, 2},//电表状态字1
			{0xc861, 0x04000502, 2, 2},//电表状态字2
			{0xc862, 0x04000503, 2, 2},//电表状态字3
			{0xc863, 0x04000504, 2, 2},//电表状态字4
			{0xc864, 0x04000505, 2, 2},//电表状态字5
			{0xc865, 0x04000506, 2, 2},//电表状态字6
			{0xc866, 0x04000507, 2, 2},//电表状态字7
			{0xc86f, 0x040005FF, 2*7, 2*7},//电表状态字数据块		

			{0xc870, 0x0280000A, 4, 4},//电池工作时间	
			{0xc871, 0x04000B01, 2, 2},//每月第一结算日	
			{0xc872, 0x04000B02, 2, 2},//每月第二结算日		
			{0xc873, 0x04000B03, 2, 2},//每月第三结算日	
			{0xc874, 0x1A010001, 3, 3},//DL645A相断流次数
			{0xc875, 0x1A020001, 3, 3},//DL645B相断流次数
			{0xc876, 0x1A030001, 3, 3},//DL645C相断流次数

			{0xc890, 0x00850000, 4, 4},//当前铜损铁损有功示值
			{0xc891, 0x00860000, 4, 4},//当前铜损铁损有功示值
			{0xc8a0, 0x00850001, 4, 4},//上一结算日铜损铁损有功示值
			{0xc8a1, 0x00860001, 4, 4},//上一结算日铜损铁损有功示值

			{0xc8b0, 0x02070100, 2, 2},		//瞬时A相相角(电流与电压夹角)
			{0xc8b1, 0x02070200, 2, 2},		//瞬时B相相角(电流与电压夹角)
			{0xc8b2, 0x02070300, 2, 2},		//瞬时C相相角(电流与电压夹角)
			{0xc8bf, 0x0207FF00, 2*3, 2*3},//瞬时相角块			

			//以下只列出块ID，对应的分项ID经处理同样支持
			{0xc90f, 0x04020000, 42, 42},//第二套第时区表起始日期及时段表号
			{0xc91f, 0x04020001, 42, 42},//第二套第一日时段表第1时段起始时间及费率号	
			{0xc92f, 0x04020002, 42, 42},//第二套第二日时段表第1时段起始时间及费率号	
			{0xc93f, 0x04020003, 42, 42},//第二套第三日时段表第1时段起始时间及费率号	
			{0xc94f, 0x04020004, 42, 42},//第二套第四日时段表第1时段起始时间及费率号	
			{0xc95f, 0x04020005, 42, 42},//第二套第五日时段表第1时段起始时间及费率号	
			{0xc96f, 0x04020006, 42, 42},//第二套第六日时段表第1时段起始时间及费率号	
			{0xc97f, 0x04020007, 42, 42},//第二套第七日时段表第1时段起始时间及费率号	
			{0xc98f, 0x04020008, 42, 42},//第二套第八日时段表第1时段起始时间及费率号
		
			//电量型预付费表
			{0xc990, 0x00900100, 4,	4}, //（当前）剩余电量
			{0xc991, 0x00900101, 4,	4}, //（当前）透支电量

			{0xc9a0, 0x03320101, 5,	5}, //（上一次）购电日期
			{0xc9a1, 0x03320201, 2,	2}, //购电后总购电次数
			{0xc9a2, 0x03320301, 4,	4}, //购电量
			{0xc9a3, 0x03320401, 4, 4}, //购电前剩余电量
			{0xc9a4, 0x03320501, 4,	4},	//购电后剩余电量	
			{0xc9a5, 0x03320601, 4,	4},	//购电后累计购电量

			//电费型预付费表(本地结算)
			{0xc9b0, 0x00900200, 4,	4}, //（当前）剩余金额
			{0xc9b1, 0x00900201, 4,	4}, //（当前）透支金额

			{0xc9c0, 0x03330101, 5,	5}, //（上一次）购电日期
			{0xc9c1, 0x03330201, 2,	2}, //购电后总购电次数
			{0xc9c2, 0x03330301, 4,	4}, //购电金额
			{0xc9c3, 0x03330401, 4,	4}, //购电前剩余金额
			{0xc9c4, 0x03330501, 4,	4},	//购电后剩余金额	
			{0xc9c5, 0x03330601, 4,	4},	//购电后累计购电金额

			{0xc9d0, 0x04000f04, 4,	4},	//赊欠门限电量（暂取透支电量限值）
			{0xc9d1, 0x04000f01, 4,	4},	//报警电量(暂取报警电量1限值)
			{0xc9d2, 0x04000f02, 4,	4},	//故障电量(暂取报警电量2限值)		

			{0xc9e0, 0x04000503, 1, 2},//电表通断电状态(电表状态字3)
			{0xc9e1, 0x1e000101, 6, 6},//最近一次通电时间
			{0xc9e2, 0x1d000101, 6, 6},//最近一次断电时间

			//结算信息1
			{0xd000, 0x000b0001, 4,	4}, //已结有功总电能(取上1结算周期组合有功总累计用电量)			
			{0xd010, 0x000b0000, 4,	4}, //未结有功总电能(取当前结算周期组合有功总累计用电量)		
			//结算信息2
			{0xd02f, 0x0000ff01, 4*5,	4*5}, //已结有功电能量(取上1结算周期组合有功电能量)			
			{0xd03f, 0x0000ff00, 4*5,	4*5}, //未结有功电能量(取当前组合有功电能量)	

			///////////////////////////////////////////////////////////
			//以下为以色列智能电表扩展
			{0xd100, 0x05001500, 7,       7},	//15分钟有功电量增量数据 要特殊处理

			{0xd101, 0x05001101, 7,       7},	//上1次定时冻结每天最大15分钟增电量

			{0xd102, 0x05001201, 3,       3},	//上1次定时冻结每天最大15分钟电流A
			{0xd103, 0x05001301, 3,       3},	//上1次定时冻结每天最大15分钟电流B
			{0xd104, 0x05001401, 3,       3},	//上1次定时冻结每天最大15分钟电流C

			//掉电总次数
			{0xd200, 0x03110000, 4,       3},	//上1次掉电总次数
			{0xd201, 0x03110001, 12,      12},	//上1次掉电记录数据 发生时间 结束时间

			//A B C分相失压总次数
			{0xd210, 0x03010000, 9,      9},	//A B C分相失压总次数 
			{0xd211, 0x03010101, 14,     14},	//上1次A相失压记录 发生时刻 结束时刻 失压期间A相电压
			{0xd212, 0x03010201, 14,     14},	//上1次B相失压记录 发生时刻 结束时刻 失压期间B相电压
			{0xd213, 0x03010301, 14,     14},	//上1次C相失压记录 发生时刻 结束时刻 失压期间C相电压

			//A B C分相欠压总次数
			{0xd220, 0x03020000, 9,      9},	//A B C分相欠压总次数 
			{0xd221, 0x03020101, 14,     14},	//上1次A相欠压记录 发生时刻 结束时刻 欠压期间A相电压
			{0xd222, 0x03020201, 14,     14},	//上1次B相欠压记录 发生时刻 结束时刻 欠压期间B相电压
			{0xd223, 0x03020301, 14,     14},	//上1次C相欠压记录 发生时刻 结束时刻 欠压期间C相电压

			//A B C分相过压总次数
			{0xd230, 0x03030000, 9,      9},	//A B C分相过压总次数 
			{0xd231, 0x03030101, 14,     14},	//上1次A相过压记录 发生时刻 结束时刻 过压期间A相电压
			{0xd232, 0x03030201, 14,     14},	//上1次B相过压记录 发生时刻 结束时刻 过压期间B相电压
			{0xd233, 0x03030301, 14,     14},	//上1次C相过压记录 发生时刻 结束时刻 过压期间C相电压

			//A B C分相断相总次数
			{0xd240, 0x03040000, 9,      9},	//A B C分相断相总次数 
			{0xd241, 0x03040101, 14,     14},	//上1次A相断相记录 发生时刻 结束时刻 断相期间A相电压
			{0xd242, 0x03040201, 14,     14},	//上1次B相断相记录 发生时刻 结束时刻 断相期间B相电压
			{0xd243, 0x03040301, 14,     14},	//上1次C相断相记录 发生时刻 结束时刻 断相期间C相电压

			//A B C分相失流总次数
			{0xd250, 0x030B0000, 9,      9},	//A B C分相失流总次数 
			{0xd251, 0x030B0101, 15,     15},	//上1次A相失流记录 发生时刻 结束时刻 失流期间A失流时刻电流
			{0xd252, 0x030B0201, 15,     15},	//上1次B相失流记录 发生时刻 结束时刻 失流期间B失流时刻电流
			{0xd253, 0x030B0301, 15,     15},	//上1次C相失流记录 发生时刻 结束时刻 失流期间C失流时刻电流

			//A B C分相过流总次数 
			{0xd260, 0x030C0000, 9,      9},	//A B C分相过流总次数 
			{0xd261, 0x030C0101, 15,     15},	//上1次A相过流记录 发生时刻 结束时刻 过流期间A失流时刻电流
			{0xd262, 0x030C0201, 15,     15},	//上1次B相过流记录 发生时刻 结束时刻 过流期间B失流时刻电流
			{0xd263, 0x030C0301, 15,     15},	//上1次C相过流记录 发生时刻 结束时刻 过流期间C失流时刻电流

			//分相功率因素过低
			{0xd270, 0x03310300, 9,      9},	//分相功率因素过低总次数
			{0xd271, 0x03310301, 14,     14},	//上1次(A相功率因素过低记录):发生时间 结束时间 发生时刻功率因素值
			{0xd272, 0x03310401, 14,     14},	//上1次B相过流记录 发生时刻 结束时刻 过流期间B失流时刻电流
			{0xd273, 0x03310501, 14,     14},	//上1次C相过流记录 发生时刻 结束时刻 过流期间C失流时刻电流

			//编程总次数
			{0xd280, 0x03300000, 3,      3},	//编程总次数 
			{0xd281, 0x03300001, 50,     50},	//上1次(编程记录):发生时间 操作者代码 编程前10个数据标识码(不足补FF)

			//事件清零总次数
			{0xd290, 0x03300300, 3,      3},	//事件清零总次数
			{0xd291, 0x03300301, 14,     14},	//上1次(事件清零记录):发生时间 操作者代码 事件清零数据标识码

			//更改密码总次数
			{0xd2a0, 0x03320300, 3,      3},	//更改密码总次数
			{0xd2a1, 0x03320301, 10,     10},	//上1次(更改密码记录):发生时间 旧密码值
			{0xd2a2, 0x03320302, 10,     10},	//上2次(更改密码记录):发生时间 旧密码值
			{0xd2a3, 0x03320303, 10,     10},	//上3次(更改密码记录):发生时间 旧密码值

			{0xd2b0, 0x0000ff00, 20,     20},	//F212当前组合有功电能示值（总、费率1～M）
			{0xd2c0, 0x0009ff00, 20,     20},	//F213当前正向视在电量示值（总、费率1～M）
			{0xd2d0, 0x000aff00, 20,     20},	//F214当前反向视在电量示值（总、费率1～M）

			////////////////////////////////////////////////////////////////////		
			//以下为网络表使用的数据
			{0xd800, 0x02CC0000, 2*6, 2*6},//瞬时相位角块 (网络表专用)
			{0xd801, 0x04000901, 1, 1},//负荷记录模式字
			{0xd802, 0x04000902, 1, 1},//定时冻结数据模式字
			{0xd803, 0x04000903, 1, 1},//瞬时冻结数据模式字
			{0xd804, 0x04000904, 1, 1},//约定冻结数据模式字
			{0xd805, 0x04000905, 1, 1},//整点冻结数据模式字
			{0xd806, 0x04000906, 1, 1},//日冻结数据模式字		

			{0xe940, 0x04000306, 3, 3},//CT变比
			{0xe941, 0x04000307, 3, 3},//PT变比
			{0xe942, 0x03350000, 3, 3},//恒定磁场干扰总次数
			{0xe943, 0x03350001, 12, 28},//（上一次）恒定磁场干扰记录：发生时刻+结束时刻

			{0xea10, 0x10000001, 3, 3},//失压总次数
			{0xea11, 0x10010002, 3, 3},//失压总累计时间
			{0xea12, 0x10000101, 6, 6},//最近1次失压发生时刻
			{0xea13, 0x10000201, 6, 6},//最近1次失压结束时刻

			{0xea14, 0x10010001, 3, 3},//A相失压总次数
			{0xea15, 0x10010002, 3, 3},//A相失压总累计时间
			{0xea16, 0x10020001, 3, 3},//B相失压总次数
			{0xea17, 0x10020002, 3, 3},//B相失压总累计时间
			{0xea18, 0x10030001, 3, 3},//C相失压总次数
			{0xea19, 0x10030002, 3, 3},//C相失压总累计时间
			{0xea20, 0x03050000, 6, 6},//全失压总次数，总累计时间

			//失流
			{0xea40, 0x18010001, 3, 3},//A相失流总次数
			{0xea41, 0x18010002, 3, 3},//A相失流总累计时间
			{0xea42, 0x18020001, 3, 3},//B相失流总次数
			{0xea43, 0x18020002, 3, 3},//B相失流总累计时间
			{0xea44, 0x18030001, 3, 3},//C相失流总次数
			{0xea45, 0x18030002, 3, 3},//C相失流总累计时间

			//潮流反向
			{0xea60, 0x1B010001, 3, 3},//A相潮流反向总次数
			{0xea61, 0x1B010002, 3, 3},//A相潮流反向总累计时间
			{0xea62, 0x1B020001, 3, 3},//B相潮流反向总次数
			{0xea63, 0x1B020002, 3, 3},//B相潮流反向总累计时间
			{0xea64, 0x1B030001, 3, 3},//C相潮流反向总次数
			{0xea65, 0x1B030002, 3, 3},//C相潮流反向总累计时间

};

//07版645协议自用函数
bool IsSpecId(WORD wID);
bool IsV07CurveId(WORD wID);
bool IsRateParaId(WORD wID);
bool IsCmbReEng(DWORD dwProId);
int	 BinarySchInx(WORD wID);
WORD DL645V07MakeFrm(TV07Tmp* pTmpV07, BYTE* pbTxBuf, BYTE* pbAddr, BYTE bCmd, BYTE bLen);
int DL645V07TxRx(struct TMtrPro* pMtrPro, TV07Tmp* pTmpV07, TItemList* ptItem, WORD wLen);
int DL645V07AskItem1(struct TMtrPro* pMtrPro, TV07Tmp* pTmpV07, WORD wID, BYTE* pbBuf, WORD wSubIdx);
extern int MtrReadFrz(struct TMtrPro* pMtrPro, TV07Tmp* pTmpV07, WORD wID, BYTE* pbBuf, WORD wSubIdx);

bool Mtr645V07Init(struct TMtrPro* pMtrPro, bool fInit, BYTE bThrId)
{
	//TV07Priv* pMtr645V07 = (TV07Priv* )pMtrPro->pvMtrPro;

	pMtrPro->pfnAskItem = DL645V07AskItem;	
	pMtrPro->pfnRcvBlock = DL645V07RcvBlock;
	pMtrPro->pfnGetProPrintType = DL645V07GetProPrintType;
	
	pMtrPro->pbTxBuf = &m_MtrTxBuf[bThrId][0];
	pMtrPro->pbRxBuf = &m_MtrRxBuf[bThrId][0];
	memset(pMtrPro->pbRxBuf, 0, MTR_FRM_SIZE); 
	memset(pMtrPro->pbRxBuf, 0, MTR_FRM_SIZE); 	
	return true;
}

//描述：读取97版645ID数据标识的数据的接口
//备注：有些块ID需拆分处理
int DL645V07AskItem(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf)
{
	BYTE i, j;
	WORD wSubID, wIdLen;
	BYTE bNum, bItemLen;
	int iLen, iRv;    
    BYTE bTemp[100];
	bool bfind=false;		

	//TV07Priv* pMtrV07 = (TV07Priv* )pMtrPro->pvMtrPro;

	//收帧解析的临时变量,每次处理重新开始
	TV07Tmp tTmpV07;
	memset(&tTmpV07, 0, sizeof(TV07Tmp));

	if (wID==0xb31f || wID==0xb32f || wID==0xb33f || wID==0xb34f || wID==0xc31f || ((wID&0xf)==0xf && IsPhaseEngId(wID)))	
	{
		if (wID == 0xc31f) //费率时区
		{
			bNum = 5;
			bItemLen = 1;
		}
		else
		{
			bNum = GetBlockIdNum(wID);
			bItemLen = Get645TypeLength(wID);
			if (wID == 0xb31f)
				bItemLen = 3;
		}

		memset(pbBuf, m_bInvdData, bNum*bItemLen);	

		iLen = 0;
		for (i=0; i<bNum; i++)
		{
			if ((wID==0xb31f || wID==0xb32f|| wID==0xb33f || wID==0xb34f) && i==0) //没有断相总
			{			
				iLen += bItemLen;
				continue;
			}

			wSubID = wID&0xfff0 + i;			

			//注意，实际分相电量数据已转换成int64，长度也用int64的长度返回
			iRv = DL645V07AskItem1(pMtrPro, &tTmpV07, wSubID, pbBuf+iLen, 0);

			if (iRv > 0)
				iLen += iRv; 
			else if (iRv < 0)
				iLen += bItemLen; 
			else
				return iRv;			
		}
		return iLen;
	}	
	else if (wID == 0xc9c1) //购电次数,针对购费类型与购电量类型两种电表自动尝试
	{	
		iRv = DL645V07AskItem1(pMtrPro, &tTmpV07, 0xc9c1, pbBuf, 0);
		if (iRv <= 0)
			return DL645V07AskItem1(pMtrPro, &tTmpV07, 0xc9a1, pbBuf, 0);
		else
			return iRv;
	}
	else if (wID == 0x128f) //当前ABC三相电压、电流2～N次谐波含有率
	{	
		iLen = 0;
		for (i=0; i<6; i++)
		{
			iRv = DL645V07AskItem1(pMtrPro, &tTmpV07, 0x1280+i, pbBuf+iLen, 0);
			if (iRv > 0)
				iLen += iRv;
			else
				return iRv;
		}		
		return iLen;
	}
	else
	{	
		if ( IsV07CurveId(wID) )
		{			
			//读取给定时间记录块 目前认作读取终端时间最近的1小时及未来1个点的数据	
			//请求最近9个点(即最近1小时的数据,及未来1个曲线点的数据，以防止终端时标晚于电表时，当前点取不到)				
			tTmpV07.tRdLoadInfo.bNum = 5;
			SecondsToTime((GetCurSec()-3600)/(15*60)*(15*60), &tTmpV07.tRdLoadInfo.tmStart);		
		}
		
		iRv = DL645V07AskItem1(pMtrPro, &tTmpV07, wID, pbBuf, 0);

		if (iRv >= 0)
			return iRv;
		else if ((IsSpecId(wID) || (wID&0xFF00)==0xB600) && (wID&0x000f)==0x000f)//拆分不支持的常见块ID
		{		
			bNum = GetBlockIdNum(wID);				
			wIdLen = SetCommDefault(wID, pbBuf);

			iLen = 0;
			for (i=0; i<bNum; i++)
			{
				memset(bTemp, 0, sizeof(bTemp));
				if (wID==0xB61F || wID==0xB62F)
					j = i+1;			
				else
					j = i;
				iRv = DL645V07AskItem1(pMtrPro, &tTmpV07, (wID&0xFFF0)+j, bTemp, 0);			
				if (iRv > 0)
				{
					bfind = true;
					memcpy(pbBuf+iLen, bTemp, iRv);
					iLen += iRv;
				}
				else
				{	
					if (iRv < 0) //如果ID表示不支持（含通信返回不支持以及非通信检测不支持的情况），也算通信OK的，上次不予以补抄
						bfind = true;

					if (bNum == 5) //电量、需量、需量时间之类的数据
					{
						if (iRv<0 && i==0) //如果第一个子ID就不支持，则块ID视作不支持
							return -1;
						else
							iLen = wIdLen;
						break;
					}
					else //瞬时量
					{
						if (j == 0)	//总量不予判断
							iLen += (wIdLen/bNum);
						else	//以A相为依据返回,后面的子ID不抄了
						{
							if (iRv<0 && j==1) //如果A相就不支持，则块ID视作不支持
							{
								if ( bfind ) //表示总已经回OK了
									iLen = wIdLen;
								else
									return -1;
							}
							else
								iLen = wIdLen;
							break;						
						}						
					}
				}					
			}	

			if ( !bfind ) //子ID一个都没回答
				return 0;

			return iLen;
		}
		else //其他不支持的ID 直接返回
			return iRv;	
	}

	//return -1;
}


//描述：接收验证
bool DL645V07RcvBlock(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize)
{
	WORD i;
	BYTE b; 

	//TV07Priv* pMtrV07 = (TV07Priv* )pMtrPro->pvMtrPro;
	TV07Tmp* pTmpV07 = (TV07Tmp* )pTmpInf;
	BYTE* pbRxBuf = pMtrPro->pbRxBuf; 
	BYTE* pbTxBuf = pMtrPro->pbTxBuf; 

	for ( ; dwLen; dwLen--)
	{
		b = *pbBlock++;

		switch (pTmpV07->nRxStep) 
		{
		case 0:   //0x68
			if (b == 0x68)
			{
				pbRxBuf[0] = 0x68;
				pTmpV07->wRxPtr = 1;
				pTmpV07->wRxCnt = 9;       
				pTmpV07->nRxStep = 1;
			}
			break;
		case 1:    //数据域前的数据
			pbRxBuf[pTmpV07->wRxPtr++] = b;
			pTmpV07->wRxCnt --;
			if (pTmpV07->wRxCnt == 0)   //接收完，进行校验
			{
				if (memcmp(&pbRxBuf[1],&pbTxBuf[1],6)==0 && pbRxBuf[7]==0x68)
				{
					pTmpV07->wRxDataLen = pbRxBuf[DL645V07_LEN];
					pTmpV07->wRxCnt = pTmpV07->wRxDataLen + 2;
					pTmpV07->nRxStep = 2;
				}
				else
				{
					pTmpV07->nRxStep = 0;
				}
			}
			break;
		case 2:     //数据 + 检验码 + 结束码
			pbRxBuf[pTmpV07->wRxPtr++] = b;
			pTmpV07->wRxCnt -- ;
			if (pTmpV07->wRxCnt == 0)   //接收完，进行校验
			{
				pTmpV07->nRxStep = 0;

				if (pbRxBuf[pTmpV07->wRxPtr-1]==0x16 && pbRxBuf[pTmpV07->wRxPtr-2]==CheckSum(pbRxBuf, pTmpV07->wRxDataLen+10))
				{
					for (i=10; i<10+pTmpV07->wRxDataLen; i++)
						pbRxBuf[i] -= 0x33;

					return true;    //接收到完整的一帧
				}
			}
			break;
		default:
			pTmpV07->nRxStep = 0;
			break;
		} //switch (pMtrV07->nRxStep) 
	}

	return false;
}	


void DL645V07GetProPrintType(BYTE* pbPrintPro, char* pszProName)
{
	*pbPrintPro = DB_DL645V07;
	memcpy(pszProName, "DL645V07", sizeof("DL645V07"));
}


/////////////////////////////////////////////////////
//以下为09版645协议内部使用的函数定义

//描述：处理对电能、需量、需量时间等的分费率ID或块ID变为总ID以方便查找
//		再ID对照表中对于这些特殊ID只给的总ID
bool IsSpecId(WORD wID)
{
	if ((wID>>8)==0xa0 || (wID>>8)==0xa1
		|| (wID>>8)==0xa4 || (wID>>8)==0xa5
		|| (wID>>8)==0xb0 || (wID>>8)==0xb1
		|| (wID>>8)==0xb4 || (wID>>8)==0xb5
		|| ((wID>>8)==0x90 && (wID&0x00f0)<=0x20) //除掉分相电能
		|| ((wID>>8)==0x91 && (wID&0x00f0)<=0x60)
		|| ((wID>>8)==0x94 && (wID&0x00f0)<=0x20)
		|| ((wID>>8)==0x95 && (wID&0x00f0)<=0x60) )//除掉分相电能
	{		
		return true;
	}
	
	return false;
}

//描述：处理费率块的分ID变为总ID以方便查找
//		在ID对照表中对于这些特殊ID只给的块ID
bool IsRateParaId(WORD wID)
{
	if ((wID>>4)==0xc32 || (wID>>4)==0xc33
		|| (wID>>4)==0xc34 || (wID>>4)==0xc35
		|| (wID>>4)==0xc36 || (wID>>4)==0xc37
		|| (wID>>4)==0xc38 || (wID>>4)==0xc39
		|| (wID>>4)==0xc3a ||
		(wID>>4)==0xc90 || (wID>>4)==0xc91
		|| (wID>>4)==0xc92 || (wID>>4)==0xc93
		|| (wID>>4)==0xc94 || (wID>>4)==0xc95
		|| (wID>>4)==0xc96 || (wID>>4)==0xc97
		|| (wID>>4)==0xc98)
	{		
		return true;
	}
	 
	return false;
}

//描述：是否组合无功电能或组合无功需量，这些数据需进行符号处理
bool IsCmbReEng(DWORD dwProId)
{
	if ( (dwProId>>16)==0x0000 //组合有功
		|| (dwProId>>16)==0x0003 || (dwProId>>16)==0x0004 //组合无功
		|| (dwProId>>16)==0x0103 || (dwProId>>16)==0x0104 //组合无功需量
		|| (dwProId>>16)==0x0017 || (dwProId>>16)==0x0018 //A相组合无功	
		|| (dwProId>>16)==0x0117 || (dwProId>>16)==0x0118 //A相组合无功需量	
		|| (dwProId>>16)==0x002B || (dwProId>>16)==0x002C //B相组合无功	
		|| (dwProId>>16)==0x012B || (dwProId>>16)==0x012C //B相组合无功需量
		|| (dwProId>>16)==0x003F || (dwProId>>16)==0x0040 //C相组合无功	
		|| (dwProId>>16)==0x013F || (dwProId>>16)==0x0140  //C相组合无功需量
		|| (dwProId>>8)==0x050603 || (dwProId>>8)==0x050604) //日冻结组合无功
	{		
		return true;
	}
	
	return false;
}

//描述：是否曲线冻结ID
bool IsV07CurveId(WORD wID)
{
	if ( wID>=0x3681 && wID<=0x3696
		|| wID>=0x3701 && wID<=0x3708
		|| wID>=0x3745 && wID<=0x3748
		|| wID == 0xd100)
	{
		return true;
	}
	
	return false;
}

//描述:二分法查找ID
int BinarySchInx(WORD wID)
{
	int little, big, mid;
	WORD num = sizeof(DL645_to_DL645V07)/sizeof(TItemList);
	if (wID<DL645_to_DL645V07[0].wDL645Id|| wID>DL645_to_DL645V07[num-1].wDL645Id)
		return -1;

	little = 0;
	big = num;
	while (little <= big)
	{                               
		mid = (little + big) / 2;       //二分

		if (DL645_to_DL645V07[mid].wDL645Id == wID) 
		{
			return mid;
		}
		else if (wID > DL645_to_DL645V07[mid].wDL645Id)
		{
			little = mid + 1;
		} 
		else  //if (wID < DL645_to_DL645V07[mid].wDL645Id)
		{
			big = mid - 1;
		}

		mid = (little + big) / 2;
	}

	return -1;
}


//描述：DL645到07版645的ID转换
bool DL645toDL645V07(WORD wPn, WORD wDL645ID, TItemList* ptDL645V07)
{ 
	int inx;
	WORD wID = wDL645ID;

	if ( IsSpecId(wDL645ID) ) //电能需量等列表只有总量,没有分费率及块
			wID = wDL645ID&0xfff0;
	else if ( IsRateParaId(wDL645ID) ) //费率列表里只列出了块ID,没有分ID
			wID = (wDL645ID&0xfff0) + 0xf;
	
	if ((inx=BinarySchInx(wID)) < 0)
		return false;

	
	memcpy((BYTE*)ptDL645V07, (BYTE*)&DL645_to_DL645V07[inx], sizeof(TItemList));

	if ( IsSpecId(wDL645ID) ) //电能需量等列表只有总量,没有分费率及块
	{
		ptDL645V07->wDL645Id = wDL645ID;				

		if ((wDL645ID&0xf)==0xf)	
		{
			ptDL645V07->dwProId += 0xFF00;
			ptDL645V07->wDL645Len  *= 5;
			ptDL645V07->wProLen *=5;
		}
		else
			ptDL645V07->dwProId += (wDL645ID-wID)*0x100;
	}
	else  if ( IsRateParaId(wDL645ID) ) //费率列表里只列出了块ID,没有分ID
	{
		ptDL645V07->wDL645Id = wDL645ID;	

		if ((wDL645ID&0xf)!=0xf)	
		{					
			ptDL645V07->wDL645Len  = 3;				
		}				
	}

	if (wDL645ID==0x3701 &&  IsSinglePhaseV07Mtr(wPn))
	{
		ptDL645V07->dwProId = 0x0504FF01;				
		ptDL645V07->wProLen = 13;
	}

	return true;
}	


//描述：组帧发送
WORD DL645V07MakeFrm(TV07Tmp* pTmpV07, BYTE* pbTxBuf, BYTE* pbAddr, BYTE bCmd, BYTE bLen)
{
	WORD i;	
	
	pbTxBuf[0] = 0x68;
	memcpy(&pbTxBuf[1], pbAddr, 6);
	pbTxBuf[7] = 0x68;
	pbTxBuf[8] = bCmd;	

	if (bCmd == DL645V07_CMD_ASK_NEXT)
	{	
		bLen ++; //增加帧序号
		pTmpV07->bRdNextSeq ++;
		if (pTmpV07->bRdNextSeq == 0)
			pTmpV07->bRdNextSeq = 1;
		pbTxBuf[14] = pTmpV07->bRdNextSeq;
	}
	else	
	{
		pTmpV07->fRdNext = false;
		pTmpV07->bRdNextSeq = 0;		
	}
	
	pbTxBuf[9] = bLen;

    //+0x33
    for (i=10; i<(WORD)bLen+10; i++)
	{
  	    pbTxBuf[i] += 0x33;
	}	 
	
	pbTxBuf[10+(WORD)bLen] = CheckSum(pbTxBuf, (WORD)bLen+10);
	pbTxBuf[11+(WORD)bLen] = 0x16;

	return bLen+12;
}  

//描述：帧解析
int DL645V07TxRx(struct TMtrPro* pMtrPro, TV07Tmp* pTmpV07, TItemList* ptItem, WORD wLen)
{	
	bool fReadSuccess;
	TMtrPara* pMtrPara = pMtrPro->pMtrPara;
	//TV07Priv* pMtrV07 = (TV07Priv* )pMtrPro->pvMtrPro;	
	BYTE* pbRxBuf = pMtrPro->pbRxBuf;
	BYTE* pbTxBuf = pMtrPro->pbTxBuf;
	if (MtrProSend(pMtrPara->CommPara.wPort, pbTxBuf, wLen) != wLen)
	{
		DTRACE(DB_DL645V07, ("TxRx : fail to write comm.\r\n")); 
		return 0;
	}
	
	pTmpV07->nRxStep = 0;	

	if ((ptItem->dwProId&0xffff00ff)==0x03300001 && ptItem->wProLen>100) //对于编程次数等数据量大的ID,读取时间放慢
	{
		DTRACE(DB_DL645V07, ("TxRx :%x delay time to read! \r\n", ptItem->dwProId)); 
		fReadSuccess = ReadCommFrm(pMtrPro, (void*)pTmpV07, 100, 5, 2, 200, MTR_FRM_SIZE, 0, NULL, 0);
	}
	else
		fReadSuccess = ReadCommFrm(pMtrPro, (void*)pTmpV07, 0, 4, 2, 200, MTR_FRM_SIZE, 0, NULL, 0);

	if (fReadSuccess)	//接收到一个完整的帧
	{	
		if ((pbRxBuf[DL645V07_CMD]&DL645V07_CMD_GET) == (pbTxBuf[DL645V07_CMD]&DL645V07_CMD_GET))
		{
			DWORD dwRxId = pbRxBuf[DL645V07_DATA] + (DWORD)(pbRxBuf[DL645V07_DATA+1]<<8) + (DWORD)(pbRxBuf[DL645V07_DATA+2]<<16) + (DWORD)(pbRxBuf[DL645V07_DATA+3]<<24);
			if ((pbRxBuf[DL645V07_CMD]-pbTxBuf[DL645V07_CMD]) == 0x80)   //帧校验正确、正常回答
			{		
				pTmpV07->fRdNext = false;
				if (dwRxId != ptItem->dwProId)											
					DTRACE(DB_DL645V07, (" Tx_ID:%x != Rx_ID:\r\n", ptItem->dwProId)); 
				else					
	 	 			return 1;
							
			}
			else if ((pbRxBuf[DL645V07_CMD]-pbTxBuf[DL645V07_CMD]) == 0xC0) //帧校验正确、数据错误
			{
				DTRACE(DB_DL645V07, ("TxRx : Tx_ID:%x is not surport data.Err=%x \r\n", ptItem->dwProId, pbRxBuf[10])); 
				pTmpV07->fRdNext = false;
	 			return -1;
			}
			else if ((pbRxBuf[DL645V07_CMD]-pbTxBuf[DL645V07_CMD]) == 0xA0) //帧校验正确、有后续帧
			{
				DTRACE(DB_DL645V07, ("TxRx :Tx_ID:%x has next data \r\n", ptItem->dwProId)); 
				pTmpV07->fRdNext = true;
				return 1;
			}
		}
	}
	DTRACE(DB_DL645V07, ("TxRx : fail to rx frame.\r\n")); 

	return 0;
}


WORD GetDataVal(TItemList* ptItem, BYTE* pbSrcBuf, BYTE* pbDstBuf, WORD wSrcLen)
{ 
	WORD wId = ptItem->wDL645Id;
	WORD woLen = 0, wnLen = 0;	//旧645分项长度，新645分项长度
	WORD wmLen = 0, wtLen = 0;	//公共格式分项长度，公共格式块长度
	BYTE bSign = 0;//符号位
	int32 iVal32;
	BYTE i;	
	WORD n = 0, m = 0, wNum = 0;
	WORD weLen = 0;
	BYTE bBuf[100];
    WORD wLoc = 0;

	if (ptItem->wDL645Id == 0xd800)
		wId = 0xb66f;

	if ( IsV07CurveId(wId) ) //曲线冻结的解析 
	{	
		if (wId == 0xd100) //沙特表的15分钟增量曲线,内容直接为7字节一笔的记录数据
		{
			pbDstBuf[0] = wSrcLen/7;
			wtLen = 1+wSrcLen/7*7;
			memcpy(pbDstBuf+1, pbSrcBuf, wtLen-1);			
		}
		else if ((ptItem->dwProId>>24)==0x05 && wSrcLen>0) //整点冻结数据返回
		{
			memset(pbDstBuf, m_bInvdData, 22);
			pbDstBuf[0] = 1;	
			//memcpy(pbDstBuf+1, pbSrcBuf, wSrcLen);
			for (i=0; i<wSrcLen; i++)		//模拟表有AA分隔
			{
				if (pbSrcBuf[i] != 0xaa)							
					pbDstBuf[1+m++] = pbSrcBuf[i]; 
			}
			wtLen = 22;
		}
		else //曲线返回
		{		
			while(n < wSrcLen)
			{
				if (pbSrcBuf[n]==0xa0 && pbSrcBuf[n+1]==0xa0) //负荷块的起始码
				{
					weLen = pbSrcBuf[n+2]; //负荷记录字节数
					if (pbSrcBuf[n+weLen+4]==0xe5 && pbSrcBuf[n+weLen+3]==CheckSum(pbSrcBuf+n, weLen+3)) //验证每笔的校验与块结束合格
					{
						for (i=0; i<weLen; i++)		//检出有效时间及数据
						{
							if (pbSrcBuf[n+3+i] != 0xaa)							
								pbDstBuf[1+m++] = pbSrcBuf[n+3+i]; 
						}
						wNum++;
					}	
					n += (weLen+5); 
				}
				else	break;	//不正确的数据
			}	
			wtLen = m;

			if (wNum > 0)
			{
				pbDstBuf[0] = wNum;	//取得的有效的记录条数
				wtLen = m + 1;		
			}
		}		
	}
	else if (IsSpecId(wId) || (wId>>12)==0x9) //电量(含分相电能)、需量、需量时间,精度和长度与老的DL645都一样
	{			
		if (wId == 0x9a00) //上一次日冻结时间
		{
			memcpy(pbDstBuf, pbSrcBuf,  ptItem->wDL645Len);
			wtLen = ptItem->wDL645Len;		
			return wtLen;
		}
		else if ( IsDemdId(wId) )  //需量及需量时间在07版645里是合成一个ID的，需特殊处理
		{	
			wnLen = 8;
			if ( IsDemdTime(wId) )	//需量时间(去掉年)	xzz 南网不要去掉年	
			{
				woLen = 5;				
				for (i=0; i<ptItem->wDL645Len/woLen; i++)										
					memcpy(pbDstBuf+i*woLen, pbSrcBuf+i*wnLen+3, woLen);				
			}
			else //最大需量	
			{
				woLen = 3;				
				if ( IsCmbReEng(ptItem->dwProId) )//组合无功需量(含各分相)
					bSign = 1;	
				else 
					bSign = 0;				

				for (i=0; i<ptItem->wDL645Len/woLen; i++)		
				{
					if (bSign && (pbSrcBuf[i*wnLen+woLen-1]&0x80)==0x80)	//最高位去掉符号位
						pbSrcBuf[i*wnLen+woLen-1] &= 0x7f;

					memcpy(pbDstBuf+i*woLen, pbSrcBuf+i*wnLen, woLen);													
				}
			}				
		}
		else //电量(只取4个费率的长度)
		{
			woLen = 4;
			wnLen = 4;		

			if ( IsCmbReEng(ptItem->dwProId) )//组合无功(含各分相)			
				bSign = 1;	
			else
				bSign = 0;

			for (i=0; i<ptItem->wDL645Len/woLen; i++)		
			{
				if (bSign && (pbSrcBuf[i*wnLen+woLen-1]&0x80)==0x80)	//最高位去掉符号位
					pbSrcBuf[i*wnLen+woLen-1] &= 0x7f;

				memcpy(pbDstBuf+i*woLen, pbSrcBuf+i*wnLen, woLen);					
			}		
		}

		wtLen = Data645ToComm(ptItem->wDL645Id, pbDstBuf, ptItem->wDL645Len); //2010-06-22 	包含调整小数点到公共格式
	}
	else
	{
		switch(wId>>8)
		{	
			case 0xb6:	//瞬时量
				wmLen = sizeof(int32);		
			
				if ((wId>>4)==0xb61 || (wId>>4)==0xb65 || (wId>>4)==0xb66 || wId==0xb680)
				{
					woLen = 2;
					wnLen = 2;	
				}
				else  if ((wId>>4)==0xb62 || wId==0xb6a0)
				{
					woLen = 3;
					wnLen = 3;	
				}
				else if ((wId>>4)==0xb63 || (wId>>4)==0xb64 || (wId>>4)==0xb67)
				{
					woLen = 3;
					wnLen = 3;
				}
				else 
					break;
				
				memset(bBuf, m_bInvdData, ptItem->wDL645Len); //有的表块数据返回的长度可能不全				
				memcpy(bBuf, pbSrcBuf, wSrcLen);

				for (i=0; i<ptItem->wDL645Len/woLen; i++)
				{
					if ((pbSrcBuf[i*wnLen+wnLen-1]&0x80)==0x80 && (wId>>4)!=0xb61 && (wId>>4)!=0xb68) //有符号(电压以及频率为不带符号数据)
					{
						bSign = 1;
						bBuf[i*wnLen+wnLen-1] &= 0x7f;
					}
					else
						bSign = 0;

					if ( !IsBcdCode(bBuf+i*wnLen, wnLen) ) //对单相表，有不支持的分相数据返回FF的要避开
						iVal32 = INVALID_VAL;
					else
					{						
						/*if ((wId>>4) == 0xb61) //调整小数点到公共格式
							CheckDecimalNew((BYTE)wnLen, (BYTE)woLen, 1, m_pMeterPara->bVolPtPos, &pbSrcBuf[i*wnLen], &bBuf[i*woLen]);								
						else if ((wId>>4)==0xb62 || (wId>>4)==0xb6a) //调整小数点到公共格式
							CheckDecimalNew((BYTE)wnLen, (BYTE)woLen, 3, m_pMeterPara->bCurPtPos, &pbSrcBuf[i*wnLen], &bBuf[i*woLen]);							
						else if ((wId>>4) == 0xb65) //调整小数点到公共格式
							CheckDecimalNew((BYTE)wnLen, (BYTE)woLen, 3, m_pMeterPara->bPowerFactorPtPos, &pbSrcBuf[i*wnLen], &bBuf[i*woLen]);	
						else if ((wId>>4)==0xb63 || (wId>>4)==0xb67) //调整小数点到公共格式
							CheckDecimalNew((BYTE)wnLen, (BYTE)woLen, 4, m_pMeterPara->bActPowerPtPos, &pbSrcBuf[i*wnLen], &bBuf[i*woLen]);								
						else if ((wId>>4) == 0xb64 ) //调整小数点到公共格式
							CheckDecimalNew((BYTE)wnLen, (BYTE)woLen, 4, m_pMeterPara->bReActPowerPtPos, &pbSrcBuf[i*wnLen], &bBuf[i*woLen]);	
							*/
						
						iVal32 = BcdToDWORD(&bBuf[i*wnLen], wnLen);	
						if (bSign != 0)
							iVal32 = -iVal32;						
					}
					
					memcpy(pbDstBuf+i*wmLen, (BYTE*)&iVal32, wmLen);
				}
			
				wtLen = i*wmLen;
				break;
			case 0xb2: //编程时间之类
				if (wId == 0xb210) //去掉年、秒
					memcpy(pbDstBuf, pbSrcBuf+1, ptItem->wDL645Len);
				else if (wId==0xb212 || wId==0xb213 || wId==0xb214) //去掉1个字节
					memcpy(pbDstBuf, pbSrcBuf, ptItem->wDL645Len);
				else if (wId == 0xb211)
					memcpy(pbDstBuf, pbSrcBuf+1, ptItem->wDL645Len);//去掉年和秒以及记录数据
				else
					break;		

				wtLen = ptItem->wDL645Len;
				break;
			case 0xb3: //断相之类				            
				if ((wId>>4)==0xb33 || (wId>>4)==0xb34)
				{
					woLen = 4;
					wnLen = 6;					
					wLoc = 1;

					memcpy(pbDstBuf, pbSrcBuf+wLoc, woLen);	
					wtLen = woLen;					
				}			
				else if ((wId>>4) == 0xb31 || (wId>>4) == 0xb32)
				{
					memcpy(pbDstBuf, pbSrcBuf,  ptItem->wDL645Len);
					wtLen = ptItem->wDL645Len;					
				}
				break;
			//2007版645扩展ID的数据
			case 0xc8: //事件记录类
				wtLen = ptItem->wDL645Len;
				if (wId == 0xc851) //校时记录需除掉操作者代码
					memcpy(pbDstBuf, pbSrcBuf+4, wtLen);
				else if (wId == 0xc854) //校时记录需除掉操作者代码
					memcpy(pbDstBuf, pbSrcBuf+10, wtLen);
				else
					memcpy(pbDstBuf, pbSrcBuf, wtLen);			
				break;
			case 0xc3: //时段费率
				wtLen = ptItem->wDL645Len;
				if ((wId&0xf)==0xf || (wId>>4)==0xc31) 
					memcpy(pbDstBuf, pbSrcBuf,  ptItem->wDL645Len);
				else
					memcpy(pbDstBuf, pbSrcBuf+((wId&0xf)-1)*3, wtLen);			
				break;		
			case 0xc9: 
				wtLen = ptItem->wDL645Len;
				if (wId == 0xc9e0) //通断电状态
				{				
					if ((pbSrcBuf[0] & 0x10)==0x10)
						pbDstBuf[0]=0;
					else
						pbDstBuf[0]=0x11; 					
				}
				else
				{
					memcpy(pbDstBuf, pbSrcBuf,  ptItem->wDL645Len);				
				}
				break;	
			case 0x12://谐波含量
			{
				DWORD dwVal;
				BYTE bTotalNum;
				wtLen = 0;
				if ((wId>>4) == 0x128) 
				{
					bTotalNum = ((wSrcLen>>1) <= 19)?(wSrcLen>>1):19;

					if (wId == 0x1280) //A相时计算谐波次数添上
						pbDstBuf[wtLen++] = bTotalNum;

					if (wId <= 0x1282) //谐波电压要添上总含量
					{
						memset(pbDstBuf+wtLen, m_bInvdData, 2);
						wtLen += 2;
					}

					for (i=0; i<bTotalNum-1; i++) //只取2～19次谐波
					{
						dwVal = (BcdToDWORD(pbSrcBuf+2*i+2, 2)+5)/10;
						DWORDToBCD(dwVal, pbDstBuf+wtLen, 2);
						wtLen +=2;
					}
				}
				break;
			}
			case 0xd2://沙特表扩展
				memset(pbDstBuf, m_bInvdData, ptItem->wDL645Len);
				memcpy(pbDstBuf, pbSrcBuf, wSrcLen);				
				wtLen = ptItem->wDL645Len;
				break;
			default:
				//0xc020、0xC314去掉了多余的高字节
				memcpy(pbDstBuf, pbSrcBuf,  ptItem->wDL645Len);
				wtLen = ptItem->wDL645Len;
				break;
		}
	}	
	return wtLen;
}

//描述：读取2007版645支持的数据标识的数据
int DL645V07AskItem1(struct TMtrPro* pMtrPro, TV07Tmp* pTmpV07, WORD wID, BYTE* pbBuf, WORD wSubIdx)
{		
	int iRet, iLen;
	TItemList tItem;
	BYTE eLen, bCmd, n;	
	WORD wFrmLen, wRxPtr=0;
	WORD wTmpId = wID;
	DWORD dwProId; 
	BYTE mBuf[MTR_FRM_SIZE];		

	TMtrPara* pMtrPara = pMtrPro->pMtrPara;	
	//TV07Priv* pMtrV07 = (TV07Priv* )pMtrPro->pvMtrPro;	
	BYTE* pbTxBuf = pMtrPro->pbTxBuf; 
	BYTE* pbRxBuf = pMtrPro->pbRxBuf; 
	
	if ( !MtrProOpenComm(&pMtrPara->CommPara) )		
		return 0;
	
	if ( !DL645toDL645V07(pMtrPara->wPn, wTmpId, &tItem) ) 
		return -1;
	
	memset(mBuf, 0, MTR_FRM_SIZE);

	dwProId = tItem.dwProId + wSubIdx;
	tItem.dwProId = dwProId;	
	
	for (n=0; n<TXRX_RETRYNUM; n++) //因为有续帧，所以重发需改成整帧重发
	{
		wRxPtr = 0;
		pTmpV07->fRdNext = false;

		if(n > 0) //重发的时候先清除一下串口
		{			
		    CommRead(pMtrPara->CommPara.wPort, NULL, 0, 200);					
		}
	
		do
		{
			memcpy(pbTxBuf+DL645V07_DATA, (BYTE*)&dwProId, sizeof(DWORD));	

			if ( pTmpV07->fRdNext )
				bCmd = DL645V07_CMD_ASK_NEXT;			
			else
				bCmd = DL645V07_CMD_ASK_DATA;

			if (!pTmpV07->fRdNext && ((dwProId&0xff00ffff)==0x06000001 || wID==0xd100)) //读给定时间记录块的首帧
			{
				BYTE bInx = 4;
				pbTxBuf[DL645V07_DATA+bInx++] = pTmpV07->tRdLoadInfo.bNum;
				if (wID == 0xd100)	
					pbTxBuf[DL645V07_DATA+bInx++] = 0;	//沙特表个数为2字节
				pbTxBuf[DL645V07_DATA+bInx++] = ByteToBcd(pTmpV07->tRdLoadInfo.tmStart.nMinute);
				pbTxBuf[DL645V07_DATA+bInx++] = ByteToBcd(pTmpV07->tRdLoadInfo.tmStart.nHour);
				pbTxBuf[DL645V07_DATA+bInx++] = ByteToBcd(pTmpV07->tRdLoadInfo.tmStart.nDay);
				pbTxBuf[DL645V07_DATA+bInx++] = ByteToBcd(pTmpV07->tRdLoadInfo.tmStart.nMonth);
				pbTxBuf[DL645V07_DATA+bInx++] = ByteToBcd(pTmpV07->tRdLoadInfo.tmStart.nYear-2000);					
				wFrmLen = DL645V07MakeFrm(pTmpV07, pbTxBuf, pMtrPara->bAddr, bCmd, bInx);
			}
			else wFrmLen = DL645V07MakeFrm(pTmpV07, pbTxBuf, pMtrPara->bAddr, bCmd, 4);	
            	
            if (g_fDirRd && !g_bDirRdStep)
                return 0;

			iRet = DL645V07TxRx(pMtrPro, pTmpV07, &tItem, wFrmLen);
			if (iRet < 0)		
			{
				if (bCmd==DL645V07_CMD_ASK_NEXT && wRxPtr>0) //是续帧不支持，需要解析前帧
				{
					iRet = wRxPtr;
					break;
				}
				else
					return iRet;
			}
			else if (iRet == 0)	
			{
				if (bCmd==DL645V07_CMD_ASK_NEXT && wRxPtr>0) //是续帧通信失败，需要解析前帧				
					iRet = wRxPtr;					
							
				break; 
			}
			
			if (((wID&0x000f)==0x000f || (tItem.dwProId&0x000000ff)==0x000000ff || (tItem.dwProId&0x0000ff00)==0x0000ff00)
				&& (pbRxBuf[DL645V07_DATA+pTmpV07->wRxDataLen-1]==0xaa || pbRxBuf[DL645V07_DATA+pTmpV07->wRxDataLen-1]==0xdd))//科陆表实测		
				iLen = pTmpV07->wRxDataLen-5;			
			else		
				iLen = pTmpV07->wRxDataLen-4;	
			
			if (iLen<0 || (wRxPtr+iLen)>=MTR_FRM_SIZE)
			{
				DTRACE(DB_DL645V07, ("AskItem1 : Reading break for wPn=%d, iLen=%d wRxPtr=%d.\r\n", pMtrPara->wPn, iLen, wRxPtr));

				if (bCmd==DL645V07_CMD_ASK_NEXT && wRxPtr>0) //是续帧长度不正确，需要解析前帧
				{
					iRet = wRxPtr;
					break;
				}
				else
					return 0;
			}
			else 
			{
				if (bCmd == DL645V07_CMD_ASK_NEXT)
				{
					if (pbRxBuf[DL645V07_DATA+4+iLen-1] != pTmpV07->bRdNextSeq)
					{
						if (wRxPtr > 0) //是续帧序号不对，需要解析前帧
						{
							iRet = wRxPtr;
							break;
						}
						else
							return 0;
					}
					else //去掉序号
					{
						memcpy(mBuf+wRxPtr, pbRxBuf+DL645V07_DATA+4, iLen-1);
						wRxPtr += iLen-1;
					}
				}
				else
				{
					memcpy(mBuf+wRxPtr, pbRxBuf+DL645V07_DATA+4, iLen);
					wRxPtr += iLen;
				}
			}

		}while (pTmpV07->fRdNext);
		

		if (iRet > 0)
			break;

		if (IsCctPn(pMtrPara->wPn))
		{
			break;;//载波抄表不补抄,可能后面还要解决后续帧抄读,待优化
		}
	}	 

	 if (iRet == 0)	 //两遍均读取失败
		return 0;

	 if (IsV07CurveId(wID) && ((dwProId>>24)==0x06 || wID==0xd100) && wRxPtr<=8) //曲线数据块返回不正确
	 {
		 DTRACE(DB_DL645V07, ("AskItem1 : Tx_ID:%x has no data for = %d-%02d-%02d:%02d:%02d. \r\n", wID,
			 pTmpV07->tRdLoadInfo.tmStart.nYear, pTmpV07->tRdLoadInfo.tmStart.nMonth, 
			 pTmpV07->tRdLoadInfo.tmStart.nDay, pTmpV07->tRdLoadInfo.tmStart.nHour, pTmpV07->tRdLoadInfo.tmStart.nMinute)); 
		 return -4; //注意曲线的返回不能用-1,此处并非不支持,而是电表曲线冻结无此记录但通信正常
	 }

	if ((wID&0xf) == 0xf) //费率转换
	{	
		//AdjRateNumByRdEng(wID, wRxPtr); //根据电能数据返回长度调整费率数

		if ((dwProId&0xff00FF00) == 0x0100FF00 || dwProId>=0x05000901 && dwProId<=0x05000a01) //需量块
		{			
			eLen = 8;	
			iLen = TOTAL_RATE_NUM*eLen; //长度过长的截掉，过短的补上			
			CheckRate(pMtrPara->bRateTab, mBuf+eLen, eLen);	

			if (wRxPtr == eLen) //若块只返回一个总量数据,则将总量也拷贝到费率1
				memcpy(mBuf+eLen, mBuf, eLen);
		}
		else if ((dwProId&0xff00FF00) == 0x0000FF00 || dwProId>=0x05000101 && dwProId<=0x05000801) //电量块
		{				
			eLen = 4;	
			iLen = TOTAL_RATE_NUM*eLen;	
			CheckRate(pMtrPara->bRateTab, mBuf+eLen, eLen);	

			if (wRxPtr == eLen) //若块只返回一个总量数据,则将总量也拷贝到费率1
				memcpy(mBuf+eLen, mBuf, eLen);
		}		
		else if (dwProId>=0x04010000 && dwProId<=0x04020008)//费率时段块
		{			
			if (iLen != 42)//长度正常的	
			{
				iLen = 42;				
			}
		}			
	}	

	return GetDataVal(&tItem, mBuf, pbBuf, wRxPtr);
}

int MtrReadFrz(struct TMtrPro* pMtrPro, TV07Tmp* pTmpV07, WORD wID, BYTE* pbBuf, WORD wSubIdx)
{
	return DL645V07AskItem1(pMtrPro, pTmpV07, wID, pbBuf, wSubIdx);
}
	

