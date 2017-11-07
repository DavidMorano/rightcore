/* config */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written as a KSH built-in command.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)dhtext "
#define	BANNER		"Double Height (high-wide) Text"
#define	SEARCHNAME	"dhtext"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"DHTEXT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"DHTEXT_BANNER"
#define	VARSEARCHNAME	"DHTEXT_NAME"
#define	VAROPTS		"DHTEXT_OPTS"
#define	VARFILEROOT	"DHTEXT_FILEROOT"
#define	VARLOGTAB	"DHTEXT_LOGTAB"
#define	VARAFNAME	"DHTEXT_AF"
#define	VAREFNAME	"DHTEXT_EF"
#define	VAROFNAME	"DHTEXT_OF"
#define	VARIFNAME	"DHTEXT_IF"

#define	VARDEBUGFNAME	"DHTEXT_DEBUGFILE"
#define	VARDEBUGFD1	"DHTEXT_DEBUGFD"
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
#define	VARTERM		"TERM"
#define	VARCOLUMNS	"COLUMNS"

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
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/dhtext"		/* mutex PID file */
#define	LOGFNAME	"var/log/dhtext"		/* activity log */
#define	LOCKFNAME	"spool/locks/dhtext"	/* lock mutex file */

#define	LOGSIZE		(80*1024)


