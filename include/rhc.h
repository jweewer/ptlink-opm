/*****************************************************************
 * PTlink IRCd is (C) CopyRight PTlink Coders Team 1999-2002     *
 * http://www.ptlink.net/Coders - coders@PTlink.net              *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: rhc.h
  Desc: Recent Hash Cache
  Author: Lamego@PTlink.net
*/

/*
 * Recent host cache size
 *
 */
#define RHC_SIZE    16384

#define CACHE_BOT       1
#define CACHE_ZOMBIE    2

struct rhc_entry {
  unsigned long ip;
  int           value;      /* CACHE_*/
  time_t        expire;     /* this entry will expire on .. */  
};

void clear_rhc(void);
extern void set_rhc_status(unsigned long ip, int value);
extern int get_rhc_status(unsigned long ip);
extern void rhc_stats(char *target);
