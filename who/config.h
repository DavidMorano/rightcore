/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)who "
#define	BANNER		"Who "
#define	SEARCHNAME	"who"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"WHO_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"WHO_BANNER"
#define	VARSEARCHNAME	"WHO_NAME"
#define	VAROPTS		"WHO_OPTS"
#define	VARFILEROOT	"WHO_FILEROOT"
#define	VARLOGTAB	"WHO_LOGTAB"
#define	VAREFNAME	"WHO_EF"
#define	VARERRORFNAME	"WHO_ERRORFILE"

#define	VARDEBUGFNAME	"WHO_DEBUGFILE"
#define	VARDEBUGFD1	"WHO_DEBUGFD"
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

#define	PIDFNAME	"run/who"		/* mutex PID file */
#define	LOGFNAME	"var/log/who"		/* activity log */
#define	LOCKFNAME	"spool/locks/who"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */


