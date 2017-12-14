/* config */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0b"
#define	WHATINFO	"@(#)rfinger "
#define	BANNER		"Remote Finger"
#define	SEARCHNAME	"rfinger"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"RFINGER_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"RFINGER_BANNER"
#define	VARSEARCHNAME	"RFINGER_NAME"
#define	VAROPTS		"RFINGER_OPTS"
#define	VARLINELEN	"RFINGER_LINELEN"
#define	VARAFNAME	"EFINGER_AF"
#define	VAREFNAME	"RFINGER_EF"

#define	VARDEBUGFNAME	"RFINGER_DEBUGFILE"
#define	VARDEBUGFD1	"RFINGER_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"
#define	VARDEBUGFD3	"ERROR_FD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARHZ		"HZ"
#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARDOMAIN	"DOMAIN"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARTERM		"TERM"
#define	VARPRINTER	"PRINTER"
#define	VARLPDEST	"LPDEST"
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARNAME		"NAME"
#define	VARFULLNAME	"FULLNAME"
#define	VARTZ		"TZ"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"
#define	VARPATH		"PATH"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	LOGCNAME	"log"

#define	SYSFNAME1	"sn"
#define	SYSFNAME2	"systems"

#define	LOGDNAME	"log"
#define	CONFFNAME1	"etc/rfinger/rfinger.conf"
#define	CONFFNAME2	"etc/rfinger/conf"
#define	CONFFNAME3	"etc/rfinger.conf"

#define	LOGFNAME	"log/rfinger"
#define	HELPFNAME	"lib/rfinger/help"

#define	LOCALHOST	"localhost"

#define	SVCSPEC_RFINGER	"finger"

#define	DEFPRECISION	5		/* default precision numbers */

#define	LOGSIZE		(80*1024)

#define	TO_OPEN		10
#define	TO_READ		5


