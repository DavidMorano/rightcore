/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)termnote "
#define	BANNER		"Shell Concatenate"
#define	SEARCHNAME	"termnote"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"TERMNOTE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TERMNOTE_BANNER"
#define	VARSEARCHNAME	"TERMNOTE_NAME"
#define	VAROPTS		"TERMNOTE_OPTS"
#define	VARAFNAME	"TERMNOTE_AF"
#define	VAREFNAME	"TERMNOTE_EF"
#define	VARIFNAME	"TERMNOTE_IF"
#define	VARMAXTERMS	"TERMNOTE_MAXTERMS"
#define	VARFILEROOT	"TERMNOTE_FILEROOT"
#define	VARLOGTAB	"TERMNOTE_LOGTAB"

#define	VARDEBUGFNAME	"TERMNOTE_DEBUGFILE"
#define	VARDEBUGFD1	"TERMNOTE_DEBUGFD"
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
#define	VARTERM		"TERM"
#define	VARCOLUMNS	"COLUMNS"
#define	VARTMPDNAME	"TMPDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"
#define	VARPREXTRA	"EXTRA"

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

#define	PIDFNAME	"run/termnote"
#define	LOGFNAME	"log/termnote"
#define	LOCKFNAME	"spool/locks/termnote"

#define	LOGSIZE		(80*1024)


