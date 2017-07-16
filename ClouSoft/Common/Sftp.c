/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Sftp.c
 * 摘    要：本文件实现了Sftp下载协议
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
#include "ComAPI.h"
#include "Trace.h"
#include "FaCfg.h"
#include "FlashMgr.h"
#include "Sftp.h"
#include "FaAPI.h"
#include "SysDebug.h"
#include "FlashIf.h"

#define	SFTP_CMD_DIR			1	//目录
#define	SFTP_CMD_RDF0 			2	//读文件启动帧
#define	SFTP_CMD_RDFN   		3	//读文件后续帧
#define	SFTP_CMD_WRF0   		4	//写文件启动帧
#define	SFTP_CMD_WRFN   		5	//写文件后续帧
#define	SFTP_CMD_FINISH 		6	//传输结束帧
#define	SFTP_CMD_CANCEL 		7	//取消传输帧

#define 	SFTP_ERR_OK         		0	//正确
#define 	SFTP_ERR_PATHNAME 	    1	//文件路径名错误
#define 	SFTP_ERR_FILE       		2	//打开文件失败
#define 	SFTP_ERR_READ       		3	//文件读错误
#define 	SFTP_ERR_WRITE      		4	//文件写错误
#define 	SFTP_ERR_FILEID     		5	//错误的文件标识
#define 	SFTP_ERR_OFFSET		6	//文件偏移错误
#define 	SFTP_ERR_FILESIZE		7	//文件大小错误
#define 	SFTP_ERR_TRANSTYPE	8	//文件传输类型错误
#define 	SFTP_ERR_CRC			9	//CRC Check Error
#define	  SFTP_ERR_CMDINVALID	10	//无效命令
#define	  SFTP_ERR_MEMORY		11

#define 	TYPE_DIR				1	//目录
#define	  TYPE_READ			2	//读文件
#define	  TYPE_WRITE			3	//写文件

#define 	CMD_UP           		0x80
#define 	CMD_DOWN         	0x00
#define 	CMD_UP_ERR       	0xC0

#define 	SFTP_BLOCKSIZE   	256

TSftp g_tSftpInfo;

void SftpInit()
{
	g_tSftpInfo.m_bTransType = 0;
	g_tSftpInfo.m_dwFileID = 0;
	g_tSftpInfo.m_dwFileSize = 0;
	g_tSftpInfo.m_IsFinish = true;
	memset(g_tSftpInfo.m_szPathName, 0, sizeof(g_tSftpInfo.m_szPathName));
	g_tSftpInfo.m_wBlockSize = 0;
	g_tSftpInfo.m_wPermission = 0;
	g_tSftpInfo.m_wRxLen = 0;
	g_tSftpInfo.m_wTxLen = 0;
	g_tSftpInfo.m_pbDataBuf = NULL;
	memset(&g_tSftpInfo.m_tmLastRecv, 0, sizeof(g_tSftpInfo.m_tmLastRecv));
}

//return 0-不是升级文件，1-BOOTLOADER，2-应用程序, 3-参数配置文件
BYTE IsSftpFwUpdate()
{
//#ifdef SYS_UCOSII
	if (strstr(g_tSftpInfo.m_szPathName, "bootloader.bin") != NULL)
		return BOOTLOADER;
    if (strstr(g_tSftpInfo.m_szPathName, "CL195N4.bin") != NULL)
        return FILE_APP;
    if (strstr(g_tSftpInfo.m_szPathName, "loadpara.dft") != NULL)
        return PARA_CFG;
	return FILE_ERR;
//#endif
	//return false;
}

//删除文件名前的路径，因为加了路径太长了
BYTE DelFilePath()
{
//#ifdef SYS_UCOSII
	if (strstr(g_tSftpInfo.m_szPathName, "bootloader.bin") != NULL)
    {
        strcpy(g_tSftpInfo.m_szPathName, "bootloader.bin");   //将前面的路径去掉
		return BOOTLOADER;
    }
    if (strstr(g_tSftpInfo.m_szPathName, "CL195N4.bin") != NULL)
    {
        strcpy(g_tSftpInfo.m_szPathName, "cl818k5.bin");	//bootloader判升级文件名cl818k5.bin
        return FILE_APP;
    }
    if (strstr(g_tSftpInfo.m_szPathName, "loadpara.dft") != NULL)
    {
        strcpy(g_tSftpInfo.m_szPathName, "loadpara.dft"); 
        return PARA_CFG;
    }
	return FILE_ERR;
//#endif
	//return false;
}

