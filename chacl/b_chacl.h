/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)chacl "
#define	BANNER		"Change ACLs"
#define	SEARCHNAME	"chacl"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"CHACL_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"CHACL_BANNER"
#define	VARSEARCHNAME	"CHACL_NAME"
#define	VAROPTS		"CHACL_OPTS"
#define	VARFILEROOT	"CHACL_FILEROOT"
#define	VARAFNAME	"CHACL_AF"
#define	VAREFNAME	"CHACL_EF"
#define	VAROFNAME	"CHACL_OF"

#define	VARDEBUGFNAME	"CHACL_DEBUGFILE"
#define	VARDEBUGFD1	"CHACL_DEBUGFD"
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

#define	PIDFNAME	"run/chacl"		/* mutex PID file */
#define	LOGFNAME	"var/log/chacl"		/* activity log */
#define	LOCKFNAME	"spool/locks/chacl"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFACLS		10

/* default option switch settings */
#define	DEFMINMAX	1			/* MIN-MAX mode */
#define	DEFMASKCALC	1			/* re-calculate the "mask" */


