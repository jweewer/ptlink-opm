/************************************************************************
 *   IRC - Internet Relay Chat, include/msg.h
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
 */
#ifndef INCLUDED_msg_h
#define INCLUDED_msg_h

struct Client;

/* 
 * Message table structure 
 */
struct  Message
{
  char  *cmd;
  int   (* func)();
  unsigned int  count;                  /* number of times command used */
  int   parameters;
  char  flags;							/* maybe used some time - Lamego */
  unsigned long bytes;
};

struct MessageTree
{
  char*               final;
  struct Message*     msg;
  struct MessageTree* pointers[26];
}; 

typedef struct MessageTree MESSAGE_TREE;


#define MSG_PRIVATE  "PRIVMSG"  /* PRIV */
#define MSG_WHO      "WHO"      /* WHO  -> WHOC */
#define MSG_WHOIS    "WHOIS"    /* WHOI */
#define MSG_WHOWAS   "WHOWAS"   /* WHOW */
#define MSG_USER     "USER"     /* USER */
#define MSG_NICK     "NICK"     /* NICK */
#define MSG_NNICK    "NNICK"	/* NNICK */
#define MSG_SERVER   "SERVER"   /* SERV */
#define MSG_LIST     "LIST"     /* LIST */
#define MSG_TOPIC    "TOPIC"    /* TOPI */
#define MSG_INVITE   "INVITE"   /* INVI */
#define MSG_VERSION  "VERSION"  /* VERS */
#define MSG_QUIT     "QUIT"     /* QUIT */
#define MSG_SQUIT    "SQUIT"    /* SQUI */
#define MSG_KILL     "KILL"     /* KILL */
#define MSG_INFO     "INFO"     /* INFO */
#define MSG_LINKS    "LINKS"    /* LINK */
#define MSG_STATS    "STATS"    /* STAT */
#define MSG_USERS    "USERS"    /* USER -> USRS */
#define MSG_HELP     "HELP"     /* HELP */
#define MSG_HELPSYS  "HELPSYS"  /* HELP */
#define MSG_ERROR    "ERROR"    /* ERRO */
#define MSG_AWAY     "AWAY"     /* AWAY */
#define MSG_CONNECT  "CONNECT"  /* CONN */
#define MSG_PING     "PING"     /* PING */
#define MSG_PONG     "PONG"     /* PONG */
#define MSG_OPER     "OPER"     /* OPER */
#define MSG_PASS     "PASS"     /* PASS */
#define MSG_WALLOPS  "WALLOPS"  /* WALL */
#define MSG_GLOBOPS	 "GLOBOPS"	/* -> WALLOPS */
#define MSG_TIME     "TIME"     /* TIME */
#define MSG_NAMES    "NAMES"    /* NAME */
#define MSG_ADMIN    "ADMIN"    /* ADMI */
#define MSG_TRACE    "TRACE"    /* TRAC */
#define MSG_LTRACE   "LTRACE"   /* LTRA */
#define MSG_NOTICE   "NOTICE"   /* NOTI */
#define MSG_JOIN     "JOIN"     /* JOIN */
#define MSG_PART     "PART"     /* PART */
#define MSG_LUSERS   "LUSERS"   /* LUSE */
#define MSG_MOTD     "MOTD"     /* MOTD */
#define MSG_MODE     "MODE"     /* MODE */
#define MSG_SAMODE   "SAMODE"   /* MODE */
#define MSG_KICK     "KICK"     /* KICK */
#define MSG_KILL     "KILL"     /* KILL */
#define MSG_USERHOST "USERHOST" /* USER -> USRH */
#define MSG_ISON     "ISON"     /* ISON */
#define MSG_REHASH   "REHASH"   /* REHA */
#define MSG_RESTART  "RESTART"  /* REST */
#define MSG_CLOSE    "CLOSE"    /* CLOS */
#define MSG_SVINFO   "SVINFO"   /* SVINFO */
#define MSG_SJOIN    "SJOIN"    /* SJOIN */
#define MSG_NJOIN    "NJOIN"	/* NJOIN */
#define MSG_CAPAB    "CAPAB"    /* CAPAB */
#define MSG_DIE      "DIE"      /* DIE */
#define MSG_HASH     "HASH"     /* HASH */
#define MSG_DNS      "DNS"      /* DNS  -> DNSS */
#define MSG_KLINE    "KLINE"    /* KLINE */
#define MSG_UNKLINE  "UNKLINE"  /* UNKLINE */
#define MSG_DLINE    "DLINE"    /* DLINE */
#define MSG_HTM      "HTM"      /* HTM */
#define MSG_SET      "SET"      /* SET */
#define MSG_DCONF	 "DCONF"		/* DCONF */
#define MSG_SANOTICE	"SANOTICE"	/* SANOTICE */
#define MSG_NEWMASK		"NEWMASK"	/* NEWMASK */

#define MSG_GLINE    	"GLINE"    	/* GLINE */
#define MSG_UNGLINE    "UNGLINE"    /* UNGLINE */

#define MSG_SGLINE    	"SGLINE"    /* SGLINE */
#define MSG_UNSGLINE    "UNSGLINE"	/* UNGLINE */

