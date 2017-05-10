/* config (PCSGETMAIL) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0b"
#define	WHATINFO	"@(#)PCSGETMAIL "
#define	BANNER		"PCS Get Mail"
#define	SEARCHNAME	"pcsgetmail"
#define	VARPNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"PCSGETMAIL_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"PCSGETMAIL_BANNER"
#define	VARSEARCHNAME	"PCSGETMAIL_NAME"
#define	VAROPTS		"PCSGETMAIL_OPTS"
#define	VARLOGFNAME	"PCSGETMAIL_LOGFILE"
#define	VARMB		"PCSGETMAIL_MB"
#define	VARAFNAME	"PCSGETMAIL_AF"
#define	VAREFNAME	"PCSGETMAIL_EF"
#define	VARLFNAME	"PCSGETMAIL_LF"
#define	VARMAILDNAMESP	"PCSGETMAIL_MAILDIRS"
#define	VARMAILUSERSP	"PCSGETMAIL_MAILUSERS"
#define	VARERRORFNAME	"PCSGETMAIL_ERRORFILE"

#define	VARDEBUGFNAME	"PCSGETMAIL_DEBUGFILE"
#define	VARDEBUGFD1	"PCSGETMAIL_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARHOMEDNAME	"HOME"
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
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARMAIL		"MAIL"
#define	VARMBOX		"MBOX"
#define	VARFOLDER	"folder"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"
#define	VARMAILUSERS	"MAILUSERS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define MAILDNAME	"/var/mail"
#define MAILSPOOLDNAME	"/var/mail"
#define	LOGDNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#ifndef	HELPFNAME
#define	HELPFNAME	"help"
#endif

/* mail spool area stuff */

#define	MAILLOCKAGE	(5 * 60)
#define	DEFTIMEOUT	(20 * 4)

#define	LOGSIZE		80000		/* maximum logfile length */

/* local system stuff */

#ifndef	MAILGNAME
#define	MAILGNAME	"mail"
#endif

#ifndef MAILGID
#define MAILGID		6
#endif

#define	ORGDOMAIN	"rightcore.com"

#define	TO_MAILBOX	30
#define	TO_MAILSPOOL	30

#define	PO_MAILUSERS	"mailusers"
#define	PO_MAILDIRS	"maildirs"

#define	USERFSUF	"users"
#define	MAILFOLDER	"mail"
#define	MAILBOX		"new"

#define	OPT_LOGPROG	TRUE


