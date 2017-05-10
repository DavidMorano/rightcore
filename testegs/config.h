/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testentropy "

#define	VARPROGRAMROOT1	"TESTENTROPY_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testentropy"

#define	CONFIGFILE1	"etc/testentropy/testentropy.conf"
#define	CONFIGFILE2	"etc/testentropy/conf"
#define	CONFIGFILE3	"etc/testentropy.conf"

#define	LOGFNAME	"log/testentropy"
#define	HELPFNAME	"lib/testentropy/help"
#define	LOCKDIR		"/tmp/locks/testentropy/"
#define	PIDDIR		"spool/run/testentropy/"

#define	MAILDNAME		"/var/spool/mail"
#define	BANNER		"Test Alarm"

#define	LOCKTIMEOUT	(5 * 60)	/* lockfile timeout */

#define	VARDEBUGFD1	"TESTENTROPY_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"



