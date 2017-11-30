/* config */


/* revision history:

	= 2009-05-01, David A­D­ Morano

	This code was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testdialerprog "
#define	SEARCHNAME	"testdialerprog"
#define	BANNER		"Test Dialer Program"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"TESTDIALERPROG_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"TESTDIALERPROG_NAME"
#define	VARBANNER	"TESTDIALERPROG_BANNER"
#define	VAROPTS		"TESTDIALERPROG_OPTS"
#define	LOGHOSTVAR	"TESTDIALERPROG_LOGHOST"
#define	VARSVC		"TESTDIALERPROG_SVC"

#define	VARERRORFNAME	"TESTDIALERPROG_ERRORFILE"
#define	VARDEBUGFNAME	"TESTDIALERPROG_DEBUGFILE"
#define	VARDEBUGFD1	"TESTDIALERPROG_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

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
#define	VARPATH		"PATH"
#define	VARMANPATH	"MANPATH"
#define	VARCDPATH	"CDPATH"
#define	VARLIBPATH	"LD_LIBRARY_PATH"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	SYSFNAME1	"sn"
#define	SYSFNAME2	"systems"

#define	CONFFNAME1	"etc/testdialerprog/testdialerprog.conf"
#define	CONFFNAME2	"etc/testdialerprog/conf"
#define	CONFFNAME3	"etc/testdialerprog.conf"

#define	LOGFNAME	"log/testdialerprog"

#define	HELPFNAME	"help"

#define	SVCSPEC_TESTDIALERPROG	"testdialerprog"

#define	MAILHOST	"www.rightcore.com"

#define	LOGHOST		"www.rightcore.com"		/* default LOGHOST */
#define	LOGPRIORITY	"user.info"
#define	LOGTAG		""

#define	LOCKTIMEOUT	(5 * 60)	/* lockfile timeout */


