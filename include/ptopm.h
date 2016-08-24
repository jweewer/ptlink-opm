/*****************************************************************
 * PTlink IRCd is (C) CopyRight PTlink Coders Team 1999-2000     *
 * http://www.ptlink.net/Coders - coders@PTlink.net              *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: services.h
  Desc: services general header
  Author: Lamego@PTlink.net
  Date: Mon 15 Jan 2001 06:49:00 PM WET
*/
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

extern time_t         CurrentTime;
extern time_t	      StartTime;
extern time_t         ConnectTime;
extern struct Counter Count;
extern struct Client* GlobalClientList;
extern struct Client  me;
extern int irc_fd;
extern char* connserverlist[];
extern int is_connected;
extern int netjoined;
extern int debug;

/* TS constants */
#define TS_CURRENT      9       /* current TS protocol version */
#define TS_MIN          3       /* minimum supported TS protocol version */
