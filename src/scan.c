/*****************************************************************
 * PTlink OPM is (C) CopyRight PTlink Coders Team 1999-2004      *
 * http://www.ptlink.net/Coders/ - coders@PTlink.net             *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: m_scan.c
  Author: Lamego@PTlink.net
  
  The scan code was adapted from BOPM
  
  Blitzed Open Proxy Monitor, version 2.2r2
  http://blitzed.org/bopm/  
  Copyright (C) 2002  Erik Fears
  
*/

#include "stdinc.h"
#include "irc_string.h"

#define WORDS_BIGENDIAN

#ifdef USE_POLL
# include <sys/poll.h>
#endif

#include "config.h"
#include "scan.h"
#include "s_log.h"
#include "dconf_vars.h"
#include "ircdio.h"
#include "ptopm.h"
#include "send.h"
#include "misc.h"
#include "sockutil.h"
#include "rhc.h"

static void scan_memfail(void);
static void scan_establish(scan_struct *conn);
#ifdef USE_POLL
static void scan_negfail(scan_struct *conn);
#endif
static void scan_readready(scan_struct *conn);
static void scan_read(scan_struct *conn);
static void scan_openproxy(scan_struct *conn);
static void scan_writeready(scan_struct *conn);
static void scan_add(scan_struct *newconn);
static void scan_del(scan_struct *delconn);
static int scan_w_squid(struct scan_struct *conn);
static int scan_w_socks4(struct scan_struct *conn);
static int scan_w_socks5(struct scan_struct *conn);
static int scan_w_cisco(struct scan_struct *conn);
static int scan_w_wingate(struct scan_struct *conn);
static int scan_w_text(struct scan_struct *conn);
static void send_report(struct scan_struct *ss);

/* Linked list head for connections. */
struct scan_struct *CONNECTIONS = 0;

char SENDBUFF[513];

/* Keep track of numbers of open FD's, for use with MaxScans . */
unsigned int FD_USE = 0;


unsigned int total_scans = 0; /* total scans (active+queued) */
unsigned int last_closed_count = 0;  /* total connections closed on last timeout check (closed+timedout) */
/*
 * Protocol Name, Port, Write Handler, Read Handler
 *
 * Always scan Cisco before Wingate, because cisco routers only allow 4
 * connects at once.
 */

static scan_rule scan_rules[MAX_ROLES];

int scan_rules_count;	/* count of scan roles we have defined */

char* dnsbl_zones[MAX_DNSBLZONES];
static int dnsbl_hits[MAX_DNSBLZONES];
static int dnsbl_scans[MAX_DNSBLZONES];

void do_scan_init(void)
{
  if(DNSBLZones)
    parse_multi(DNSBLZones, ",", dnsbl_zones, MAX_DNSBLZONES);
  else
    dnsbl_zones[0] = NULL;
}


static void scan_memfail(void)
{
	opmlog(L_ERROR, "SCAN -> Error allocating memory.");
	exit(EXIT_FAILURE);
}



/*
 * We received a NICK, lets scan if it's not cached
 */
