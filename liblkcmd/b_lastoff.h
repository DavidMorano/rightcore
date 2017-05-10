/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)lastoff "
#define	BANNER		"Last Off"
#define	SEARCHNAME	"lastoff"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"LASTOFF_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"LASTOFF_BANNER"
#define	VARSEARCHNAME	"LASTOFF_NAME"
#define	VAROPTS		"LASTOFF_OPTS"
#define	VARFILEROOT	"LASTOFF_FILEROOT"
#define	VARLOGTAB	"LASTOFF_LOGTAB"
#define	VARAFNAME	"LASTOFF_AF"
#define	VAREFNAME	"LASTOFF_EF"
#define	VARERRFILE	"LASTOFF_ERRFILE"

#define	VARDEBUGFD1	"LASTOFF_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARTMPDNAME	"TMPDIR"

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

#define	PIDFNAME	"run/lastoff"		/* mutex PID file */
#define	LOGFNAME	"var/log/lastoff"	/* activity log */
#define	LOCKFNAME	"spool/locks/lastoff"	/* lock mutex file */

#define	LOGSIZE		(80*1024)


