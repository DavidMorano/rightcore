/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0b"
#define	WHATINFO	"@(#)prtdb "
#define	BANNER		"Printer Database"
#define	SEARCHNAME	"prtdb"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"PRTDB_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"PRTDB_BANNER"
#define	VARSEARCHNAME	"PRTDB_NAME"
#define	VARLPGET	"PRTDB_LPGET"
#define	VARUTILNAME	"PRTDB_UTILITY"
#define	VARFILEROOT	"PRTDB_FILEROOT"
#define	VARDBFNAME	"PRTDB_DBFILE"
#define	VARLOGTAB	"PRTDB_LOGTAB"
#define	VARAFNAME	"PRTDB_AFNAME"
#define	VAREFNAME	"PRTDB_EFNAME"

#define	VARDEBUGFNAME	"PRTDB_DEBUGFILE"
#define	VARDEBUGFD1	"PRTDB_DEBUGFD"
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

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	UTILSEARCHNAME	"prt"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	PIDDNAME	"var/run"
#define	LOGDNAME	"var/log"
#define	LOCKDNAME	"spool/locks"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	PDBFNAME	"pdb"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	LOGFNAME	"prtdb"			/* activity log */
#define	PIDFNAME	"prtdb"			/* mutex PID file */
#define	LOCKFNAME	"prtdb"			/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	DEFUTILITY	"prt"			/* default utility name */
#define	DEFPRINTER	"_default"
#define	DEFPRINTERKEY	"use"

#define	PROG_LPGET	"lpget"

#define	PO_OPTION	"option"
#define	PO_SUFFIX	"suffix"

#define	TO_CHILD	5


