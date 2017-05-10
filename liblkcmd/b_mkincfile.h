/* config */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)mkincfile "
#define	BANNER		"Make Include File"
#define	SEARCHNAME	"mkincfile"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"MKINCFILE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MKINCFILE_BANNER"
#define	VARSEARCHNAME	"MKINCFILE_NAME"
#define	VAROPTS		"MKINCFILE_OPTS"
#define	VARFILEROOT	"MKINCFILE_FILEROOT"
#define	VARLOGTAB	"MKINCFILE_LOGTAB"
#define	VARAFNAME	"MKINCFILE_AF"
#define	VAREFNAME	"MKINCFILE_EF"

#define	VARDEBUGFNAME	"MKINCFILE_DEBUGFILE"
#define	VARDEBUGFD1	"MKINCFILE_DEBUGFD"
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

#define	PIDFNAME	"run/mkincfile"		/* mutex PID file */
#define	LOGFNAME	"var/log/mkincfile"		/* activity log */
#define	LOCKFNAME	"spool/locks/mkincfile"	/* lock mutex file */

#define	LOGSIZE		(80*1024)


