/*****************************************************************
 * PTlink IRCd is (C) CopyRight PTlink Coders Team 1999-2002     *
 * http://www.ptlink.net/Coders - coders@PTlink.net              *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: rhc.c
  Desc: Recent Hash Cache
  Author: Lamego@PTlink.net
*/
#ifndef INCLUDED_sys_types_h
#include <sys/types.h>       /* time_t */
#define INCLUDED_sys_types_h
#endif
#ifndef INCLUDED_netinet_in_h
#include <netinet/in.h>      /* in_addr */
#define INCLUDED_netinet_in_h
#endif
#include <string.h>
#include "config.h"
#include "ptopm.h"
#include "rhc.h"
#include "send.h"
#include "dconf_vars.h"

static struct rhc_entry rhc_list[RHC_SIZE];

/* total values - for stats */
static int t_hits = 0;
static int t_cached = 0;
static int t_collisions = 0;
/*
 * hash_ip()
 * 
 * input        - unsigned long ip address
 * output       - integer value used as index into hash table
 * side effects - hopefully, none
 */
 
static int hash_ip(unsigned long ip)
{
  int hash;
  ip = ntohl(ip);
  hash = ((ip >> 12) + ip) & (RHC_SIZE-1);
  return(hash);
}

void clear_rhc(void)
{
  t_hits = 0;
  t_cached = 0;
  t_collisions = 0;
  memset(&rhc_list,0, sizeof(struct rhc_entry) * RHC_SIZE);
}

void set_rhc_status(unsigned long ip, int value)
{
  struct rhc_entry *rhc = &rhc_list[hash_ip(ip)];
  if(rhc->ip==0)
   ++t_cached;
  else if(rhc->ip!=ip)
    t_collisions++;
  rhc->ip = ip;
  rhc->value = value;
  rhc->expire = CurrentTime  + CACHE_TIME;
}

int get_rhc_status(unsigned long ip)
{
  struct rhc_entry *rhc = &rhc_list[hash_ip(ip)];
  if(CurrentTime>rhc->expire)
      rhc->ip=0;
  else if(rhc->ip==ip)
    {
      ++t_hits;
      return rhc->value;
    }
  return 0;
}

void rhc_stats(char *t)
{
    if(t_cached==0)
	    return;
   send_msg(PT_Nick,t,"Scan cache: %d cached, %d hits, %d collisions, ef=%2.1f%%",
     t_cached, t_hits, t_collisions , 
     ((float) t_hits/(t_cached+t_hits+t_collisions))*100);
};
