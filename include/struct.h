/* - Internet Relay Chat, include/struct.h
 *   Copyright (C) 1990 Jarkko Oikarinen and
 *                      University of Oulu, Computing Center
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 */
 
#ifndef INCLUDED_struct_h
#define INCLUDED_struct_h

#include <sys/types.h>       /* time_t */ 

struct Channel;
struct Ban;

typedef struct  ConfItem aConfItem;
typedef struct  Server  aServer;
typedef struct  SLink   Link;
typedef struct  Mode    Mode;
typedef struct  Zdata   aZdata;
/* general link structure used for chains */

struct SLink
{
  struct        SLink   *next;
  union
  {
    struct Client   *cptr;
    struct Channel  *chptr;
    struct ConfItem *aconf;
#ifdef BAN_INFO
    struct Ban   *banptr;
#endif
    char      *cp;
	struct Watch*	wptr;
  } value;
  int   flags;
  char    		*lmsg;          /* Last message sent to channel */
  int     		lsize;          /* Last message size */
  time_t  		lmsg_time;      /* Last message time */   
};

#endif /* INCLUDED_struct_h */
