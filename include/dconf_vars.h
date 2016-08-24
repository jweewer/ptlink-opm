/*****************************************************************
 * PTlink Services is (C) CopyRight PTlink Coders Team 1999-2004 *
 * http://www.ptlink.net/Coders - coders@PTlink.net              *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: conf_vars.h
  Desc: dconf variables
  Author: Lamego@PTlink.net
*/
#ifndef include_conf_vars_h_

#define include_conf_vars_h_

#define E extern


/* Connection Configuration */
E char *LocalAddress;
E char *IRCServer;
E int	IRCPort;
E char *IRCPass;
E char *AltIRCServer;
E char *ServerName;
E char *ServerDesc;
E char *LogChan;
E int   RetryDelay;

/* Scan Configuration */
E int       NetJoinScan;
E void      ScanRule(char *line);
E char*     ScanNotice;
E int       MaxScans;
E int       ScanTimeOut;
E char*     DestIP;
E int       DestPort;
E int       GLineTime;
E char*     GLineReason;
E char*	    ReportEmail;
E char*     DNSBLZones;
E char*     DNSBLZoneMsg;

/* PT */
char *PT_Nick;
char *PT_Mask;
char *PT_Info;
#undef E

#endif /* conf_vars_h_ */
