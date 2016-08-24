/*****************************************************************
 * PTlink Services is (C) CopyRight PTlink Coders Team 1999-2002 *
 * http://www.ptlink.net/Coders/ - coders@PTlink.net             *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: m_nick.c
  Author: Lamego@PTlink.net
*/
#include "stdinc.h"
#include "ptopm.h"   /* CurrentTime */
#include "ircd.h"       /* ircd */
#include "irc_string.h"  /* strncpy_irc */
#include "send.h"        /* sendto_one */
#include "s_log.h"
#include "scan.h"
#include "hash.h"
#include "ircd_defs.h"
#include "dconf_vars.h"
#include "m_commands.h"

/* ignore counters */
static int ign_umode = 0;
static int ign_mask = 0;
static int ign_host = 0;

/*
** m_nick
**	parv[0] = sender prefix
**  parv[1] = nickname
**  parv[2] = hopcount when new user; TS when nick change
**  parv[3] = TS
**  parv[4] = umode
**	parv[5] = username
**  parv[6] = hostname
**  parv[7] = spoofed hostname
**  parv[8] = server
**  parv[9] = info
*/
int m_nick(int parc, char *parv[])
{

  char *nick,*user,*host, *umodes;  

  if (parc < 7)
    return 0;

  nick = parv[1];
  umodes = parv[4];	
  user = parv[5];
  host = parv[6];
  

  /* check if we are receiving a service nick */
  if((!irccmp(PT_Nick, nick))
    )
    {
      sendto_ircd(NULL, "KILL %s :Missing Q:Line, forbidding services nick"
        , nick);

      return 0;
    }

  if( NetJoinScan || netjoined )
    {          

      if(strchr(umodes,'B') || strchr(umodes,'o'))
        {
          ++ign_umode;
          return 0;
        }
	  
      if(find_host_hash(host))
        {
          ++ign_host;
	      return 0;
        }
    
      if(is_ignore_mask(host))
        {        
          ++ign_mask;
          return 0;
        }

      opmlog(L_DEBUG,"Scanning nick %s, host %s", nick, host);        
      if(ScanNotice)
        send_notice(PT_Nick, nick, ScanNotice);
      scan_connect(host, nick, user, 0, 0);
          
    }
    
  return 0;
  
}

