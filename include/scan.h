#ifndef SCAN_H
#define SCAN_H

#define MAX_ROLES	50	/* maximum number of roles we support */
#define MAX_DNSBLZONES 	10	/* maximum number of dnsblzones we accept */

typedef struct scan_rule scan_rule;
typedef struct scan_struct scan_struct;
typedef int (*scan_function) (scan_struct *);

#define STATE_UNESTABLISHED 1
#define STATE_ESTABLISHED   2
#define STATE_SENT          3
#define STATE_CLOSED        4   
 
struct scan_rule
{
	char *name;                 /* Just a rule name */
	char *type;                 /* Plaintext name of protocol to scan   */
	int port;                   /* Port to scan protocol on             */
	scan_function w_handler;    /* Function to handle specific protocol */
	unsigned int stat_num;
	unsigned int stat_numopen;
	char *scan_string;	
	char *send_string;
	char *reason;
	u_int32_t bytes_read;
	u_int32_t bytes_write;
};

struct scan_struct
{
	scan_struct *next;   
	char *addr;                  /* Address of remote host (IP)                      */
	char *irc_addr;              /* Hostname of user on IRC (for kline)              */ 
	char *irc_nick;              /* Nickname of user on IRC (for logging)            */
	char *irc_user;              /* Username of user on IRC (for logging)            */ 
	char *data;                  /* Buffered data                                    */
	int  datasize;               /* Length of buffered data                          */

	int fd;                      /* File descriptor of socket                        */
	struct sockaddr_in sockaddr; /* holds information about remote host for socket() */
	time_t create_time;          /* Creation time, for timeout                       */         
	int state;                   /* Status of scan                                   */
	unsigned int bytes_read;     /* Number of bytes received                         */
	scan_rule *rule;             /* Pointer to scan rule                             */
	int verbose;                 /* report progress to channel verbosely?            */
};

extern void do_scan_init(void);
void scan_connect(char *irc_addr, char *irc_nick,
  char *irc_user, int verbose, int bypasscache);    
extern void scan_check(void);
extern void scan_timeouts(void);
extern void scan_reset(void);
extern void scan_stats(char *t);
extern void scan_netstats(char *t);
#endif 
