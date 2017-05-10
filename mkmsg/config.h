/* config */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#define	VERSION		"0e"
#define	WHATINFO	"@(#)MKMSG "
#define	BANNER		"Make-Message"
#define	SEARCHNAME	"mkmsg"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"MKMSG_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MKMSG_BANNER"
#define	VARSEARCHNAME	"MKMSG_NAME"
#define	VAROPTS		"MKMSG_OPTS"
#define	VARORG		"MKMSG_ORG"
#define	VARPROGMAILFROM	"MKMSG_MAILFROM"
#define	VARAFNAME	"MKMSG_AF"
#define	VAREFNAME	"MKMSG_EF"

#define	VARDEBUGFNAME	"MKMSG_DEBUGFILE"
#define	VARDEBUGFD1	"MKMSG_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
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
#define	VARHZ		"HZ"
#define	VARTZ		"TZ"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"
#define	VARMAILFROM	"MAILFROM"
#define	VARMAILSENDER	"MAILSENDER"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"

#define	LOGCNAME	"log"

#define	CONFIGFNAME	"etc/mkmsg/conf"
#define	STRMAILERFNAME	"etc/mkmsg/mailer"
#define	DISCLAIMER	"etc/mkmsg/disclaimer"
#define	TYPESFNAME	"etc/mimetypes"
#define	LOGFNAME	"log/mkmsg"
#define	USERFNAME	"log/mkmsg.users"
#define	HELPFNAME	"help"

#define	FACEFNAME	".xface"

#define	STR_FACILITY	"Personal Communication Services"
#define	STR_MAILER	\
	"%{varprname}-%s-%v (%{facility} - %b/%v)"

#define	MIMEVERSION	"1.0"

#define	FROM_ESCAPE	'\b'

#ifndef	ORGLEN
#define	ORGLEN		80
#endif

#define	LOGSIZE		(80*1024)

#define	OPT_LOGPROG	TRUE		/* logging */
#define	OPT_CRNL	TRUE		/* does the stupid CRNL line-trailing */
#define	OPT_MIME	FALSE		/* forces MIME when not needed */


