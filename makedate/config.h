/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#define	VERSION		"0a"
#define	WHATINFO	"@(#)MAKEDATE "
#define	BANNER		"MakeDate"
#define	SEARCHNAME	"makedate"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"MAKEDATE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MAKEDATE_BANNER"
#define	VARSEARCHNAME	"MAKEDATE_NAME"
#define	VAROPTS		"MAKEDATE_OPTS"
#define	VARAFNAME	"MAKEDATE_AF"
#define	VAREFNAME	"MAKEDATE_EF"
#define	VARERRORFNAME	"MAKEDATE_ERRORFILE"

#define	VARDEBUGFNAME	"MAKEDATE_DEBUGFILE"
#define	VARDEBUGFD1	"MAKEDATE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
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
#define	VARHZ		"HZ"
#define	VARTZ		"TZ"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	LOGCNAME	"log"

#define	CONFIGFNAME	"conf"
#define	HELPFNAME	"help"
#define	LOGFNAME	"log/makedate"
#define	USERFNAME	"log/makedate.users"
#define	CMDHELPFNAME	"lib/makedate/cmdhelp"

#ifndef	ORGLEN
#define	ORGLEN		MAXNAMELEN
#endif

#define	LOGSIZE		(80*1024)

#define	ORGANIZATION	"RightCore"


