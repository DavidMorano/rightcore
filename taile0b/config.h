/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0b"
#define	WHATINFO	"@(#)taile "
#define	BANNER		"Tail File"
#define	SEARCHNAME	"taile"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"TAILE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TAILE_BANNER"
#define	VARSEARCHNAME	"TAILE_NAME"
#define	VAROPTS		"TAILE_OPTS"
#define	VARFILE		"TAILE_FILE"
#define	VARLINELEN	"TAILE_LINELEN"
#define	VARTRACKPID	"TAILE_TPID"
#define	VARAFNAME	"TAILE_AF"
#define	VAREFNAME	"TAILE_EF"
#define	VARERRORFNAME	"TAILE_ERRORFILE"

#define	VARDEBUGFNAME	"TAILE_DEBUGFILE"
#define	VARDEBUGFD1	"TAILE_DEBUGFD"
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
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"

#define	HELPFNAME	"help"
#define	LIBDNAME	"lib/taile"

#ifndef	COLUMNS
#define	COLUMNS		80		/* output columns (should be 80) */
#endif

#define	DEFINTERVAL	2		/* default poll interval */
#define	DEFPRECISION	5		/* default precision numbers */

#define	PO_PPM		"ppm"


