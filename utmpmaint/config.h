/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)utmpmaint "
#define	BANNER		"Maintenance UTMP"
#define	SEARCHNAME	"utmpmaint"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"UTMPMAINT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"UTMPMAINT_BANNER"
#define	VARSEARCHNAME	"UTMPMAINT_NAME"
#define	VAROPTS		"UTMPMAINT_OPTS"
#define	VARBASEDNAME	"UTMPMAINT_BASEDIR"
#define	VARQS		"UTMPMAINT_QS"
#define	VARDB		"UTMPMAINT_DB"
#define	VARDBFNAME	"UTMPMAINT_DBFILE"
#define	VARAFNAME	"UTMPMAINT_AF"
#define	VAREFNAME	"UTMPMAINT_EF"
#define	VARERRORFNAME	"UTMPMAINT_ERRORFILE"

#define	VARDEBUGFNAME	"UTMPMAINT_DEBUGFILE"
#define	VARDEBUGFD1	"UTMPMAINT_DEBUGFD"
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

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/utmpmaint"		/* mutex PID file */
#define	LOGFNAME	"var/log/utmpmaint"	/* activity log */
#define	LOCKFNAME	"spool/locks/utmpmaint"	/* lock mutex file */
#define	UTMPXFNAME	"/var/adm/utmpx"
#define	WTMPXFNAME	"/var/adm/wtmpx"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#define	PO_OPTION	"option"

#define	OPT_HDR		TRUE


