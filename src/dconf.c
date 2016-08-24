/*****************************************************************
 * PTlink IRCd is (C) CopyRight PTlink Coders Team 1999-2004     *
 * http://www.ptlink.net/Coders - coders@PTlink.net              *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: dconf.c
  Desc: general .dconf parsing 
  Author: Lamego@PTlink.net
*/

#include <stdinc.h>

#include "s_log.h"
#include "irc_string.h"
#include "dconf.h"
#include "dconf_vars.h"


/* Connection Configuration  */
char*   LocalAddress = NULL;
char*   IRCServer = NULL;
int	    IRCPort = 0;
char*   IRCPass = NULL;
char*   ServerName = NULL;
char*   ServerDesc = NULL;
char*   LogChan = NULL;
int     RetryDelay = 0;
char*   AltIRCServer = NULL;

/* Scan Configuration */
int	    NetJoinScan = 0;
char*   ScanNotice = NULL;
char*   DestIP = NULL;
int	    DestPort = 0;
int	    GLineTime = 24*60;
char*   GLineReason = NULL;
char*   ReportEmail = NULL;
int     MaxScans = 0;
int     ScanTimeOut = 60;
char*   DNSBLZones = NULL;
char*	DNSBLZoneMsg = NULL;

/* SC */
char*   PT_Nick = NULL;
char*   PT_Mask = NULL;
char*   PT_Info = NULL;

ConfItem conf_items[] = {
	/* Connection Configuration */
	DCONF_DESC   ( "Connection Configuration" ),
	DCONF_STRING ( LocalAddress, 0 ),
	DCONF_STRING ( IRCServer, CF_REQ | CF_LOCKED ),
	DCONF_INT    ( IRCPort, CF_REQ | CF_LOCKED ),
	DCONF_STRING ( IRCPass, CF_REQ | CF_LOCKED ),	
        { "AltIRCServer", CT_STARRAY, &AltIRCServer, 0 },
        DCONF_STRING ( ServerName, CF_REQ | CF_LOCKED ),
        DCONF_STRING ( ServerDesc, CF_REQ | CF_LOCKED ),
        DCONF_INT    ( RetryDelay, 0 ),
        DCONF_STRING ( LogChan, CF_REQ | CF_LOCKED ),
	/* Scan Configuration */                
        DCONF_DESC   ( "Scan Configuration" ) ,
        DCONF_FUNC   ( ScanRule ),
        DCONF_SET    ( NetJoinScan, 0 ),
        DCONF_STRING ( ScanNotice, 0 ),
        DCONF_STRING ( DestIP, CF_REQ ),
        DCONF_INT    ( DestPort, CF_REQ ),
        DCONF_INT    ( MaxScans, CF_REQ ),
        DCONF_TIME   ( ScanTimeOut, CF_REQ ),
        DCONF_TIME   ( GLineTime, CF_REQ ),
        DCONF_STRING ( GLineReason, CF_REQ ),
        DCONF_STRING ( ReportEmail, 0 ),
        DCONF_STRING ( DNSBLZones, 0 ),
        DCONF_STRING ( DNSBLZoneMsg, 0 ), 
        /* OPM User Configuration */
	DCONF_DESC   ( "OPM User Configuration" ),
	DCONF_STRING ( PT_Nick, CF_REQ ),
	DCONF_STRING ( PT_Mask, CF_REQ ),	
	DCONF_STRING ( PT_Info, CF_REQ ),	
	{ NULL,	0,	NULL, 0 }
};

int conf_find_item(char *name);
char* conf_item_str(int i);
int  conf_change_item(char *name, char* value, int rehashing);
/*
 * conf_find_item
 *
 * inputs       - item name
 * output       - item index
 * side effects - looks for a specific item name on the conf items list
 */
int conf_find_item(char *name) {
    int i=0;
	
    while (conf_items[i].name && strcasecmp(conf_items[i].name, name))
	  ++i;
	  
    return conf_items[i].name ? i : -1;
}

/*
 * conf_item_str
 *
 * inputs       - item index
 * output       - item value (as string)
 * side effects - returns a string with the item value
 */
