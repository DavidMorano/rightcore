/* config */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)HELLO "
#define	BANNER		"Hello"
#define	SEARCHNAME	"hello"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"HELLO_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"HELLO_BANNER"
#define	VARSEARCHNAME	"HELLO_NAME"
#define	VAROPTS		"HELLO_OPTS"
#define	VARFTYPES	"HELLO_FTYPES"
#define	VARSUFREQ	"HELLO_SUFREQ"
#define	VARSA		"HELLO_SUFACC"
#define	VARSR		"HELLO_SUFREJ"
#define	VARPRUNE	"HELLO_PRUNE"
#define	VARTARDNAME	"HELLO_TARDIR"
#define	VARRFNAME	"HELLO_RF"
#define	VARAFNAME	"HELLO_AF"
#define	VAREFNAME	"HELLO_EF"

#define	VARDEBUGLEVEL	"HELLO_DEBUGLEVEL"
#define	VARDEBUGFNAME	"HELLO_DEBUGFILE"
#define	VARDEBUGFD1	"HELLO_DEBUGFD"
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

#define	CONFIGFNAME	"etc/hello/conf"
#define	LOGFNAME	"log/hello"
#define	HELPFNAME	"help"

#define	PO_OPTION	"option"
#define	PO_FTS		"fts"			/* file-types */
#define	PO_SUFREQ	"sufreq"		/* suffix-required */
#define	PO_SUFREJ	"sufacc"		/* suffix-reject */
#define	PO_SUFACC	"sufrej"		/* suffix-accept */
#define	PO_PRUNE	"prune"			/* prune components */
#define	PO_TARDIRS	"tardirs"


