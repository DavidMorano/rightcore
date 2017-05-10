/* config */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)UDD "
#define	SEARCHNAME	"udd"
#define	BANNER		"Update Directory Daemon (UDD)"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"UDD_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	TMPDIR		"/tmp"
#define	WORKDIR		"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ACCFNAME	"acctab"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	REQFNAME	"req"
#define	MSGQFNAME	"msgq"

#define	LOGFNAME	"log/udd"		/* activity log */
#define	PIDFNAME	"spool/run/udd"		/* mutex PID file */
#define	LOCKFNAME	"spool/locks/udd"	/* lock mutex file */
#define	SERIALFNAME	"/tmp/serial"

#define	PORTNAME	"tcpmux"
#define	PORTNUM		"5108"			/* default TCP port */

#define	LOGSIZE		(80*1024)

#define	DEFPATH		"/bin:/usr/sbin"

#define	TO_SVC		60			/* service aquire timeout */
#define	TI_MARKTIME	(3600*12)


