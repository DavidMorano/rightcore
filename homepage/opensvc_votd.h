/* opensvc_votdc */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvcvotdc "
#define	BANNER		"Open Service VOTDC"
#define	SEARCHNAME	"opensvcvotdc"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"OPENSVCVOTDC_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCVOTDC_BANNER"
#define	VARSEARCHNAME	"OPENSVCVOTDC_NAME"
#define	VAROPTS		"OPENSVCVOTDC_OPTS"
#define	VARLANG		"OPENSVCVOTDC_LANG"
#define	VARFILEROOT	"OPENSVCVOTDC_FILEROOT"
#define	VARLTAB		"OPENSVCVOTDC_LF"
#define	VARAFNAME	"OPENSVCVOTDC_AF"
#define	VARNDB		"OPENSVCVOTDC_NDB"
#define	VARVDB		"OPENSVCVOTDC_NDB"
#define	VARSDB		"OPENSVCVOTDC_NDB"
#define	VARPDB		"OPENSVCVOTDC_NDB"
#define	VARLINELEN	"OPENSVCVOTDC_LINELEN"

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

#define	PIDFNAME	"run/opensvcvotdc"
#define	LOGFNAME	"var/log/opensvcvotdc"
#define	LOCKFNAME	"spool/locks/opensvcvotdc"
#define	MSFNAME		"ms"

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50
#define	DEFPRECISION	5		/* default precision numbers */
#define	DEFLANG		"english"

#define	TO_CACHE	2

#define	OPT_DEFNULL	1		/* boolean */
#define	OPT_BOOKNAME	1		/* boolean */
#define	OPT_SEPARATE	0		/* boolean */
#define	OPT_TRYCACHE	1		/* boolean */


