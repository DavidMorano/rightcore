/* config */


#define	VERSION		"0b"
#define	WHATINFO	"@(#)term-print "
#define	SEARCHNAME	"term-print"
#define	BANNER		"Terminal Print"

#define	VARPROGRAMROOT1	"TERMPRINT_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARFILEROOT	"TERMPRINT_FILEROOT"
#define	VARLOGTAB	"TERMPRINT_LOGTAB"

#define	VARDEBUGFD1	"TERMPRINT_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/term-print"		/* mutex PID file */
#define	LOGFNAME	"var/log/term-print"	/* activity log */
#define	LOCKFNAME	"spool/locks/term-print"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */



