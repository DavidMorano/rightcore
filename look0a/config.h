/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)look "
#define	BANNER		"Look"
#define	SEARCHNAME	"look"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"LOOK_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"LOOK_BANNER"
#define	VARSEARCHNAME	"LOOK_NAME"
#define	VAROPTS		"LOOK_OPTS"
#define	VARWORDS	"LOOK_WORDS"
#define	VARFILEROOT	"LOOK_FILEROOT"
#define	VARAFNAME	"LOOK_AF"
#define	VAREFNAME	"LOOK_EF"
#define	VARLFNAME	"LOOK_LF"
#define	VARERRORFNAME	"LOOK_ERRORFILE"

#define	VARDEBUGFNAME	"LOOK_DEBUGFILE"
#define	VARDEBUGFD1	"LOOK_DEBUGFD"
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

#define	PIDFNAME	"run/look"		/* mutex PID file */
#define	LOGFNAME	"var/log/look"		/* activity log */
#define	LOCKFNAME	"spool/locks/look"	/* lock mutex file */
#define	WORDSFNAME	"/usr/add-on/ncmp/share/dict/words"


