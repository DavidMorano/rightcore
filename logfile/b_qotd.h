/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"1"
#define	WHATINFO	"@(#)qotd "
#define	BANNER		"Quote-Of-The-Day"
#define	SEARCHNAME	"qotd"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"QOTD_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"QOTD_BANNER"
#define	VARSEARCHNAME	"QOTD_NAME"
#define	VAROPTS		"QOTD_OPTS"
#define	VARFILEROOT	"QOTD_FILEROOT"
#define	VARLOGTAB	"QOTD_LOGTAB"
#define	VARINTRUN	"QOTD_INTRUN"
#define	VARAFNAME	"QOTD_AF"
#define	VAREFNAME	"QOTD_EF"

#define	VARDEBUGFNAME	"QOTD_DEBUGFILE"
#define	VARDEBUGFD1	"QOTD_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"
#define	VARTERM		"TERM"
#define	VARCOLUMNS	"COLUMNS"

#define	VARTMPDNAME	"TMPDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/qotd"		/* mutex PID file */
#define	LOGFNAME	"var/log/qotd"		/* activity log */
#define	LOCKFNAME	"spool/locks/qotd"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	PORTSPEC_QUOTE	"quote"

#define	INT_RUN		(4*60)

#define	TO_OPEN		5
#define	TO_READ		5
#define	TO_RECVMSG	(5*60)

#define	OPT_LOGPROG	TRUE


