/* config */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)CHECKBACKSLASH "
#define	BANNER		"Check Backslash"
#define	SEARCHNAME	"checkbackslash"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"CHECKBACKSLASH_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"CHECKBACKSLASH_BANNER"
#define	VARSEARCHNAME	"CHECKBACKSLASH_NAME"
#define	VAROPTS		"CHECKBACKSLASH_OPTS"
#define	VARFTYPES	"CHECKBACKSLASH_FTYPES"
#define	VARSUFREQ	"CHECKBACKSLASH_SUFREQ"
#define	VARSA		"CHECKBACKSLASH_SUFACC"
#define	VARSR		"CHECKBACKSLASH_SUFREJ"
#define	VARTARDNAME	"CHECKBACKSLASH_TARDIR"
#define	VARRFNAME	"CHECKBACKSLASH_RF"
#define	VARAFNAME	"CHECKBACKSLASH_AF"
#define	VAREFNAME	"CHECKBACKSLASH_EF"

#define	VARDEBUGFNAME	"CHECKBACKSLASH_DEBUGFILE"
#define	VARDEBUGFD1	"CHECKBACKSLASH_DEBUGFD"
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

#define	CONFIGFNAME	"etc/checkbackslash/conf"
#define	LOGFNAME	"log/checkbackslash"
#define	HELPFNAME	"help"

#define	PO_OPTION	"option"
#define	PO_FTS		"fts"			/* file-types */
#define	PO_SUFREQ	"sufreq"		/* suffix-required */
#define	PO_SUFREJ	"sufacc"		/* suffix-reject */
#define	PO_SUFACC	"sufrej"		/* suffix-accept */

#define	PO_SUFFIX	"suf"


