/*****************************************************************
 * PTlink Services is (C) CopyRight PTlink Coders Team 1999-2002 *
 * http://www.ptlink.net/Coders/ - coders@PTlink.net             *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: m_server.c
  Author: Lamego@PTlink.net
*/
#include "stdinc.h"
#include "ptopm.h"   /* CurrentTime */
#include "ircd.h"       /* ircd */
#include "irc_string.h"  /* strncpy_irc */
#include "send.h"        /* sendto_one */
#include "ircdio.h"     /* introduce_client */
#include "s_log.h"
#include "ircd_defs.h"
#include "dconf_vars.h"
#include "m_commands.h"


/*
 * m_server - SERVER message handler
 *      parv[0] = sender prefix
 *      parv[1] = servername
 *      parv[2] = hopcount
 *		parv[3]	= version
 *      parv[4] = serverinfo
 */
int m_server(int parc, char *parv[])
{
  char             info[REALLEN + 1];
  char             *host, *rversion;
  char             *user;
  int              hop;

  info[0] = '\0';

  if (parc < 2 || *parv[1] == '\0')
    {
      sendto_ircd(NULL,"ERROR :No servername");
      return 0;
    }
	
  host = parv[1];
  hop = atoi(parv[2]);
  rversion = parv[3];
	
  if(parv[0]==NULL && !is_connected) /* This is our link */
    {
      sendto_ircd(NULL,"SVINFO %d %d", TS_CURRENT, TS_MIN);

      user = strtok(PT_Mask,"@");  
      if(user)
      	host = strtok(NULL,"");      	      	
      if((user == NULL) || (host == NULL))
      	{
      	  opmlog(L_ERROR,"Invalid PT_MASK");
      	}      	
      introduce_user(PT_Nick, user, host, PT_Info, "+rp");
      *(host-1)='@'; /* fix old user */
      sendto_ircd(NULL,"SJOIN %lu #%s +Os :@%s\n", CurrentTime, LogChan, PT_Nick);
      is_connected = -1;
      ConnectTime = CurrentTime;
    }

  return 0;
}


