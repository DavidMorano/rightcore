/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)shcat "
#define	BANNER		"Shell Concatenate"
#define	SEARCHNAME	"shcat"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"SHCAT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SHCAT_BANNER"
#define	VARSEARCHNAME	"SHCAT_NAME"
#define	VAROPTS		"SHCAT_OPTS"
#define	VARFILEROOT	"SHCAT_FILEROOT"
#define	VARLOGTAB	"SHCAT_LOGTAB"
#define	VARAFNAME	"SHCAT_AF"
#define	VAREFNAME	"SHCAT_EF"

#define	VARDEBUGFNAME	"SHCAT_DEBUGFILE"
#define	VARDEBUGFD1	"SHCAT_DEBUGFD"
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
#define	VARTERM		"TERM"
#define	VARCOLUMNS	"COLUMNS"

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
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/shcat"		/* mutex PID file */
#define	LOGFNAME	"var/log/shcat"		/* activity log */
#define	LOCKFNAME	"spool/locks/shcat"	/* lock mutex file */

#define	LOGSIZE		(80*1024)


