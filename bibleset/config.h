/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"1"
#define	WHATINFO	"@(#)bibleset "
#define	BANNER		"Bible Set "
#define	SEARCHNAME	"bibleset"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"BIBLESET_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"BIBLESET_BANNER"
#define	VARSEARCHNAME	"BIBLESET_NAME"
#define	VAROPTS		"BIBLESET_OPTS"
#define	VARFILEROOT	"BIBLESET_FILEROOT"
#define	VARLOGTAB	"BIBLESET_LOGTAB"
#define	VARAFNAME	"BIBLESET_AF"
#define	VAREFNAME	"BIBLESET_EF"
#define	VAROFNAME	"BIBLESET_OF"
#define	VARIFNAME	"BIBLESET_IF"
#define	VARERRORFNAME	"BIBLESET_ERRORFILE"

#define	VARDEBUGFNAME	"BIBLESET_DEBUGFILE"
#define	VARDEBUGFD1	"BIBLESET_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
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
#define	VARHZ		"HZ"
#define	VARTZ		"TZ"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"

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

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/bibleset"		/* mutex PID file */
#define	LOGFNAME	"var/log/bibleset"	/* activity log */
#define	LOCKFNAME	"spool/locks/bibleset"	/* lock mutex file */
#define	BIBLEDBFNAME	"share/misc/bibledb.txt"
#define	BBOOKSFNAME	"share/misc/biblebooks"
#define	TMACFNAME	"share/tmac/tmac.bs"

#define	LOGSIZE		(80*1024)

#define	PUBLISHER	"BB"			/* abbreviation */

#define	NDBNAME		"english"		/* DB for bible book-names */
#define	WDBNAME		"english"		/* DB for bible meta-words */
#define	VDBNAME		"kjv"			/* DB for bible verses */

#define	COLUMNS		80			/* for display */
#define	LINEWIDTH	76			/* for formatting */
#define	VERSEZEROLEN	35			/* verse-zero length */

#define	DEFTITLE	"The Holy Bible"
#define	DEFPS		10
#define	DEFVS		12

#define	STR_BOOKEND	"\\(sq"

#define	OPT_IBZ		TRUE


