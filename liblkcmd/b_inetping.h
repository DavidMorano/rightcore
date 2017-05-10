/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0c"
#define	WHATINFO	"@(#)inetping "
#define	SEARCHNAME	"inetping"
#define	BANNER		"Internet Ping"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"INETPING_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"INETPING_BANNER"
#define	VARSEARCHNAME	"INETPING_NAME"
#define	VAROPTS		"INETPING_OPTS"
#define	VARAFNAME	"INETPING_AF"
#define	VAREFNAME	"INETPING_EF"
#define	VARERRORFNAME	"INETPING_ERRORFILE"

#define	VARDEBUGFNAME	"INETPING_DEBUGFILE"
#define	VARDEBUGFD1	"INETPING_DEBUGFD"
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

#define	CONFIGFILE1	"etc/inetping/inetping.conf"
#define	CONFIGFILE2	"etc/inetping/conf"
#define	CONFIGFILE3	"etc/inetping.conf"

#define	LOGFNAME	"log/inetping"
#define	HELPFNAME	"lib/inetping/help"

#define	INETPING_BANNER	"Inet Ping"

#define	INETPING_TIMEOUT	20

#define	PO_OPTION	"option"

#define	EX_FORWARDED	1		/* mail is being forwarded */
#define	EX_ACCESS	2		/* could not acccess mail */
#define	EX_NOSPACE	3		/* no space on user's filesystem */
#define	EX_INVALID	4		/* something invalid */
#define	EX_LOCKED	5		/* user's mail is locked */
#define	EX_NETUNREACH	6		/* network unreachable */
#define	EX_HOSTUNREACH	7		/* network unreachable */
#define	EX_HOSTDOWN	8		/* host is down */


