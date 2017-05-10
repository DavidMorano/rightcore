/* config */


#define	VERSION		"b"
#define	WHATINFO	"@(#)testgetnodedomain "
#define	BANNER		"Test STRTAB"

#define	VARPROGRAMROOT1	"TESTGETNODEDOMAIN_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTGETNODEDOMAIN_BANNER"
#define	VARSEARCHNAME	"TESTGETNODEDOMAIN_NAME"
#define	VARFILEROOT	"TESTGETNODEDOMAIN_FILEROOT"
#define	VARLOGTAB	"TESTGETNODEDOMAIN_LOGTAB"
#define	VARERRORFNAME	"TESTGETNODEDOMAIN_ERRORFILE"

#define	VARDEBUGFNAME	"TESTGETNODEDOMAIN_DEBUGFILE"
#define	VARDEBUGFD1	"TESTGETNODEDOMAIN_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARTMPDNAME	"TMPDIR"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testgetnodedomain"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/testgetnodedomain"
#define	LOGFNAME	"var/log/testgetnodedomain"
#define	LOCKFNAME	"spool/locks/testgetnodedomain"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */



