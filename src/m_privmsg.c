/**************************************************************
 * PTlink OPM is (C) CopyRight PTlink Coders Team 1999-2002   *
 * http://www.ptlink.net/Coders/ - coders@PTlink.net          *
 * This program is distributed under GNU Public License       *
 * Please read the file COPYING for copyright information.    *
 **************************************************************
 
  File: m_privmsg.c
  Desc: message
  Author: Lamego@PTlink.net
*/

/*
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
*/

#include "stdinc.h"
#include "s_log.h"
#include "dconf_vars.h"
#include "scan.h"
#include "hash.h"
#include "rhc.h"
#include "send.h"
#include "m_commands.h"

/*
 * m_privmsg - SERVER message handler
 *      parv[0] = sender prefix
 *      parv[1] = target
 *      parv[2] = message
 */
int m_privmsg(int parc, char *parv[])
{
  char *cmd, *trg;
  char *rmsg;
  char *to = parv[1];
  int res;
  
  if(parc<3 || parv[1][0]!='#')
    return 0;
  
  cmd = strtok(parv[2]," ");
  if(!cmd)
    return 0;
  if(strcasecmp(cmd,"!help")==0)
    {
      send_msg(PT_Nick, to,"*** Available commands *************************************");
      send_msg(PT_Nick, to," !check <host>      - Scan host for open proxies");
      send_msg(PT_Nick, to," !igadd <host|mask> - Add host/mask to the ignore list");
      send_msg(PT_Nick, to," !igdel <host|mask> - Delete host/mask from the ignore list");
      send_msg(PT_Nick, to," !iglist            - Show ignore list");      
      send_msg(PT_Nick, to," !igsave            - Save ignore list");
      send_msg(PT_Nick, to," !stats             - Show activity statistics");      
      send_msg(PT_Nick, to," !netstats          - Show network usage statistics");
      send_msg(PT_Nick, to," !die               - Shutdown PTOPM");
      send_msg(PT_Nick, to,"************************************************************");
      return 0;
    }
  if(strcasecmp(cmd,"!die")==0) 
    {
      opmlog(L_INFO,"Terminating on !die requeste");
      sendto_ircd(PT_Nick,"QUIT :!die request");
      sendto_ircd(NULL,"SQUIT :!die request");
      exit(0);
    }
  else if(strcasecmp(cmd,"!stats")==0) 
    {
      scan_stats(parv[1]);
      rhc_stats(parv[1]);
      return 0;
    }  
  else if(strcasecmp(cmd,"!netstats")==0) 
    {
      scan_netstats(parv[1]);
      return 0;
    }      
  else if(strcasecmp(cmd, "!iglist") == 0)
    {
	  ignore_mask_list(to);
	  list_host_hash(to);
	  return 0;
	} 
  else if(strcasecmp(cmd, "!igsave") == 0)
    {
	  if(ignore_save())
	    send_msg(PT_Nick, to,"An error occured saving the ignore list");
	  else
	    send_msg(PT_Nick, to,"Ignore list saved.");
        				
	  return 0;		
    }
       
  trg = strtok(NULL," ");
  
  if(!trg)
    return 0;
    
  if(strcasecmp(cmd,"!check")==0)
    {
      scan_connect(trg, "manual", "scan", 1, 1);    
      return 0;
    }
  else if(strcasecmp(cmd, "!igadd") == 0)
	{
	  if(strlen(trg)>64)
	    trg[63] = '\0';
							
	  if(strchr(trg,'*')||strchr(trg,'?'))
        {            
		  res = add_ignore_mask(trg);
		  switch(res)
	        {
			  case 0: rmsg = "Mask added to the ignore list"; break;
			  case -1: rmsg = "Mask already exists on the ignore list"; break; 
			  case -2: rmsg = "Ignore list is full"; break;			  
			  default: rmsg = "Invalid add_ignore_mask return!!!"; break;
			}
          send_msg(PT_Nick,to, "%s", rmsg);
        }
      else 
        {  
		  if(find_host_hash(trg))
		    {
		      send_msg(PT_Nick,to, "Host %s is already on the ignore list",
			    trg);
			  return 0;
			}	
		  else
		    {	
		      send_msg(PT_Nick, to, "Host %s added to the ignore list", trg);
		  	  add_host_hash(trg);
			}
        }
	  }
	else if(strcasecmp(cmd, "!igdel") == 0)
	  {

		if(*trg == '\0')
			return 0;	
			
		if(strlen(trg)>64)
		  trg[63] = '\0';
		  					
		if(strchr(trg,'*')||strchr(trg,'?'))
      	  {            
			if(del_ignore_mask(trg))
          	  send_msg(PT_Nick, to,"Mask %s deleted",
				trg);
			else
			  send_msg(PT_Nick, to,"Mask %s not found",
				trg);
          }
      	else 
          {  
			if(!find_host_hash(trg))
			  {
				send_msg(PT_Nick, to,"Host %s not found",
				  trg);
				return 0;
			  }	
			else
			  {	
				del_host_hash(trg);
				send_msg(PT_Nick, to,"Host %s deleted",
				  trg);			  	
			  }
          }
	
		return 0;
	  }
      
  return 1;
}