#define MSG_SVLINE    	"SVLINE"    /* SVLINE */
#define MSG_UNSVLINE    "UNSVLINE"	/* UNSVLINE */


#define MSG_ZLINE    	"ZLINE"    	/* ZLINE */
#define MSG_UNZLINE    	"UNZLINE"  	/* UNZLINE */


#define MSG_SQLINE 		"SQLINE"    /* SQLINE */
#define MSG_UNSQLINE   	"UNSQLINE" 	/* UNSQLINE */

#define MSG_LOCOPS   	"LOCOPS"   	/* LOCOPS */
#ifdef LWALLOPS
#define MSG_LWALLOPS 	"LWALLOPS" 	/* Same as LOCOPS */
#endif /* LWALLOPS */
#define MSG_KNOCK       "KNOCK"  /* KNOCK */
#define MSG_MAP       	"MAP"  /* MAP */
#define MSG_NEWS       	"NEWS"  /* NEWS */
#define MSG_ZOMBIE		"ZOMBIE" /* ZOMBIE */
#define MSG_UNZOMBIE    "UNZOMBIE" /* UNZOMBIE */
#define MSG_DCCDENY		"DCCDENY"
#define MSG_DCCALLOW	"DCCALLOW"
#define MSG_SVSINFO		"SVSINFO"	/* SVSINFO */

/* Services only messages */
#define MSG_SVSJOIN		"SVSJOIN"
#define MSG_SVSPART		"SVSPART"
#define MSG_SVSMODE 	"SVSMODE"
#define MSG_SVSNICK		"SVSNICK"


/* Services aliases */
#define MSG_NICKSERV	"NICKSERV"
#define MSG_CHANSERV	"CHANSERV"
#define MSG_MEMOSERV	"MEMOSERV"
#define MSG_NEWSSERV	"NEWSSERV"
#define MSG_OPERSERV	"OPERSERV"
#define MSG_IDENTIFY	"IDENTIFY"

#define MSG_SILENCE		"SILENCE"
#define	MSG_WATCH		"WATCH"
#define MSG_IRCOPS		"IRCOPS"

#define MAXPARA    15 

#define MSG_TESTLINE "TESTLINE"

#ifdef MSGTAB
#ifndef INCLUDED_m_commands_h
#include "m_commands.h"       /* m_xxx */
#endif
struct Message msgtab[] = {
  { MSG_SERVER,   m_server, 0, MAXPARA, 0, 0L },  
  { MSG_NICK,     m_nick,   0, MAXPARA, 0, 0L },    
  { MSG_NNICK,    m_nick,   0, MAXPARA, 0, 0L },      
  { MSG_PING,     m_ping,   0, MAXPARA, 0, 0L },  
  { MSG_ERROR,    m_error,  0, MAXPARA, 0, 0L },    
  { MSG_SVSINFO,  m_null,   0, MAXPARA, 0, 0L },      
  { MSG_SVINFO,   m_svinfo, 0, MAXPARA, 0, 0L },
  { MSG_QUIT,     m_null,   0, MAXPARA, 0, 0L },      
  { MSG_PASS,     m_null,   0, MAXPARA, 0, 0L },
  { MSG_CAPAB,    m_null,   0, MAXPARA, 0, 0L },  
  { MSG_NOTICE,   m_null,   0, MAXPARA, 0, 0L },
  { MSG_PRIVATE,  m_privmsg,0, MAXPARA, 0, 0L },
  { MSG_KICK,     m_kick,   0, MAXPARA, 0, 0L },  
  { MSG_KILL,     m_kill,   0, MAXPARA, 0, 0L },    
  { MSG_WALLOPS,  m_null,   0, MAXPARA, 0, 0L },
  { MSG_AWAY,     m_null,   0, MAXPARA, 0, 0L },
  { MSG_TOPIC,    m_null,   0, MAXPARA, 0, 0L },
  { MSG_SJOIN,    m_null,   0, MAXPARA, 0, 0L },
  { MSG_NJOIN,    m_null,   0, MAXPARA, 0, 0L },
  { MSG_GLINE,    m_null,   0, MAXPARA, 0, 0L },
  { MSG_NEWMASK,  m_null,   0, MAXPARA, 0, 0L },
  { MSG_PART,     m_null,   0, MAXPARA, 0, 0L },  
  { MSG_MODE,     m_null,   0, MAXPARA, 0, 0L },      
  { MSG_SVLINE,   m_null,   0, MAXPARA, 0, 0L },    
  { MSG_SVSMODE,   m_null,   0, MAXPARA, 0, 0L },      
  { MSG_GLOBOPS,   m_null,   0, MAXPARA, 0, 0L },      
  { MSG_LOCOPS,   m_null,   0, MAXPARA, 0, 0L },        
  { (char *) 0, (int (*)()) 0 , 0, 0, 0L }
};

struct MessageTree* msg_tree_root;

#else
extern struct Message       msgtab[];
extern struct MessageTree*  msg_tree_root;
#endif

#endif /* INCLUDED_msg_h */

