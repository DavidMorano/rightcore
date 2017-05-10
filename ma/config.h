/* config */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#define	P_MA	1

#define	VERSION		"0"
#define	WHATINFO	"@(#)MA "
#define	SEARCHNAME	"ma"
#define	BANNER		"Mail-Action (MA)"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"MA_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MA_BANNER"
#define	VARSEARCHNAME	"MA_NAME"
#define	VAROPTS		"MA_OPTS"
#define	CONFVAR		"MA_CONF"
#define	TMPDIRVAR	"TMPDIR"

#define	VARDEBUGFD1	"MA_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

/* search for these */

#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ACCFNAME	"acctab"
#define	ENVFNAME	"env"
#define	PATHFNAME	"path"
#define	XENVFNAME	"xenv"
#define	XPATHFNAME	"xpath"
#define	PASSWDFNAME	"passwd"
#define	HELPFNAME	"help"

/* search for and optionally create these */

#define	LOGFNAME	"log/%S"		/* activity log */
#define	REQFNAME	"var/%S/req"
#define	PASSFNAME	"var/%S/pass"		/* pass-FD file */
#define	MSFNAME		"var/ms"
#define	PIDFNAME	"var/run/%S"		/* mutex PID file */
#define	LOCKFNAME	"var/spool/locks/%S"

/* create these ? */

#define	SHMFNAME	"var/spool/%S/info"
#define	SERIALFNAME1	"var/spool/serial"
#define	SERIALFNAME2	"/tmp/serial"

#define	VARDNAME	"var/%S"
#define	SPOOLDNAME	"var/spool/%S"
#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"

#define	PORTNAME	"tcpmux"
#define	PORTNUM		"5108"			/* default TCP port */

#define	LOGSIZE		(80*1024)

#define	ORGCODE		"RC"

#define	DEFPATH		"/bin:/usr/sbin"

#define	PROG_SENDMAIL	"/usr/lib/sendmail"

#define	TI_MARKTIME	(3600*12)
#define	TI_POLLINT	60

#define	TO_SVC		60			/* service acquire timeout */