bool SftpHandleFrm(BYTE* pbRx, BYTE* pbTx)
{
	switch (*pbRx)  //命令
	{
	case SFTP_CMD_DIR:
		//m_wTxLen = Dir( pbRx, wRxLen, pbTx );
		return true;

	case SFTP_CMD_RDF0:
		g_tSftpInfo.m_wTxLen = SftpReadFirst( pbRx, pbTx );
		return true;

	case SFTP_CMD_RDFN:
		g_tSftpInfo.m_wTxLen = SftpReadNext( pbRx, pbTx );
		return true;

	case SFTP_CMD_WRF0:
		g_tSftpInfo.m_wTxLen = SftpWriteFirst( pbRx, pbTx );
		return true;

	case SFTP_CMD_WRFN:
		g_tSftpInfo.m_wTxLen = SftpWriteNext( pbRx, pbTx );
		return true;

	case SFTP_CMD_FINISH:
		g_tSftpInfo.m_wTxLen = SftpTransferFinish( pbRx, pbTx );
		return true;

	case SFTP_CMD_CANCEL:
		g_tSftpInfo.m_wTxLen = SftpTransferCancel( pbRx, pbTx );
		return true;

	default:
		DTRACE(DB_FAPROTO, ("unknow cmd = %x\n",*pbRx));
		break;
	}	
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//向服务器上传文件启动帧
int SftpReadFirst(BYTE* pbRx, BYTE* pbTx)
{
	BYTE* pbTx0 = pbTx;
	*pbTx0++ = SFTP_CMD_RDF0 + CMD_UP_ERR;
	*pbTx0++ = SFTP_ERR_CMDINVALID;
	return 2;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//向服务器上传文件后续帧
int SftpReadNext(BYTE* pbRx, BYTE* pbTx)
{
	BYTE* pbTx0 = pbTx;
	*pbTx0++ = SFTP_CMD_RDFN + CMD_UP_ERR;
	*pbTx0++ = SFTP_ERR_FILEID;
	return 2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//从服务器下载文件启动帧
int SftpWriteFirst(BYTE* pbRx, BYTE* pbTx)
{
    int nFileSize;
	WORD wNameLen, wRcrc, wMycrc, wDownLen, wOffsetNum;
	BYTE* pbTx0 = pbTx;
    BYTE bType;

	if (g_tSftpInfo.m_IsFinish)
	{	
		pbRx++;   							//命令
/////////////////////////	
		wNameLen = ByteToWord(pbRx);  	//文件名长度
		
		if (wNameLen > PATHNAME_LEN)
		{
			*pbTx0++ = SFTP_CMD_WRF0 + CMD_UP_ERR;
			*pbTx0++ = SFTP_ERR_PATHNAME;
			return 2;
		}
////////////////////////	
		pbRx += 2;
		memcpy(g_tSftpInfo.m_szPathName, pbRx, wNameLen);	//文件名
		
		g_tSftpInfo.m_szPathName[wNameLen] = '\0';

        bType = IsSftpFwUpdate();
		if (FILE_ERR == bType)
		{
			DTRACE(DB_FAPROTO, ("SftpWriteFirst : Don't Update File %s\n", g_tSftpInfo.m_szPathName));
			*pbTx0++ = SFTP_CMD_WRF0 + CMD_UP_ERR;
			*pbTx0++ = SFTP_ERR_FILE;
			return 2;
		}

        StopMtrRd(0);
//////////////////////	
		pbRx += wNameLen;
		nFileSize = ByteToDWORD(pbRx, 4);		//文件长度
		g_tSftpInfo.m_dwFileSize= nFileSize;
//////////////////////	
		pbRx += 4;
		g_tSftpInfo.m_wPermission = ByteToWord(pbRx);		//文件属性
//////////////////////
		pbRx += 2;
		wRcrc = ByteToWord(pbRx);			//crc check
//////////////////////
		pbRx += 2;
		wDownLen = ByteToWord(pbRx);	//下载长度

		if( wDownLen > nFileSize )
		{
			DTRACE(DB_FAPROTO, ("SftpWriteFirst : File len error\n"));
			*pbTx0++ = SFTP_CMD_WRF0 + CMD_UP_ERR;
			*pbTx0++ = SFTP_ERR_FILE;
			return 2;
		}
//////////////////////		
		g_tSftpInfo.m_dwFileID++;							//文件ID
		if (g_tSftpInfo.m_dwFileID == 0)
		{
			g_tSftpInfo.m_dwFileID = 1;
		}
		
		pbRx += 2;	
		wMycrc = get_crc_16(0, pbRx, wDownLen);
		if(wMycrc != wRcrc)
		{
			DTRACE(DB_FAPROTO, ("SftpWriteFirst : CRC Check Error\n"));
			DTRACE(DB_FAPROTO, ("SftpWriteFirst : Recv CRC = %d\n",wRcrc));
			DTRACE(DB_FAPROTO, ("SftpWriteFirst : My CRC = %d\n",wMycrc));
			*pbTx0++ = SFTP_CMD_WRF0 + CMD_UP_ERR;
			*pbTx0++ = SFTP_ERR_CRC;
			return 2;
		}
	
#ifdef	DEBUG_WriteFirst
		DTRACE(DB_FAPROTO, ("SftpWriteFirst : download from server: %s %d\n",g_tSftpInfo.m_szPathName, wNameLen));
		DTRACE(DB_FAPROTO, ("SftpWriteFirst : Filesize = %d\n", nFileSize));
		DTRACE(DB_FAPROTO, ("SftpWriteFirst : File Permission = %o\n", g_tSftpInfo.m_wPermission));
		DTRACE(DB_FAPROTO, ("SftpWriteFirst : download len = %d\n",wDownLen));
		DTRACE(DB_FAPROTO, ("SftpWriteFirst : CRC Check = %d\n", wMycrc));
#endif	

        if (bType == PARA_CFG) //参数配置文件
        {
            Program(pbRx, IN_PARACFG_ADDR, wDownLen);
        }
        else
    		WriteUpdateProg(pbRx, wDownLen, 0);

		GetCurTime(&g_tSftpInfo.m_tmLastRecv);
		
		wOffsetNum = 0;
		
		g_tSftpInfo.m_bTransType = TYPE_WRITE;
		
		//组正确的帧
		*pbTx++ = SFTP_CMD_WRF0 + CMD_UP;       	//cmd		
		DWordToByte(g_tSftpInfo.m_dwFileID, pbTx);  	//文件ID	
		pbTx += 4;		
		WordToByte(wOffsetNum, pbTx);   	//偏移号
		pbTx += 2;
		*pbTx++ = 0x00;			       			//执行结果, 0: OK; FF: ERROR

		return pbTx - pbTx0;
	}
	else
	{
		*pbTx0++ = SFTP_CMD_RDF0 + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_CMDINVALID;
		return 2;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//从服务器下载文件后续帧
int SftpWriteNext(BYTE* pbRx, BYTE* pbTx)
{
    //int f = -1;
	DWORD dwFileID, dwOffset;
	WORD wOffsetNum, wRcrc, wMycrc, wDownLen;
	BYTE* pbTx0 = pbTx;
    BYTE bType;

	pbRx++;   							//命令

	dwFileID = ByteToDWORD(pbRx, 4);  	//文件ID
	
	if (dwFileID != g_tSftpInfo.m_dwFileID)
	{
		DTRACE(DB_FAPROTO, ("SftpWriteNext : WriteNext FileID error\n"));
		*pbTx0++ = SFTP_CMD_WRFN + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_FILEID;
		return 2;
	}

	pbRx += 4;
	wOffsetNum = ByteToWord(pbRx);  	//偏移号

	//偏移地址=偏移号*256
	dwOffset = (DWORD )wOffsetNum * SFTP_BLOCKSIZE;

	pbRx += 2;
	wRcrc = ByteToWord(pbRx);  		//CRC Check	

	pbRx += 2;
	wDownLen = ByteToWord(pbRx);   		//下载长度

    bType = IsSftpFwUpdate();
	if (FILE_ERR == bType)
	{
		DTRACE(DB_FAPROTO, ("SftpWriteNext : Don't Update File!\n"));
		*pbTx++ = SFTP_CMD_WRFN + CMD_UP_ERR;
		*pbTx++ = SFTP_ERR_FILE;
		return 2;
	}

	pbRx += 2;

	wMycrc = get_crc_16(0, pbRx, wDownLen);
	if(wMycrc != wRcrc)
	{
		DTRACE(DB_FAPROTO, ("SftpWriteNext : CRC Check Error\n"));
		DTRACE(DB_FAPROTO, ("SftpWriteNext : Recv CRC = %d\n",wRcrc));
		DTRACE(DB_FAPROTO, ("SftpWriteNext : My CRC = %d\n",wMycrc));
		*pbTx0++ = SFTP_CMD_WRFN + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_CRC;
		return 2;
	}	
    if (bType == PARA_CFG) //参数配置文件
    {
        Program(pbRx, IN_PARACFG_ADDR+dwOffset, wDownLen);
    }
    else
	    WriteUpdateProg(pbRx, wDownLen, dwOffset);
	
	GetCurTime(&g_tSftpInfo.m_tmLastRecv);
	
	//组正确的帧
	*pbTx++ = SFTP_CMD_WRFN + CMD_UP;       	//cmd	
	DWordToByte(g_tSftpInfo.m_dwFileID, pbTx);  	//文件ID	
	pbTx += 4;	
	WordToByte(wOffsetNum, pbTx);   	//偏移号	
	pbTx += 2;	
	*pbTx++ = 0x00;      					//执行结果

    StopMtrRd(0);
	return pbTx - pbTx0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//传输结束帧
///////////////////////////////////////////////////////////////////////////
int SftpTransferFinish(BYTE* pbRx, BYTE* pbTx)
{
    //int f = -1;
	BYTE bType;
	BYTE bBuf[256];
	DWORD i, dwFileID;
	WORD wRcrc, wMycrc;
	BYTE* pbTx0 = pbTx;
    BYTE bType2;

	pbRx++;   							//命令	
	bType = *pbRx++;					//传输类型
 
	dwFileID = ByteToDWORD(pbRx, 4);  	//文件ID

	if( bType != g_tSftpInfo.m_bTransType)
	{
		DTRACE(DB_FAPROTO, ("SftpTransferFinish : TransType error\n"));
		*pbTx0++ = SFTP_CMD_FINISH + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_TRANSTYPE;
		return 2;
	}	
	if (dwFileID != g_tSftpInfo.m_dwFileID)
	{
		DTRACE(DB_FAPROTO, ("SftpTransferFinish : FileID error\n"));
		*pbTx0++ = SFTP_CMD_FINISH + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_FILEID;
		return 2;
	}
	pbRx +=4;
	wRcrc = ByteToWord(pbRx);

    bType2 = IsSftpFwUpdate();
    if (FILE_ERR == bType2)
	{
		DTRACE(DB_FAPROTO, ("SftpTransferFinish : Don't Update File %s\n", g_tSftpInfo.m_szPathName));
		*pbTx++ = SFTP_CMD_FINISH+ CMD_UP_ERR;
		*pbTx++ = SFTP_ERR_FILE;
		return 2;
	}

	wMycrc = 0;
	
/////////////////////////////////////		
	for(i=0; i<g_tSftpInfo.m_dwFileSize/sizeof(bBuf); i++)
	{
        if (bType2 == PARA_CFG)
        {
            memcpy(bBuf, (void *)(IN_PARACFG_ADDR+i*sizeof(bBuf)), sizeof(bBuf));
        }
        else
    		ReadUpdateProg(bBuf, sizeof(bBuf), i*sizeof(bBuf));
		wMycrc = get_crc_16(wMycrc, bBuf, sizeof(bBuf));
	}
	
	if(g_tSftpInfo.m_dwFileSize%sizeof(bBuf) > 0)
	{
        if (bType2 == PARA_CFG)
        {
            memcpy(bBuf, (void *)(IN_PARACFG_ADDR+g_tSftpInfo.m_dwFileSize-g_tSftpInfo.m_dwFileSize%sizeof(bBuf)), g_tSftpInfo.m_dwFileSize%sizeof(bBuf));
        }
        else
    		ReadUpdateProg(bBuf, g_tSftpInfo.m_dwFileSize%sizeof(bBuf), g_tSftpInfo.m_dwFileSize-g_tSftpInfo.m_dwFileSize%sizeof(bBuf));
		wMycrc = get_crc_16(wMycrc, bBuf, g_tSftpInfo.m_dwFileSize%sizeof(bBuf));
	}
////////////////////////////////////
	
	if(wMycrc != wRcrc)
	{
		DTRACE(DB_FAPROTO, ("SftpTransferFinish : CRC Check Error\n"));
		DTRACE(DB_FAPROTO, ("SftpTransferFinish : Recv CRC = %x\n",wRcrc));
		DTRACE(DB_FAPROTO, ("SftpTransferFinish : My CRC = %x\n",wMycrc));
		*pbTx0++ = SFTP_CMD_FINISH + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_CRC;
		return 2;
	}
	
    if (bType2 != PARA_CFG)  //参数配置文件没有这些标志
    {
        DelFilePath();
	    WriteUpdProgCrc(g_tSftpInfo.m_szPathName, g_tSftpInfo.m_dwFileSize, wMycrc);
    }

    //ExFlashRd(0, bBuf, 16);
    
	//组正确的帧
   	*pbTx++ = SFTP_CMD_FINISH + CMD_UP;       	//cmd	
    *pbTx++ = bType;						//传输类型	
    DWordToByte(g_tSftpInfo.m_dwFileID, pbTx);  	//文件ID	
    pbTx += 4;
    WordToByte(wMycrc, pbTx);
    pbTx += 2;
    *pbTx++ = 0x00;      					//保留
    
    if (bType2 == FILE_APP)
    {
    	SetInfo(INFO_APP_RST);
    }
    else if (bType2 == BOOTLOADER)//更新BOOTLOADER
    {
        SetInfo(INFO_UPDATE_BOOTLOADER);
    }
    /*else if (bType2 == PARA_CFG)
    {
    }*/
	DTRACE(DB_FAPROTO, ("SftpTransferFinish : TransferFinished\n"));	
	return pbTx - pbTx0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//取消传输帧
int SftpTransferCancel(BYTE* pbRx, BYTE* pbTx)
{
	BYTE* pbTx0 = pbTx;
	BYTE bType;
	DWORD dwFileID;

	pbRx++;   							//命令
	
	bType = *pbRx;					//传输类型
	if( bType != g_tSftpInfo.m_bTransType)
	{
		*pbTx0++ = SFTP_CMD_CANCEL + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_TRANSTYPE;
		return 2;
	}
	pbRx++;   
	
	dwFileID = ByteToDWORD(pbRx, 4);  	//文件ID

	if (dwFileID != g_tSftpInfo.m_dwFileID)
	{
		*pbTx0++ = SFTP_CMD_CANCEL + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_FILEID;
		return 2;
	}

	if( g_tSftpInfo.m_bTransType == TYPE_WRITE)
	{
		//unlink(g_tSftpInfo.m_szPathName);
	}
	
    g_fStopMtrRd = false;
	//组正确的帧
	*pbTx++ = SFTP_CMD_CANCEL + CMD_UP;       	//cmd

	*pbTx++ = bType;				//传输类型

	DWordToByte(dwFileID, pbTx);  	//文件ID
	
	pbTx += 4;
	
	*pbTx++ = 0x00;      					//保留
	
	return pbTx - pbTx0;
}
