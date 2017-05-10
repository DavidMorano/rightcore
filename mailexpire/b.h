/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)mailexpire "
#define	SEARCHNAME	"mailexpire"
#define	BANNER		"Mailbox Expire"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"MAILEXPIRE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MAILEXPIRE_BANNER"
#define	VARSEARCHNAME	"MAILEXPIRE_NAME"
#define	VAROPTS		"MAILEXPIRE_OPTS"
#define	VARFILEROOT	"MAILEXPIRE_FILEROOT"
#define	VARLOGTAB	"MAILEXPIRE_LOGTAB"
#define	VARMSGTO	"MAILEXPIRE_MSGTO"
#define	VARMAILUSERSP	"MAILEXPIRE_MAILUSERS"
#define	VARMAILDNAMESP	"MAILEXPIRE_MAILDIRS"
#define	VARCFNAME	"MAILEXPIRE_CF"
#define	VARLFNAME	"MAILEXPIRE_LF"
#define	VARAFNAME	"MAILEXPIRE_AF"
#define	VAREFNAME	"MAILEXPIRE_EF"
#define	VARDEBUGFNAME	"MAILEXPIRE_DEBUGFILE"
#define	VARDEBUGFD1	"MAILEXPIRE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARTMPDNAME	"TMPDIR"
#define	VARHOMEDNAME	"HOME"
#define	VARMAILUSERS	"MAILUSERS"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	MAILDNAME	"/var/mail"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"
#define	LOGCNAME	"log"
#define	FOLDER		"mail"

#define	PIDFNAME	"run/mailexpire"		/* mutex PID file */
#define	LOGFNAME	"var/log/mailexpire"		/* activity log */
#define	LOCKFNAME	"spool/locks/mailexpire"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFMBOX		"trash"

#define	TO_MSGEXPIRE	(120*24*3600)		/* MSG expiration */


