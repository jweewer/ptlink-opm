/*****************************************************************
 * PTlink IRCd is (C) CopyRight PTlink Coders Team 1999-2002     *
 * http://www.ptlink.net/Coders/ - coders@PTlink.net             *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: misc.c
  Desc: miscellaneous functions
  Author: Lamego@PTlink.net
*/
#include "stdinc.h"
#include "config.h"
#include "ptopm.h"
#include "dconf_vars.h"
#include "s_log.h"

/* functions declaration */

void remove_pidfile(void);
void write_pidfile(void);
void build_connserver_list(void);
char *dissect_time(time_t ttime);

void remove_pidfile(void)
{
  unlink(PIDFilename);
}

void write_pidfile(void)
{
    FILE *pidfile;

    pidfile = fopen(PIDFilename, "w");
    if (pidfile) 
	  {
        fprintf(pidfile, "%d\n", (int)getpid());
        fclose(pidfile);
        atexit(remove_pidfile);
	  } 
	  else
		log_perror(L_ERROR, "Warning: cannot write to PID file %s", PIDFilename);
}


void build_connserver_list(void)
{
  char *srv;
  int i = 1;
  connserverlist[0] = strdup(IRCServer);

  if(AltIRCServer==NULL || *AltIRCServer=='0')
    {
      connserverlist[1]=NULL;
      return;
    }
  
  srv = strtok(AltIRCServer, "\n");    
  while(srv && (i<49))
    {
      connserverlist[i++]=strdup(srv);
      srv = strtok(NULL, "\n");
    }
    
  connserverlist[i]=NULL; /* signal end of list */
}

/*
 * Split a time_t into an English-language explanation of how
 * much time it represents, e.g. "2 hours 45 minutes 8 seconds"
 */
/* copied from BOPM -Lamego */
char *dissect_time(time_t ttime)
{
	static char buf[64];
	unsigned int years, weeks, days, hours, minutes, seconds;

	years = weeks = days = hours = minutes = seconds = 0;

	while (ttime >= 60 * 60 * 24 * 365) {
		ttime -= 60 * 60 * 24 * 365;
		years++;
	}

	while (ttime >= 60 * 60 * 24 * 7) {
		ttime -= 60 * 60 * 24 * 7;
		weeks++;
	}

	while (ttime >= 60 * 60 * 24) {
		ttime -= 60 * 60 * 24;
		days++;
	}

	while (ttime >= 60 * 60) {
		ttime -= 60 * 60;
		hours++;
	}

	while (ttime >= 60) {
		ttime -= 60;
		minutes++;
	}

	seconds = ttime;

	if (years) {
		snprintf(buf, sizeof(buf),
		    "%d year%s, %d week%s, %d day%s, %02d:%02d:%02d",
		    years, years == 1 ? "" : "s", weeks,
		    weeks == 1 ? "" : "s", days, days == 1 ? "" : "s",
		    hours, minutes, seconds);
	} else if (weeks) {
		snprintf(buf, sizeof(buf),
		    "%d week%s, %d day%s, %02d:%02d:%02d", weeks,
		    weeks == 1 ? "" : "s", days, days == 1 ? "" : "s",
		    hours, minutes, seconds);
	} else if (days) {
		snprintf(buf, sizeof(buf), "%d day%s, %02d:%02d:%02d",
		    days, days == 1 ? "" : "s", hours, minutes, seconds);
	} else if (hours) {
		if (minutes || seconds) {
			snprintf(buf, sizeof(buf),
			    "%d hour%s, %d minute%s, %d second%s", hours,
			    hours == 1 ? "" : "s", minutes,
			    minutes == 1 ? "" : "s", seconds,
			    seconds == 1 ? "" : "s");
		} else {
			snprintf(buf, sizeof(buf), "%d hour%s", hours,
			    hours == 1 ? "" : "s");
		}
	} else if (minutes) {
		snprintf(buf, sizeof(buf), "%d minute%s, %d second%s",
		    minutes, minutes == 1 ? "" : "s", seconds,
		    seconds == 1 ? "" : "s");
	} else {
		snprintf(buf, sizeof(buf), "%d second%s", seconds,
		    seconds == 1 ? "" : "s");
	}

	return(buf);
}
