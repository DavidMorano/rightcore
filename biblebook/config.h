/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)biblebook "
#define	BANNER		"Bible Book"
#define	SEARCHNAME	"biblebook"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"BIBLEBOOK_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"BIBLEBOOK_BANNER"
#define	VARSEARCHNAME	"BIBLEBOOK_NAME"
#define	VAROPTS		"BIBLEBOOK_OPTS"
#define	VARDB		"BIBLEBOOK_DB"
#define	VARFILEROOT	"BIBLEBOOK_FILEROOT"
#define	VARLOGTAB	"BIBLEBOOK_LOGTAB"
#define	VARAFNAME	"BIBLEBOOK_AF"
#define	VAREFNAME	"BIBLEBOOK_EF"

#define	VARDEBUGFNAME	"BIBLEBOOK_DEBUGFILE"
#define	VARDEBUGFD1	"BIBLEBOOK_DEBUGFD"
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

#define	PIDFNAME	"run/biblebook"		/* mutex PID file */
#define	LOGFNAME	"var/log/biblebook"	/* activity log */
#define	LOCKFNAME	"spool/locks/biblebook"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	DBNAME		"english"

#define	LOGSIZE		(80*1024)

#define	DEFPRECISION	2		/* default precision numbers */