char* conf_item_str(int i) {
  static char tmpbuf[512];
  int tmpval;
  
  tmpbuf[0]='0';
  
  if(!conf_items[i].name)
	return tmpbuf;
	
  if(!(conf_items[i].flags & CF_DEFINED))	
	{
	  strcpy(tmpbuf,"Undefined");
	}
  else
	{
    switch(conf_items[i].type) 
	  {	
		case CT_DESC:
		break;		
		  
		case CT_STRING:	
		  ircsprintf(tmpbuf,"\"%s\"",*(char**) conf_items[i].vptr);
	  	break;
		
		case CT_SET:
		if (*(int*)conf_items[i].vptr == -1)
		  ircsprintf(tmpbuf,"Yes");
		else
		  ircsprintf(tmpbuf,"No");
		break;
			
		case CT_TIME:
		  tmpval = *(int*) conf_items[i].vptr;

		  if(tmpval>59)
			ircsprintf(tmpbuf,"%dm",tmpval / 60);
		  else
	  		ircsprintf(tmpbuf,"%ds",tmpval);
		break;
		
		case CT_INT:
		    tmpval = *(int*) conf_items[i].vptr;
	  		ircsprintf(tmpbuf,"%i",tmpval);
		break;
		
		default:
		  strcpy(tmpbuf,"Unknown format");
  	  }	
	  
    }
	
	return tmpbuf;	
}

/*
 * conf_change_item
 *
 * inputs       - item name, new value, rehashing state
 * output       - change success
 * side effects - log any possible problem with the new setting
 */
int  conf_change_item(char *name, char* value, int rehashing) 
  {
    int tmpval;
    char *vitem;	
    int i = conf_find_item(name);

	
    if(i<0)
	  {
		if(!rehashing)
		  opmlog(L_WARN,"Ignoring unknown directive %s", name);
		return (UNKNOWN_ITEM);
	  }
	  	  
	while(IsSpace(*value)) 
		  ++value;

	/* Let's remove the surrounding "" */
	if ((*value) == '\"') 
		*(value++)=' ';	
			  
	if (*value && (value[strlen(value)-1] == '\"') )
		value[strlen(value)-1]='\0';

        if(conf_items[i].type == CT_FUNC) /* we must execute it */
          {
             ((int (*)(char *)) conf_items[i].vptr)(value);
             return 0;
          }
	  
	if ((rehashing<2) && (conf_items[i].flags & CF_REQ) && (value[0]=='\0')) /* cannot undefine */
		  return (LOCKED_ITEM);
	  	  
    if ((conf_items[i].flags & CF_DEFINED) && (conf_items[i].type != CT_STARRAY)) 
	  { /* this value is already defined  */
		if (rehashing) 
		  {
	  		if ((rehashing==1) && (conf_items[i].flags & CF_LOCKED)) /* item is locked */
			  return (LOCKED_ITEM);
	  		else 
			  {
				if (conf_items[i].type==CT_STRING) 
	  		  	  free( *(char**) conf_items[i].vptr);
			  }	    
		  }	
		else 
		  {
	  		opmlog(L_WARN,"Ignoring duplicated directive %s", name);		
	  		return 0;
		  }
  	  }
	
    switch(conf_items[i].type) 
	  {	
		case CT_DESC:
		break;		
		  
		case CT_STRING:	
			
		  /* Let's remove the surrounding "" */
		  if ((*value) == '\"') 
			*(value++)=' ';	
				  
		  if (*value && (value[strlen(value)-1] == '\"') )
			value[strlen(value)-1]='\0';
			
		  if(value[0]=='\0')
			{
	  	  	  *(char **)conf_items[i].vptr = NULL;
			  conf_items[i].flags &= ~CF_DEFINED;
			  return 1;
			}
		  else /* save the new data */
	  		*(char **)conf_items[i].vptr = (value[0]) ? strdup(value) : NULL;
		break;

        case CT_STARRAY:
		  /* Let's remove the surrounding "" */
		  if ((*value) == '\"') 
			*(value++)=' ';	
				  
		  if (*value && (value[strlen(value)-1] == '\"') )
			value[strlen(value)-1]='\0';
			
		  if(*value=='\0')
			{
	  	  	  *(char **)conf_items[i].vptr = NULL;
			  conf_items[i].flags &= ~CF_DEFINED;
			  return 1;
			}
		  else
            { /* save the new data */       
              if(*(char **)conf_items[i].vptr == NULL) /* allocate 4k for strings */
                {
                  vitem = malloc(4096);
                  bzero(vitem, 4096);
                  *(char **)conf_items[i].vptr = vitem;
                }
              else
                vitem = *(char **)conf_items[i].vptr;
                               
              if(strlen(value)+strlen(vitem)>=4096)
                {
                  opmlog(L_WARN,"Ignoring value for %s, out of memory", name);
                  return 0;
                }
              strcat(vitem, value);
              strcat(vitem, "\n"); /* mark end of line */
            }
        break;
		
		case CT_SET:
		  if ( strcasecmp(value,"yes") == 0)
			*(int*) conf_items[i].vptr = -1;
		  else if ( strcasecmp(value,"no") == 0)
			*(int*) conf_items[i].vptr = 0;
		  else 
		  {
		  	if (!rehashing)
              opmlog(L_WARN,"Invalid value (%s) on .conf item %s, please use Yes/No", value, name);
			  
		    return (INV_SET_ITEM);
		  }		  
		break;
			
		case CT_TIME:
	  	  tmpval = time_str(value);
	  	  if (tmpval<=0) 
			{ 
			  if (!rehashing)
		  		opmlog(L_WARN,"Invalid time format on .conf item %s", name);
    		  return (INV_TIME_ITEM);
	  		} 			  
	  	  else *(int*) conf_items[i].vptr = tmpval;
		  conf_items[i].flags |= CF_DEFINED;
		break;
		
		default:
		  *(int*) conf_items[i].vptr = atoi(value);
  	  }
	    	
	conf_items[i].flags |= CF_DEFINED;	
    return 1;
  }


