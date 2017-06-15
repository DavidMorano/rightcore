/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)linefold "
#define	BANNER		"Line Fold"
#define	SEARCHNAME	"linefold"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"LINEFOLD_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"LINEFOLD_BANNER"
#define	VARSEARCHNAME	"LINEFOLD_NAME"
#define	VAROPTS		"LINEFOLD_OPTS"
#define	VARFILEROOT	"LINEFOLD_FILEROOT"
#define	VARLOGTAB	"LINEFOLD_LOGTAB"
#define	VARAFNAME	"LINEFOLD_AF"
#define	VAREFNAME	"LINEFOLD_EF"
#define	VARERRORFNAME	"LINEFOLD_ERRORFILE"

#define	VARDEBUGFNAME	"LINEFOLD_DEBUGFILE"
#define	VARDEBUGFD1	"LINEFOLD_DEBUGFD"
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
#define	VARLPDEST	"LPDEST"
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARINDENT	"INDENT"
#define	VARNAME		"NAME"
#define	VARFULLNAME	"FULLNAME"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

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
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/linefold"		/* mutex PID file */
#define	LOGFNAME	"var/log/linefold"	/* activity log */
#define	LOCKFNAME	"spool/locks/linefold"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#ifndef	MAILMSGLINELEN
#define	MAILMSGLINELEN	72
#endif

#define	DEFINDENT	2

#define	OPT_LOGPROG	TRUE


