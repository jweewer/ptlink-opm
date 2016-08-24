#ifndef INCLUDED_ircd_defs_h
#define INCLUDED_ircd_defs_h

#define HOSTLEN         63      /* Length of hostname.  Updated to         */
                                /* comply with RFC1123                     */
#define NICKLEN         20       /* 20 for me -Lamego */
#define USERLEN         10
#define REALLEN         50
#define BUFSIZE         512

/* 
 * Macros everyone uses :/ moved here from sys.h
 */
#define MyFree(x)       if ((x)) free((x))

#endif /* INCLUDED_ircd_defs_h */
