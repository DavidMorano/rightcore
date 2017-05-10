/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)bibleq "
#define	BANNER		"Bible Query"
#define	SEARCHNAME	"bibleq"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"BIBLEQ_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"BIBLEQ_BANNER"
#define	VARSEARCHNAME	"BIBLEQ_NAME"
#define	VARFILEROOT	"BIBLEQ_FILEROOT"
#define	VARLOGTAB	"BIBLEQ_LOGTAB"
#define	VAROPTS		"BIBLEQ_OPTS"
#define	VARNDB		"BIBLEQ_NDB"
#define	VARVDB		"BIBLEQ_VDB"
#define	VARPDB		"BIBLEQ_PDB"
#define	VARDBDIR	"BIBLEQ_DBDIR"
#define	VARLINELEN	"BIBLEQ_LINELEN"
#define	VARAFNAME	"BIBLEQ_AF"
#define	VAREFNAME	"BIBLEQ_EF"

#define	VARDEBUGFNAME	"BIBLEQ_DEBUGFILE"
#define	VARDEBUGFD1	"BIBLEQ_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARHZ		"HZ"
#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"
#define	VARTZ		"TZ"
#define	VARNAME		"NAME"
#define	VARFULLNAME	"FULLNAME"

#define	VARTMPDNAME	"TMPDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	LOGCNAME	"log"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/bibleq"		/* mutex PID file */
#define	LOGFNAME	"var/log/bibleq"	/* activity log */
#define	LOCKFNAME	"spool/locks/bibleq"	/* lock mutex file */

#define	NDBNAME		"english"
#define	VDBNAME		"av"

#define	LOGSIZE		(80*1024)

#define	DEFPRECISION	5		/* default precision numbers */

#define	OPT_BOOKNAME	TRUE
#define	OPT_INDENT	1


