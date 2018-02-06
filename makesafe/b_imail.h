/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)imail "
#define	BANNER		"Inject Mail"
#define	SEARCHNAME	"imail"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"IMAIL_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"IMAIL_BANNER"
#define	VARSEARCHNAME	"IMAIL_NAME"
#define	VAROPTS		"IMAIL_OPTS"
#define	VARAFNAME	"IMAIL_AF"
#define	VAREFNAME	"IMAIL_EF"
#define	VARIFNAME	"IMAIL_IF"
#define	VARCFNAME	"IMAIL_CF"
#define	VARCONFIG	"IMAIL_CONF"
#define	VARFILEROOT	"IMAIL_FILEROOT"
#define	VARLOGFNAME	"IMAIL_LOGFILE"
#define	VARLOGTAB	"IMAIL_LOGTAB"
#define	VARMSFNAME	"IMAIL_MSFILE"
#define	VARUTFNAME	"IMAIL_UTFILE"
#define	VARIMAILFROM	"IMAIL_MAILFROM"
#define	VARMAILHOST	"IMAIL_MAILHOST"

#define	VARDEBUGFNAME	"IMAIL_DEBUGFILE"
#define	VARDEBUGFD1	"IMAIL_DEBUGFD"
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
#define	VARMAILREPLYTO	"MAILREPLYTO"
#define	VARMAILERRORSTO	"MAILERRORSTO"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"
#define	VARPREXTRA	"EXTRA"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	VCNAME		"var"
#define	LOGCNAME	"log"
#define	LOGDNAME	"log"
#define	FOLDERDNAME	"mail"
#define	RUNDNAME	"var/run"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	LOGFNAME	"imail"			/* activity log */
#define	PIDFNAME	"imail"			/* mutex PID file */
#define	SERIALFNAME	"serial"
#define	TSFNAME		".lastmaint"		/* time-stamp filename */

#define	MAILERNAME	"imail (RightCore Network Services)"

#define	SUBJ_MAILNOTE	"MAILNOTE"	/* used in MAILNOTE mode */

#define	MAILHOST	"mailhost"

#ifndef	ORGLEN
#define	ORGLEN		MAXNAMELEN
#endif

#define	LOGSIZE		(160*1024)

#define	MAXPOSTARGS	4

#define	HOUR_MAINT	18

#define	DEFINTRUN	60
#define	DEFINTPOLL	8
#define	DEFNODES	50

#define	MB_COPY		"copy"
#define	MB_DEAD		"dead"

#define	PROG_RMAIL	"rmail"
#define	PROG_POSTMAIL	"postmail"
#define	PROG_SENDMAIL	"sendmail"

/* this is only used in MAILNOTE mode */
#define	UTI_APPLENOTE	"com.apple.mail-note"

#define	OPT_DELIVER	TRUE		/* default should be to deliver */
#define	OPT_CMBNAME	TRUE		/* copy mailbox name ("copy") */
#define	OPT_MAILER	TRUE		/* include 'x-mailer' header */
#define	OPT_ORG		TRUE		/* add org-header if not present */
#define	OPT_SENDER	TRUE		/* allow SENDER header */
#define	OPT_REQSUBJ	TRUE		/* require a "subject" */
#define	OPT_ADDSUBJ	TRUE		/* add a "subject" if we have one */
#define	OPT_ADDSENDER	FALSE		/* add a "sender" address */
#define	OPT_ADDFROM 	TRUE		/* add a "from" if we have one */
#define	OPT_ONEFROM 	TRUE		/* allow only one "from" */
#define	OPT_ONESENDER	TRUE		/* allow only one "sender" */
#define	OPT_TAKE	TRUE		/* operate in "take" mode */
#define	OPT_USECLEN	TRUE		/* use "clen" */
#define	OPT_USECLINES	FALSE		/* use "clines" */


