/*
 *****************************************************************
 * PTlink OPM is (C) CopyRight PTlink Coders Team 1999-2004      *
 *                  http://software.pt-link.net                  *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: ptopm.c
  Desc: PTlink Open Proxy Montior main file
  Author: Lamego@PTlink.net

*/

#include "stdinc.h"
#if defined(HAVE_GETOPT_H)
#include <getopt.h>
#endif /* HAVE_GETOPT_H */


#include "setup.h"
#include "ptopm.h"
#include "path.h"
#include "s_log.h"
#include "dconf.h"
#include "version.h"
#include "irc_parse.h"
#include "ircdio.h"
#include "sockutil.h"
#include "msg.h"
#include "dconf_vars.h"
#include "config.h"
#include "misc.h"
#include "send.h"
#include "hash.h" /* load_ignore */
#include "scan.h" /* check_ */
#include "signals.h"    /*  setup_signals() */



/* local functions declare */
static void parse_command_line(int argc, char* argv[]);
static void bad_command();

struct Client* GlobalClientList = 0; /* Pointer to beginning of Client list */
time_t         CurrentTime;
time_t	       StartTime; /* time of the service start */
time_t	       ConnectTime; /* time of the last connect to irc */

/* list of servers we should try to connect */
char* connserverlist[50];

int nofork	=	0;	/* should we fork to backgound ? */
int debug = 0; /* debug */
int irc_fd =	0;
int mypid;
int is_connected = 0;
int netjoined = 0;
/*
 * check_pidfile
 *
 * inputs	- nothing
 * output	- nothing
 * side effects - reads pid from pidfile and checks if ircd is in process
 *                list. if it is, gracefully exits
 * -kre
 */
static void check_pidfile(void)
{
  int fd;
  char buff[20];
  pid_t pidfromfile;
  
  if ((fd = open(PIDFilename, O_RDONLY)) >= 0 )
  {
    if (read(fd, buff, sizeof(buff)) == -1)
    {
      /* printf("NOTICE: problem reading from %s (%s)\n", PPATH,
	  strerror(errno)); */
    }
    else    
    {
      pidfromfile = atoi(buff);
      if (pidfromfile != (int)getpid() && !kill(pidfromfile, 0))
      {
        printf("ERROR: daemon is already running with pid=%i\n", (int) 
          pidfromfile);
        exit(-1);
      }
    }
    close(fd);
  }
  else if(errno != ENOENT)
  {
    printf("WARNING: problem opening %s: %s\n", PIDFilename, strerror(errno));
  }
}


/*
 * main routine
 */
int main(int argc, char *argv[])
  {

  uid_t       uid;  
  uid_t       euid;
  int         conni=0;   /* connection index */
  time_t  lasttscan = 0; /* last timeout scan */
  char	logfn[256];
  uid = getuid();   
  euid = geteuid();
  

  StartTime = CurrentTime = time(NULL);
  
#ifndef SERVICES_UID
  if ((uid != euid) && (euid==0))
    {
      fprintf(stderr,
              "ERROR: do not run ptopm setuid root. " \
              "Make it setuid a normal user.\n");
      exit(-1);
    }
#endif  



    
#ifdef CHROOT_DIR
#ifdef HAVE_CHROOT
  if (chdir(CHROOT_DIR)) 
    {
	  perror("chdir " CHROOT_DIR);
	  exit(-1);
    }
    
  if (chroot(CHROOT_DIR))
    {
	  perror("ERROR:  Cannot chroot");
	  
	  exit(5);
    }
/* lets override paths for chrooted compilations */
#undef ETCPATH
#undef VARPATH
#define ETCPATH "/etc"
#define VARPATH "/var"
#else
#warning You specified chroot dir but chroot() was not found on your system
#endif /*HAVE_CHROOT*/
#endif /*CHROOT_DIR*/

  if (chdir(ETCPATH)) 
    {
	  perror("chdir " ETCPATH);
	  exit(-1);
    }    
    /*
      TO DO
      
      SETUID/SETGID/CHROOT
      
    */
  snprintf(logfn,sizeof(logfn),"%s/log/ptopm.log", VARPATH);
  
  setup_signals();
  printf("%s - (C) PTlink IRC Software 1999-2004\n", 
		program_version);
		
  parse_command_line(argc, argv);
  check_pidfile();

  if(!init_log(logfn))
    {
      fprintf(stderr, "Could not open log file!!!\n");
      return -1;
    }

  opmlog(L_NOTICE,"Starting %s",
	program_version);  	  
    
  printf("Reading configuration...\n");
    
  if ((dconf_read("ptopm.dconf", 0) == -1))
    {
      opmlog(L_CRIT,"Terminating: error opening ptopm.dconf");
      fprintf(stderr, "Terminating: error opening ptopm.dconf\n");
      return -1;
    }
      
#ifdef USE_POLL
    if(MaxScans > MAX_POLL)
    	opmlog(L_WARN,"Reducing MaxScans to MAX_POLL (%i)", 
    		MAX_POLL);
#endif

    if( !dconf_check(1) )
        return -1;
    printf("Loading ignore list\n");        
    load_ignore();
    
    /* build connection servers list */
    build_connserver_list();
    
	if(nofork)
	  {
	  	printf("Running in foreground...\n");
		opmlog(L_NOTICE,"Running in foreground...");	
	  }
	else
	  {		  
		printf("Going to background...\n");
		opmlog(L_NOTICE,"Going to background...");
	  }
		  
	mypid = nofork ? 0 : fork();
    
	if(mypid!=0) /* the child will do the work from now on */
      return 0;

	write_pidfile();	/* write pid to file */
	init_tree_parse(msgtab); /* init irc parse tree */
        do_scan_init();
        
    do  /* retry_count */
      {
      
        ircd_buff_init();
        irc_fd = sock_conn(connserverlist[conni], IRCPort);		
        if(irc_fd<0)
          {
            opmlog(L_ERROR,"Could not connect to IRC server");
            if(connserverlist[++conni]==NULL)
              conni = 0;
            opmlog(L_INFO, "Retrying in %d seconds...", RetryDelay);
            sleep(RetryDelay);                  
          }
        else
          {  
            ircd_connect();
          
            while(check_ircd_buffer()) /* main loop */
              {
                /* ok,
                 we need irc data to make the scan_check
                 with a minimum irc traffic this should work smoothly
                 maybe I'll fix it later    - Lamego
                */
                scan_check();
                /* check for scan timeouts every 5 secs */
                if((CurrentTime-lasttscan) > 5) 
                  {
                    scan_timeouts();
                    lasttscan = CurrentTime;
                  }
              }                
      
            /* stop all scans in progress */
            scan_reset();
            
            /* close irc connection */
            if(irc_fd)
                close(irc_fd);

            netjoined = 0;
            
            opmlog(L_INFO, "Retrying in %d seconds...", RetryDelay);
            sleep(RetryDelay);                  
              
	      }        
      } while(RetryDelay);                  
	return 0;
  }

static void parse_command_line(int argc, char* argv[])
{
  char *options = "nd";
  int opt;
  while ((opt = getopt(argc, argv, options)) != EOF) 
    {
      switch (opt) 
        {
          case 'n': nofork = -1; break;
          case 'd': debug = -1; break;
          default:
          	bad_command();
          break;             
        }    
    }
}

/*
 * bad_command
 *      This is called when the commandline is not acceptable.
 *      Give error message and exit without starting anything.
 */
static void bad_command()
{
  fprintf(stderr,
          "Usage: ptopm [-n]\n");
  fprintf(stderr, "PTOPM not started\n");
  exit(-2);
}
