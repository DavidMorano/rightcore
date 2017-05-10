/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)CALYEAR "
#define	SEARCHNAME	"calyear"
#define	BANNER		"Calendar Year"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"CALYEAR_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"CALYEAR_BANNER"
#define	VARSEARCHNAME	"CALYEAR_NAME"
#define	VARCALROOT	"CALYEAR_CALROOT"
#define	VARLOGTAB	"CALYEAR_LOGTAB"
#define	VAROPTS		"CALYEAR_OPTS"
#define	VARCALDNAMES	"CALYEAR_CALDIRS"
#define	VARCALNAMES	"CALYEAR_CALNAMES"
#define	VARLINELEN	"CALYEAR_LINELEN"
#define	VARAFNAME	"CALYEAR_AF"
#define	VAREFNAME	"CALYEAR_EF"
#define	VARCFNAME	"CALYEAR_CF"
#define	VARDBNAME	"CALYEAR_DB"
#define	VARERRFILE	"CALYEAR_ERRFILE"
#define	VARCONFIG	"CALYEAR_CONF"

#define	VARDEBUGFNAME	"CALYEAR_DEBUGFILE"
#define	VARDEBUGFD1	"CALYEAR_DEBUGFD"
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

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	LOGDNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	CALFNAME	".calendar"
#define	LOGTABFNAME	"logtab"
#define	LOGFNAME	"log/calyear"		/* activity log */
#define	PIDFNAME	"run/calyear"		/* mutex PID file */
#define	LOCKFNAME	"spool/locks/calyear"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFLOGSIZE	100000		/* default target log size */

#define	DEFPRECISION	5		/* default precision numbers */
#define	DEFINDENT	7


