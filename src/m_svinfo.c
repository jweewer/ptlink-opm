/*****************************************************************
 * PTlink OPM is (C) CopyRight PTlink Coders Team 1999-2002      *
 * http://www.ptlink.net/Coders/ - coders@PTlink.net             *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: m_svinfo.c
  Author: Lamego@PTlink.net
*/
#include "ptopm.h"   
#include "s_log.h"
#include "m_commands.h"

/*
 * m_svinfo - SVINFO message handler
 *	parv[0] = sender prefix
 *	parv[1] = TS_CURRENT for the server
 *	parv[2] = TS_MIN for the server
 */
int m_svinfo(int parc, char *parv[])
{
  if(parc<3)
    return 0;
    
  if (TS_CURRENT < atoi(parv[2]))
    {    
      opmlog(L_ERROR,"SVINFO: We are %i, ircd supports >=%i", TS_CURRENT, atoi(parv[2]));
      opmlog(L_ERROR,"Terminated: IRC server does not support our protocol version");      
      exit(1);
    }
   if(atoi(parv[1]) < TS_MIN)
    {
      opmlog(L_ERROR,"SVINFO: We support >=%i, ircd is %i", TS_MIN, atoi(parv[1]));
      opmlog(L_ERROR,"Terminated: IRC server protocol version is not supported");
      exit(1);
    }   
  return 0;
}
