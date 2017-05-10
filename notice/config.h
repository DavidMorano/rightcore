/* config */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)NOTICE "
#define	BANNER		"Notice"
#define	SEARCHNAME	"notice"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"NOTICE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"NOTICE_BANNER"
#define	VARSEARCHNAME	"NOTICE_NAME"
#define	VAROPTS		"NOTICE_OPTS"
#define	VARNTERMS	"NOTICE_NTERMS"
#define	VARAFNAME	"NOTICE_AF"
#define	VAREFNAME	"NOTICE_EF"
#define	VARERRORFNAME	"NOTICE_ERRORFILE"

#define	VARDEBUGFNAME	"NOTICE_DEBUGFILE"
#define	VARDEBUGFD1	"NOTICE_DEBUGFD"
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
#define	WORKDNAME	"/tmp"
#define	VCNAME		"var"
#define	STAMPDNAME	"spool/timestamps"	/* timestamp directory */
#define	LOGDNAME	"log"
#define	LOGCNAME	"log"

#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	HELPFNAME	"help"
#define	LOGFNAME	"notice"		/* activity log */
#define	SERIALFNAME	"serial"
#define	PIDFNAME	"var/run/notice"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/notice"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	NDEFTERMS	3		/* number for "default" terminals */
#define	NALLTERMS	100		/* number for "all" terminals */


