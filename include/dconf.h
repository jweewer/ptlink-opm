/*****************************************************************
 * PTlink IRCd is (C) CopyRight PTlink Coders Team 1999-2000     *
 * http://www.ptlink.net/Coders - coders@PTlink.net              *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: conf.h
  Desc: general .conf parsing
  Author: Lamego@PTlink.net
  Date: Mon 15 Jan 2001 06:49:00 PM WET
*/

/* Configuration item types */
#define CT_DESC		0	/* Description */
#define CT_STRING 	1	/* string */
#define CT_TIME		2	/* time */
#define CT_SET		3	/* set (YES/NO) */
#define CT_INT		4	/* integer */
#define CT_STARRAY      5   	/* string array */
#define CT_FUNC		6	/* call function option */

/* Configuration item flags */
#define CF_REQ		0x0001	/* Required */
#define CF_LOCKED	0x0002	/* cannot be changed at runtime */
#define CF_DEFINED	0x0004	/* value is defined */

/* change item return errors */
#define UNKNOWN_ITEM	-1
#define LOCKED_ITEM		-2
#define INV_SET_ITEM	-3
#define INV_TIME_ITEM	-4

typedef struct {
    char *name;
    int	 type;
    void *vptr;
    int flags;
} ConfItem;

/* */
int dconf_read(char *fn, int rehashing);
int dconf_check(int verbose);
/* some usefull macros */

#define DCONF_DESC(x) { (x), CT_DESC, NULL, 0 }
#define DCONF_STRING(x,y) { #x, CT_STRING, &(x), (y) }
#define DCONF_INT(x,y) { #x, CT_INT, &(x), (y) }
#define DCONF_SET(x,y) { #x, CT_SET, &(x), (y) }
#define DCONF_TIME(x,y) { #x, CT_TIME, &(x), (y) }
#define DCONF_FUNC(x) { #x, CT_FUNC, (void*) &(x), 0 }
