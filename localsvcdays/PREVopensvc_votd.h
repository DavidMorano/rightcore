/* config */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvcvotds "
#define	BANNER		"Open Service VOTDS"
#define	SEARCHNAME	"opensvcvotds"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"OPENSVCVOTDS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCVOTDS_BANNER"
#define	VARSEARCHNAME	"OPENSVCVOTDS_NAME"
#define	VARFILEROOT	"OPENSVCVOTDS_FILEROOT"
#define	VARLOGTAB	"OPENSVCVOTDS_LOGTAB"
#define	VARMSFNAME	"OPENSVCVOTDS_MSFILE"
#define	VARUTFNAME	"OPENSVCVOTDS_UTFILE"
#define	VARAFNAME	"OPENSVCVOTDS_AF"
#define	VARNDB		"OPENSVCVOTDS_NDB"
#define	VARVDB		"OPENSVCVOTDS_NDB"
#define	VARSDB		"OPENSVCVOTDS_NDB"
#define	VARPDB		"OPENSVCVOTDS_NDB"
#define	VARLINELEN	"OPENSVCVOTDS_LINELEN"
#define	VAROPTS		"OPENSVCVOTDS_OPTS"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
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

#define	NDBNAME		"english"	/* default name-db */
#define	VDBNAME		"av"		/* default verse-db */
#define	SDBNAME		"bibles"	/* default structure-db */
#define	PDBNAME		"default"	/* default paragraph-db (automatic) */

#define	PIDFNAME	"run/opensvcvotds"
#define	LOGFNAME	"var/log/opensvcvotds"
#define	LOCKFNAME	"spool/locks/opensvcvotds"
#define	MSFNAME		"ms"

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50
#define	DEFPRECISION	5		/* default precision numbers */
#define	DEFNULL		1		/* boolean */

#define	TO_CACHE	2


