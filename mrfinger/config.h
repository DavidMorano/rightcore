/* config */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#define	VERSION		"0"
#define	WHATINFO	"@(#)mrfinger "
#define	BANNER		"Mac Remote Finger"
#define	SEARCHNAME	"mrfinger"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"MRFINGER_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"MRFINGER_NAME"
#define	VARFILEROOT	"MRFINGER_FILEROOT"
#define	VARLOGTAB	"MRFINGER_LOGTAB"
#define	VARERRORFNAME	"MRFINGER_ERRORFILE"

#define	VARDEBUGFNAME	"MRFINGER_DEBUGFILE"
#define	VARDEBUGFD1	"MRFINGER_DEBUGFD"
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
#define	VARCOLUMNS	"COLUMNS"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/mrfinger"		/* mutex PID file */
#define	LOGFNAME	"var/log/mrfinger"	/* activity log */
#define	LOCKFNAME	"spool/locks/mrfinger"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	TO_CONNECT	20			/* seconds */
#define	TO_READ		8			/* seconds */

#define	LINELEN		80			/* output line-length */



