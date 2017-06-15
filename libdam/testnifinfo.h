/* config (testnifinfo) */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testnifinfo "
#define	BANNER		"Test NifInfo"
#define	SEARCHNAME	"testnifinfo"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"TESTNIFINFO_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTNIFINFO_BANNER"
#define	VARSEARCHNAME	"TESTNIFINFO_NAME"
#define	VAROPTS		"TESTNIFINFO_OPTS"
#define	VARFILEROOT	"TESTNIFINFO_FILEROOT"
#define	VARLOGTAB	"TESTNIFINFO_LOGTAB"
#define	VARNETRC	"TESTNIFINFO_NETRC"
#define	VARAFNAME	"TESTNIFINFO_AF"
#define	VAREFNAME	"TESTNIFINFO_EF"
#define	VARERRORFNAME	"TESTNIFINFO_ERRORFILE"

#define	VARDEBUGFNAME	"TESTNIFINFO_DEBUGFILE"
#define	VARDEBUGFD1	"TESTNIFINFO_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARTERM		"TERM"
#define	VARPRINTER	"PRINTER"
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	HOMEDNAME	"/home"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	FULLFNAME	".fullname"
#define	AUTHFNAME	".auth"
#define	NETRCFNAME	".netrc"

#define	PIDFNAME	"run/testnifinfo"		/* mutex PID file */
#define	LOGFNAME	"var/log/testnifinfo"	/* activity log */
#define	LOCKFNAME	"spool/locks/testnifinfo"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */


