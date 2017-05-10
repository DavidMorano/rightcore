/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)biblespeak "
#define	BANNER		"Bible Speak"
#define	SEARCHNAME	"biblespeak"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"BIBLESPEAK_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"BIBLESPEAK_BANNER"
#define	VARSEARCHNAME	"BIBLESPEAK_NAME"
#define	VAROPTS		"BIBLESPEAK_OPTS"
#define	VARFILEROOT	"BIBLESPEAK_FILEROOT"
#define	VARLOGTAB	"BIBLESPEAK_LOGTAB"
#define	VAROPTS		"BIBLESPEAK_OPTS"
#define	VARNDB		"BIBLESPEAK_NDB"
#define	VARVDB		"BIBLESPEAK_VDB"
#define	VARDBDIR	"BIBLESPEAK_DBDIR"
#define	VARLINELEN	"BIBLESPEAK_LINELEN"
#define	VARAFNAME	"BIBLESPEAK_AF"
#define	VAREFNAME	"BIBLESPEAK_EF"

#define	VARDEBUGFNAME	"BIBLESPEAK_DEBUGFILE"
#define	VARDEBUGFD1	"BIBLESPEAK_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARHZ		"HZ"
#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"
#define	VARTZ		"TZ"
#define	VARNAME		"NAME"
#define	VARFULLNAME	"FULLNAME"

#define	VARTMPDNAME	"TMPDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	BBFNAME		"biblebooks"

#define	PIDFNAME	"run/biblespeak"
#define	LOGFNAME	"var/log/biblespeak"
#define	LOCKFNAME	"spool/locks/biblespeak"

#define	NDBNAME		"english"
#define	VDBNAME		"av"

#define	LOGSIZE		(80*1024)

#define	MAXDBENTRIES	32000		/* hack: assume as max DB entries */

#define	DEFPRECISION	5		/* default precision numbers */


