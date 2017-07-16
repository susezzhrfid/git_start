/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: cc.h,v 1.4 2002/01/08 07:13:40 adam Exp $
 */
#ifndef __ARCH_CC_H__
#define __ARCH_CC_H__

/* Include some files for defining library routines */
#include <stdio.h> /* printf, fflush, FILE */
#include <stdlib.h> /* abort */
#include <stddef.h>
#include "SysDebug.h"

typedef unsigned   char    u8_t;
typedef signed     char    s8_t;
typedef unsigned   short   u16_t;
typedef signed     short   s16_t;
typedef unsigned   long    u32_t;
typedef signed     long    s32_t;

/* Define (sn)printf formatters for these lwIP types */
//#define X8_F  "02x"
#define X8_F  "x"
//#define U16_F "hu"
#define U16_F "d"
//#define S16_F "hd"
#define S16_F "d"
//#define X16_F "hx"
#define X16_F "x"
//#define U32_F "lu"
#define U32_F "l"
//#define S32_F "ld"
#define S32_F "l"
//#define X32_F "lx"
#define X32_F "x"
#define SZT_F U32_F


#ifndef __u_char_defined

// Type definitions for BSD code. 
#ifndef u_long
typedef unsigned long u_long;
#endif

#ifndef u_int
typedef unsigned int u_int;
#endif

#ifndef u_short
typedef unsigned short u_short;
#endif

#ifndef u_char
typedef unsigned char u_char;
#endif

#endif


#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END
#define PACK_USE_PRAGMA_PUSH_POP
#define STDCALL __stdcall

#define PACK_STRUCT_STRUCT


/* Plaform specific diagnostic output */
#define LWIP_PLATFORM_DIAG(x)   do { DebugPrintf x; } while(0)


#define LWIP_PLATFORM_ASSERT(x) do { DebugPrintf("Assertion \"%s\" failed at line %d in %s\n", \
                                     x, __LINE__, __FILE__); fflush(NULL); abort(); } while(0)


//#define LWIP_ERROR(message, expression, handler) do { if (!(expression)) { \
//  DebugPrintf("Assertion \"%s\" failed at line %d in %s\n", message, __LINE__, __FILE__); \
//  handler;} } while(0)


#include <string.h>
#endif /* __ARCH_CC_H__ */
