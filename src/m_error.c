/*****************************************************************
 * PTlink OPM is (C) CopyRight PTlink Coders Team 1999-2002      *
 * http://www.ptlink.net/Coders/ - coders@PTlink.net             *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: m_error.c
  Author: Lamego@PTlink.net
*/
#include "ptopm.h"   
#include "s_log.h"
#include "m_commands.h"

/*	 
** m_error
**	parv[0] = sender prefix
**	parv[1] = description
*/ 
int m_error(int parc, char *parv[])
{
  opmlog(L_WARN,"Server ERROR : %s", parc>1 ? parv[1] : "");
  return 0;
}


