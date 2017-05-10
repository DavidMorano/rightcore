/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testnetfile "
#define	BANNER		"Test NetFile"
#define	SEARCHNAME	"testnetfile"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"TESTNETFILE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTNETFILE_BANNER"
#define	VARSEARCHNAME	"TESTNETFILE_NAME"
#define	VAROPTS		"TESTNETFILE_OPTS"
#define	VARFILEROOT	"TESTNETFILE_FILEROOT"
#define	VARLOGTAB	"TESTNETFILE_LOGTAB"
#define	VARNETRC	"TESTNETFILE_NETRC"
#define	VARAFNAME	"TESTNETFILE_AF"
#define	VAREFNAME	"TESTNETFILE_EF"
#define	VARERRORFNAME	"TESTNETFILE_ERRORFILE"

#define	VARDEBUGFNAME	"TESTNETFILE_DEBUGFILE"
#define	VARDEBUGFD1	"TESTNETFILE_DEBUGFD"
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

#define	PIDFNAME	"run/testnetfile"		/* mutex PID file */
#define	LOGFNAME	"var/log/testnetfile"	/* activity log */
#define	LOCKFNAME	"spool/locks/testnetfile"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */


