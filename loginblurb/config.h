/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)LOGINBLURB "
#define	BANNER		"Login Blurb"
#define	SEARCHNAME	"loginblurb"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"LOGINBLURB_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"LOGINBLURB_BANNER"
#define	VARSEARCHNAME	"LOGINBLURB_NAME"
#define	VAROPTS		"LOGINBLURB_OPTS"
#define	VARSTRING	"LOGINBLURB_STR"
#define	VARMNTFNAME	"LOGINBLURB_MNTFILE"
#define	VARAFNAME	"LOGINBLURB_AF"
#define	VAREFNAME	"LOGINBLURB_EF"

#define	VARDEBUGFNAME	"LOGINBLURB_DEBUGFILE"
#define	VARDEBUGFD1	"LOGINBLURB_DEBUGFD"
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
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARTERM		"TERM"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	LOGCNAME	"log"

#define	HELPFNAME	"help"
#define	LOGFNAME	"log/loginblurb"
#define	USERFILE	"log/loginblurb.users"
#define	CMDHELPFNAME	"lib/loginblurb/cmdhelp"

#define	OUTPUTDEV	"/dev/fd/1"
#define	MSGLOGDEV	"/dev/msglog"
#define	CONSOLEDEV	"/dev/console"

#define	TO_OPEN		5
#define	TO_POLL		10
#define	TO_TMPX		3
#define	TO_SYSMISC	3
#define	TO_CHECK	2
#define	TO_WAIT		20

 
