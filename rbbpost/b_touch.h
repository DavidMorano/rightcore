/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)touch "
#define	BANNER		"Touch"
#define	SEARCHNAME	"touch"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"TOUCH_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TOUCH_BANNER"
#define	VARSEARCHNAME	"TOUCH_NAME"
#define	VAROPTS		"TOUCH_OPTS"
#define	VARFILEROOT	"TOUCH_FILEROOT"
#define	VARLOGTAB	"TOUCH_LOGTAB"
#define	VARAFNAME	"TOUCH_AF"
#define	VAREFNAME	"TOUCH_EF"
#define	VAROFNAME	"TOUCH_OF"
#define	VAROFNAME	"TOUCH_OF"
#define	VARIFNAME	"TOUCH_IF"

#define	VARDEBUGFNAME	"TOUCH_DEBUGFILE"
#define	VARDEBUGFD1	"TOUCH_DEBUGFD"
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
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/touch"		/* mutex PID file */
#define	LOGFNAME	"var/log/touch"		/* activity log */
#define	LOCKFNAME	"spool/locks/touch"	/* lock mutex file */

#define	LOGSIZE		(80*1024)


