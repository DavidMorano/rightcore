/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)xtmpdirs "
#define	BANNER		"X TMP Directories"
#define	SEARCHNAME	"xtmpdirs"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"XTMPDIRS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"XTMPDIRS_BANNER"
#define	VARSEARCHNAME	"XTMPDIRS_NAME"
#define	VAROPTS		"XTMPDIRS_OPTS"
#define	VARFILEROOT	"XTMPDIRS_FILEROOT"
#define	VARLFNAME	"XTMPDIRS_LG"
#define	VARLOGTAB	"XTMPDIRS_LOGTAB"
#define	VARAFNAME	"XTMPDIRS_AF"
#define	VAREFNAME	"XTMPDIRS_EF"
#define	VARERRORFNAME	"XTMPDIRS_ERRORFILE"

#define	VARDEBUGFNAME	"XTMPDIRS_DEBUGFILE"
#define	VARDEBUGFD1	"XTMPDIRS_DEBUGFD"
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
#define	VARFPATH	"FPATH"
#define	VARMAIL		"MAIL"
#define	VARMBOX		"MBOX"
#define	VARFOLDER	"folder"
#define	VARMAILFROM	"MAILFROM"
#define	VARMAILREPLY	"MAILREPLY"
#define	VARNCPU		"NCPU"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"
#define	VARPWD		"PWD"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"
#define	SERIALFNAME	"serial"

#define	PIDFNAME	"run/xtmpdirs"		/* mutex PID file */
#define	LOGFNAME	"var/log/xtmpdirs"		/* activity log */
#define	LOCKFNAME	"spool/locks/xtmpdirs"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	TMPGNAME	"sys"
#define	TMPGID		3


