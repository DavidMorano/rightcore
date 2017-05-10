/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testcfnum "

#define	VARPROGRAMROOT1	"TESTCFNUM_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testcfnum"

#define	CONFIGFILE1	"etc/testcfnum/testcfnum.conf"
#define	CONFIGFILE2	"etc/testcfnum/conf"
#define	CONFIGFILE3	"etc/testcfnum.conf"

#define	LOGFNAME	"log/testcfnum"
#define	HELPFNAME	"lib/testcfnum/help"
#define	LOCKDIR		"/tmp/locks/testcfnum/"
#define	PIDDIR		"spool/run/testcfnum/"

#define	MAILDNAME		"/var/spool/mail"
#define	BANNER		"Test CFNUM"

#define	LOCKTIMEOUT	(5 * 60)	/* lockfile timeout */

#define	ERRORFDVAR	"ERROR_FD"



