/*****************************************************************
 * PTlink IRCd is (C) CopyRight PTlink Coders Team 1999-2002     *
 * http://www.ptlink.net/Coders/ - coders@PTlink.net             *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: services.h
  Desc: services general header
  Author: Lamego@PTlink.net
  Date: Mon 15 Jan 2001 06:49:00 PM WET
*/

int check_ircd_buffer(void);
void ircd_connect(void);
void ircd_buff_init(void);
void introduce_user(char* nick, char *user, char *host, char *info, char *umodes);
