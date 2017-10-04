/* config */


/* revision history:

	= 2000-03-02, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)webcounter "
#define	BANNER		"Web Counter"
#define	SEARCHNAME	"webcounter"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"WEBCOUNTER_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"WEBCOUNTER_BANNER"
#define	VARSEARCHNAME	"WEBCOUNTER_NAME"
#define	VAROPTS		"WEBCOUNTER_OPTS"
#define	VARBASEDNAME	"WEBCOUNTER_BASEDIR"
#define	VARQS		"WEBCOUNTER_QS"
#define	VARDB		"WEBCOUNTER_DB"
#define	VARDBFNAME	"WEBCOUNTER_DBFILE"
#define	VARCFNAME	"WEBCOUNTER_CF"
#define	VARLFNAME	"WEBCOUNTER_LF"
#define	VARAFNAME	"WEBCOUNTER_AF"
#define	VAREFNAME	"WEBCOUNTER_EF"

#define	VARDEBUGFNAME	"WEBCOUNTER_DEBUGFILE"
#define	VARDEBUGFD1	"WEBCOUNTER_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARHZ		"HZ"
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
#define	VARTZ		"TZ"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"
#define	VARPATH		"PATH"
#define	VARMANPATH	"MANPATH"
#define	VARCDPATH	"CDPATH"
#define	VARLIBPATH	"LD_LIBRARY_PATH"
#define	VARQUERYSTRING	"QUERY_STRING"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	LOGCNAME	"log"
#define	VDNAME		"var"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/webcounter"		/* mutex PID file */
#define	LOGFNAME	"var/log/webcounter"		/* activity log */
#define	LOCKFNAME	"spool/locks/webcounter"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	OPT_LOGPROG	TRUE


