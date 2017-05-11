/* config (testpcsns) */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testpcsns "
#define	BANNER		"Test PCS Name-Server"
#define	SEARCHNAME	"testpcsns"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"TESTPCSNS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTPCSNS_BANNER"
#define	VARSEARCHNAME	"TESTPCSNS_NAME"
#define	VAROPTS		"TESTPCSNS_OPTS"
#define	VARFILEROOT	"TESTPCSNS_FILEROOT"
#define	VARLOGTAB	"TESTPCSNS_LOGTAB"
#define	VARNETRC	"TESTPCSNS_NETRC"
#define	VARAFNAME	"TESTPCSNS_AF"
#define	VAREFNAME	"TESTPCSNS_EF"
#define	VARERRORFNAME	"TESTPCSNS_ERRORFILE"

#define	VARDEBUGFNAME	"TESTPCSNS_DEBUGFILE"
#define	VARDEBUGFD1	"TESTPCSNS_DEBUGFD"
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

#define	PIDFNAME	"run/testpcsns"		/* mutex PID file */
#define	LOGFNAME	"var/log/testpcsns"	/* activity log */
#define	LOCKFNAME	"spool/locks/testpcsns"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */


