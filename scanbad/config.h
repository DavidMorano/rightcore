/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)scanbad "
#define	BANNER		"Scan for Bad blocks"
#define	SEARCHNAME	"scanbad"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"SCANBAD_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SCANBAD_BANNER"
#define	VARSEARCHNAME	"SCANBAD_NAME"
#define	VAROPTS		"SCANBAD_OPTS"
#define	VARSCANSPEC	"SCANBAD_SCANSPEC"
#define	VARFILEROOT	"SCANBAD_FILEROOT"
#define	VARDEBUGLEVEL	"SCANBAD_DEBUGLEVEL"
#define	VARLOGTAB	"SCANBAD_LOGTAB"
#define	VARAFNAME	"SCANBAD_AF"
#define	VAREFNAME	"SCANBAD_EF"

#define	VARDEBUGFNAME	"SCANBAD_DEBUGFILE"
#define	VARDEBUGFD1	"SCANBAD_DEBUGFD"
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
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/scanbad"		/* mutex PID file */
#define	LOGFNAME	"var/log/scanbad"	/* activity log */
#define	LOCKFNAME	"spool/locks/scanbad"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	BLOCKLEN	(1 * 1024 * 1024)


