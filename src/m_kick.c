/*****************************************************************
 * PTlink OPM is (C) CopyRight PTlink Coders Team 1999-2004      *
 * http://www.ptlink.net/Coders/ - coders@PTlink.net             *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: m_kick.c
  Author: Lamego@PTlink.net
*/

#include "ptopm.h"   
#include "s_log.h"
#include "irc_string.h"
#include "dconf_vars.h"
#include "send.h"
#include "m_commands.h"

/*	 
** m_kick
**	parv[0] = sender prefix
**	parv[1] = chan
**	parv[2] = target
*/ 
int m_kick(int parc, char *parv[])
{
  static int kick_count = 0;
  
  if(parc<3)
    return 0;
    
  if(irccmp(PT_Nick, parv[2]) != 0) /* we are not the target for the kick */
    return 0;
    
  if(kick_count>64)
    {
      opmlog(L_NOTICE,"Aborting rejoin on kick due to excessive kicks");
      return 0;
    }
  ++kick_count;
  opmlog(L_NOTICE,"Rejoining %s after kick", parv[1]);
  sendto_ircd(NULL,"SJOIN %lu %s +Os :@%s\n", 
    CurrentTime, parv[1], PT_Nick);
  return 0;
}


