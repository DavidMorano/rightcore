/* config */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)SIGDUMP "
#define	BANNER		"Signal Dump Daemon"
#define	SEARCHNAME	"sigdump"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"SIGDUMP_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SIGDUMP_BANNER"
#define	VARSEARCHNAME	"SIGDUMP_NAME"
#define	VAROPTS		"SIGDUMP_OPTS"
#define	VARFTYPES	"SIGDUMP_FTYPES"
#define	VARSUFREQ	"SIGDUMP_SUFREQ"
#define	VARSA		"SIGDUMP_SUFACC"
#define	VARSR		"SIGDUMP_SUFREJ"
#define	VARTARDNAME	"SIGDUMP_TARDIR"
#define	VARRFNAME	"SIGDUMP_RF"
#define	VARAFNAME	"SIGDUMP_AF"
#define	VAREFNAME	"SIGDUMP_EF"

#define	VARDEBUGFNAME	"SIGDUMP_DEBUGFILE"
#define	VARDEBUGFD1	"SIGDUMP_DEBUGFD"
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

#define	CONFIGFNAME	"etc/sigdump/conf"
#define	LOGFNAME	"log/sigdump"
#define	HELPFNAME	"help"

#define	PO_OPTION	"option"
#define	PO_FTS		"fts"			/* file-types */
#define	PO_SUFREQ	"sufreq"		/* suffix-required */
#define	PO_SUFREJ	"sufacc"		/* suffix-reject */
#define	PO_SUFACC	"sufrej"		/* suffix-accept */


