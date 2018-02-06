/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)querystring "
#define	BANNER		"Query-String"
#define	SEARCHNAME	"querystring"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"QUERYSTRING_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"QUERYSTRING_BANNER"
#define	VARSEARCHNAME	"QUERYSTRING_NAME"
#define	VAROPTS		"QUERYSTRING_OPTS"
#define	VARFILEROOT	"QUERYSTRING_FILEROOT"
#define	VARQS		"QUERYSTRING_QS"
#define	VARLOGTAB	"QUERYSTRING_LOGTAB"
#define	VARAFNAME	"QUERYSTRING_AF"
#define	VAREFNAME	"QUERYSTRING_EF"

#define	VARDEBUGFNAME	"QUERYSTRING_DEBUGFILE"
#define	VARDEBUGFD1	"QUERYSTRING_DEBUGFD"
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
#define	VARQUERYSTRING	"QUERY_STRING"

#define	VARTMPDNAME	"TMPDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	LOGSIZE		(80*1024)

#define	OPT_LOGPROG	TRUE


