/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testcpuspeed "
#define	SEARCHNAME	"testcpuspeed"
#define	BANNER		"Test CPUspeed"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"TESTCPUSPEED_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTCPUSPEED_BANNER"
#define	VARSEARCHNAME	"TESTCPUSPEED_NAME"
#define	VAROPTS		"TESTCPUSPEED_OPTS"
#define	VARFILEROOT	"TESTCPUSPEED_FILEROOT"
#define	VARLOGTAB	"TESTCPUSPEED_LOGTAB"
#define	VARAFNAME	"TESTCPUSPEED_AF"
#define	VAREFNAME	"TESTCPUSPEED_EF"

#define	VARDEBUGFD1	"TESTCPUSPEED_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/testcpuspeed"
#define	LOGFNAME	"var/log/testcpuspeed"
#define	LOCKFNAME	"spool/locks/testcpuspeed"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */



