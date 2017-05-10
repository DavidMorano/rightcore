/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testalarm "

#define	VARPROGRAMROOT1	"TESTALARM_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testalarm"

#define	CONFIGFILE1	"etc/testalarm/testalarm.conf"
#define	CONFIGFILE2	"etc/testalarm/conf"
#define	CONFIGFILE3	"etc/testalarm.conf"

#define	LOGFNAME	"log/testalarm"
#define	HELPFNAME	"lib/testalarm/help"
#define	LOCKDIR		"/tmp/locks/testalarm/"
#define	PIDDIR		"spool/run/testalarm/"

#define	MAILDNAME		"/var/spool/mail"
#define	BANNER		"Test Alarm"

#define	LOCKTIMEOUT	(5 * 60)	/* lockfile timeout */

#define	ERRORFDVAR	"ERROR_FD"



