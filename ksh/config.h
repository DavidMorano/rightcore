/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)ksh "
#define	SEARCHNAME	"ksh"
#define	BANNER		"Korn Shell"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"KSH_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"KSH_BANNER"
#define	VARSEARCHNAME	"KSH_NAME"
#define	VARFILEROOT	"KSH_FILEROOT"
#define	VARAFNAME	"KSH_AF"
#define	VAREFNAME	"KSH_EF"
#define	VARERRFIL	"KSH_ERRFILE"

#define	VARDEBUGFD1	"KSH_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

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

#define	PIDFNAME	"run/ksh"		/* mutex PID file */
#define	LOGFNAME	"var/log/ksh"		/* activity log */
#define	LOCKFNAME	"spool/locks/ksh"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */


