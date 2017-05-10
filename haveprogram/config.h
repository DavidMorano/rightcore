/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)haveprogram "
#define	BANNER		"Have Program"
#define	SEARCHNAME	"haveprogram"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"HAVEPROGRAM_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"HAVEPROGRAM_BANNER"
#define	VARSEARCHNAME	"HAVEPROGRAM_NAME"
#define	VAROPTS		"HAVEPROGRAM_OPTS"
#define	VARFILEROOT	"HAVEPROGRAM_FILEROOT"
#define	VARAFNAME	"HAVEPROGRAM_AF"
#define	VAREFNAME	"HAVEPROGRAM_EF"
#define	VARERRORFNAME	"HAVEPROGRAM_ERRORFILE"

#define	VARDEBUGFNAME	"HAVEPROGRAM_DEBUGFILE"
#define	VARDEBUGFD1	"HAVEPROGRAM_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	NODEVAR		"NODE"
#define	CLUSTERVAR	"CLUSTER"
#define	SYSTEMVAR	"SYSTEM"
#define	PRINTERVAR	"PRINTER"

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

#define	PIDFNAME	"run/haveprogram"		/* mutex PID file */
#define	LOGFNAME	"var/log/haveprogram"		/* activity log */
#define	LOCKFNAME	"spool/locks/haveprogram"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */


