/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)kshbi "
#define	BANNER		"KSH Builtin"
#define	SEARCHNAME	"kshbi"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"KSHBI_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"KSHBI_BANNER"
#define	VARSEARCHNAME	"KSHBI_NAME"
#define	VAROPTS		"KSHBI_OPTS"
#define	VARBILIB	"KSHBI_LIBRARY"
#define	VARFILEROOT	"KSHBI_FILEROOT"
#define	VARLOGTAB	"KSHBI_LOGTAB"
#define	VARAFNAME	"KSGBI_AF"
#define	VAREFNAME	"KSHBI_EF"

#define	VARDEBUGFNAME	"KSHBI_DEBUGFILE"
#define	VARDEBUGFD1	"KSHBI_DEBUGFD"
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

#define	VAREXECPATH	"PATH"
#define	VARLIBPATH	"LD_LIBRARY_PATH"

#define	VARTMPDNAME	"TMPDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	VDNAME		"var"
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	DEFLIBFNAME	"lkcmd"
#define	TSFNAME		".lastmaint"		/* time-stamp filename */

#define	PIDFNAME	"run/kshbi"		/* mutex PID file */
#define	LOGFNAME	"var/log/kshbi"		/* activity log */
#define	LOCKFNAME	"spool/locks/kshbi"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	HOUR_MAINT	18


