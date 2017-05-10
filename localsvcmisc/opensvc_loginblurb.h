/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvcloginblurb "
#define	BANNER		"Open-Service Login Blurb"
#define	SEARCHNAME	"opensvcloginblurb"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"OPENSVCLOGINBLURB_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCLOGINBLURB_BANNER"
#define	VARSEARCHNAME	"OPENSVCLOGINBLURB_NAME"
#define	VARFILEROOT	"OPENSVCLOGINBLURB_FILEROOT"
#define	VARLOGTAB	"OPENSVCLOGINBLURB_LOGTAB"
#define	VARMSFNAME	"OPENSVCLOGINBLURB_MSFILE"
#define	VARUTFNAME	"OPENSVCLOGINBLURB_UTFILE"
#define	VARERRORFNAME	"OPENSVCLOGINBLURB_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCLOGINBLURB_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCLOGINBLURB_DEBUGFD"
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

#define	PIDFNAME	"run/opensvcloginblurb"
#define	LOGFNAME	"var/log/opensvcloginblurb"
#define	LOCKFNAME	"spool/locks/opensvcloginblurb"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


