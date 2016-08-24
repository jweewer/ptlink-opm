/*****************************************************************
 * PTlink OPM is (C) CopyRight PTlink Coders Team 1999-2004      *
 * http://www.ptlink.net/Coders/ - coders@PTlink.net             *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: m_kill.c
  Author: Lamego@PTlink.net
*/

#include "ptopm.h"   
#include "s_log.h"
#include "irc_string.h"
#include "dconf_vars.h"
#include "send.h"
#include "ircdio.h"
#include "m_commands.h"

/*	 
** m_kill
**	parv[0] = sender prefix
**	parv[1] = target
**	parv[2] = reason
*/ 
int m_kill(int parc, char *parv[])
{
  static int kill_count = 0;
  char *user, *host;
  
  if(parc<2)
    return 0;
    
  if(irccmp(PT_Nick, parv[1]) != 0) /* we are not the target for the kill */
    return 0;
    
  if(kill_count>32)
    {
      opmlog(L_NOTICE,"Aborting reintroduction on kill due to excessive kills");
      return 0;
    }
  ++kill_count;
  opmlog(L_NOTICE,"Reintroducing after kill from %s with reason: %s", 
    parv[0], parc>2 ? parv[2] : "");

  user = strtok(PT_Mask,"@");
  if(user)
    host = strtok(NULL,"");
  else
    host = NULL;
  if((user == NULL) || (host == NULL))
    {
       opmlog(L_ERROR,"Invalid PT_MASK");
    }
  introduce_user(PT_Nick, user, host, PT_Info, "+rp");
  *(host-1)='@'; /* fix old user */
  sendto_ircd(NULL,"SJOIN %lu #%s +Os :@%s\n", 
    CurrentTime, LogChan, PT_Nick);
            
  return 0;
}



