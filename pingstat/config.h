/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)PINGSTAT "
#define	BANNER		"Ping Status"
#define	SEARCHNAME	"pingstat"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"PINGSTAT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"PINGSTAT_BANNER"
#define	VARSEARCHNAME	"PINGSTAT_NAME"
#define	VAROPTS		"PINGSTAT_OPTS"
#define	VARPINGTAB	"PINGSTAT_PINGTAB"
#define	VARERRORFNAME	"PINGSTAT_ERRORFILE"
#define	VARAFNAME	"PINGSTAT_AF"
#define	VAREFNAME	"PINGSTAT_EF"

#define	VARDEBUGFNAME	"PINGSTAT_DEBUGFILE"
#define	VARDEBUGFD1	"PINGSTAT_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARTERM		"TERM"
#define	VARPRINTER	"PRINTER"
#define	VARLPDEST	"LPDEST"
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARNAME		"NAME"
#define	VARFULLNAME	"FULLNAME"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	ETCDNAME	"etc"
#define	VDNAME		"var"
#define	PTDNAME		"pingtab"
#define	RUNDNAME	"var/run"
#define	LOGDNAME	"var/log"

#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	DEFPTFNAME	"default"
#define	PSFNAME		"db"
#define	PIDFNAME	"pid"
#define	PTFNAME		"pingtab"

#define	LOGSIZE		(160*1024)

#define	TO_PING		5		/* time-out for a ping */

/* defaults */
#define	INTMINPING	(1*60)		/* def minimum interval between pings */
#define	INTMINUPDATE	(2*60)		/* def minimum time between updates */
#define	INTMININPUT	120		/* def minimum input interval (to) */

#define	PORTSPEC_PINGSTAT	"pingstat"

#define	PORT_PINGSTAT		5112

#define	OPT_LOGPROG		TRUE


