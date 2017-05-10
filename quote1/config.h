/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)quote "
#define	BANNER		"Quote Query"
#define	SEARCHNAME	"quote"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"QUOTE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"QUOTE_BANNER"
#define	VARSEARCHNAME	"QUOTE_NAME"
#define	VARFILEROOT	"QUOTE_FILEROOT"
#define	VARLOGTAB	"QUOTE_LOGTAB"
#define	VAROPTS		"QUOTE_OPTS"
#define	VARCONFIG	"QUOTE_CONF"
#define	VARCALDNAMES	"QUOTE_QUOTEDIRS"
#define	VARQUOTENAMES	"QUOTE_QUOTENAMES"
#define	VARNDB		"QUOTE_NDB"
#define	VARVDB		"QUOTE_VDB"
#define	VARDBDIR	"QUOTE_DBDIR"
#define	VARLINELEN	"QUOTE_LINELEN"
#define	VAREFNAME	"QUOTE_EF"

#define	VARDEBUGFNAME	"QUOTE_DEBUGFILE"
#define	VARDEBUGFD1	"QUOTE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARHZ		"HZ"
#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"
#define	VARTZ		"TZ"
#define	VARNAME		"NAME"
#define	VARFULLNAME	"FULLNAME"

#define	VARTMPDNAME	"TMPDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	LOGDNAME	"log"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/quote"		/* mutex PID file */
#define	LOGFNAME	"var/log/quote"		/* activity log */
#define	LOCKFNAME	"spool/locks/quote"	/* lock mutex file */

#define	NDBNAME		"english"
#define	VDBNAME		"av"

#define	LOGSIZE		(80*1024)

#define	DEFPRECISION	5		/* default precision numbers */


