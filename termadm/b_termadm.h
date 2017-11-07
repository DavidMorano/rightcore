/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)termadm "
#define	BANNER		"Terminal Administration"
#define	SEARCHNAME	"termadm"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"TERMADM_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TERMADM_BANNER"
#define	VARSEARCHNAME	"TERMADM_NAME"
#define	VAROPTS		"TERMADM_OPTS"
#define	VARFILEROOT	"TERMADM_FILEROOT"
#define	VARLOGTAB	"TERMADM_LOGTAB"
#define	VARMSFNAME	"TERMADM_MSFILE"
#define	VARUTFNAME	"TERMADM_UTFILE"
#define	VARTERMLINE	"TERMADM_LINE"
#define	VARTERMDB	"TERMADM_DB"
#define	VARAFNAME	"TERMADM_AF"
#define	VAREFNAME	"TERMADM_EF"
#define	VAROFNAME	"TERMADM_OF"
#define	VARIFNAME	"TERMADM_IF"

#define	VARDEBUGFNAME	"TERMADM_DEBUGFILE"
#define	VARDEBUGFD1	"TERMADM_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"

#define	VARTMPDNAME	"TMPDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/termadm"		/* mutex PID file */
#define	LOGFNAME	"var/log/termadm"	/* activity log */
#define	LOCKFNAME	"spool/locks/termadm"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	OPT_LOGPROG	TRUE ;

#define	USAGECOLS	4


