/* config */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)daytime "
#define	BANNER		"Daytime"
#define	SEARCHNAME	"daytime"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"DAYTIME_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"DAYTIME_BANNER"
#define	VARSEARCHNAME	"DAYTIME_NAME"
#define	VAROPTS		"DAYTIME_OPTS"
#define	VARAFNAME	"DAYTIME_AF"
#define	VAREFNAME	"DAYTIME_EF"
#define	VARERRORFNAME	"DAYTIME_ERRORFILE"

#define	VARDEBUGFNAME	"DAYTIME_DEBUGFILE"
#define	VARDEBUGFD1	"DAYTIME_DEBUGFD"
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

#define	LOCKDNAME	"/tmp/locks/daytime/"
#define	PIDDNAME	"spool/run/daytime/"
#define	MAILDNAME	"/var/spool/mail"
#define	LOGCNAME	"log"

#define	CONFIGFILE1	"etc/daytime/daytime.conf"
#define	CONFIGFILE2	"etc/daytime/conf"
#define	CONFIGFILE3	"etc/daytime.conf"

#define	HELPFNAME	"help"
#define	LOGFNAME	"log/daytime"

#ifndef	ORGANIZATION
#define	ORGANIZATION	"RC"
#endif

#ifndef	INETSVC_DAYTIME
#define	INETSVC_DAYTIME	"daytime"
#endif

#ifndef	ORGLEN
#define	ORGLEN		MAXNAMELEN
#endif

#define	LOGSIZE		(80*1024)

#define	DEF_OFFSET	(5*60)		/* default offset 5 minutes */
#define	DEF_TIMEOUT	(8 * 60)	/* default screen blanking timeout */
#define	DEF_REFRESH	(1 * 60)	/* automatic refresh interval */
#define	DEF_MAILTIME	(1 * 60)	/* active mail display time */

#define	TO_LOCK		(5 * 60)	/* lockfile timeout */
#define	TO_READ		10


