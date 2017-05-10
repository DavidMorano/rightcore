/* config */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0c"
#define	WHATINFO	"@(#)HOSTADDRINFO "
#define	SEARCHNAME	"hostaddrinfo"
#define	BANNER		"Host Address Information"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"HOSTADDRINFO_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"HOSTADDRINFO_BANNER"
#define	VARSEARCHNAME	"HOSTADDRINFO_NAME"
#define	VAROPTS		"HOSTADDRINFO_OPTS"
#define	VARAFNAME	"HOSTADDRINFO_AF"
#define	VAREFNAME	"HOSTADDRINFO_EF"
#define	VARERRORFNAME	"HOSTADDRINFO_ERRORFILE"

#define	VARDEBUGFNAME	"HOSTADDRINFO_DEBUGFILE"
#define	VARDEBUGFD1	"HOSTADDRINFO_DEBUGFD"
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

#define	CONFIGFNAME	"etc/hostaddrinfo/conf"	/* not currently used */
#define	LOGFNAME	"log/hostaddrinfo"
#define	HELPFNAME	"help"


