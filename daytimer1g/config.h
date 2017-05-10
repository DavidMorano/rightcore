/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"1g"
#define	WHATINFO	"@(#)daytimer "
#define	SEARCHNAME	"daytimer"
#define	BANNER		"Login Daytimer"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"DAYTIMER_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"DAYTIMER_BANNER"
#define	VARSEARCHNAME	"DAYTIMER_NAME"
#define	VAROPTS		"DAYTIMER_OPTS"
#define	VARAFNAME	"DAYTIMER_AF"
#define	VAREFNAME	"DAYTIMER_EF"
#define	VARLFNAME	"DAYTIMER_LF"
#define	VARLOGFILE	"DAYTIMER_LOGFILE"
#define	VARERRORFNAME	"DAYTIMER_ERRORFILE"

#define	VARDEBUGFNAME	"DAYTIMER_DEBUGFILE"
#define	VARDEBUGFD1	"DAYTIMER_DEBUGFD"
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

#define	TMPDNAME	"/tmp"
#define	LOCKDNAME	"/tmp/locks/daytimer"
#define	MAILDNAME	"/var/mail"
#define	PIDDNAME	"var/run/daytimer"

#define	LOGCNAME	"log"

#define	CONFIGFILE1	"etc/daytimer/daytimer.conf"
#define	CONFIGFILE2	"etc/daytimer/conf"
#define	CONFIGFILE3	"etc/daytimer.conf"

#define	LOGFNAME	"log/daytimer"
#define	SUMFNAME	"log/daytimer.sum"
#define	HELPFNAME	"help"

#ifndef	LOGSIZE
#define	LOGSIZE		80000
#endif

#define	DEF_REFRESH	(1 * 60)	/* automatic refresh interval */

#define	TO_BLANK	(8 * 60)	/* default screen blanking timeout */
#define	TO_LOCK		(5 * 60)	/* lockfile timeout */

#define	MAILINT		(1 * 60)	/* active mail display time */
#define	POLLINT		7

#define	MAIL_TICS	3


