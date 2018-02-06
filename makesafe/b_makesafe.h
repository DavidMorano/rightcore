/* config */


/* revision history:

	= 2004-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)MAKESAFE "
#define	BANNER		"MakeSafe"
#define	SEARCHNAME	"makesafe"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"MAKESAFE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MAKESAFE_BANNER"
#define	VARSEARCHNAME	"MAKESAFE_NAME"
#define	VAROPTS		"MAKESAFE_OPTS"
#define	VARCPP		"MAKESAFE_CPP"
#define	VARINCDIRS	"MAKESAFE_INCDIRS"
#define	VARNPAR		"MAKESAFE_NPAR"
#define	VARAFNAME	"MAKESAFE_AF"
#define	VAREFNAME	"MAKESAFE_EF"
#define	VAROFNAME	"MAKESAFE_OF"
#define	VARIFNAME	"MAKESAFE_IF"
#define	VARDEBUGLEVEL	"MAKESAFE_DEBUGLEVEL"
#define	VARERRORFNAME	"MAKESAFE_ERRORGFILE"

#define	VARDEBUGFNAME	"MAKESAFE_DEBUGFILE"
#define	VARDEBUGFD1	"MAKESAFE_DEBUGFD"
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

#define	CONFIGFNAME	"etc/makesafe/conf"
#define	LOGFNAME	"log/makesafe"
#define	HELPFNAME	"help"
#define	CPPFNAME	"cpp"
#define	TSFNAME		".lastmaint"	/* time-stamp filename */

#define	TO_READ		3600		/* timeout to read CPP output */
#define	TO_TMPFILES	(5*3600)

#define	LOGSIZE		(80*1024)

#define	PROG_CPP	"/usr/ccs/lib/cpp"

#define	HOUR_MAINT	18

#define	OPT_CACHE	TRUE


