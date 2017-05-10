/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testprecedence "

#define	VARPROGRAMROOT1	"TESTPRECEDENCE_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testprecedence"

#define	CONFIGFILE1	"etc/testprecedence/testalarm.conf"
#define	CONFIGFILE2	"etc/testprecedence/conf"
#define	CONFIGFILE3	"etc/testprecedence.conf"

#define	LOGFNAME	"log/testprecedence"
#define	HELPFNAME	"lib/testprecedence/help"
#define	LOCKDIR		"/tmp/locks/testprecedence/"
#define	PIDDIR		"spool/run/testprecedence/"

#define	MAILDNAME		"/var/spool/mail"
#define	BANNER		"Test Precedence"

#define	LOCKTIMEOUT	(5 * 60)	/* lockfile timeout */

#define	ERRORFDVAR	"ERROR_FD"