/*
 * conf_read
 *
 * inputs       - .conf filename and rehash mode
 * output       - number of errors during .conf reading
 * side effects - log any possible problems
 */
int dconf_read(char *fn, int rehashing) 
  {
    char line[512];	/* buffer to read .conf line */
    FILE *confile;
    char* directive;
    char* value;
    char* auxptr;
    int	i;
    int errors = 0; /* errors found reading conf */
    int linenum = 0;
		
    confile = fopen(fn,"rt");

    if(!confile) 
	return -1;
	
    if(rehashing)
      opmlog(L_NOTICE,"Reading configuration file %s (during rehash)", fn);
    else
      opmlog(L_NOTICE,"Reading configuration file %s", fn);	  
   
    while (!feof(confile)) 
	  {
	        ++linenum;
	        line[0]='\0';
		fgets(line, sizeof(line), confile);		
		auxptr = strchr(line,'#');
		
		auxptr = line;
		
		while(IsSpace(*auxptr))
			++auxptr;
					
		directive = strtok(auxptr,"\t\n ");
		
		if( !directive || (directive[0] == '#') )
		  continue;
		  
		value = strtok(NULL,"\r\n");

		if (!value) 
		  {
	    	    opmlog(L_WARN,"Missing value for directive %s reading %s, line %d", 
	    	      directive, fn, linenum);
		    continue;
		  }
		
		while(IsSpace(*value)) 
		  ++value;
		
		if ( !strcmp(directive,".include") )
		  {
			if (dconf_read(value, rehashing) == -1)
			  {
				opmlog(L_ERROR,"Error opening include file %s",value);
			  }
			  continue;
		  }
		  		  
		i = conf_change_item(directive, value, rehashing);
		

		if(i<0)
		  ++errors;

	  }
	  
	fclose(confile);
	
	if(errors)
	  {
		opmlog(L_ERROR,"%s had %i errors", fn, errors);
	  }	  	
	
    return errors;	  
  }
/*
 * dconf_check
 *
 * inputs       - verbose if true errors will be printed to stderr
 * output       - 1 if valid conf, 0 otherwise
 * side effects - check if any required item is missing
 */

int dconf_check(int verbose) 
  {
	int i = 0;
	int errors = 0;
	int flags;
	while (conf_items[i].name)
	  {
		flags = conf_items[i].flags;
        
		if ((flags & CF_REQ) && !(flags & CF_DEFINED))
		  {
            if(verbose)
              fprintf(stderr,"Missing required directive: %s\n", conf_items[i].name);
            else
			  opmlog(L_ERROR,"Missing required directive: %s", conf_items[i].name);

			++errors;
		  }
		  ++i;
	  }	  
	  
	if(errors) 
	  {
		opmlog(L_ERROR,"Terminating because of %i required item(s)", errors);
		return 0;
	  }	
	  
	return 1;
  }


