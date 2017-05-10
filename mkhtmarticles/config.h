/* config */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	VERSION		"0"
#define	WHATINFO	"@(#)mkhtmarticles "
#define	BANNER		"Make HTM Articles"
#define	SEARCHNAME	"mkhtmarticles"
#define	VARPRNAME	VARPRLOCAL

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"MKHTMARTICLES_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MKHTMARTICLES_BANNER"
#define	VARSEARCHNAME	"MKHTMARTICLES_NAME"
#define	VAROPTS		"MKHTMARTICLES_OPTS"
#define	VARAFNAME	"MKHTMARTICLES_AF"
#define	VAREFNAME	"MKHTMARTICLES_EF"
#define	VARLFNAME	"MKHTMARTICLES_LF"
#define	VARFILEROOT	"MKHTMARTICLES_FILEROOT"
#define	VARLOGTAB	"MKHTMARTICLES_LOGTAB"
#define	VARERRORFNAME	"MKHTMARTICLES_ERRORFILE"

#define	VARDEBUGFNAME	"MKHTMARTICLES_DEBUGFILE"
#define	VARDEBUGFD1	"MKHTMARTICLES_DEBUGFD"
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
#define	VARLPDEST	"LPDEST"
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	LEADFNAME	"leader.itm"
#define	TRAILFNAME	"trailer.itm"

#define	PIDFNAME	"run/mkhtmarticles"		/* mutex PID file */
#define	LOGFNAME	"var/log/mkhtmarticles"		/* activity log */
#define	LOCKFNAME	"spool/locks/mkhtmarticles"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */
#define	DEFTITLE	"Bush-Is-Bad"

#ifndef	LINEFOLDLEN
#define	LINEFOLDLEN	76
#endif


