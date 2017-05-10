/* config */

/* last modified %G% version %I% */


#define	VERSION		"0g"
#define	WHATINFO	"@(#)RMAILERD "
#define	BANNER		"Remote Mailer Daemon (RMAILERD)"
#define	SEARCHNAME	"rmailerd"
#define	VARPRNAME	"PCS"

#define	VARPROGRAMROOT1	"RMAILERD_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"RMAILERD_BANNER"
#define	VARSEARCHNAME	"RMAILERD_NAME"
#define	VAROPTS		"RMAILERD_OPTS"
#define	VARAFNAME	"RMAILERD_AF"
#define	VAREFNAME	"RMAILERD_EF"
#define	VARERRORFNAME	"RMAILERD_ERRORFILE"

#define	VARDEBUGFNAME	"RMAILERD_DEBUGFILE"
#define	VARDEBUGFD1	"RMAILERD_DEBUGFD"
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

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"		/* system "init" */

#define	CONFIGFNAME	"conf"
#define	HELPFNAME	"help"
#define	LOGFNAME	"log/rmailerd"			/* activity log */
#define	PIDFNAME	"spool/run/rmailerd"		/* mutex PID file */
#define	LOCKFNAME	"spool/locks/rmailerd"		/* mutex lock file */
#define	SERIALFNAME	"spool/serial"			/* serial file */

#define	PORTNAME	"rmailer"			/* default TCP port */
#define	PORT		5107				/* default TCP port */
#define	LOGSIZE		(80*1024)

#define	PROG_SENDMAIL	"/usr/postfix/bin/sendmail"
#define	NSEND		4				/* number at a time */
#define	PROTONAME	"rmailer"			/* protocol name */

#define	FILEMAGIC	"RMAILER"			/* file magic */
#define	FILEVERSION	0				/* file version */



