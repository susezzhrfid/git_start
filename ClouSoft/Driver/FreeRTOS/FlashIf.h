/*********************************************************************************************************
* Copyright (c) 2010,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�Flash.h
* ժ    Ҫ�����ļ���Ҫ���ڶ�FLASH�ӿڽ��з�װ
* ��ǰ�汾��1.0
* ��    �ߣ�᯼���
* ������ڣ�2010-12-28
*********************************************************************************************************/
#include "FlashMgr.h"

#define INFLASH_SIZE	0x80000	//2*256K
//#define INSECT_SIZE		2048	//Ƭ��FLASH������С
#define INPROG_ADDR		0x80000		//Ƭ��Flash����ռ���ʼ��ַ
#define ININFO_ADDR		0x8E800		//Ƭ��Flash��Ϣ����ʼ��ַ
#define INAPP_ADDR		0x8E900		//Ƭ��FlashӦ�ó�����ʼ��ַ
#define IN_PARACFG_ADDR 0xF2900     //Ƭ��Flash���������ļ���ַ
#define IN_PARACFG_SIZE 0xF00       //Ƭ��Flash���������ļ���С  3.75K
#define INDATA_ADDR		0xF3800		//Ƭ��Flash���ݿռ���ʼ��ַ
//#define INDATA_SIZE     0xC800     //FLASH���ݿռ��С  50K

#define BL_ADDR                INPROG_ADDR
#define USER_BOOT_VER_ADDR     ININFO_ADDR    //32�ֽ�
#define BL_LEN                 (USER_BOOT_VER_ADDR-BL_ADDR)   //58K  BOOTLOADER
#define BL_USE_LEN             (USER_BOOT_VER_ADDR+32+4+4)

#define INFLASH_LOCK_ADDR    INPROG_ADDR   //ע�����Ҫ����BOOTLOADER����򿪴˴�
#define INFLASH_END     0x100000     //Ƭ��FLASH������ַ

#define EXFLASH_1_SIZE  0x1000000   //��һƬFLASH��С
#define EXFLASH_2_SIZE  0x1000000   //�ڶ�ƬFLASH��С
#define EXFLASH_SIZE	(EXFLASH_1_SIZE + EXFLASH_2_SIZE)//0x2000000   //32M
//#define EXSECT_SIZE	(4*1024)	//4K Ƭ��Flash������С
#define EXFLASH_PAGE_SIZE	256			//256���ֽ� Ƭ��Flashҳ��С��ҳ��д�����ĵ�λ��

//#define EXFLASH_PARA_OFFSET	    FADDR_EXTPARA		//Ƭ��Flash��չ������ʼƫ�Ƶ�ַ
#define EXFLASH_ADDR	0	//Flash�ռ���ʼ��ַ
#define EXFLASH_FST_ADDR	EXFLASH_ADDR
#define EXFLASH_SND_ADDR	(EXFLASH_ADDR + EXFLASH_1_SIZE)

#define INFLASH_RD_LEN	4	//�ڲ�Flashһ�ζ��ĳ��ȣ�4���ֽ�

#define FLASH_ERR_OK			-1	//û�д���
#define FLASH_ERR_UNKNOW		-2	//δ֪����
#define FLASH_ERR_PARAMETER 	-3	//��������
#define FLASH_ERR_RD_FAIL		-4	//��ʧ��
#define FLASH_ERR_ERASE			-5	//��������ʧ��
#define FLASH_ERR_WRT_FAIL		-6	//дʧ��
#define FLASH_ERR_ERASE_CHIP	-7	//оƬ����ʧ��

//�����õ����ⲿFlash����
#define CMD_EXFSH_WRT_EN	0x06	//д����
#define CMD_EXFSH_WRT_DIS	0x04	//д��ֹ
#define CMD_EXFSH_RD		0x03	//������
#define CMD_EXFSH_SCT_ERS	0x20	//������������
#define CMD_EXFSH_CHIP_ERS	0x60	//FlashƬ��������
#define CMD_EXFSH_GET_ID	0x90	//������ID (8 bits manufacturer ID & 8bits device ID)
#define CMD_EXFSH_GET_ID2	0x9f	//Manufacturer ID(8 bits) & device ID(16 bits)
#define CMD_EXFSH_WRT		0x02	//д����---д����256��PAGEΪ��λ
#define CMD_EXFSH_REG_L		0x05	//״̬�Ĵ����ĵ�λ
#define CMD_EXFSH_REG_H		0x35	//״̬�Ĵ����ĸ�λ
#define CMD_EXFSH_BLK_32_ERS 0x52   //32K�����
#define CMD_EXFSH_BLK_64_ERS 0xd8   //64K��س�

//�ⲿFlash״̬
#define STATUS_EXFSH_BUSY	0x01	//WIPλΪ1
#define STATUS_EXFSH_WRT_EN	0x02	//WELλΪ1

#define   FLASH_BASE_MAIN             0x005C00//0x004400            //��Flash����ʼ��ַ

