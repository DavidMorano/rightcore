/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)sysvar "
#define	BANNER		"System Variables"
#define	SEARCHNAME	"sysvar"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"SYSVAR_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SYSVAR_BANNER"
#define	VARSEARCHNAME	"SYSVAR_NAME"
#define	VAROPTS		"SYSVAR_OPTS"
#define	VARDBNAME	"SYSVAR_DB"
#define	VARFILEROOT	"SYSVAR_FILEROOT"
#define	VARLOGTAB	"SYSVAR_LOGTAB"
#define	VARMSFNAME	"SYSVAR_MSFILE"
#define	VARAFNAME	"SYSVAR_AF"
#define	VAREFNAME	"SYSVAR_EF"
#define	VAROFNAME	"SYSVAR_OF"
#define	VARIFNAME	"SYSVAR_IF"

#define	VARDEBUGFNAME	"SYSVAR_DEBUGFILE"
#define	VARDEBUGFD1	"SYSVAR_DEBUGFD"
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

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	DEVDNAME	"/dev"
#define	DBDNAME		"/var/tmp"
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/sysvar"		/* mutex PID file */
#define	LOGFNAME	"var/log/sysvar"	/* activity log */
#define	LOCKFNAME	"spool/locks/sysvar"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	TO_OPEN		5
#define	TO_READ		5

#define	DBNAME		"sysvars"

#define	LOGSIZE		(80*1024)

#define	KEYBUFLEN	100
#define	DEFVARS		1000


