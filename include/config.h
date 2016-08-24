/*****************************************************************
 * PTlink Services is (C) CopyRight PTlink Coders Team 1999-2001 *
 * http://www.ptlink.net/Coders - coders@PTlink.net              *
 * This program is distributed under GNU Public License          *
 * Please read the file COPYING for copyright information.       *
 *****************************************************************
 
  File: config.h
  Desc: config settings
  Author: Lamego@PTlink.net
*/


/* SERVICES_UID SERVICES_GID - user and group services should switch to if run as root
 * If you start the services as root but wish to have it run as another user,
 * define SERVICES_UID to that UID.  This should only be defined if you are running
 * as root and even then perhaps not.
 */
/* #define SERVICES_UID 1001 */  
/* #define SERVICES_GID 31 */

/* CHROOTDIR - chroot() before reading conf  
	* Define for value added security if you are paranoid.
	* All files you access must be in the directory you define as DPATH.
	* (This may effect the PATH locations above, though you can symlink it)
 */	 
#undef CHROOTDIR

#define INBUFFER_SIZE		0xffff	/* maximum data we can read from irc server */
#define PIDFilename		"ptopm.pid"
#define IGNOREFILE      	"ptopm.ignore"

#define MAX_POLL 1024

#define SCANBUFFER 128

/* Max data to read from any port before closing the connection
 * (to avoid flood attempts)
 */
#define MAXREAD 4096

/* intervals betweens scan timeout checks
 * the lower you use, faster the FDs are are freed at CPU usage expense
 */
#define TIMEOUT_CHECK 5

/* Drop the ircd connection if >IRC_READ_TIMEOUT
 *  without getting data from ircd (seconds) 
 */
#define IRC_READ_TIMEOUT 370

/* Defines for how much time the cache for alreasy scanned hosts
 * will be valid. (8 hours)
 */ 
#define CACHE_TIME (8*60*60)


