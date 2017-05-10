/* config */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)BBDIRCACHE "
#define	BANNER		"BB DirCache"
#define	SEARCHNAME	"bbdircache"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"BBDIRCACHE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"BBDIRCACHE_BANNER"
#define	VARSEARCHNAME	"BBDIRCACHE_NAME"
#define	VAROPTS		"BBDIRCACHE_OPTS"
#define	VARNEWSDNAME	"BBDIRCACHE_NEWSDIR"
#define	VARAFNAME	"BBDIRCACHE_AF"
#define	VAREFNAME	"BBDIRCACHE_EF"
#define	VARCFNAME	"BBDIRCACHE_CF"
#define	VARLFNAME	"BBDIRCACHE_LF"
#define	VARERRORFNAME	"BBDIRCACHE_ERRORFILE"

#define	VARDEBUGFNAME	"BBDIRCACHE_DEBUGFILE"
#define	VARDEBUGFD1	"BBDIRCACHE_DEBUGFD"
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
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARBBNEWSDNAME	"BBNEWSDIR"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"
#define	WORKDNAME	"/tmp"

#define	VCNAME		"var"
#define	LOGCNAME	"log"
#define	NEWSDNAME	"spool/boards"		/* news directory */
#define	STAMPDNAME	"var/timestamps"	/* timestamps */
#define	LOGFNAME	"log/bbdircache"		/* activity log */
#define	PIDFNAME	"var/run/bbdircache"		/* mutex PID file */
#define	SERIALFNAME	"var/serial"			/* serial file */
#define	LOCKFNAME	"var/spool/locks/bbdircache"	/* lock mutex file */

#define	HELPFNAME	"help"
#define	CONFFNAME	"conf"
#define	STAMPFNAME	"bbdircache"
#define	SRVFNAME	"srvtab"
#define	ACCFNAME	"acctab"
#define	ENVFNAME	"env"
#define	PATHFNAME	"path"
#define	XENVFNAME	"xenv"
#define	XPATHFNAME	"xpath"
#define	USERFNAME	"bbdircache"
#define	DIRCACHEFNAME	".dircache"

#define	USERFSUF	"users"

#define	LOGSIZE		(80*1024)	/* nominal log file length */

#define	TI_POLLSVC	(5 * 60)	/* default interval (minutes) */
#define	TI_MINCHECK	(1 * 60)	/* minimal check interval */

#define	TO_PIDLOCK	5


