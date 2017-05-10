/* config */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)SHOWRECORDS "
#define	BANNER		"Show Records"
#define	SEARCHNAME	"showrecords"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"SHOWRECORDS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SHOWRECORDS_BANNER"
#define	VARSEARCHNAME	"SHOWRECORDS_NAME"
#define	VAROPTS		"SHOWRECORDS_OPTS"
#define	VARFTYPES	"SHOWRECORDS_FTYPES"
#define	VARSUFREQ	"SHOWRECORDS_SUFREQ"
#define	VARSA		"SHOWRECORDS_SUFACC"
#define	VARSR		"SHOWRECORDS_SUFREJ"
#define	VARTARDNAME	"SHOWRECORDS_TARDIR"
#define	VARRFNAME	"SHOWRECORDS_RF"
#define	VARAFNAME	"SHOWRECORDS_AF"
#define	VAREFNAME	"SHOWRECORDS_EF"

#define	VARDEBUGFNAME	"SHOWRECORDS_DEBUGFILE"
#define	VARDEBUGFD1	"SHOWRECORDS_DEBUGFD"
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

#define	TMPDNAME	"/tmp"

#define	CONFIGFNAME	"etc/showrecords/conf"
#define	LOGFNAME	"log/showrecords"
#define	HELPFNAME	"help"

#define	PO_OPTION	"option"
#define	PO_FTS		"fts"			/* file-types */
#define	PO_SUFREQ	"sufreq"		/* suffix-required */
#define	PO_SUFREJ	"sufacc"		/* suffix-reject */
#define	PO_SUFACC	"sufrej"		/* suffix-accept */

#define	MAXNREC		1000000000

#define	BLOCKSIZE	512	/* ALWAYS -- UNIX standard */
#define	MEGABYTE	(1024 * 1024)


