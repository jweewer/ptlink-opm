/*****************************************************************
 * PTlink OPM is (C) CopyRight PTlink Coders Team 1999-2002      *
 * http://www.ptlink.net/Coders/ - coders@PTlink.net             *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: IRC I/O
  Desc: IRCD I/O functions
  Author: Lamego@PTlink.net
*/
#include "ptopm.h"
#include "config.h"
#include "sockutil.h"
#include "ircd.h"
#include "irc_string.h"
#include "s_log.h"
#include "irc_parse.h"
#include "send.h"
#include "dconf_vars.h"
#include "version.h"
#include "ircdio.h"

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

SockBuffer ircd_buffer_in;  /* incoming data buffer */
SockBuffer ircd_buffer_out; /* outgoing data buffer */
time_t last_irc_read;

void ircd_buff_init(void)
{  
  sockbuf_init(&ircd_buffer_in, INBUFFER_SIZE);
  last_irc_read = time(NULL);
}

/*
 * Check ircd buffer and parse data if available
 * Returns:
 * -1 = Sucess
 *  0 = Connection to ircd terminated
 */
int check_ircd_buffer(void)
{
#if 0
  fd_set fds_r;
  fd_set fds_w;
  struct timeval tv = {1,0}; /* 1 s timeout */
  int   retval;  
#endif  
  int   res;
  int fd = irc_fd;
  char *bufpos , *bufend;
  int	rc = 0;

  if (fd < 0)
    {
	  opmlog(L_ERROR,"invalid fd on check_ircd_buffer()");
	  exit(1);
    }
    
  if( time(NULL)-last_irc_read > IRC_READ_TIMEOUT)
    {
      opmlog(L_ERROR,"Closed on IRC_READ_TIMEOUT");
      return 0;
    }
    
#if 0              
  FD_ZERO(&fds_r);
  FD_ZERO(&fds_w);
  FD_SET(fd, &fds_r);

  retval = select(fd+1, &fds_r, &fds_w, 0, &tv);

  if(retval>0 && FD_ISSET(fd, &fds_r))
    {
#endif    
      rc = sockbuf_read(fd, &ircd_buffer_in);

	  if(rc>0)
	    {
	      last_irc_read = time(NULL);
          bufpos = ircd_buffer_in.data;
          bufend = strchr(bufpos,'\r');	          

	      while(bufend && *bufpos)
		    {		
                      (*bufend--)='\0';
         
		      while(*bufpos && ((*bufpos=='\r') || (*bufpos=='\n')))
                       ++bufpos;                    

                      CurrentTime = time(NULL);
	              irc_parse(bufpos, bufend);

	              bufpos = bufend+1;
	              if(bufpos<ircd_buffer_in.seekpos)  
                        {                  
                          ++bufpos;
	                  bufend = strchr(bufpos,'\r');
                        }
		    }
          res = (ircd_buffer_in.seekpos-bufpos)+1;
		  if(res>0)
            {
		      memmove(ircd_buffer_in.data, bufpos, res);
              ircd_buffer_in.seekpos = ircd_buffer_in.data+(res-1);
            }                      
	    }
      else if (rc==-1)
		{
          log_perror(L_ERROR,"Connection closed by server: ");
		  return 0; /* going to retry */
		}
	  else
	    {
#if 0        
        /* read buffer is full */
		}
#endif        
    }
        
  return -1;
} 

void ircd_connect(void)
{
  is_connected = 0;
  opmlog(L_INFO,"Netjoin in progress");
  sendto_ircd(NULL, "PASS %s :TS", IRCPass);
  sendto_ircd(NULL, "CAPAB :PTS4");
  sendto_ircd(NULL, "SERVER %s 1 %s :%s",
    ServerName, program_version, ServerDesc);
}

/* Introduce one irc user */
void introduce_user(char* nick, char *user, char *host, char *info, char *umodes)
{
  sendto_ircd(ServerName, "NICK %s 1 %d %s %s %s %s %s :%s",
    nick, CurrentTime, umodes, user, host, host, ServerName, info);
}
