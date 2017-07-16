/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：LibDbStruct.h
 * 摘    要：本文件主要实现标准系统库库的数据结构定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
 *********************************************************************************************************/
#ifndef LIBDBSTRUCT_H
#define LIBDBSTRUCT_H
#include "sysarch.h"
#include "LibDbConst.h"
#include "LibDbCfg.h"


//数据项属性标志位
#define DI_READ     (1<<0)
#define DI_WRITE    (1<<1)
#define DI_NTS     	(1<<2)		//No TimeStamp 	
								//在一个带时标的BANK中,某个数据项置上本标志,就表示该数据项不支持时标
								//主要用在BANK1后面的BANK,数据有带时标和不带时标的,但是又不能分开SECT的情况
								//可以整个BANK配置成支持时标访问,但个别数据项配置成不支持时标
#define DI_CMB     	(1<<3)		//组合ID,加上本标志且数据项长度配置为0,则不分配存储空间和时标空间
#define DI_SELF	 	(1<<4)		//本数据项是否能自成独立数据项
#define DI_BLK		(1<<5)		//块ID
#define DI_NSP     	(1<<6)		//No Space 不占用空间，使用其长度信息，且占用时间标志

//权限占用(7,8)暂不定义


//扩展协议用，系统库目前不用的定义
#define DI_LOW_PERM    0x00
#define DI_HIGH_PERM   0x11
#define DI_PRG_PERM    0x40		//编程开关闭合


typedef struct{
    WORD  wID;			//数据项标识
    WORD  wLen;			//数据项长度,块数据项在数据项描述表中必须配置为0
    WORD  wProp;		//*数据项属性标志位
    WORD  wOffset;		//相对所属数据块开始的偏移,上电时计算
						//如果同时存在f和ff数据块,则wOffset是ff数据块的偏移
    WORD  wPnNum;		//测量点的个数,只有块数据在数据项描述表中配置真实的个数,其它的数据项都配1
						//如果数据项存在ff数据块,则f数据块也配1.
						//为的是在方便在初始化阶段计算数据项的偏移
	BYTE  bWrOp;		//*写完数据项时的数据库执行的自动操作,填写要发送的消息号
	BYTE  bPnMapSch;	//本数据项选用的动态测量点方案号,0x00表示不支持动态测量点
	

	//以下字段在上电时计算,数据项描述表中不用填写
	DWORD dwBlockStart;  //块数据的开始数据项索引
	DWORD dwBlockOffset; //块数据的相对于本数据表开头的偏移
						 //如果同时存在f和ff数据块,则dwBlockOffset是ff数据块的偏移
	
	DWORD dwBlkIndex;	 //块在数据库中的起始索引,按照测量点展开后
	WORD  wBlockLen; 	 //块数据的长度
	BYTE  bBlkIndexNum;	 //数据项所在整块的数据项数,最大为2级块
	BYTE  bBlkIdIndexNum;//块ID自己含有的子数据项的个数	
	BYTE  bInnerIndex;	 //本数据项相在块内的序号,按照测量点展开后的顺序
}TItemDesc;         //数据项描述

//一个数据项的偏移:dwBlockOffset+测量点*wBlockLen+wOffset
//数据项的拷贝长度:wLen
//按照测量点展开后一个数据项的索引:dwBlkIndex+测量点*bBlkIndexNum+bInnerIndex

typedef struct{
	WORD wLen;		//长度
	WORD wSect;		//段
	WORD wPnNum;	//测量点个数
}TItemInfo;		//数据项的信息


typedef struct{
	WORD wID;     	//BANK0或者645ID
	WORD wPn;  		//测量点号
}TBank0Item;		//主要用来进行数据项的时间查询,
					//目前只支持BANK0,如果将来支持到别的BANK,名称可改为TBankItem,
					//且加上bBank字段

typedef struct{
	WORD wBn;  		//BANK号
	WORD wPn;  		//测量点号
	WORD wID;     	//数据项ID
}TBankItem;			//主要用来进行数据项的时间查询


//BANK存储类型
#define DB_STORAGE_RAM	0	//RAM
#define DB_STORAGE_IN	1	//内部FLASH固定存储
#define DB_STORAGE_EX	2	//外部FLASH固定存储
#define DB_STORAGE_DYN	3	//动态导入导出
#define DB_STORAGE_FRAM	4	//FRAM,FLASH跟RAM同时保存，目前靠保存在掉电变量中

