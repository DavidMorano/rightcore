/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"2b"
#define	WHATINFO	"@(#)ENVSET "
#define	BANNER		"Environment Set"
#define	SEARCHNAME	"envset"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"ENVSET_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME	
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"ENVSET_BANNER"
#define	VARSEARCHNAME	"ENVSET_NAME"
#define	VAROPTS		"ENVSET_OPTS"
#define	VARDEFPROG	"ENVSET_DEFPROG"
#define	VARDEBUGLEVEL	"ENVSET_DEBUGLEVEL"
#define	VARCFNAME	"ENVSET_CF"
#define	VARDFNAME	"ENVSET_DF"
#define	VARXFNAME	"ENVSET_XF"
#define	VAREFNAME	"ENVSET_EF"
#define	VARERRORFNAME	"ENVSET_ERRORFILE"

#define	VARDEBUGFNAME	"ENVSET_DEBUGFILE"
#define	VARDEBUGFD1	"ENVSET_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARNODE		"NODE"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARPLATFORM	"PLATFORM"
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
#define	VARNCPU		"NCPU"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"tmp"
#define	REPORTDNAME	"/var/tmp/reports"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	HELPFNAME	"help"
#define	ENVFNAME	"env"
#define	PATHFNAME	"path"
#define	MAPFNAME	"map"
#define	DEFSFNAME	"def"
#define	XEFNAME		"xe"
#define	PVARSFNAME	"pvar"

#define	XPATHFNAME	"xpath"
#define	XFPATHFNAME	"xfpath"
#define	XCDPATHFNAME	"xcdpath"
#define	XLIBFNAME	"xlibpath"
#define	XMANFNAME	"xmanpath"
#define	XFSFNAME	"xfspath"
#define	XUFSFNAME	"xufspath"
#define	XENVFNAME	"xenv"

#define	DEFPROGNAME	"ksh"
#define	DEFPROGFNAME	"/usr/bin/ksh"

#define	ADMINUSER	"admin"

#define	PO_NOPRELOAD	"nopreload"

#define	DISARGLEN	50

#define	DEFDEBUGLEVEL	5
#define	DEFNDEFS	20
#define	DEFNXENVS	200

#define	OPT_ENVSORT	TRUE


