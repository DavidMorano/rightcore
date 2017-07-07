/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"1"
#define	WHATINFO	"@(#)MKINDEXING "
#define	BANNER		"Make Indexing"
#define	SEARCHNAME	"mkindexing"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"MKINDEXING_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MKINDEXING_BANNER"
#define	VARSEARCHNAME	"MKINDEXING_NAME"
#define	VAROPTS		"MKINDEXING_OPTS"
#define	VARLINELEN	"MKTAGPRINT_LINELEN"
#define	VARTABLEN	"MKTAGPRINT_TABLEN"
#define	VARNPAR		"MKTAGPRINT_NPAR"
#define	VARAFNAME	"MKINDEXING_AF"
#define	VAREFNAME	"MKINDEXING_EF"
#define	VARLFNAME	"MKINDEXING_LF"
#define	VARDBNAME	"MKINDEXING_DB"
#define	VARERRORFNAME	"MKINDEXING_ERRORFILE"

#define	VARDEBUGFNAME	"MKINDEXING_DEBUGFILE"
#define	VARDEBUGFD1	"MKINDEXING_DEBUGFD"
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
#define	ETCDIR1		"etc/mkindexing"
#define	ETCDIR2		"etc"
#define	LOGDNAME	"log"

#define	LOGCNAME	"log"

#define	CONFIGFILE1	"mkindexing.conf"
#define	CONFIGFILE2	"conf"
#define	LOGFNAME	"mkindexing"
#define	HELPFNAME	"help"
#define	EIGENFNAME	"share/dict/eign"
#define	EIGENFNAME1	"/usr/add-on/ncmp/share/dict/eign"
#define	EIGENFNAME2	"/usr/dict/eign"

#define	LOGSIZE		(80*1024)

/* tuning defaults */

#define	IGNORECHARS	"XYZ"
#define	DEFDELIM	" "
#define	MINWORDLEN	3
#define	MAXWORDLEN	6
#define	MAXEIGENWORDS	1000		/* default maximum eigenwords */
#define	MAXKEYS		100000		/* maximum keys per entry */
#define	MAXTAGS		100000

#define	TABLEN		(2 * 1024)

#define	FE_HASH		"hash"
#define	FE_TAG		"tag"

#define	OPT_SENDPARAMS	TRUE
#define	OPT_LOGPROG	TRUE


