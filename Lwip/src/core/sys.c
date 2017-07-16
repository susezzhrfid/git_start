/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "lwip/sys.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/memp.h"

//int  tocnt = 0;

void
sys_timeout(u32_t msecs, sys_timeout_handler h, void *arg)
{
  struct sys_timeouts *timeouts;
  struct sys_timeout *timeout, *t;

  timeout = (struct sys_timeout *)memp_malloc(MEMP_SYS_TIMEOUT);
  if (timeout == NULL) {
    return;
  }
  
  //tocnt++;  
  
  timeout->next = NULL;
  timeout->h = h;
  timeout->arg = arg;
  timeout->time = msecs;

  timeouts = sys_arch_timeouts();

  //PrintTime();
  LWIP_DEBUGF(SYS_DEBUG, ("sys_timeout: %p msecs=%lu h=%p arg=%p now=%ld\r\n",
    (void *)timeout, msecs, (void *)h, (void *)arg, GetTick()));

  LWIP_ASSERT("sys_timeout: timeouts != NULL", timeouts != NULL);

  if (timeouts->next == NULL) 
  {
    //LWIP_DEBUGF(SYS_DEBUG, ("sys_timeout: append null\r\n"));
    timeouts->next = timeout;
    return;
  }

  if (timeouts->next->time > msecs) //比第一个timeout的时间还短,放在第一个
  {
  	//LWIP_DEBUGF(SYS_DEBUG, ("sys_timeout: append first\r\n"));
    timeouts->next->time -= msecs;
    timeout->next = timeouts->next;
    timeouts->next = timeout;
  } 
  else 
  {
    for(t = timeouts->next; t != NULL; t = t->next) 
    {
      timeout->time -= t->time;
      if (t->next == NULL || t->next->time > timeout->time) 
      {
        if (t->next != NULL) 
        {
          t->next->time -= timeout->time;
        }
        timeout->next = t->next;
        t->next = timeout;
        //LWIP_DEBUGF(SYS_DEBUG, ("sys_timeout: append mid\r\n"));
        break;
      }
    }
  }

}

/* Go through timeout list (for this task only) and remove the first matching entry,
   even though the timeout has not triggered yet.
*/

void sys_untimeout(sys_timeout_handler h, void *arg)
{
    struct sys_timeouts *timeouts;
    struct sys_timeout *prev_t, *t;

    timeouts = sys_arch_timeouts();

    if (timeouts->next == NULL)
        return;

    for (t = timeouts->next, prev_t = NULL; t != NULL; prev_t = t, t = t->next)
    {
        if ((t->h == h) && (t->arg == arg))
        {
			LWIP_DEBUGF(SYS_DEBUG, ("sys_untimeout: h=%p arg=%p now=%ld\r\n",
							(void *)h, (void *)arg, GetTick()));
        	
            /* We have a match */
            /* Unlink from previous in list */
            if (prev_t == NULL)
                timeouts->next = t->next;
            else
                prev_t->next = t->next;
            /* If not the last one, add time of this one back to next */
            if (t->next != NULL)
                t->next->time += t->time;
            memp_free(MEMP_SYS_TIMEOUT, t);
			//tocnt--;  
            return;
        }
    }
    return;
}

void sys_timeout_callback()
{
	struct sys_timeouts *timeouts;
	struct sys_timeout *to;
	u32_t lasttime;
	u32_t past;

	u32_t nowtime = GetTick();

	timeouts = sys_arch_timeouts();
	lasttime = timeouts->lasttime;
	timeouts->lasttime = nowtime;

	LWIP_ASSERT("sys_timeout: timeouts != NULL", timeouts != NULL);

	if (timeouts->next == NULL) 
	{
		return;
	}
	
	//LWIP_DEBUGF(SYS_DEBUG, ("1ne!=nu\r\n"))

	past = nowtime - lasttime;

	to = timeouts->next;
	if (to->time > past)	 //超时没到
	{
		to->time -= past;
	}
	else
	{
		LWIP_DEBUGF(SYS_DEBUG, ("sys_timeout_callback: fun %p, arg %p, now=%ld\r\n",
								  (void *)to->h, (void *)to->arg, GetTick()));
		
		timeouts->next = to->next;
			//timeouts必须在这里更新,因为to->h(to->arg);有可能注册新的超时函数
		to->h(to->arg);
		memp_free(MEMP_SYS_TIMEOUT, to);
				//past的时间可能超过to->time,剩下的时间就不更新了,因为:
				//1.250毫秒的误差应该可以接收,
				//2.有可能注册新的超时函数
		//tocnt--;					
	}
	
/*if (timeouts->next == NULL)
{	
	LWIP_DEBUGF(SYS_DEBUG, ("2ne==nu\r\n"));
}
else
{
	LWIP_DEBUGF(SYS_DEBUG, ("2ne!=nu\r\n"));
}*/

}


void sys_timeout_clear()
{
	struct sys_timeouts *timeouts = sys_arch_timeouts();
	struct sys_timeout *timeout = timeouts->next;
	
	while (timeout != NULL)
	{
		LWIP_DEBUGF(SYS_DEBUG, ("sys_timeout_clear: fun %p, arg %p\r\n",
								  (void *)timeout->h, (void *)timeout->arg));
		
		struct sys_timeout* to = timeout->next;
		memp_free(MEMP_SYS_TIMEOUT, timeout);
		timeout = to;
		//tocnt--;		
	}
	
	timeouts->next = NULL;
}
