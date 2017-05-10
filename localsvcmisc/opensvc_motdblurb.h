/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvcmotdblurb "
#define	BANNER		"Open-Service Login Blurb"
#define	SEARCHNAME	"opensvcmotdblurb"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"OPENSVCMOTDBLURB_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCMOTDBLURB_BANNER"
#define	VARSEARCHNAME	"OPENSVCMOTDBLURB_NAME"
#define	VARFILEROOT	"OPENSVCMOTDBLURB_FILEROOT"
#define	VARLOGTAB	"OPENSVCMOTDBLURB_LOGTAB"
#define	VARMSFNAME	"OPENSVCMOTDBLURB_MSFILE"
#define	VARUTFNAME	"OPENSVCMOTDBLURB_UTFILE"
#define	VARERRORFNAME	"OPENSVCMOTDBLURB_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCMOTDBLURB_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCMOTDBLURB_DEBUGFD"
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

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/opensvcmotdblurb"
#define	LOGFNAME	"var/log/opensvcmotdblurb"
#define	LOCKFNAME	"spool/locks/opensvcmotdblurb"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


