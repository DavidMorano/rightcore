/* config */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)DNSRES "
#define	BANNER		"DNS Resolver"
#define	SEARCHNAME	"dnsres"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"DNSRES_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"DNSRES_BANNER"
#define	VARSEARCHNAME	"DNSRES_NAME"
#define	VAROPTS		"DNSRES_OPTS"
#define	VARSTRING	"DNSRES_STR"
#define	VARMNTFNAME	"DNSRES_MNTFILE"
#define	VARAFNAME	"DNSRES_AF"
#define	VAREFNAME	"DNSRES_EF"
#define	VARERRORFNAME	"DNSRES_ERRORFILE"

#define	VARDEBUGFNAME	"DNSRES_DEBUGFILE"
#define	VARDEBUGFD1	"DNSRES_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARTERM		"TERM"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"

#define	HELPFNAME	"help"
#define	LOGFNAME	"log/dnsres"
#define	USERFILE	"log/dnsres.users"
#define	CMDHELPFNAME	"lib/dnsres/cmdhelp"

#define	OUTPUTDEV	"/dev/fd/1"
#define	MSGLOGDEV	"/dev/msglog"
#define	CONSOLEDEV	"/dev/console"

#define	TO_OPEN		5
#define	TO_POLL		10
#define	TO_TMPX		3
#define	TO_SYSMISC	3
#define	TO_CHECK	2
#define	TO_WAIT		20

 
