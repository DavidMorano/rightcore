/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testmsgq "

#define	VARPROGRAMROOT1	"TESTMSGQ_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testmsgq"

#define	CONFIGFILE1	"etc/testmsgq/testmsgq.conf"
#define	CONFIGFILE2	"etc/testmsgq/conf"
#define	CONFIGFILE3	"etc/testmsgq.conf"

#define	LOGFNAME	"log/testmsgq"
#define	HELPFNAME	"lib/testmsgq/help"
#define	LOCKDIR		"/tmp/locks/testmsgq/"
#define	PIDDIR		"spool/run/testmsgq/"

#define	MAILDNAME		"/var/spool/mail"
#define	BANNER		"Test Message Queue"

#define	LOCKTIMEOUT	(5 * 60)	/* lockfile timeout */

#define	ERRORFDVAR	"ERROR_FD"



