/* config */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)TERMSIZE "
#define	BANNER		"Terminal SizeOperation"
#define	SEARCHNAME	"termsize"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"TERMSIZE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TERMSIZE_BANNER"
#define	VARSEARCHNAME	"TERMSIZE_NAME"
#define	VAROPTS		"TERMSIZE_OPTS"
#define	VARFTYPES	"TERMSIZE_FTYPES"
#define	VARSUFREQ	"TERMSIZE_SUFREQ"
#define	VARSA		"TERMSIZE_SUFACC"
#define	VARSR		"TERMSIZE_SUFREJ"
#define	VARPRUNE	"TERMSIZE_PRUNE"
#define	VARTARDNAME	"TERMSIZE_TARDIR"
#define	VARRFNAME	"TERMSIZE_RF"
#define	VARAFNAME	"TERMSIZE_AF"
#define	VAREFNAME	"TERMSIZE_EF"

#define	VARDEBUGFNAME	"TERMSIZE_DEBUGFILE"
#define	VARDEBUGFD1	"TERMSIZE_DEBUGFD"
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

#define	CONFIGFNAME	"etc/termsize/conf"
#define	LOGFNAME	"log/termsize"
#define	HELPFNAME	"help"

#define	PO_OPTION	"option"
#define	PO_FTS		"fts"			/* file-types */
#define	PO_SUFREQ	"sufreq"		/* suffix-required */
#define	PO_SUFREJ	"sufacc"		/* suffix-reject */
#define	PO_SUFACC	"sufrej"		/* suffix-accept */
#define	PO_PRUNE	"prune"			/* prune components */
#define	PO_TARDIRS	"tardirs"