#if 0
/*
** m_dconf - general conf handling routine
**      parv[0] = sender prefix
**      parv[1] = command (list/rehash/set)
**      parv[2] = setting item  (or mask for /list)
**      parv[3] = new value (only required for set)
*/
int	m_dconf(int parc, char *parv[])
{
  int errors = 0;
  int res;
  char *cmask = NULL;
  char reply[100];
  int i;
  char *item, *newval;
  
  if ( !IsAdmin(sptr) && !IsService(sptr) ) 
	{	
      sendto_one(sptr, form_str(ERR_NOPRIVILEGES), me.name, parv[0]);
      return 0;
	}

  if (parc < 2 || *parv[1] == '\0')
    {
	  if(MyClient(sptr))
    	  sendto_one(sptr, form_str(ERR_NEEDMOREPARAMS),
                 me.name, parv[0], "DCONF");
      return -1;
    }

	
  if ( !strcasecmp(parv[1],"rehash") )
	{
	
  	  sendto_one(sptr, ":%s NOTICE %s :Rehashing main.conf",
           me.name,parv[0]);		  	
		   
	  if( (errors = dconf_read("main.dconf", 2)) )
		{
      	  sendto_one(sptr, ":%s NOTICE %s :*** (%i) errors found",
           me.name,parv[0], errors);		  
		}
	  else
	    sendto_one(sptr, ":%s NOTICE %s :*** Rehash terminated without errors",
           me.name,parv[0]);
	}
  else if ( !strcasecmp(parv[1],"list") )
	{
	  if (parc>1)
		cmask = parv[2];
		
	
	  sendto_one(sptr, ":%s NOTICE %s :---------- Start of dconf list ----------",
		  me.name,parv[0]);
		  
	  i = 0;

  	  while (conf_items[i].name)
		{
		  if (!cmask || match(cmask, conf_items[i].name))
			{
			if(conf_items[i].type == CT_DESC) 
			  {
		  		if(i)
				  sendto_one(sptr, ":%s NOTICE %s : ",
					me.name,parv[0]);			
		  		sendto_one(sptr, ":%s NOTICE %s :[ %s ]",
				  me.name,parv[0], conf_items[i].name);				  
			  }
			else
			  sendto_one(sptr, ":%s NOTICE %s :%s = %s",
				  me.name,parv[0],
				  conf_items[i].name, conf_item_str(i));
			}
		  ++i;
		}	  
	  
  	  sendto_one(sptr, ":%s NOTICE %s :---------- End of dconf list ----------",		
		  me.name,parv[0]);	  
	}
  else if ( !strcasecmp(parv[1],"set") )
	{
	  if (parc<4) 
		{
		  sendto_one(sptr, ":%s NOTICE %s :Usage DCONF SET <option> <value>",
			me.name, parv[0]);
		  return -2;
		}
		
	  item = parv[2];
	  newval = parv[3];
	  
	  if(strlen(item) > 40)
		item[40]='\0';
  	  if(strlen(newval) > 40)
		newval[40]='\0';
		
	  res = conf_change_item(item, newval, 1);
	  reply[0]='\0';
	  
	  switch(res) 
		{
		  case UNKNOWN_ITEM: 
			ircsprintf(reply,"Unknown item \2%s\2",item);
			break;
		  case LOCKED_ITEM:
			ircsprintf(reply,"Item \2%s\2 cannot be changed", item);
			break;
		  case INV_SET_ITEM: 
			ircsprintf(reply,"Invalid value \2%s\2, please use Yes/No", newval);
		  	break;
		  case INV_TIME_ITEM: 
			ircsprintf(reply,"Invalid time value \2%s\2, please use NNd|NNm|NNs", newval);
		  	break;
		  default:
		    ircsprintf(reply,"%s = \2%s\2", item, newval);		  			
			break;
		}
			  
	  opmlog(L_NOTICE,"DCONF SET from %s : %s", get_client_name(sptr, REAL_IP), reply);
		  
	  if(IsService(sptr))
		sendto_serv_butone(cptr, ":%s DCONF SET %s :%s", parv[0], item, newval);
	  else
		{
		  sendto_one(sptr, ":%s NOTICE %s :%s", me.name, parv[0], reply);				
		}
	}
  else 
	sendto_one(sptr, ":%s NOTICE %s :Usage DCONF < REHASH | LIST [mask] | SET option value] >",
	  me.name, parv[0]);
	  
  return 0;
}

#endif
