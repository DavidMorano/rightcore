/* config */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)commandment "
#define	BANNER		"Commandment"
#define	SEARCHNAME	"commandment"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"COMMANDMENT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"COMMANDMENT_BANNER"
#define	VARSEARCHNAME	"COMMANDMENT_NAME"
#define	VAROPTS		"COMMANDMENT_OPTS"
#define	VARDB		"COMMANDMENT_DB"
#define	VARDBDIR	"COMMANDMENT_DBDIR"
#define	VARFILEROOT	"COMMANDMENT_FILEROOT"
#define	VARLOGTAB	"COMMANDMENT_LOGTAB"
#define	VARLINELEN	"COMMANDMENT_LINELEN"
#define	VARAFNAME	"COMMANDMENT_AF"
#define	VAREFNAME	"COMMANDMENT_EF"

#define	VARDEBUGFNAME	"COMMANDMENT_DEBUGFILE"
#define	VARDEBUGFD1	"COMMANDMENT_DEBUGFD"
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
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"
#define	VARTZ		"TZ"
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
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/commandment"
#define	LOGFNAME	"var/log/commandment"
#define	LOCKFNAME	"spool/locks/commandment"
#define	MSFNAME		"ms"

#define	DBNAME		"ten"

#define	LOGSIZE		(80*1024)

#define	NDBENTS		1000		/* hack: number of DB entries */

#define	DEFPRECISION	2		/* default precision numbers */

#define	DEFOPT_SEPARATE	0		/* separation */
#define	DEFOPT_ROTATE	0		/* rotate after end of list */


