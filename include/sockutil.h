/*****************************************************************
 * PTlink Services is (C) CopyRight PTlink Coders Team 1999-2000 *
 * http://www.ptlink.net/Coders - coders@PTlink.net              *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: sockutil.h
  Desc: 
  Author: Lamego@PTlink.net
*/
typedef struct {
    char* data;
    char* seekpos;
    int size;
} SockBuffer;

int sockprintf(int , char *, ...);
int sock_conn(char *hostname, unsigned short portnum);
int sockbuf_read(int sock, SockBuffer *sbuf);
int sockbuf_init(SockBuffer *sbuf, int bsize);
void sockbuf_reset(SockBuffer *sbuf, int bsize);
int set_non_blocking(int fd);