void scan_connect(char *irc_addr, char *irc_nick,
    char *irc_user, int verbose, int bypasscache)
{
  size_t i;                
  scan_struct *newconn; 
  struct hostent *he;
  struct in_addr ia;    
  char *ip;
  int ipl;
  char* buf;
  int buflen;
  int a,b,c,d;
  int i2 = 0;
    
  ip = irc_addr;
  if((ia.s_addr = inet_addr(irc_addr)) == -1)      
    {
      he = gethostbyname(irc_addr);
      if(!he)
        {
          if(LogChan)
            sendto_ircd(PT_Nick, "PRIVMSG #%s :Could not resolve %s .", LogChan, irc_addr);		
          return;		
  	}
       memcpy((char *)&ia.s_addr, he->h_addr_list[0], sizeof(unsigned long));
       ip = inet_ntoa(*((struct in_addr *) he->h_addr));
    }

    ipl = ntohl(ia.s_addr);
    if(bypasscache == 0)
      {
        if(get_rhc_status(ipl)==-1) /* */
          return;
        else
          set_rhc_status(ntohl(ia.s_addr), -1); /* flag as on scan/scanned */
      }
      
    while(dnsbl_zones[i2])
      {
        a = (int) (ia.s_addr >> 24) & 0xFF;
        b = (int) (ia.s_addr >> 16) & 0xFF;
        c = (int) (ia.s_addr >> 8) & 0xFF;
        d = (int) ia.s_addr & 0xFF;

        buflen = 18 + strlen(dnsbl_zones[i2]);
        buf = malloc(buflen * sizeof(char));
     
  #ifdef WORDS_BIGENDIAN
        snprintf(buf, buflen, "%d.%d.%d.%d.%s.", a, b, c, d, dnsbl_zones[i2]);
  #else
        snprintf(buf, buflen, "%d.%d.%d.%d.%s.", d, c, b, a, dnsbl_zones[i2]);
  #endif
        dnsbl_scans[i2]++;
        if((he = gethostbyname(buf)) != NULL)  /* open proxy was found */
          {
            dnsbl_hits[i2]++;
            if(DNSBLZoneMsg)            
              sendto_ircd(PT_Nick, "GLINE *@%s %d %s :(%s) %s",
                ip, GLineTime, PT_Nick, dnsbl_zones[i2], DNSBLZoneMsg);
            else
              sendto_ircd(PT_Nick, "GLINE *@%s %d %s :(%s)",
                ip, GLineTime, PT_Nick, dnsbl_zones[i2]);
                             
            opmlog(L_WARN,"OPEN PROXY -> %s: %s!%s@%s", "DNSBL (%s)",
              irc_nick, irc_user, irc_addr, dnsbl_zones[i2]);

            sendto_ircd(PT_Nick, "PRIVMSG #%s :%s (%s): OPEN PROXY -> %s!%s@%s",
              LogChan, "DNSBL", dnsbl_zones[i2], irc_nick, irc_user, irc_addr, dnsbl_zones[i2]);
            free(buf);        
            return;
          }     
        free(buf);
        ++i2;
      }
	/*
	 * Loop through the protocols creating a seperate connection struct
	 * for each port/protocol.
	 */

	for (i = 0; i < scan_rules_count; i++) {
		newconn = malloc(sizeof(*newconn));

		if (!newconn)
			scan_memfail();               

		newconn->addr = strdup(irc_addr);
		newconn->irc_addr = strdup(irc_addr);
		newconn->irc_nick = strdup(irc_nick);
		newconn->irc_user = strdup(irc_user);
		/* This is allocated later on in scan_establish to save on
		 * memory.
		 */
		newconn->data = 0;
		newconn->verbose = verbose;
		newconn->bytes_read = 0; 
		newconn->fd = 0;
		/*
		 * Give struct a link to information about the protocol it
		 * will be handling.
		 */
		newconn->rule = &(scan_rules[i]);

		/* Fill in sockaddr with information about remote host. */
        	newconn->sockaddr.sin_addr.s_addr = ia.s_addr;
		newconn->sockaddr.sin_family = AF_INET;
		newconn->sockaddr.sin_port =
		htons(newconn->rule->port); 
            
	    /* Queue connection. */
        	newconn->state = STATE_UNESTABLISHED;

		/* Add struct to list of connections. */
		scan_add(newconn);
                
		/* If we have available FD's, overide queue. */
		if (FD_USE < MaxScans )
		  scan_establish(newconn);
		/* else  scan was queued */
	}
}

/*
 * Get FD for new socket, bind to interface and connect() (non blocking) then
 * set conn to ESTABLISHED for write check.
 */
