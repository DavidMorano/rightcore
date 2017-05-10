/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)filefilter "
#define	SEARCHNAME	"filefilter"
#define	VARPNAME	"LOCAL"

#define	BANNER			"File Filter"
#define	BANNER_FILTERNAME	"Filter Name"
#define	BANNER_FILEFILTER	"File Filter"
#define	BANNER_FILEUNIQ		"File Unique"
#define	BANNER_FILEPROG		"File Program"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"FILEFILTER_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"FILEFILTER_BANNER"
#define	VARSEARCHNAME	"FILEFILTER_NAME"
#define	VAROPTS		"FILEFILTER_OPTS"
#define	VARFILEROOT	"FILEFILTER_FILEROOT"
#define	VARSA		"FILEFILTER_SUFACC"
#define	VARSR		"FILEFILTER_SUFREJ"
#define	VARAFNAME	"FILEFILTER_AF"
#define	VAREFNAME	"FILEFILTER_EF"

#define	VARDEBUGFNAME	"FILEFILTER_DEBUGFILE"
#define	VARDEBUGFD1	"FILEFILTER_DEBUGFD"
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
#define	VARPAGER	"PAGER"

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
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/filefilter"		/* mutex PID file */
#define	LOGFNAME	"var/log/filefilter"		/* activity log */
#define	LOCKFNAME	"spool/locks/filefilter"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFNAMES	100
#define	DEFLINKS	100

#define	PO_SUFREJ	"sr"			/* suffix-reject */
#define	PO_SUFACC	"sa"			/* suffix-accept */


