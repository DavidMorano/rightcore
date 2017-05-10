/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvcnewsdirs "
#define	BANNER		"Open-Service Name"
#define	SEARCHNAME	"opensvcnewsdirs"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"OPENSVCNEWSDIRS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCNEWSDIRS_BANNER"
#define	VARSEARCHNAME	"OPENSVCNEWSDIRS_NAME"
#define	VARFILEROOT	"OPENSVCNEWSDIRS_FILEROOT"
#define	VARLOGTAB	"OPENSVCNEWSDIRS_LOGTAB"
#define	VARMSFNAME	"OPENSVCNEWSDIRS_MSFILE"
#define	VARUTFNAME	"OPENSVCNEWSDIRS_UTFILE"
#define	VARERRORFNAME	"OPENSVCNEWSDIRS_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCNEWSDIRS_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCNEWSDIRS_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARDOMAIN	"DOMAIN"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"
#define	VARCOLUMNS	"COLUMNS"
#define	VARBBNEWSDNAME	"BBNEWSDIR"

#define	VARTMPDNAME	"TMPDIR"

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

#define	PIDFNAME	"run/opensvcnewsdirs"
#define	LOGFNAME	"var/log/opensvcnewsdirs"
#define	LOCKFNAME	"spool/locks/opensvcnewsdirs"
#define	BBNEWSDNAME	"spool/boards"

#define	LOGSIZE		(80*1024)
#define	COLUMNS		80

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


