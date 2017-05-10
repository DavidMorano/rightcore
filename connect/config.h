/* config */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)CONNECT "
#define	SEARCHNAME	"connect"
#define	BANNER		"Connect"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"CONNECT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"CONNECT_BANNER"
#define	VARSEARCHNAME	"CONNECT_NAME"
#define	VAROPTS		"CONNECT_OPTS"
#define	VARAFNAME	"CONNECT_AF"
#define	VAREFNAME	"CONNECT_EF"
#define	VARERRORFNAME	"CONNECT_ERRORFILE"

#define	VARDEBUGFNAME	"CONNECT_DEBUGFILE"
#define	VARDEBUGFD1	"CONNECT_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARHZ		"HZ"
#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARDOMAIN	"DOMAIN"
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
#define	VARTZ		"TZ"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"
#define	VARPATH		"PATH"
#define	VARMANPATH	"MANPATH"
#define	VARCDPATH	"CDPATH"
#define	VARLIBPATH	"LD_LIBRARY_PATH"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	CONFFNAME	"conf"
#define	HELPFNAME	"help"
#define	SRVFNAME	"srvtab"
#define	ENVFNAME	"environ"
#define	PATHFNAME	"path"
#define	REQFNAME	"req"

#define	LOGFNAME	"log/connect"		/* activity log */
#define	PIDFNAME	"var/run/connect"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/connect"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	PORTSPEC_TCPMUX	"tcpmux"
#define	PORTSPEC_TCPNLS	"listen"
#define	PORTSPEC_ECHO	"echo"

#define	SVCSPEC_ECHO	"echo"

#define	PORT_ECHO	7

#define	DIALTIME	20
#define	DEFKEEPTIME	(3 * 60)	/* sanity check timeout */

#define	SANITYFAILURES	5		/* something w/ sanity? */


