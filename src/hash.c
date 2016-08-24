/* 
 * mmm, hash.
 * Yes, kids, it's time to find the root of the problem. The *dun dun duuun*
 * HASH BUCKET ARRAYS!
 */

#include <stdinc.h>

#include "config.h"
#include "common.h"
#include "s_log.h"
#include "send.h"
#include "hash.h"
#include "irc_string.h"
#include "dconf_vars.h"

#define HASHSIZE 65269 	/* Largest prime < 65536-256 */
#define PRIMENUM 607	/* Some random prime, change for optimization */

#define IGMASKSIZE  512

/* hvalues lookup table:
 * f(x) = (x * primenum) % HASHSIZE
 * 5-10% faster than doing this calculation every time.. 
 */

static unsigned short hvalues[HASHSIZE + 256];

static struct client *clhashtable[HASHSIZE];
static char* igmasklist[IGMASKSIZE];
static int igmaskcount = 0;

Client *headclient;
long cstructs;


void munge_rn(char *x);

void setup_hash();

void setup_hash()
{
	unsigned short i;

	bzero((char *)&clhashtable, sizeof(int) * HASHSIZE);

	for (i = 0; i < HASHSIZE + 256; i++)
		hvalues[i] = (i * PRIMENUM) % HASHSIZE;

	headclient = NULL;
	cstructs = 0;
}

static unsigned short hash_name(register char *n)
{
	register unsigned short hv = tolower(*n++);

	while (*n)
		hv = hvalues[hv] + tolower(*n++);

	return hvalues[hv];
}

void del_host_hash(char *host)
{
	unsigned short hashv = hash_name(host);
	Client *ncl = clhashtable[hashv];
	Client *cp;

	if(!ncl) /* client not in bucket.. */
		return;

	for(cp = ncl; cp; cp=cp->hnext)
	{
		if(!(strcasecmp(host, cp->host)))
			break;
	}

	if(!cp) /* other clients in bucket, but not "host".. */
		return;

	if(cp->hprev) /* not at top of bucket */
		cp->hprev->hnext = cp->hnext;
	else
		if((clhashtable[hashv] = cp->hnext))
			clhashtable[hashv]->hprev = NULL;

	if(cp->hnext)
		cp->hnext->hprev = cp->hprev;

	if(cp->prev) /* in the middle of the linked list .. */
		cp->prev->next = cp->next;
	else
		if((headclient = cp->next))
			headclient->prev = NULL;

	if(cp->next)
		cp->next->prev = cp->prev;

	/* ERROR! ERROR! */
	/* I left this running for two days and I get 15mb of memory usage.. */
	/* oops, forgot a call to free() :) */

	cstructs--;

	free(cp);
}


Client *find_host_hash(char *host)
{
	unsigned short hashv = hash_name(host);
	Client *cp = clhashtable[hashv];

	if(cp)
	{
		for(; cp; cp=cp->hnext)
		{
			if(!(strcasecmp(host, cp->host)))
				return cp;
		}
	}

	return NULL;
}

Client *add_host_hash(char *host)
{
	unsigned short hashv = hash_name(host);
	Client *ncl = clhashtable[hashv];
	Client *cp;

	if(ncl)
	{
		for(cp = ncl; cp; cp=cp->hnext)
		{
			if(!(strcasecmp(host, cp->host)))
				return cp;
		}
	}


	cp = (Client *) malloc(sizeof(Client));
	memset(cp, '\0', sizeof(Client));
	strcpy(cp->host, host);

  	if(ncl) 
		ncl->hprev = cp;
	cp->hnext = ncl;
	cp->hprev = NULL;
	clhashtable[hashv] = cp;

	cp->prev = NULL;
	cp->next = headclient;
	headclient = cp;

	if(cp->next)
		cp->next->prev = cp;
	
	cstructs++;

	return cp;
}


void munge_rn(char *x)
{
	char *y;

	y = strchr(x, '\n');
	if(y) *y = '\0';
	y = strchr(x, '\r');
	if(y) *y = '\0';
}


void load_ignore()
{
	FILE *fp;
	char buf[256];
	Client *cp;
	char *u, *val;

	fp = fopen(IGNOREFILE, "r");
	if(!fp) return;

	while(fgets(buf, 256, fp))
	{
		munge_rn(buf);
		u = buf;
		val = index(buf, ' ');
		if(val)
			*val = '\0';
		
		if(*u == '#' || *u == '\0') 
          continue;
                
        if(strchr(u,'*')||strchr(u,'?'))
          {
            
            if (igmaskcount>=IGMASKSIZE)
              opmlog(L_ERROR,"Maximum host masks (%i) reached!!",igmaskcount);
            else
              igmasklist[igmaskcount++] = strdup(u);
          }
        else 
          {  
		    cp = add_host_hash(u);
          }
	}

	fclose(fp);
}