//����BOOTLOADER�й�
#define EX_UPDATA_FLAG_ADDR      0x0000
#define EX_USER_APP_NAME_ADDR    (EX_UPDATA_FLAG_ADDR+2)
#define EX_USER_APP_LEN_ADDR     (EX_USER_APP_NAME_ADDR+16)
#define EX_USER_APP_CRC_ADDR     (EX_USER_APP_LEN_ADDR+4)
#define EX_USER_APP_TYPE         (EX_USER_APP_CRC_ADDR+2)//��������
#define EX_USER_APP_NO           (EX_USER_APP_TYPE+1)
#define EX_USER_APP_ADDR         ((EX_USER_APP_NO+1)+2) //��2���4��������




#define EX_UPDATA_FLAG_1         0x55
#define EX_UPDATA_FLAG_2         0xaa
//------------------------------------------------------------
//�ڲ�FLASH������ֹ
//��������CPU��⵽�����Ӧ�ý�ֹ�ڲ�FLASH��д����
void DisInFlash(void);

// �ڲ�Flash���
//�������ڲ�Flash��ʼ��
//������NONE
//���أ�NONE
void InFlashInit(void);

//�������ڲ���ַת��
//������@dwLogicAddr �����߼�ƫ��
//���أ�������Flash�еľ��Ե�ַ
BYTE* ParaAddrConv(DWORD dwLogicAddr);

//�������ڲ�Flash��
//������@dwAddr ���ڲ�Flash�ĵ�ַ
//		@pbBuf ���ڲ�Flash�Ļ�����
//		@dwLen �����ݵĳ���
//���أ���ȷ�������ض����ݵĳ��ȣ����򷵻���Ӧ������
int InFlashRd(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen);	//�ڲ�FLASH���޶������Ķ�

//�������ڲ�Flashд�޶�����
//������@dwAddr д�ڲ�Flash�ĵ�ַ
//		@pbBuf д���ڲ�Flash������
//		@dwSectSize д���ڲ�Flash�Ĵ�С
//���أ���ȷд�뷵��д�볤�ȣ����򷵻���Ӧ������
int InFlashWrSect(DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize); 

//���������ϵͳ�������ڲ�Flash��ȫ������
//������NONE
//���أ���ȷ�������1�����򷵻���Ӧ������
int ClrAllPara();

//������BootLoader����
//���أ���ȷд�뷵��д�볤�ȣ����򷵻���Ӧ������
bool BootLoaderUpd(void);

//������Ƭ��Flash��ʽ������������ȫ�����
//������NONE
//���أ���ȷ����true,���󷵻�false
bool InFlashFormat();

//��������ȡBOOTLOADER�İ汾��
//���أ��汾�ŵĳ���
BYTE GetBootVer(char *pcBuf, BYTE bBufSize);

//��鲢������������BOOTLOADER����
void UpdatBL(void);

//���������Ա�������ڲ�FLASH��������
bool Program(BYTE* pbBuf, DWORD dwAddr, DWORD dwLen);

//�ڲ�Flash��غ����������
//------------------------------------------------------------
//�ⲿFLASH������ֹ
//��������CPU��⵽�����Ӧ�ý�ֹ�ⲿFLASH��д����
void DisExFlash(void);

// �ⲿFlash���
//�������ⲿFlash��ʼ��
//������NONE
//���أ�NONE
void ExFlashInit(void);

//����:�����ⲿFlash��ָ������
//������@dwSectAddr ������������ʼ��ַ
//���أ������ȷ��������ERR_OK��0�������򷵻���Ӧ������
int ExFlashEraseSect(DWORD dwSectAddr);

//����:�����ⲿFlash��ָ����
//������@dwBlockAddr ���������ʼ��ַ
//      @bBlockType  0-32K�Ŀ飬1-64K�Ŀ�
//���أ������ȷ��������ERR_OK��0�������򷵻���Ӧ������
int ExFlashEraseBlock(DWORD dwBlockAddr, BYTE bBlockType);

//�������ⲿFlashд�޶�����
//������@dwAddr д�ⲿFlash�ĵ�ַ
//		@pbBuf д���ⲿFlash������
//		@dwSectSize д���ⲿFlash�Ĵ�С
//���أ���ȷд�뷵��д�볤�ȣ����򷵻���Ӧ������
int ExFlashWrSect(DWORD dwAddr, BYTE* pbBuf, DWORD dwSectSize);	//�ⲿFLASH�޶�������д

//�������ⲿFlash��
//������@dwAddr ���ⲿFlash�ĵ�ַ
//		@pbBuf ���ⲿFlash�Ļ�����
//		@dwLen �����ݵĳ���
//���أ���ȷ�������ض����ݵĳ��ȣ����򷵻���Ӧ������
int ExFlashRd(DWORD dwAddr, BYTE* pbBuf, DWORD dwLen);	//�ⲿFLASH���޶������Ķ�

//����:�ⲿFlash����
//������@bChip FlashƬѡ��Ϣ
//		@dwWaitMS ��ʱ�ȴ�ʱ��
//���أ������ȵ�Flashʹ��Ȩ����true�����򷵻�false
bool ExFlashPret(BYTE bChip, DWORD dwWaitMS);

//����:�����ⲿFlash��ָ������
//������@dwSectAddr ������������ʼ��ַ
//���أ������ȷ��������ERR_OK��0�������򷵻���Ӧ������
int ExFlashEraseChip();

//��ȡƬ�ⱸ�ݳ������Ϣ
bool GetBakPara(char *psName, DWORD *pdwLen, WORD *pwCrc);