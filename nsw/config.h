/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)nsw "
#define	BANNER		"NetScape Watcher"
#define	SEARCHNAME	"nsw"
#define	VARPRNAME	"EXTRA"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/extra"
#endif

#define	VARPROGRAMROOT1	"NSW_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"NSW_BANNER"
#define	VARSEARCHNAME	"NSW_NAME"
#define	VAROPTS		"NSW_OPTS"
#define	VARFILEROOT	"NSW_FILEROOT"
#define	VARPROGNAMES	"NSW_PROGNAMES"
#define	VARAFNAME	"NSW_AF"
#define	VAREFNAME	"NSW_EF"
#define	VARLFNAME	"NSW_LF"
#define	VARDEBUGFNAME	"NSW_DEBUGFILE"

#define	VARDEBUGFD1	"NSW_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARTMPDNAME	"TMPDIR"

#define	TMPDNAME	"/tmp"
#define	PROCDNAME	"/proc"
#define	LOGCNAME	"log"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/nsw"		/* mutex PID file */
#define	LOGFNAME	"var/log/nsw"		/* activity log */
#define	LOCKFNAME	"spool/locks/nsw"	/* lock mutex file */

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	LOGSIZE		(80*1024)

#define	DEFINTPOLL	10
#define	DEFINTRUN	(12 * 60 * 60)

#define	SETPRIO		-2			/* priority to set to */

#define	PROG_PS		"ps"

#define	DEFPROGNAME	"netscape"

#define	OPT_LOGPROG	TRUE


