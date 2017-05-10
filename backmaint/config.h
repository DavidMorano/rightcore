/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)backmaint "
#define	BANNER		"Backup Maintenance"
#define	SEARCHNAME	"backmaint"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"BACKMAINT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"BACKMAINT_BANNER"
#define	VARSEARCHNAME	"BACKMAINT_NAME"
#define	VAROPTS		"BACKMAINT_OPTS"
#define	VARFILEROOT	"BACKMAINT_FILEROOT"
#define	VARDNAMES	"BACKMAINT_DIRS"
#define	VARAFNAME	"BACKMAINT_AF"
#define	VAREFNAME	"BACKMAINT_EF"
#define	VAROFNAME	"BACKMAINT_OF"

#define	VARDEBUGFNAME	"BACKMAINT_DEBUGFILE"
#define	VARDEBUGFD1	"BACKMAINT_DEBUGFD"
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

#define	PIDFNAME	"run/backmaint"
#define	LOGFNAME	"var/log/backmaint"
#define	LOCKFNAME	"spool/locks/backmaint"

#define	LOGSIZE		(80*1024)

/* default option switch settings */
#define	DEFMINMAX	1			/* MIN-MAX mode */
#define	DEFMASKCALC	1			/* re-calculate the "mask" */


