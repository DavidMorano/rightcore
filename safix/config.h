/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)safix "
#define	SEARCHNAME	"safix"
#define	BANNER		"SpamAssassin Fix"
#defien	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"SAFIX_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SAFIX_BANNER"
#define	VARSEARCHNAME	"SAFIX_NAME"
#define	VAROPTS		"SAFIX_OPTS"
#define	VARFILEROOT	"SAFIX_FILEROOT"
#define	VARLOGTAB	"SAFIX_LOGTAB"
#define	VARLFNAME	"SAFIX_LF"
#define	VAREFNAME	"SAFIX_EF"

#define	VARDEBUGFD1	"SAFIX_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/safix"		/* mutex PID file */
#define	LOGFNAME	"var/log/safix"		/* activity log */
#define	LOCKFNAME	"spool/locks/safix"	/* lock mutex file */

#define	LOGSIZE		(80*1024)


