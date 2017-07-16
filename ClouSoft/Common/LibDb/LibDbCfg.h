/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：LibDbCfg.h
 * 摘    要：本文件主要实现系统库的配置
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2011年3月
 *********************************************************************************************************/
#ifndef LIBDBCFG_H
#define LIBDBCFG_H

//编译开关选项
//#define EN_DB_PNMAP			1		//支持测量点动态映射
#define EN_DB_QRYTM 		1		//支持时标和查询接口QueryItemTimeX()
#define EN_DB_ITEMINFO		1		//支持数据项信息
#define EN_DB_MULTI			1		//支持多ID同时读/写
#define EN_DB_CMB			1		//支持组合ID
#define EN_DB_SECTHOLDER	1		//支持扇区保持

#endif //LIBDBCFG_H
