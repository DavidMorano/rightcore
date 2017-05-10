/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvctermnote "
#define	BANNER		"Open-Service Hello"
#define	SEARCHNAME	"opensvctermnote"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"OPENSVCTERMNOTE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCTERMNOTE_BANNER"
#define	VARSEARCHNAME	"OPENSVCTERMNOTE_NAME"
#define	VAROPTS		"OPENSVCTERMNOTE_OPTS"
#define	VARFILEROOT	"OPENSVCTERMNOTE_FILEROOT"
#define	VARLOGTAB	"OPENSVCTERMNOTE_LOGTAB"
#define	VARMSFNAME	"OPENSVCTERMNOTE_MSFILE"
#define	VARUTFNAME	"OPENSVCTERMNOTE_UTFILE"
#define	VARERRORFNAME	"OPENSVCTERMNOTE_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCTERMNOTE_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCTERMNOTE_DEBUGFD"
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

#define	PIDFNAME	"run/opensvctermnote"
#define	LOGFNAME	"var/log/opensvctermnote"
#define	LOCKFNAME	"spool/locks/opensvctermnote"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


