/* config */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	P_TELNET	1

#define	VERSION		"0"
#define	WHATINFO	"@(#)telnetd "
#define	BANNER		"Telnet Daemon"
#define	SEARCHNAME	"telnetd"
#define	VARPRNAME	"EXTRA"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/extra"
#endif

#define	VARPROGRAMROOT1	"TELNETD_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TELNETD_BANNER"
#define	VARSEARCHNAME	"TELNETD_NAME"
#define	VAROPTS		"TELNETD_OPTS"
#define	VARFILEROOT	"TELNETD_FILEROOT"
#define	VARLOGTAB	"TELNETD_LOGTAB"
#define	VARMSFNAME	"TELNETD_MSFILE"
#define	VARUTFNAME	"TELNETD_UTFILE"
#define	VARAFNAME	"TELNETD_AF"
#define	VAREFNAME	"TELNETD_EF"
#define	VARERRORFNAME	"TELNETD_ERRORFILE"

#define	VARDEBUGFNAME	"TELNETD_DEBUGFILE"
#define	VARDEBUGFD1	"TELNETD_DEBUGFD"
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
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/telnetd"
#define	LOGFNAME	"log/telnetd"
#define	LOCKFNAME	"spool/locks/telnetd"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2
#define	TO_LOADAVG	1