/*
  is_ignore_mask
  returns:
    -1: host should be ignored for check
     0: host should be checked 
*/  
int is_ignore_mask(char *host)
{
  int count=0;
  while(count<igmaskcount)
    {
      if(match(igmasklist[count++], host))
        return -1;
    }   
  return 0;
}


/* ignore_list_size
	returns the lenght (bytes) of memory used by ignore mask strings
*/
long ignore_list_size(void)
  {
	int i;
	long tot=0;
	
	for(i=0;i<igmaskcount;++i)
	  tot += strlen(igmasklist[i])+1;
	  
	return tot;
  }

/* del_ignore_mask
   returns:
      0: mask does not exist
	 -1: mask deleted
*/
int del_ignore_mask(char *mask)
  {
	int i=0;
	
    while(i<igmaskcount)
  	  {
    	if(strcmp(igmasklist[i], mask)==0)
      		break;
		++i;
  	  }
	  
	if(i<igmaskcount)
	  {
		free(igmasklist[i]);
		while(i<igmaskcount-1)
		  {
			igmasklist[i] = igmasklist[i+1];
			++i;
		  }
		--igmaskcount;
		return -1;
	  }
	else 
	  return 0;
  }
  
/* 
  add_ignore_mask
  returns:
    0: mask added
   -1: mask already existed
   -2: ignore list is full
*/
int add_ignore_mask(char *mask)
 {
	if(is_ignore_mask(mask))
	  return -1;
	if(igmaskcount>=IGMASKSIZE)
	  return -2;
	igmasklist[igmaskcount++] = strdup(mask);
	return 0;	  
 }
 
/*
  ignore_save
  returns:
	0 : sucessfully saved
   -1 : error
*/
int ignore_save(void)	
  {
	FILE *fp;
	Client *cp;
	int hashv;
	int i;
	
	fp = fopen(IGNOREFILE,"wt");
	fprintf(fp,"# PTlink Open Proxy Monitor ignore file\n");
	fprintf(fp,"# created with !IGSAVE\n\n");
	if(!fp)
	  {
		opmlog(L_ERROR,"Could not open "IGNOREFILE" .");	
		return -1;
	  }
	  
	for(i=0;i<igmaskcount;++i)
	  fprintf(fp,"%s\n", igmasklist[i]);

  
	for(hashv=0; hashv<HASHSIZE; ++hashv)
	  {
		cp = clhashtable[hashv];
		for(; cp; cp=cp->hnext)
		  {
			fprintf(fp,"%s\n", cp->host);
		  }
	  }
	
	fclose(fp);
	return 0;
  }
/*
 ignore_mask_list
 desc: send the ignore mask list to target
*/
void ignore_mask_list(char *to)
  {
	int i;
	
	if(igmaskcount>0)
	  send_msg(PT_Nick, to, "Listing %i host mask(s) [max=%i]",
		igmaskcount, IGMASKSIZE);
	else
	  send_msg(PT_Nick, to, "Ignore mask list is empty");
		
	for(i=0;i<igmaskcount;++i)
	  send_msg(PT_Nick, to, "Mask(%i): %s",
		i, igmasklist[i]); 
  }

void list_host_hash(char* to) 
  {
	Client *cp;
	int hashv;
	int i=0;
	for(hashv=0; hashv<HASHSIZE; ++hashv)
	  {
		cp = clhashtable[hashv];
		for(; cp; cp=cp->hnext)
		  {		  
			send_msg(PT_Nick, to, "Host(%i): %s",
			  i, cp->host);
			++i;
		  }
	  }
  }

void kill_hash()
{
	Client *ncl = headclient;
	Client *nncl;

	while(ncl)
	{
		nncl = ncl->next;

		free(ncl);

		ncl = nncl;
	}

    while(igmaskcount)
      free(igmasklist[--igmaskcount]);
      
	headclient = NULL;
	cstructs = 0;
	bzero((char *)&clhashtable, sizeof(int) * HASHSIZE);
    
}

int igmask_count(void)
  {
	return igmaskcount;
  }
