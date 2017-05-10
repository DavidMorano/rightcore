/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testcat "
#define	BANNER		"Test Shell Concatenate"
#define	SEARCHNAME	"testcat"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"TESTCAT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTCAT_BANNER"
#define	VARSEARCHNAME	"TESTCAT_NAME"
#define	VAROPTS		"TESTCAT_OPTS"
#define	VARFILEROOT	"TESTCAT_FILEROOT"
#define	VARLOGTAB	"TESTCAT_LOGTAB"
#define	VAREFNAME	"TESTCAT_EF"

#define	VARDEBUGFNAME	"TESTCAT_DEBUGFILE"
#define	VARDEBUGFD1	"TESTCAT_DEBUGFD"
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

#define	PIDFNAME	"run/testcat"		/* mutex PID file */
#define	LOGFNAME	"var/log/testcat"	/* activity log */
#define	LOCKFNAME	"spool/locks/testcat"	/* lock mutex file */

#define	LOGSIZE		(80*1024)


