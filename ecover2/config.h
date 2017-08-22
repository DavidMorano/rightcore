/* config */


/* revision history:

	= 1999-06-13, David A­D­ Morano
	Originally written for Rightcore Network Services.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)ECOVER "
#define	BANNER		"Email Cover"
#define	SEARCHNAME	"ecover"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"ECOVER_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"ECOVER_BANNER"
#define	VARSEARCHNAME	"ECOVER_NAME"
#define	VAROPTS		"ECOVER_OPTS"
#define	VARCONF		"ECOVER_CONF"
#define	VARLOGSIZE	"ECOVER_LOGSIZE"
#define	VARJOBID	"ECOVER_JOBID"
#define	VARLFNAME	"ECOVER_LF"
#define	VARIFNAME	"ECOVER_IF"
#define	VAREFNAME	"ECOVER_EF"
#define	VARERRORFNAME	"ECOVER_ERRORFILE"

#define	VARDEBUGFNAME	"ECOVER_DEBUGFILE"
#define	VARDEBUGFD1	"ECOVER_DEBUGFD"
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
#define	LOGCNAME	"log"

#define	ETCDIR1		"etc/ecover"
#define	ETCDIR2		"etc"
#define	CONFIGFNAME1	"ecover.conf"
#define	CONFIGFNAME2	"conf"
#define	LOGFNAME	"log/ecover"
#define	HELPFNAME	"help"
#define	SEEDFNAME	"seed"

#define	ENTFNAME	"/tmp/entropy"

#define	LOGSIZE		(80*1024)


