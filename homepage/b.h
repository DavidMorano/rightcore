/* config */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)homepage "
#define	BANNER		"HomePage"
#define	SEARCHNAME	"homepage"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"HOMEPAGE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"HOMEPAGE_BANNER"
#define	VARSEARCHNAME	"HOMEPAGE_NAME"
#define	VAROPTS		"HOMEPAGE_OPTS"
#define	VARFILEROOT	"HOMEPAGE_FILEROOT"
#define	VARLOGTAB	"HOMEPAGE_LOGTAB"
#define	VARWORKDNAME	"HOMEPAGE_WORKDIR"
#define	VARBASEDNAME	"HOMEPAGE_BASEDIR"
#define	VARQS		"HOMEPAGE_QS"
#define	VARDEBUGLEVEL	"HOMEPAGE_DEBUGLEVEL"
#define	VARPIDFNAME	"HOMEPAGE_PIDFILE"
#define	VARREQFNAME	"HOMEPAGE_REQFILE"
#define	VARAFNAME	"HOMEPAGE_AF"
#define	VARCFNAME	"HOMEPAGE_CF"
#define	VARLFNAME	"HOMEPAGE_LF"
#define	VARWFNAME	"HOMEPAGE_WF"		/* log-welcome file */
#define	VAREFNAME	"HOMEPAGE_EF"
#define	VAROFNAME	"HOMEPAGE_OF"
#define	VARIFNAME	"HOMEPAGE_IF"
#define	VARDEBUGFNAME	"HOMEPAGE_DEBUGFILE"
#define	VARDEBUGFD1	"HOMEPAGE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARDOMAIN	"DOMAIN"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARTERM		"TERM"
#define	VARPRINTER	"PRINTER"
#define	VARLPDEST	"LPDEST"
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARNAME		"NAME"
#define	VARFULLNAME	"FULLNAME"
#define	VARHZ		"HZ"
#define	VARTZ		"TZ"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"
#define	VARQUERYSTRING	"QUERY_STRING"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	ETCCNAME	"etc"
#define	LOGCNAME	"log"
#define	RUNDNAME	"var/run"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"
#define	LWFNAME		"/etc/logwelcome"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	FULLFNAME	".fullname"
#define	HEADFNAME	"head.txt"
#define	SVCFNAME	"svc"

#define	ETCDNAME	"etc/homepage"
#define	PIDFNAME	"run/homepage"		/* mutex PID file */
#define	LOGFNAME	"var/log/homepage"	/* activity log */
#define	LOCKFNAME	"spool/locks/homepage"	/* lock mutex file */

#define	DEFTERMTYPE	"text"
#define	DEFPAGE		"default.htm"

#define	LOGSIZE		(80*1024)

#define	TO_CACHE	(5*60)		/* cache time-out (seconds) */
#define	TO_POLL		270
#define	TO_MARK		(8 * 3600)
#define	TO_SPEED	(24 * 3600)
#define	TO_LOCK		60		/* interval lock-check */
#define	TO_CONFIG	60		/* configuration change check */
#define	TO_SVCS		60		/* services change check */
#define	TO_IDLE		5
#define	TO_WAIT		(1*60)		/* interval wait-check */
#define	TO_LOGFLUSH	30		/* interval log-flush */
#define	TO_TMPFILES	(1*3600)

#define	OPT_LOGPROG	TRUE		/* (program) logging */
#define	OPT_CACHE	1		/* use cache */

#define	COPYRIGHT	"2015-%Y (c) David A­D­ Morano. All rights reserved."
#define	WEBMASTER	"webmaster@rightcore.com"


