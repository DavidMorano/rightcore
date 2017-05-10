/* config */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)ECHOTEST "
#define	BANNER		"Echo Test"
#define	SEARCHNAME	"echotest"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"ECHOTEST_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"ECHOTEST_BANNER"
#define	VARSEARCHNAME	"ECHOTEST_NAME"
#define	VAROPTS		"ECHOTEST_OPTS"
#define	VARAFNAME	"ECHOTEST_AF"
#define	VARLFNAME	"ECHOTEST_LF"
#define	VAREFNAME	"ECHOTEST_EF"
#define	VARERRORFNAME	"ECHOTEST_ERRORFILE"

#define	VARDEBUGFNAME	"ECHOTEST_DEBUGFILE"
#define	VARDEBUGFD1	"ECHOTEST_DEBUGFD"
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
#define	VARFPATH	"FPATH"
#define	VARMAIL		"MAIL"
#define	VARMBOX		"MBOX"
#define	VARFOLDER	"folder"
#define	VARMAILFROM	"MAILFROM"
#define	VARMAILREPLY	"MAILREPLY"
#define	VARNCPU		"NCPU"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"
#define	VARPWD		"PWD"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	HELPFNAME	"help"
#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	REQFNAME	"req"
#define	MSGQFNAME	"msgq"

#define	LOGFNAME	"log/echotest"		/* activity log */
#define	PIDFNAME	"spool/run/echotest"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/echotest"	/* lock mutex file */

#define	DEFPATH		"/bin:/usr/sbin"

#define	LOGSIZE		(80*1024)

#define	PORTSPEC_ECHO	"echo"
#define	SVCSPEC_ECHO	"echo"

#define	PORT_ECHO	7

#define	DIALTIME	20