static void scan_establish(scan_struct *conn)
{
	/* For local bind() */
	struct sockaddr_in SCAN_LOCAL;

	memset(&SCAN_LOCAL, 0, sizeof(struct sockaddr_in));

	/* Setup SCAN_LOCAL for local bind() */
	if (LocalAddress) {
		if (!inet_aton(LocalAddress, &(SCAN_LOCAL.sin_addr))) {
			opmlog(L_ERROR, "SCAN -> bind(): %s is an invalid address",
			    LocalAddress);
			exit(EXIT_FAILURE);
		}

		SCAN_LOCAL.sin_family = AF_INET;
		SCAN_LOCAL.sin_port = 0;
	}

	/* Request file descriptor for socket. */
	conn->fd = socket(PF_INET, SOCK_STREAM, 0);

	/* If error, mark connection for close. */
	if (conn->fd == -1) {
		opmlog(L_ERROR,"SCAN -> Error allocating file descriptor.");
		conn->state = STATE_CLOSED;
		return;
	}

	/* Bind to specific interface designated in conf file. */
	if (LocalAddress) {
		if (bind(conn->fd, (struct sockaddr *)&SCAN_LOCAL,
		    sizeof(struct sockaddr_in)) == -1) {
			switch (errno) {
			case EACCES:
				opmlog(L_ERROR, "SCAN -> bind(): No access to bind to %s",
				    LocalAddress);
				break;
			default:
				opmlog(L_ERROR, "SCAN -> bind(): Error binding to %s",
				    LocalAddress);
				break;

			}
			exit(EXIT_FAILURE);
		}
	}

	/* Log create time of connection for timeouts. */
	time(&(conn->create_time));
	/* Flag conn established (for write). */
	conn->state = STATE_ESTABLISHED;
	/* Set socket non blocking. */
	set_non_blocking(conn->fd);
	/* Connect! */
	if((connect(conn->fd, (struct sockaddr *) &(conn->sockaddr),
	    sizeof(conn->sockaddr)))==-1)
        {
          if(errno!=EINPROGRESS)
            {
              log_perror(L_ERROR,"Could not connect()");
              close(conn->fd);
              conn->state = STATE_CLOSED;
		      return;
              
            }
        }
	/* Allocate memory for the scan buffer. */
	conn->data = malloc((SCANBUFFER + 1) * sizeof(char));
	conn->datasize = 0;

	/* Increase global FD Use counter. */      
	FD_USE++;
}



/*
 * Test for sockets to be written/read to.
 */

