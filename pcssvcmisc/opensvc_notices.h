/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvcmailmsgs "
#define	BANNER		"Open-Service Mail Messages"
#define	SEARCHNAME	"opensvcmailmsgs"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"OPENSVCMAILMSGS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCMAILMSGS_BANNER"
#define	VARSEARCHNAME	"OPENSVCMAILMSGS_NAME"
#define	VARFILEROOT	"OPENSVCMAILMSGS_FILEROOT"
#define	VARLOGTAB	"OPENSVCMAILMSGS_LOGTAB"
#define	VARMSFNAME	"OPENSVCMAILMSGS_MSFILE"
#define	VARUTFNAME	"OPENSVCMAILMSGS_UTFILE"
#define	VARERRORFNAME	"OPENSVCMAILMSGS_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCMAILMSGS_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCMAILMSGS_DEBUGFD"
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

#define	PIDFNAME	"run/opensvcmailmsgs"
#define	LOGFNAME	"var/log/opensvcmailmsgs"
#define	LOCKFNAME	"spool/locks/opensvcmailmsgs"
#define	BBNEWSDNAME	"spool/boards"

#define	LOGSIZE		(80*1024)
#define	COLUMNS		80

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