#define DB_STORAGE_TYPE_NUM	5	//BANK存储类型个数

typedef struct{
	char*	   	pszBankName;	 //本BANK的名称
	WORD 		wFile;			//本BANK数据保存的文件号
	BYTE		bStorage;		//存储类型：RAM、INFLASH、EXFLASH

	const TItemDesc* pItemDesc;     //本BANK数据项描述表
    DWORD      	dwItemNum;     //本BANK数据项描述表的数据项个数
    const BYTE*	pbDefault;     //本BANK数据库的默认值
    DWORD  	   	dwDefaultSize; //本BANK数据库的默认值的大小
	BYTE	   	bVer;		  //本BANK数据库的当前版本,0表示没有版本管理
	WORD	   	wPnNum;		  //本BANK数据实际支持的测量点个数,
							  //按照整个BANK来支持测量点,而不是在BANK里的数据项描述来单独支持测量点
							  //两者只能居其一	
	bool	   	fUpdTime;      //本BANK数据是否需要更新时间
	BYTE	   	bPnMapSch;	  //本SECTION数据选用的动态测量点方案号,0x00表示不支持整个BANK地支持动态测量点

	//以下字段在上电时计算,描述表中不用填写
	bool		fSect;			//是否是SECT

    BYTE*  	   	pbBankData;	  //本BANK数据库的数据,
							  //如果有多个测量点,则只申请一个容纳所有测量点的大缓冲,
							  //pbBankData指向总的起始地址
	DWORD      	dwBankSize;    //本BANK数据的大小
	DWORD*	   	pdwIntervTime; //本BANK数据的当前间隔，只有在fUpdTime==true时使用
	BYTE*	   	pbTimeFlg;	   //本BANK数据的更新时间，只有在fUpdTime==true时使用
	bool	   	fMutiPnInDesc; //描述表中存在多个测量点的描述
	DWORD	   	dwTotalSize;	  //本BANK所有数据的大小,即pbBankData申请空间的大小
	DWORD      	dwIndexNum;    //数据项索引的个数,即dwItemNum按照测量点个数展开后的个数
							  //目前只统计分配空间(及时标)的数据项的个数,组合ID不分配
//	DWORD      	dwIndexSize;
	WORD	   	wFileNum;	  //一共分成多少个文件
	DWORD	   	dwFileSize;	  //每个文件的大小
	
	DWORD		dwMemUsage;	  //内存使用量,单位字节,包括数据和时标存储空间
	BYTE		bBN;		  //BANK号或者SECT号
}TBankCtrl;                   //数据库的组控制块


typedef struct{
	WORD	wMaxPn;		//名义上最大支持的测量点号
	WORD	wRealNum;	//实际支持的测量点数
	WORD 	wFile;		//映射表保存的文件号
	bool	fGenerateMapWhenNoneExist;	//当没有映射表的时候，自动生成一一对应的映射表，主要是应对版本升级

	//一下字段上电后初始化
	DWORD	dwFileSize;		//映射保存的文件大小
	WORD	wAllocSize;		//存储空间分配表的大小						 
	WORD*	pwPnToMemMap;	//测量点号到存储号的映射表(需要保存到文件系统)
							//前面两个WORD用来保存控制信息,其中第一个WORD是已经映射的个数,第二个保留
	BYTE*	pbAllocTab;		//存储空间分配表(不保存到文件系统,动态更新)
}TPnMapCtrl;       	//测量点动态映射控制结构

#define DI_ACESS_BUF		0	//按照缓冲区读写
#define DI_ACESS_INT32		1	//按照整形32位读写
#define DI_ACESS_INT64		2	//按照整形64位读写
#define DI_ACESS_QRY		3	//查询数据项是否更新
#define DI_ACESS_UPD		4	//更新数据项状态
#define DI_ACESS_GI			5	//GetItem
#define DI_ACESS_INFO		6	//取数据项信息(长度和段)
#define DI_ACESS_RDUNMAP	7	//按照非映射的方式读
#define DI_ACESS_WRUNMAP	8	//按照非映射的方式写

