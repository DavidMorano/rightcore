/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)smesg "
#define	BANNER		"Set Message"
#define	SEARCHNAME	"smesg"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"SMESG_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SMESG_BANNER"
#define	VARSEARCHNAME	"SMESG_NAME"
#define	VAROPTS		"SMESG_OPTS"
#define	VARFILEROOT	"SMESG_FILEROOT"
#define	VARLOGTAB	"SMESG_LOGTAB"
#define	VARAFNAME	"SMESG_AF"
#define	VAREFNAME	"SMESG_EF"

#define	VARDEBUGFNAME	"SMESG_DEBUGFILE"
#define	VARDEBUGFD1	"SMESG_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARTERMDEVICE1	"TERMDEV"
#define	VARTERMDEVICE2	"TERMDEVICE"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARCOLUMNS	"COLUMNS"
#define	VARPRINTER	"PRINTER"
#define	VARTERM		"TERM"
#define	VARTMPDNAME	"TMPDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	DEVDNAME	"/dev"
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/smesg"		/* mutex PID file */
#define	LOGFNAME	"var/log/smesg"		/* activity log */
#define	LOCKFNAME	"spool/locks/smesg"	/* lock mutex file */

#define	LOGSIZE		80000

#define	OPT_LOGPROG	TRUE


