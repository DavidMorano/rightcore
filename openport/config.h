/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)openport "
#define	SEARCHNAME	"openport"
#define	BANNER		"Open Port"
#define	VARPRNAME	"EXTRA"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"OPENPORT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENPORT_BANNER"
#define	VARSEARCHNAME	"OPENPORT_NAME"
#define	VAROPTS		"OPENPORT_OPTS"
#define	VARDEBUGLEVEL	"OPENPORT_DEBUGLEVEL"
#define	VARFILEROOT	"OPENPORT_FILEROOT"
#define	VARDBFNAME	"OPENPORT_DBFILE"
#define	VARAFNAME	"OPENPORT_AF"
#define	VAREFNAME	"OPENPORT_EF"
#define	VARLFNAME	"OPENPORT_LF"
#define	VARERRORFNAME	"OPENPORT_ERRORFILE"

#define	VARDEBUGFNAME	"OPENPORT_DEBUGFILE"
#define	VARDEBUGFD1	"OPENPORT_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	LOGCNAME	"log"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/openport"		/* mutex PID file */
#define	LOGFNAME	"var/log/openport"	/* activity log */
#define	LOCKFNAME	"spool/locks/openport"	/* lock mutex file */
#define	USERPORTSFNAME	"/etc/userports"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	PO_OPTION	"option"