typedef struct{
	BYTE	bType;			//DI_ACESS_BUF,DI_ACESS_INT32,DI_ACESS_INT64
	WORD	wValNum;		//按值读写时数值的个数
	WORD	wValidNum;		//读数据时返回的合法子数据项的个数
	DWORD*	pdwTime;		//用来取数据项的时间,只有在DI_ACESS_BUF/DI_ACESS_INT32/DI_ACESS_INT64时可以使用
	union
	{
		BYTE*	pbBuf;			//按照缓冲区读写
		int*	piVal32;		//按照整形32位读写
		int64*  piVal64;		//按照整形64位读写	
		DWORD	dwVal;			//错误状态等
		TItemInfo* pItemInfo;   //数据项的信息
	};
}TItemAcess;

typedef struct{
	BYTE*  		pbAddr;		//只给了起始地址，不能保证数据项有没跨扇区，
							//应用程序在系统库编排的时候应该保证让直接地址访问的数据项不跨扇区
							//不能确定的话就用下面的dwOffset来访问
	DWORD 		dwOffset;
	WORD  		wLen;        //数据项长度
	//BYTE*  pbModified;
	//BYTE   bModifiedMask;
	const TBankCtrl*	pBankCtrl;
}TDataItem;

typedef struct{
	WORD wFrmBn;	//旧版本BANK
	WORD wFrmId;	//旧版本ID
	WORD wToBn;		//新版本BANK
	WORD wToId;		//新版本ID
	void (*pfnUpgFun)(WORD wFrmBn, WORD wFrmId, WORD wToBn, WORD wToId, int iPnNum);	//转换函数,为NULL时使用默认的转换函数
}TIdUpgMap;	//ID切换映射关系


typedef struct{
	BYTE 		bSchVer;		//升级方案的版本标识
	TBankCtrl* 	pBankCtrl;		//升级用到的临时BANK控制结构
	WORD 		wBankNum;		//支持的BANK数目

	TIdUpgMap*	pIdUpgMap;		//ID切换映射表
	DWORD 		dwIdNum;		//ID切换映射表的项目数
								 
	bool		fDelFile;		//升级后是否删除旧文件
	bool		fRstCPU;		//升级后是否复位终端
}TDbUpgCtrl;	//数据库版本升级控制

typedef struct{
	//BANK0的控制字段
	WORD 		wSectNum;	//BANK0中的SECT数目
	const TBankCtrl* 	pBank0Ctrl;

	//BANK控制字段
	WORD 		wBankNum;	//支持的BANK数目
	const TBankCtrl* 	pBankCtrl;

	int			iSectImg;		//485抄表数据镜像段,如果没有则配成-1
	WORD		wImgNum;		//485抄表数据镜像个数
	WORD		wSectPnData;	//对于485抄表镜像数据,需要有测量点数据与之对应,否则本参数配置成0即可
								 
	//测量点动态映射控制字段
	WORD 		wPnMapNum;	//支持的映射方案数目,整个数据库不支持测量点动态映射则设为0
	TPnMapCtrl*	pPnMapCtrl;	//整个数据库不支持测量点动态映射则设为NULL

	//其它一些参数配置
	WORD 		wPnMaskSize; //测量点屏蔽位的大小,用来分配电表测量点屏蔽位空间
	char* 		pszDbPath;	 //系统库一些控制文件的存放目录,一般配置为USER_PARA_PATH

	//数据库版本升级配置upgrade
	TDbUpgCtrl* pDbUpgCtrl;		//数据库版本升级控制,配置为NULL表示没有升级配置
	
	WORD		wSaveInterv;	//保存间隔,单位分钟
}TDbCtrl;	//外界对数据库进行参数配置的数据库控制结构

typedef struct{
	DWORD dwFileValidFlg0;	//BANK0文件有效标志位
	DWORD dwFileValidFlg1;	//扩展BANK文件有效标志位
}TDbData;	//DB DATA数据库临时数据

typedef struct{
	const TBankCtrl* pBankCtrl;	//被保持的BANK，为NULL表示无效
	WORD wFile;
	WORD wSect;
	bool fValid;	//当前保持数据是否有效
	bool fEnable;	//是否允许扇区数据保持功能
}TSectHolder;	//扇区数据保持结构

#endif //LIBDBSTRUCT_H