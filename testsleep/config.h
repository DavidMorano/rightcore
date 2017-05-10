/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testsleep "

#define	VARPROGRAMROOT1	"TESTSLEEP_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testsleep"

#define	CONFIGFILE1	"etc/testsleep/testsleep.conf"
#define	CONFIGFILE2	"etc/testsleep/conf"
#define	CONFIGFILE3	"etc/testsleep.conf"

#define	LOGFNAME	"log/testsleep"
#define	HELPFNAME	"lib/testsleep/help"
#define	LOCKDIR		"/tmp/locks/testsleep/"
#define	PIDDIR		"spool/run/testsleep/"

#define	MAILDNAME		"/var/spool/mail"
#define	BANNER		"Test Sleep"

#define	LOCKTIMEOUT	(5 * 60)	/* lockfile timeout */

#define	ERRORFDVAR	"FD_DEBUG"



