/*****************************************************************
 * PTlink OPM is (C) CopyRight PTlink Coders Team 1999-2002      *
 * http://www.ptlink.net/Coders/ - coders@PTlink.net             *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: m_server.c
  Author: Lamego@PTlink.net
*/
#include "ptopm.h"   /* CurrentTime */
#include "ircd.h"       /* ircd */
#include "irc_string.h"  /* strncpy_irc */
#include "send.h"        /* sendto_one */
#include "s_log.h"
#include "dconf_vars.h"
#include "m_commands.h"

/*	 
** m_ping
**	parv[0] = sender prefix
**	parv[1] = origin
**	parv[2] = destination
*/ 
int m_ping(int parc, char *parv[])
{
  char      *origin, *destination;

  if(!netjoined)
    opmlog(L_INFO,"[ NetJoin completed ]");
          
  netjoined = -1; /* end of netjoin */

  if (parc < 2 || *parv[1] == '\0')
	{	 
	  opmlog(L_ERROR, "ERR_NOORIGIN: %s, %s", ServerName, parv[0]);
	  return 0;
    }

  origin = parv[1];
  destination = parv[2]; /* Will get NULL or pointer (parc >= 2!!) */	    
    
  sendto_ircd(NULL,":%s PONG %s :%s", ServerName,
    (destination) ? destination : ServerName, origin);
    
  return 0;

}


