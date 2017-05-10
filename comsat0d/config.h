/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0d"
#define	WHATINFO	"@(#)comsat "
#define	SEARCHNAME	"comsat"
#define	BANNER		"Comsat"
#define	VARPRNAME	"EXTRA"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/extra"
#endif

#define	VARPROGRAMROOT1	"COMSAT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"COMSAT_BANNER"
#define	VARSEARCHNAME	"COMSAT_NAME"
#define	VAROPTS		"COMSAT_OPTS"
#define	VARMAILDNAMESP	"COMSAT_MAILDIRS"
#define	VARHOSTSPEC	"COMSAT_HOSTSPEC"
#define	VARPORTSPEC	"COMSAT_PORTSPEC"
#define	VARFILEROOT	"COMSAT_FILEROOT"
#define	VARLOGFNAME	"COMSAT_LOGFILE"
#define	VARLOGTAB	"COMSAT_LOGTAB"
#define	VARAFNAME	"COMSAT_AF"
#define	VAREFNAME	"COMSAT_EF"
#define	VARLFNAME	"COMSAT_LF"
#define	VARCFNAME	"COMSAT_CF"
#define	VARERRORFNAME	"COMSAT_ERRORFILE"

#define	VARDEBUGFNAME	"COMSAT_DEBUGFILE"
#define	VARDEBUGFD1	"COMSAT_DEBUGFD"
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

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"
#define	VARPREXTRA	"EXTRA"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	MAILDNAME	"/var/mail"
#define	DEVDNAME	"/dev"
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/comsat"		/* mutex PID file */
#define	LOGFNAME	"log/comsat"		/* activity log */
#define	LOCKFNAME	"spool/locks/comsat"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define DEFINTIDLE	(10*60)
#define	DEFINTNOTE	10
#define	DEFINTLOGSIZE	(24 * 3600)
#define	DEFINTLOGCHECK	(1 * 3600)
#define	DEFINTLOGFLUSH	30

#ifndef	ANYLHOST
#define	ANYLHOST	"anyhost"
#endif

#ifndef	LOCALHOST
#define	LOCALHOST	"localhost"
#endif

#ifndef	SVCSPEC_COMSAT
#define	SVCSPEC_COMSAT	"biff"
#endif

#ifndef	IPPORT_BIFFUDP
#define	IPPORT_BIFFUDP	512
#endif

#define	PO_MAILDIRS	"maildirs"

#define	TO_POLL		10			/* 10 seconds */
#define	TO_READ		5

#define	NOTESMAX	3


