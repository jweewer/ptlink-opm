/*****************************************************************
 * PTlink BOPM is (C) CopyRight PTlink Coders Team 1999-2002     *
 * http://www.ptlink.net/Coders/ - coders@PTlink.net             *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: hash.h
  Author: Lamego@PTlink.net
*/

#ifndef INCLUDED_hash_h
#define INCLUDED_hash_h
extern struct client *add_host_hash(char *host);
extern void del_host_hash(char *host);
extern void hash_stat(char *to);
extern int find_host_hash_cnt(char *host);
extern struct client *find_host_hash(char *host);
extern void match_host(char *to, char *mask);
extern void load_ignore();
extern int is_ignore_mask(char *host);
extern int add_ignore_mask(char *mask);
extern void ignore_mask_list(char *to);
extern void list_host_hash(char* to);
extern int del_ignore_mask(char *mask);
extern long ignore_list_size(void);
extern int igmask_count(void);
extern int ignore_save(void);
extern void kill_hash();
#endif /* INCLUDED_hash_h */
