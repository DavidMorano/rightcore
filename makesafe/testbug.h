/* config */


/* revision history:

	= 2004-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)TESTBUG "
#define	BANNER		"TestBug"
#define	SEARCHNAME	"testbug"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"TESTBUG_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTBUG_BANNER"
#define	VARSEARCHNAME	"TESTBUG_NAME"
#define	VAROPTS		"TESTBUG_OPTS"
#define	VARCPP		"TESTBUG_CPP"
#define	VARINCDIRS	"TESTBUG_INCDIRS"
#define	VARNPAR		"TESTBUG_NPAR"
#define	VARAFNAME	"TESTBUG_AF"
#define	VAREFNAME	"TESTBUG_EF"
#define	VARERRORFNAME	"TESTBUG_ERRORGFILE"

#define	VARDEBUGFNAME	"TESTBUG_DEBUGFILE"
#define	VARDEBUGFD1	"TESTBUG_DEBUGFD"
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
#define	VARPAGER	"PAGER"
#define	VARNCPU		"NCPU"

#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	LOGFNAME	"log/testbug"
#define	HELPFNAME	"help"
#define	CPPFNAME	"cpp"

#define	TO_READ		3600		/* timeout to read CPP output */
#define	TO_TMPFILES	(5*3600)

#define	LOGSIZE		(80*1024)

#define	PROG_CPP	"/usr/ccs/lib/cpp"

#define	OPT_CACHE	TRUE

#ifndef	TSFNAME
#define	TSFNAME		".lastmaint"
#endif


