/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0c"
#define	WHATINFO	"@(#)numcvt "
#define	BANNER		"Number Convert"
#define	SEARCHNAME	"numcvt"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"NUMCVT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"NUMCVT_BANNER"
#define	VARSEARCHNAME	"NUMCVT_NAME"
#define	VAROPTS		"NUMCVT_OPTS"
#define	VARFILEROOT	"NUMCVT_FILEROOT"
#define	VARLOGTAB	"NUMCVT_LOGTAB"
#define	VARAFNAME	"NUMCVT_AF"
#define	VARLFNAME	"NUMCVT_LF"
#define	VAREFNAME	"NUMCVT_EF"
#define	VAROFNAME	"NUMCVT_OF"
#define	VARIFNAME	"NUMCVT_IF"

#define	VARDEBUGFNAME	"NUMCVT_DEBUGFILE"
#define	VARDEBUGFD1	"NUMCVT_DEBUGFD"
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

#define	PIDFNAME	"run/numcvt"		/* mutex PID file */
#define	LOGFNAME	"var/log/numcvt"	/* activity log */
#define	LOCKFNAME	"spool/locks/numcvt"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFBASEI	10
#define	DEFBASEO	16

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	PO_OPTION	"option"
#define	PO_SUFFIX	"suffix"


