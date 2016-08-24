/*
 * send.h
 *
 */

#ifndef INCLUDED_send_h
#define INCLUDED_send_h
extern  void sendto_ircd(const char *, const char *, ...);
extern  void send_notice(const char *source, const char *target, const char *fmt, ...);
extern void send_msg(const char *source, const char *target, const char *fmt, ...);
#endif
