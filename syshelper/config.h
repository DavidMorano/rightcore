/* config */

/* last modified %G% version %I% */


#define	VERSION		"0b"
#define	WHATINFO	"@(#)SYSHELPER "

#define	VARPROGRAMROOT1	"SYSHELPER_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"syshelperd"

#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ACCFNAME	"acctab"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	REQFNAME	"req"
#define	MSGQFNAME	"msgq"

#define	LOGFNAME	"log/syshelperd"		/* activity log */
#define	PIDFNAME	"spool/run/syshelperd"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/syshelperd"	/* lock mutex file */
#define	SERIALFNAME	"/tmp/serial"

#define	SPOOLDNAME	"spool/syshelperd"
#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	PORTNAME	"syshelper"
#define	PORTNUM		"5108"			/* default TCP port */

#define	LOGSIZE		(80*1024)

#define	BANNER		"System Helper (SYSHELPER)"

#define	PROG_SENDMAIL	"/usr/lib/sendmail"

#define	DEFPATH		"/bin:/usr/sbin"

#define	TO_SVC		60			/* service aquire timeout */
#define	TI_MARKTIME	(3600*12)



