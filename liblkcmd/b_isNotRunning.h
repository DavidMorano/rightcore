/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)isproc "
#define	BANNER		"Is Process"
#define	SEARCHNAME	"isproc"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"ISPROC_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"ISPROC_BANNER"
#define	VARSEARCHNAME	"ISPROC_NAME"
#define	VAROPTS		"ISPROC_OPTS"
#define	VARFILEROOT	"ISPROC_FILEROOT"
#define	VARLOGTAB	"ISPROC_LOGTAB"
#define	VARAFNAME	"ISPROC_AF"
#define	VAREFNAME	"ISPROC_EF"

#define	VARDEBUGFNAME	"ISPROC_DEBUGFILE"
#define	VARDEBUGFD1	"ISPROC_DEBUGFD"
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

#define	HOMEDNAME	"/home"
#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"
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

#define	PIDFNAME	"run/isproc"
#define	LOGFNAME	"var/log/isproc"
#define	LOCKFNAME	"spool/locks/isproc"

#define	LOGSIZE		(80*1024)

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */


