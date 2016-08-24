/*****************************************************************
 * PTlink OPM is (C) CopyRight PTlink Coders Team 1999-2002      *
 * http://www.ptlink.net/Coders - coders@PTlink.net              *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: send.c
  Desc: send routines
  Author: Lamego@PTlink.net
*/

#include "stdinc.h"
#include "ptopm.h"
#include "struct.h"
#include "send.h"
#include "ircd.h"
#include "s_log.h"
#include "sockutil.h"
#include "ircd_defs.h"

/* functions declaration */
void vsendto_ircd(const char *source, const char *fmt, va_list args);

void vsendto_ircd(const char *source, const char *fmt, va_list args)
{
    char buf[BUFSIZE];

    vsnprintf(buf, sizeof(buf), fmt, args);
    if (source) {
        sockprintf(irc_fd, ":%s %s\r\n", source, buf);    
        opmlog(L_DEBUG,"Sent: :%s %s", source, buf);
    } else {
        sockprintf(irc_fd, "%s\r\n", buf);
        opmlog(L_DEBUG,"Sent: %s", buf);
    }
}

void sendto_ircd(const char *source, const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  vsendto_ircd(source, fmt, args);
  va_end(args);
}

void send_notice(const char *source, const char *target, const char *fmt, ...)
{
  char buf[512];
  va_list args;
  
  va_start(args, fmt);    
  vsnprintf(buf, sizeof(buf), fmt, args);
  sendto_ircd(source, "NOTICE %s :%s", target, buf);
  va_end(args);
}

void send_msg(const char *source, const char *target, const char *fmt, ...)
{
  char buf[512];
  va_list args;
  
  va_start(args, fmt);    
  vsnprintf(buf, sizeof(buf), fmt, args);
  sendto_ircd(source, "PRIVMSG %s :%s", target, buf);
  va_end(args);
}
