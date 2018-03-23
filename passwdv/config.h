/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0b"
#define	WHATINFO	"@(#)PASSWDV "
#define	BANNER		"Password Verification"
#define	SEARCHNAME	"passwdv"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"PASSWDV_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"PASSWDV_BANNER"
#define	VARSEARCHNAME	"PASSWDV_NAME"
#define	VAROPTS		"PASSWDV_OPTS"
#define	VARAFNAME	"PASSWDV_AF"
#define	VAREFNAME	"PASSWDV_EF"

#define	VARDEBUGFNAME	"PASSWDV_DEBUGFILE"
#define	VARDEBUGFD1	"PASSWDV_DEBUGFD"
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

#define	TMPDNAME	"/tmp"

#define	CONFIGFILE	"etc/passwdv/conf"
#define	LOGFNAME	"passwdv"
#define	HELPFNAME	"help"

#define	PO_OPTION	"option"

#define	PROMPTLEN	100


