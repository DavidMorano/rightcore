/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */



#define	VERSION		"0c"
#define	WHATINFO	"@(#)CHUNK "
#define	BANNER		"Chunk File"
#define	SEARCHNAME	"chunk"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"CHUNK_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"CHUNK_BANNER"
#define	VARSEARCHNAME	"CHUNK_NAME"
#define	VAROPTS		"CHUNK_OPTS"
#define	VARAFNAME	"CHUNK_AF"
#define	VAREFNAME	"CHUNK_EF"

#define	VARDEBUGFNAME	"CHUNK_DEBUGFILE"
#define	VARDEBUGFD1	"CHUNK_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
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

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	CONFIGFNAME	"etc/chunk/conf"	/* not currently used */
#define	LOGFNAME	"log/chunk"
#define	HELPFNAME	"log/chunk"

#define	TMPDNAME	"/tmp"

#define	DEFCHUNKLEN	(1*1024*1024)

#define	PO_SUFFIX	"suffix"
#define	PO_OPTION	"option"


