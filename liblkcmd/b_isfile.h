/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)isfile "
#define	BANNER		"Is File"
#define	SEARCHNAME	"isfile"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"ISFILE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"ISFILE_BANNER"
#define	VARSEARCHNAME	"ISFILE_NAME"
#define	VAROPTS		"ISFILE_OPTS"
#define	VARFILEROOT	"ISFILE_FILEROOT"
#define	VARLOGTAB	"ISFILE_LOGTAB"
#define	VARAFNAME	"ISFILE_AF"
#define	VAREFNAME	"ISFILE_EF"

#define	VARDEBUGFNAME	"ISFILE_DEBUGFILE"
#define	VARDEBUGFD1	"ISFILE_DEBUGFD"
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

#define	HOMEDNAME	"/home"
#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/isfile"		/* mutex PID file */
#define	LOGFNAME	"var/log/isfile"	/* activity log */
#define	LOCKFNAME	"spool/locks/isfile"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */


