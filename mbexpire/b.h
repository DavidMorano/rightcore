/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)mbexpire "
#define	SEARCHNAME	"mbexpire"
#define	BANNER		"Mailbox Expire"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"MBEXPIRE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MBEXPIRE_BANNER"
#define	VARSEARCHNAME	"MBEXPIRE_NAME"
#define	VAROPTS		"MBEXPIRE_OPTS"
#define	VARFILEROOT	"MBEXPIRE_FILEROOT"
#define	VARLOGTAB	"MBEXPIRE_LOGTAB"
#define	VARMSGTO	"MBEXPIRE_MSGTO"
#define	VARAFNAME	"MBEXPIRE_AF"
#define	VAREFNAME	"MBEXPIRE_EF"
#define	VARDEBUGFNAME	"MBEXPIRE_DEBUGFILE"
#define	VARDEBUGFD1	"MBEXPIRE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARTMPDNAME	"TMPDIR"
#define	VARHOMEDNAME	"HOME"
#define	VARMAILUSERS	"MAILUSERS"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	PIDFNAME	"run/mbexpire"		/* mutex PID file */
#define	LOGFNAME	"var/log/mbexpire"	/* activity log */
#define	LOCKFNAME	"spool/locks/mbexpire"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	TO_MSGEXPIRE	(120*24*3600)		/* MSG expiration */


