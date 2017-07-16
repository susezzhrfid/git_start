/*********************************************************************************************************
* Copyright (c) 2010,深圳科陆电子科技股份有限公司
* All rights reserved.
*
* 文件名称：Flash.h
* 摘    要：本文件主要用于对FLASH接口进行封装
* 当前版本：1.0
* 作    者：岑坚宇
* 完成日期：2010-12-28
*********************************************************************************************************/
#include "FlashMgr.h"

#define INFLASH_SIZE	0x80000	//2*256K
//#define INSECT_SIZE		2048	//片内FLASH扇区大小
#define INPROG_ADDR		0x80000		//片内Flash程序空间起始地址
#define ININFO_ADDR		0x8E800		//片内Flash信息域起始地址
#define INAPP_ADDR		0x8E900		//片内Flash应用程序起始地址
#define IN_PARACFG_ADDR 0xF2900     //片内Flash参数配置文件地址
#define IN_PARACFG_SIZE 0xF00       //片内Flash参数配置文件大小  3.75K
#define INDATA_ADDR		0xF3800		//片内Flash数据空间起始地址
//#define INDATA_SIZE     0xC800     //FLASH数据空间大小  50K

#define BL_ADDR                INPROG_ADDR
#define USER_BOOT_VER_ADDR     ININFO_ADDR    //32字节
#define BL_LEN                 (USER_BOOT_VER_ADDR-BL_ADDR)   //58K  BOOTLOADER
#define BL_USE_LEN             (USER_BOOT_VER_ADDR+32+4+4)

#define INFLASH_LOCK_ADDR    INPROG_ADDR   //注意如果要升级BOOTLOADER必须打开此处
#define INFLASH_END     0x100000     //片内FLASH结束地址

#define EXFLASH_1_SIZE  0x1000000   //第一片FLASH大小
#define EXFLASH_2_SIZE  0x1000000   //第二片FLASH大小
#define EXFLASH_SIZE	(EXFLASH_1_SIZE + EXFLASH_2_SIZE)//0x2000000   //32M
//#define EXSECT_SIZE	(4*1024)	//4K 片外Flash扇区大小
#define EXFLASH_PAGE_SIZE	256			//256个字节 片外Flash页大小（页：写操作的单位）

//#define EXFLASH_PARA_OFFSET	    FADDR_EXTPARA		//片外Flash扩展参数起始偏移地址
#define EXFLASH_ADDR	0	//Flash空间起始地址
#define EXFLASH_FST_ADDR	EXFLASH_ADDR
#define EXFLASH_SND_ADDR	(EXFLASH_ADDR + EXFLASH_1_SIZE)

#define INFLASH_RD_LEN	4	//内部Flash一次读的长度，4个字节

#define FLASH_ERR_OK			-1	//没有错误
#define FLASH_ERR_UNKNOW		-2	//未知错误
#define FLASH_ERR_PARAMETER 	-3	//参数错误
#define FLASH_ERR_RD_FAIL		-4	//读失败
#define FLASH_ERR_ERASE			-5	//扇区擦除失败
#define FLASH_ERR_WRT_FAIL		-6	//写失败
#define FLASH_ERR_ERASE_CHIP	-7	//芯片擦除失败

//驱动用到的外部Flash命令
#define CMD_EXFSH_WRT_EN	0x06	//写允许
#define CMD_EXFSH_WRT_DIS	0x04	//写禁止
#define CMD_EXFSH_RD		0x03	//读命令
#define CMD_EXFSH_SCT_ERS	0x20	//扇区擦除命令
#define CMD_EXFSH_CHIP_ERS	0x60	//Flash片擦除命令
#define CMD_EXFSH_GET_ID	0x90	//读生产ID (8 bits manufacturer ID & 8bits device ID)
#define CMD_EXFSH_GET_ID2	0x9f	//Manufacturer ID(8 bits) & device ID(16 bits)
#define CMD_EXFSH_WRT		0x02	//写命令---写是以256的PAGE为单位
#define CMD_EXFSH_REG_L		0x05	//状态寄存器的低位
#define CMD_EXFSH_REG_H		0x35	//状态寄存器的高位
#define CMD_EXFSH_BLK_32_ERS 0x52   //32K块擦除
#define CMD_EXFSH_BLK_64_ERS 0xd8   //64K块控除

//外部Flash状态
#define STATUS_EXFSH_BUSY	0x01	//WIP位为1
#define STATUS_EXFSH_WRT_EN	0x02	//WEL位为1

#define   FLASH_BASE_MAIN             0x005C00//0x004400            //主Flash的起始地址

//升级BOOTLOADER有关
#define EX_UPDATA_FLAG_ADDR      0x0000
#define EX_USER_APP_NAME_ADDR    (EX_UPDATA_FLAG_ADDR+2)
#define EX_USER_APP_LEN_ADDR     (EX_USER_APP_NAME_ADDR+16)
#define EX_USER_APP_CRC_ADDR     (EX_USER_APP_LEN_ADDR+4)
#define EX_USER_APP_TYPE         (EX_USER_APP_CRC_ADDR+2)//程序类型
#define EX_USER_APP_NO           (EX_USER_APP_TYPE+1)
#define EX_USER_APP_ADDR         ((EX_USER_APP_NO+1)+2) //加2揍成4的整数倍




#define EX_UPDATA_FLAG_1         0x55
#define EX_UPDATA_FLAG_2         0xaa
//------------------------------------------------------------
//内部FLASH操作禁止
//描述：当CPU检测到掉电后，应该禁止内部FLASH的写操作
void DisInFlash(void);

// 内部Flash相关
//描述：内部Flash初始化
//参数：NONE
//返回：NONE
void InFlashInit(void);

//描述：内部地址转换
//参数：@dwLogicAddr 参数逻辑偏移
//返回：参数在Flash中的绝对地址
BYTE* ParaAddrConv(DWORD dwLogicAddr);

//描述：内部Flash读
//参数：@dwAddr 读内部Flash的地址
//		@pbBuf 读内部Flash的缓冲区
//		@dwLen 读数据的长度
//返回：正确读出返回读数据的长度，否则返回相应错误码
int InFlashRd(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen);	//内部FLASH不限定扇区的读

//描述：内部Flash写限定扇区
//参数：@dwAddr 写内部Flash的地址
//		@pbBuf 写入内部Flash的内容
//		@dwSectSize 写入内部Flash的大小
//返回：正确写入返回写入长度，否则返回相应错误码
int InFlashWrSect(DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize); 

//描述：清除系统库存放在内部Flash的全部内容
//参数：NONE
//返回：正确清除返回1，否则返回相应错误码
int ClrAllPara();

//描述：BootLoader更新
//返回：正确写入返回写入长度，否则返回相应错误码
bool BootLoaderUpd(void);

//描述：片内Flash格式化，将参数段全部清除
//参数：NONE
//返回：正确返回true,错误返回false
bool InFlashFormat();

//描述：获取BOOTLOADER的版本号
//返回：版本号的长度
BYTE GetBootVer(char *pcBuf, BYTE bBufSize);

//检查并升级备份区的BOOTLOADER程序
void UpdatBL(void);

//本函数可以编程任意内部FLASH区域，慎用
bool Program(BYTE* pbBuf, DWORD dwAddr, DWORD dwLen);

//内部Flash相关函数定义结束
//------------------------------------------------------------
//外部FLASH操作禁止
//描述：当CPU检测到掉电后，应该禁止外部FLASH的写操作
void DisExFlash(void);

// 外部Flash相关
//描述：外部Flash初始化
//参数：NONE
//返回：NONE
void ExFlashInit(void);

//描述:擦除外部Flash的指定扇区
//参数：@dwSectAddr 擦除扇区的起始地址
//返回：如果正确擦除返回ERR_OK（0），否则返回相应错误码
int ExFlashEraseSect(DWORD dwSectAddr);

//描述:擦除外部Flash的指定块
//参数：@dwBlockAddr 擦除块的起始地址
//      @bBlockType  0-32K的块，1-64K的块
//返回：如果正确擦除返回ERR_OK（0），否则返回相应错误码
int ExFlashEraseBlock(DWORD dwBlockAddr, BYTE bBlockType);

//描述：外部Flash写限定扇区
//参数：@dwAddr 写外部Flash的地址
//		@pbBuf 写入外部Flash的内容
//		@dwSectSize 写入外部Flash的大小
//返回：正确写入返回写入长度，否则返回相应错误码
int ExFlashWrSect(DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize);	//外部FLASH限定扇区的写

//描述：外部Flash读
//参数：@dwAddr 读外部Flash的地址
//		@pbBuf 读外部Flash的缓冲区
//		@dwLen 读数据的长度
//返回：正确读出返回读数据的长度，否则返回相应错误码
int ExFlashRd(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen);	//外部FLASH不限定扇区的读

//描述:外部Flash保护
//参数：@bChip Flash片选信息
//		@dwWaitMS 超时等待时间
//返回：正常等到Flash使用权返回true，否则返回false
bool ExFlashPret(BYTE bChip, DWORD dwWaitMS);

//描述:擦除外部Flash的指定扇区
//参数：@dwSectAddr 擦除扇区的起始地址
//返回：如果正确擦除返回ERR_OK（0），否则返回相应错误码
int ExFlashEraseChip();

//获取片外备份程序的信息
bool GetBakPara(char *psName, DWORD *pdwLen, WORD *pwCrc);