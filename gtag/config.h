/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)GTAG "
#define	BANNER		"G-Tag"
#define	SEARCHNAME	"gatg"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"GTAG_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"GTAG_BANNER"
#define	VARSEARCHNAME	"GTAG_NAME"
#define	VARBIBDIRS	"GTAG_BIBDIRS"
#define	VARBIBFILES	"GTAG_BIBFILES"
#define	VAROPTS		"GTAG_OPTS"
#define	VARAFNAME	"GTAG_AF"
#define	VAREFNAME	"GTAG_EF"
#define	VARERRORFNAME	"GTAG_ERRORFILE"

#define	VARDEBUGFNAME	"GTAG_DEBUGFILE"
#define	VARDEBUGFD1	"GTAG_DEBUGFD"
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

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"

#define	TMPFX		"mmciteXXXXXXXX"
#define	ETCDIR1		"etc/mmcite"
#define	ETCDIR2		"etc"
#define	CONFIGFILE1	"mmcite.conf"
#define	CONFIGFILE2	"conf"
#define	LOGFNAME	"log/mmcite"
#define	HELPFNAME	"lib/mmcite/help"
#define	EIGENFNAME	"/usr/dict/eign"
#define	REFER		"/usr/lib/refer/papers"

#define	LOGSIZE		(80*1024)

#define	MINWORDLEN	3
#define	MAXWORDLEN	6
#define	EIGENWORDS	1000		/* default number of eigenwords */
#define	KEYS		100
#define	IGNORECHARS	"XYZ"

#define	BIBMACRO1	"BIB"
#define	BIBMACRO2	"RBD"
#define	BIBESCAPE	"cite"

#define	DBFEXT		"rbd"

#define	DEFNFILES	10

#ifndef	LINEFOLDLEN
#define	LINEFOLDLEN	76
#endif

#define	BIBBUFLEN	2048

#define	PO_OPTION	"OPTION"
#define	PO_SUFFIX	"SUFFIX"
#define	PO_BIBDIR	"BIBDIR"
#define	PO_BIBFILE	"BIBFILE"


