/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)keyauth "
#define	BANNER		"Key Authorization"
#define	SEARCHNAME	"keyauth"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"KEYAUTH_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"KEYAUTH_BANNER"
#define	VARSEARCHNAME	"KEYAUTH_NAME"
#define	VAROPTS		"KEYAUTH_OPTS"
#define	VARFILEROOT	"KEYAUTH_FILEROOT"
#define	VARLOGTAB	"KEYAUTH_LOGTAB"
#define	VARNETRC	"KEYAUTH_NETRC"
#define	VARAUTH		"KEYAUTH_AUTH"
#define	VARAUFNAME	"KEYAUTH_AUF"
#define	VARAFNAME	"KEYAUTH_AF"
#define	VAREFNAME	"KEYAUTH_EF"
#define	VARERRORFNAME	"KEYAUTH_ERRORFILE"

#define	VARDEBUGFNAME	"KEYAUTH_DEBUGFILE"
#define	VARDEBUGFD1	"KEYAUTH_DEBUGFD"
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

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	HOMEDNAME	"/home"
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	FULLFNAME	".fullname"
#define	AUTHFNAME	".auth"
#define	NETRCFNAME	".netrc"

#define	LOGSIZE		(80*1024)

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */


