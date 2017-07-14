/* config */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)FILEOP "
#define	BANNER		"File Operation"
#define	SEARCHNAME	"fileop"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"FILEOP_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"FILEOP_BANNER"
#define	VARSEARCHNAME	"FILEOP_NAME"
#define	VAROPTS		"FILEOP_OPTS"
#define	VARFTYPES	"FILEOP_FTYPES"
#define	VARSUFREQ	"FILEOP_SUFREQ"
#define	VARSA		"FILEOP_SUFACC"
#define	VARSR		"FILEOP_SUFREJ"
#define	VARPRUNE	"FILEOP_PRUNE"
#define	VARTARDNAME	"FILEOP_TARDIR"
#define	VARRFNAME	"FILEOP_RF"
#define	VARAFNAME	"FILEOP_AF"
#define	VAREFNAME	"FILEOP_EF"

#define	VARDEBUGLEVEL	"FILEOP_DEBUGLEVEL"
#define	VARDEBUGFNAME	"FILEOP_DEBUGFILE"
#define	VARDEBUGFD1	"FILEOP_DEBUGFD"
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

#define	CONFIGFNAME	"etc/fileop/conf"
#define	LOGFNAME	"log/fileop"
#define	HELPFNAME	"help"

#define	PO_OPTION	"option"
#define	PO_FTS		"fts"			/* file-types */
#define	PO_SUFREQ	"sufreq"		/* suffix-required */
#define	PO_SUFREJ	"sufacc"		/* suffix-reject */
#define	PO_SUFACC	"sufrej"		/* suffix-accept */
#define	PO_PRUNE	"prune"			/* prune components */
#define	PO_TARDIRS	"tardirs"