void scan_check(void)
{
#ifdef USE_POLL
	/* MAX_POLL is defined in config.h */
	static struct pollfd ufds[MAX_POLL];
	unsigned long size, i;
#else /* select() */
	fd_set w_fdset;
	fd_set r_fdset;
	struct timeval scan_timeout;
	int highfd;
#endif /* USE_POLL */

	struct scan_struct *ss;

#ifdef USE_POLL
	size = 0;

	/* Get size of list we're interested in. */
	for (ss = CONNECTIONS; ss; ss = ss->next) {
		if (ss->state != STATE_CLOSED &&
		    ss->state != STATE_UNESTABLISHED)
			size++;
	}
    
	i = 0;

	/* Setup each element now. */
	for (ss = CONNECTIONS; ss; ss = ss->next) {
		if (ss->state == STATE_CLOSED ||
		    ss->state == STATE_UNESTABLISHED)
			continue;

		ufds[i].events = 0;
		ufds[i].revents = 0;
		ufds[i].fd = ss->fd;

		/* Check for HUNG UP. */
		ufds[i].events |= POLLHUP;
		/* Check for INVALID FD */
		ufds[i].events |= POLLNVAL;

		switch (ss->state) {
		case STATE_ESTABLISHED:
			/* Check for NO BLOCK ON WRITE. */
			ufds[i].events |= POLLOUT;
			break;
		case STATE_SENT:
			/* Check for data to be read. */
			ufds[i].events |= POLLIN;
			break;
		}

		if (++i >= (MAX_POLL-1))
			break;
	}

#if 0

	ufds[i].revents = 0;
	ufds[i].fd = irc_fd;

    /* Check for data in */
	ufds[i].events |= POLLIN;    
	/* Check for HUNG UP. */
	ufds[i].events |= POLLHUP;
	/* Check for INVALID FD */
	ufds[i].events |= POLLNVAL;    
#endif    

#else /* select() */
	FD_ZERO(&w_fdset);
	FD_ZERO(&r_fdset);
	highfd = 0;

	/* Add connections to appropriate sets. */

	for (ss = CONNECTIONS; ss; ss = ss->next) {
		if (ss->state == STATE_ESTABLISHED) {
			if(ss->fd > highfd)    
				highfd = ss->fd;

			FD_SET(ss->fd, &w_fdset);
			continue;
		}
         
		if (ss->state == STATE_SENT) {
			if (ss->fd > highfd)
				highfd = ss->fd;
             
			FD_SET(ss->fd, &r_fdset);
		}
	}

    FD_SET(irc_fd, &r_fdset);
    
    if(irc_fd>highfd)
      highfd=irc_fd;
      
	/* No timeout. */
	scan_timeout.tv_sec = 1;
	scan_timeout.tv_usec= 0;

#endif /* USE_POLL */


#ifdef USE_POLL
	switch (poll(ufds, size, 1000)) { /* +1 to poll ircd */
#else /* select() */
	switch (select((highfd + 1), &r_fdset, &w_fdset, 0, &scan_timeout)) {
#endif /* USE_POLL */
	case -1:
		/* error in select/poll */
		return;
	case 0:
		break;
	default:
		/* Pass pointer to connection to handler. */

#ifdef USE_POLL
		for (ss = CONNECTIONS; ss; ss = ss->next) {
			for (i = 0; i < size; i++) {
				if (ufds[i].fd == ss->fd) {
					if (ufds[i].revents & POLLIN)
						scan_readready(ss);

					if (ufds[i].revents & POLLOUT)
						scan_writeready(ss);
             
					if (ufds[i].revents & POLLHUP)
						scan_negfail(ss);

					break;
				}
			}
		}
#else

		for (ss = CONNECTIONS; ss; ss = ss->next) {
			if ((ss->state == STATE_ESTABLISHED) &&
			    FD_ISSET(ss->fd, &w_fdset))
				scan_writeready(ss);

			if ((ss->state == STATE_SENT) &&
			    FD_ISSET(ss->fd, &r_fdset))                    
				scan_readready(ss);     
		}               
        
      
#endif /* USE_POLL */

	}
        
}

#ifdef USE_POLL
/*
 * Negotiation failed - Read returned false, we discard the connection as a
 * closed proxy to save CPU.                                                     
 */
static void scan_negfail(scan_struct *conn)
{
	if (conn->verbose) {
		sendto_ircd(PT_Nick,"PRIVMSG #%s :%s (%d): Connection to %s closed, "
		    "negotiation failed (%d bytes read)", LogChan,
		    conn->rule->type, conn->rule->port,
		    conn->irc_addr, conn->bytes_read);
	}
	conn->state = STATE_CLOSED;
}
#endif
/*
 * Poll or select returned back that this connection is ready for read.
 */
static void scan_readready(scan_struct *conn)
{
  char c;
  
  while(1) 
   {
     switch (read(conn->fd, &c, 1)) {
       case  0:
         if (conn->verbose) 
           {
             sendto_ircd(PT_Nick,"PRIVMSG #%s :%s (%d): Connection to %s closed, "
	        "negotiation failed (%d bytes read)", LogChan,
	        conn->rule->type, conn->rule->port,
	      conn->irc_addr, conn->bytes_read);
           }
       	 conn->state = STATE_CLOSED;                      
       case -1:
         if(errno==EAGAIN)
           return;
         if (conn->verbose) 
           {
             sendto_ircd(PT_Nick,"PRIVMSG #%s :%s (%d): Connection to %s closed, "
               "negotiation failed (%d bytes read)", LogChan,
	        conn->rule->type, conn->rule->port,
                conn->irc_addr, conn->bytes_read);
	   }
         conn->state = STATE_CLOSED;                              
	   return;

       default:
         conn->bytes_read++;
         conn->rule->bytes_read++;
	 if (c == 0 || c == '\r')
	   continue;
	                            
         if(c == '\n') 
           {
	     conn->data[conn->datasize] = 0;
	     conn->datasize = 0;
	     scan_read(conn);              
	     continue;
  	   }
			
         /* Avoid freezing from reading endless data. */
         if (conn->bytes_read >= MAXREAD) 
           {
	     conn->state = STATE_CLOSED;
	     return;
           }

         if (conn->datasize < SCANBUFFER) 
           {
	     /* -1 to pad for null term. */
	     conn->data[(++conn->datasize) - 1] = c;
	   }
     }
   }
}

/*
 * Read one line in from remote, check line against target line.
 */
static void scan_read(scan_struct *conn)
{
#if 0
	opmlog(L_INFO,"SCAN -> Checking data from %s [%s:%d] against "
		    "TARGET_STRING: %s", conn->addr, conn->rule->type,
		    conn->rule->port, conn->data);
#endif           
  if(debug)
    opmlog(L_INFO, "Received %s [%s:%d]: %s", 
      conn->addr, conn->rule->type, conn->rule->port, conn->data);
  if (strstr(conn->data, conn->rule->scan_string))
    scan_openproxy(conn);
}

/*
 * Test proved positive for open proxy.
 */
static void scan_openproxy(scan_struct *conn)
{
	scan_struct *ss;
	
    if(!conn->verbose && ReportEmail)
      send_report(conn);
    
    sendto_ircd(PT_Nick, "GLINE *@%s %d %s :%s %s(%d)",
     conn->irc_addr, GLineTime, PT_Nick, 
     conn->rule->reason ? conn->rule->reason : GLineReason, 
      conn->rule->type, conn->rule->port);
            
	opmlog(L_WARN,"OPEN PROXY -> %s: %s!%s@%s (%d)", conn->rule->type,
	    conn->irc_nick, conn->irc_user, conn->irc_addr,
	    conn->rule->port);

	sendto_ircd(PT_Nick, "PRIVMSG #%s :%s (%d): OPEN PROXY -> %s!%s@%s",
	    LogChan, conn->rule->type, conn->rule->port,
		conn->irc_nick, conn->irc_user, conn->irc_addr);

	/* Increase number OPEN (insecure) of this type. */
	conn->rule->stat_numopen++;

	conn->state = STATE_CLOSED;

	/*
	 * Flag connections with the same addr CLOSED aswell, but only if this
	 * is not a verbose check.  When it is verbose/manual, we care about
	 * all types of proxy.  When it is automatic (i.e. when a user
	 * connects) we want them killed quickly sow e can move on.
	 */
	if (!conn->verbose) {
		for (ss = CONNECTIONS;ss;ss = ss->next) {
			if (conn->sockaddr.sin_addr.s_addr == ss->sockaddr.sin_addr.s_addr)
				ss->state = STATE_CLOSED;
		}
	}
}

/*
 * Poll or select returned back that this connect is ready for write.
 */
static void scan_writeready(scan_struct *conn)
{
	/* If write returns true, flag STATE_SENT. */
	if ((*conn->rule->w_handler)(conn))
	  conn->state = STATE_SENT;

	/* Increase number attempted negotiated of this type. */
	conn->rule->stat_num++;
}

/*
 * Link struct to connection list.
 */
static void scan_add(scan_struct *newconn)
{
	scan_struct *ss;
    
    ++total_scans;
	/* Only item in list. */
         
	if (!CONNECTIONS) {
		newconn->next = 0;
		CONNECTIONS = newconn;
	} else {
		/* Link to end of list. */
		for(ss = CONNECTIONS; ss; ss = ss->next) {
			if (!ss->next) {
				newconn->next = 0;
				ss->next = newconn;
				break;
			}
		}
	}
}


/*
 * Unlink struct from connection list and free its memory.
 */
static void scan_del(scan_struct *delconn)
{

	scan_struct *ss;
	scan_struct *lastss;

    --total_scans;
	if (delconn->fd > 0) 
      {
		close(delconn->fd);

	    /* 1 file descriptor freed up for use. */
	    FD_USE--;
      }
    
	lastss = 0;

	for(ss = CONNECTIONS; ss; ss = ss->next) {
		if (ss == delconn) {     
			/* Link around deleted node */                                   
			if (lastss == 0)
				CONNECTIONS = ss->next;                     
			else
				lastss->next = ss->next;
                     
			free(ss->addr);
			free(ss->irc_addr);
			free(ss->irc_nick);
			free(ss->irc_user);

			/* If it's established, free the scan buffer. */
			if (delconn->data)
				free(delconn->data);

			free(ss);

			break;
		}

		lastss = ss;
	}
 
}

/*
 * Alarm signaled, loop through connections and remove any we don't need
 * anymore.
 */
void scan_timeouts()
{
	scan_struct *ss;
	scan_struct *nextss;
 
	time_t present;
	time(&present);
    
    last_closed_count = 0;
	/*
	 * Check for timed out connections and also check if queued
	 * (UNESTABLISHED) connections can be established now.
	 */

	for (ss = CONNECTIONS; ss;) {
		if (ss->state == STATE_UNESTABLISHED) { 
			if (FD_USE < MaxScans ) {
				scan_establish(ss);
/*                
				if (OPT_DEBUG >= 3) {
					opmlog("SCAN -> File descriptor free, "
					    "continuing queued scan on %s",
					    ss->addr);
				}
*/                
			} else {
				ss = ss->next;

				/*
				 * Continue to avoid timeout checks on an
				 * unestablished connection.
				 */
				continue;
			}
		}

		if (((present - ss->create_time) >= ScanTimeOut) ||
		    (ss->state == STATE_CLOSED)) {
			/* State closed or timed out, remove */ 
			if (ss->verbose && (ss->state != STATE_CLOSED)) {
				if (ss->bytes_read) {
					sendto_ircd(PT_Nick,"PRIVMSG #%s :%s (%d): "
					    "Negotiation to %s timed out "
					    "(%d bytes read).",
					    LogChan,
					    ss->rule->type,
					    ss->rule->port,
					    ss->irc_addr, ss->bytes_read);
				} else {
					sendto_ircd(PT_Nick,"PRIVMSG #%s :%s (%d): "
					    "Negotiation to %s timed out "
					    "(No response).",
					    LogChan,
					    ss->rule->type,
					    ss->rule->port, 
					    ss->irc_addr);
				}
			}
            ++last_closed_count;
			nextss = ss->next;
			scan_del(ss);
			ss = nextss;
			continue;
		}
   		ss = ss->next;
	}
    
}


/* Clear all existing connections */
void scan_reset(void)
{
  scan_struct *delconn = CONNECTIONS;
  scan_struct *aux;
  
  while(delconn)
    {
      aux = delconn->next;
      if (delconn->fd > 0) 
		  close(delconn->fd);

		free(delconn->addr);
		free(delconn->irc_addr);
		free(delconn->irc_nick);
		free(delconn->irc_user);

		/* If it's established, free the scan buffer. */
		if (delconn->data)
			free(delconn->data);

	    free(delconn);
        delconn = aux;
	}
    
  CONNECTIONS = NULL;
  total_scans = FD_USE = 0;
    
}

/*
 * Function for handling open HTTP data.
 *
 * Return 1 on success.
 */
static int scan_w_squid(struct scan_struct *conn)
{
  int len;  
  snprintf(SENDBUFF, 128, "CONNECT %s:%d HTTP/1.0\r\n\r\n",
    DestIP, DestPort);
  len = strlen(SENDBUFF);
  send(conn->fd, SENDBUFF, len, 0);  
  if(debug)
    opmlog(L_INFO, "Sent %s [%s:%d]: %s",
      conn->addr, conn->rule->type, conn->rule->port, SENDBUFF);          
  conn->rule->bytes_write += len;
  return 1;
}


/*
 * CONNECT request byte order for socks4
 *  
 *  		+----+----+----+----+----+----+----+----+----+----+....+----+
 *  		| VN | CD | DSTPORT |      DSTIP        | USERID       |NULL|
 *  		+----+----+----+----+----+----+----+----+----+----+....+----+
 *   # of bytes:  1    1      2              4           variable       1
 *  						 
 *  VN = Version, CD = Command Code (1 is connect request)
 */
static int scan_w_socks4(struct scan_struct *conn)
{
	struct in_addr addr;
	unsigned long laddr;
	int len;
 
	if (inet_aton(DestIP, &addr) == 0) {
		opmlog(L_ERROR,"SCAN -> scan_w_socks4 : %s is not a valid IP",
		    DestIP);
	}
    
	laddr = htonl(addr.s_addr);
 
	len = snprintf(SENDBUFF, 512, "%c%c%c%c%c%c%c%c%c",  4, 1,
	    (((unsigned short) DestPort) >> 8) & 0xFF,
	    (((unsigned short) DestPort) & 0xff),
	    (char) (laddr >> 24) & 0xFF, (char) (laddr >> 16) & 0xFF,
	    (char) (laddr >> 8) & 0xFF, (char) laddr & 0xFF, 0);

	send(conn->fd, SENDBUFF, len, 0);
	conn->rule->bytes_write += len;
	return 1;
}

/*
 * Send version authentication selection message to socks5
 *
 *       +----+----------+----------+
 *       |VER | NMETHODS | METHODS  |
 *       +----+----------+----------+
 *       | 1  |    1     | 1 to 255 |
 *       +----+----------+----------+
 *
 *  VER always contains 5, for socks version 5
 *  Method 0 is 'No authentication required'
 *
 *
 *
 *  The SOCKS request is formed as follows:
 *
 *        +----+-----+-------+------+----------+----------+
 *       |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
 *       +----+-----+-------+------+----------+----------+
 *       | 1  |  1  | X'00' |  1   | Variable |    2     |
 *       +----+-----+-------+------+----------+----------+
 *
 *     Where:
 *
 *         o  VER    protocol version: X'05'
 *         o  CMD
 *            o  CONNECT X'01'
 *            o  BIND X'02'
 *            o  UDP ASSOCIATE X'03'
 *         o  RSV    RESERVED
 *         o  ATYP   address type of following address
 *            o  IP V4 address: X'01'
 *            o  DOMAINNAME: X'03'
 *            o  IP V6 address: X'04'
 *         o  DST.ADDR       desired destination address
 *         o  DST.PORT desired destination port in network octet
 *            order
 *
 *
 */

static int scan_w_socks5(struct scan_struct *conn)
{

        struct in_addr addr;
        unsigned long laddr;
        int len;

        if (inet_aton(DestIP, &addr) == 0) {
                opmlog(L_ERROR,"SCAN -> scan_w_socks4 : %s is not a valid IP",
                    DestIP);
        }

        laddr = htonl(addr.s_addr);

        /* Form authentication string */
        /* Version 5, 1 number of methods, 0 method (no auth). */
        len = snprintf(SENDBUFF, 512, "%c%c%c", 5, 1, 0);
        send(conn->fd, SENDBUFF, len, 0);
        conn->rule->bytes_write += len;
        /* Form request string */

        /* Will need to write ipv6 support here in future
         * as socks5 is ipv6 compatible
         */

        len = snprintf(SENDBUFF, 512, "%c%c%c%c%c%c%c%c%c%c", 5, 1, 0, 1,
            (char) (laddr >> 24) & 0xFF, (char) (laddr >> 16) & 0xFF,
            (char) (laddr >> 8) & 0xFF, (char) laddr & 0xFF,
            (((unsigned short) DestPort) >> 8) & 0xFF,
            (((unsigned short) DestPort) & 0xFF)
                      );

        send(conn->fd, SENDBUFF, len, 0);
        return 1;
}


/*
 * Cisco scanning
 *
 * Some cisco routers have 'cisco' set as password which allow open telnet
 * relay. Attempt to connect using cisco as a password, then give command for
 * telnet to the scanip/scanport
 */
static int scan_w_cisco(struct scan_struct *conn)
{
	int len;

	len = snprintf(SENDBUFF, 512, "cisco\r\n");
	send(conn->fd, SENDBUFF, len, 0); 

	len = snprintf(SENDBUFF, 512, "telnet %s %d\r\n", DestIP,
	    DestPort);
	send(conn->fd, SENDBUFF, len, 0);
        conn->rule->bytes_write += len;
	return 1;
}


/*
 * Open wingates require no authentication, they will send a prompt when
 * connect. No need to send any data.
 */
static int scan_w_wingate(struct scan_struct *conn)
{
  int len;
  
  len = snprintf(SENDBUFF, 512, "%s:%d\r\n", DestIP,
    DestPort);
  send(conn->fd, SENDBUFF, len, 0);
  conn->rule->bytes_write += len;
  return 1;
}


/*
 * Plain text scanning
 * We send a text string and check if another is received
 */
static int scan_w_text(struct scan_struct *conn)
{
	int len;
        
	snprintf(SENDBUFF, 512, conn->rule->send_string);
	len = strlen(SENDBUFF);
	send(conn->fd, SENDBUFF, len, 0); 

        conn->rule->bytes_write += len;
	return 1;
}

void scan_stats(char *t)
{
  int i;
  
  send_msg(PT_Nick,t,"Active scans: %d (max=%d), %d bytes of memory used", FD_USE, MaxScans , 
    FD_USE*(((SCANBUFFER + 1) * sizeof(char))+sizeof(scan_struct)));  
  if(total_scans>FD_USE)
    send_msg(PT_Nick,t,"Queued scans: %d, %d bytes of memory used",total_scans-FD_USE,
        FD_USE*sizeof(scan_struct));
  send_msg(PT_Nick,t,  "   Scan rate: %d scan(s)/ %d second(s)", last_closed_count, TIMEOUT_CHECK);
  send_msg(PT_Nick,t,"---------- Roles Hit(s)/Scan(s) ---------");
  for (i = 0; i < scan_rules_count; i++) 
    send_msg(PT_Nick,t," %s  %d / %d", scan_rules[i].name, scan_rules[i].stat_numopen,
      scan_rules[i].stat_num);
  i = 0;
  while(dnsbl_zones[i])
    {
      send_msg(PT_Nick,t," DNSBL %-26s  %d / %d",
        dnsbl_zones[i], dnsbl_hits[i], dnsbl_scans[i]);
      ++i;
    }
  send_msg(PT_Nick,t,"-----------------------------------------");
  send_msg(PT_Nick,t,"  IRC uptime: %s", dissect_time(CurrentTime-ConnectTime));    
  send_msg(PT_Nick,t,"Total uptime: %s", dissect_time(CurrentTime-StartTime));
}

void scan_netstats(char *t)
{
  int i;
  float kup, kdown;
  
  send_msg(PT_Nick,t,"--------- Network usage stats  ----------");
  for (i = 0; i < scan_rules_count; i++)
    {
      kup = scan_rules[i].bytes_write / 1024;
      kdown = scan_rules[i].bytes_read / 1024;
      send_msg(PT_Nick,t," %s  up: %3.1f Kb , down: %3.1f Kb", 
        scan_rules[i].name, kup, kdown );
    }
  send_msg(PT_Nick,t,"-----------------------------------------");
  send_msg(PT_Nick,t,"  IRC uptime: %s", dissect_time(CurrentTime-ConnectTime));
  send_msg(PT_Nick,t,"Total uptime: %s", dissect_time(CurrentTime-StartTime));
}

/*
 * Send an email to report this open proxy.
 */
void send_report(struct scan_struct *ss)
{
#ifndef SENDMAILPATH
	return;
#else	
 	char buf[4096], cmdbuf[512];
	FILE *fp;

	if (!ss || !ss->addr)
		return;


	snprintf(cmdbuf, sizeof(cmdbuf), "%s -t", SENDMAILPATH);
	snprintf(buf, sizeof(buf),
            "From: %s\n"
            "To: %s\n"
            "Subject: PTOPM Report\n\n"
            "%s: %s\n\n", PT_Nick, ReportEmail,
            ss->rule->type, ss->addr);

	if ((fp = popen(cmdbuf, "w")) == NULL) {
		opmlog(L_ERROR,"REPORT -> Failed to create pipe to '%s' for email "
                    "report!", cmdbuf);
		sendto_ircd(PT_Nick,"PRIVMSG #%s :I was trying to create a pipe to "
                    "'%s' to send a REPORT report, and it failed!  I'll "
                    "give up for now.", LogChan, cmdbuf);
		return;
	}

        fputs(buf, fp);
	pclose(fp);
#endif	/* SENDMAILPATH */
}

/* This function will create a rule struct from a rule definition line 
   ScanRule "name:type:port:scan_string:send_string:reason"
*/
void ScanRule(char *line)
{
  char *name, *type, *strport, *scan_string, *send_string, *reason;
  int port = 0;
  
  name = strtok(line, ":");
  type = strtok(NULL, ":");
  strport = strtok(NULL, ":");
  if(strport)
    port = atoi(strport);
  scan_string = strtok(NULL, ":");
  send_string = strtok(NULL, ":");
  reason = strtok(NULL, ":");
  if(!name || !type || !port )
    {
      opmlog(L_NOTICE, "Missing type or port on rule: %s", line);
      return;
    }
  if(strcasecmp(type, "http") == 0)
    scan_rules[scan_rules_count].w_handler = &(scan_w_squid);
  else if(strcasecmp(type, "socks4") == 0)
    scan_rules[scan_rules_count].w_handler = &(scan_w_socks4);
  else if(strcasecmp(type, "socks5") == 0)
    scan_rules[scan_rules_count].w_handler = &(scan_w_socks5);
  else if(strcasecmp(type, "cisco")  == 0)
    scan_rules[scan_rules_count].w_handler = &(scan_w_cisco);
  else if(strcasecmp(type, "wingate")  == 0)
    scan_rules[scan_rules_count].w_handler = &(scan_w_wingate);
  else if(strcasecmp(type, "text") == 0)
    scan_rules[scan_rules_count].w_handler = &(scan_w_text);
  else
    {
      opmlog(L_NOTICE, "Invalid type on rule: %s", line);
      return;
    }
  scan_rules[scan_rules_count].name = strdup(name);
  scan_rules[scan_rules_count].type= strdup(type);
  scan_rules[scan_rules_count].port = port;
  scan_rules[scan_rules_count].stat_num = 0;
  scan_rules[scan_rules_count].stat_numopen = 0;
  if(scan_string)
    scan_rules[scan_rules_count].scan_string = strdup(scan_string);
  if(send_string)
    scan_rules[scan_rules_count].send_string = strdup(send_string);
  if(reason)
    scan_rules[scan_rules_count].reason = strdup(reason);
  ++scan_rules_count;
}
