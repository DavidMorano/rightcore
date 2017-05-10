/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)PCSUUCPD "

#define	VARPROGRAMROOT1	"PCSUUCPD_PROGRAMROOT"
#define	VARPROGRAMROOT2	"PCS"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	SEARCHNAME	"pcsuucpd"

#define	CONFFNAME	"conf"
#define	PASSWDFNAME	"passwd"
#define	SRVFNAME	"srvtab"
#define	ACCFNAME	"acctab"
#define	ENVFNAME	"env"
#define	PATHSFNAME	"paths"
#define	REQFNAME	"req"
#define	MSGQFNAME	"msgq"

#define	LOGFNAME	"log/pcsuucpd"		/* activity log */
#define	PIDFNAME	"var/run/pcsuucpd"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/pcsuucpd"	/* lock mutex file */
#define	SERIALFNAME	"spool/serial"

#define	SPOOLDNAME	"spool/pcsuucpd"
#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	PORTNAME	"pcsuucp"
#define	PORTNUM		"5111"			/* default TCP port */

#define	LOGSIZE		(80*1024)

#define	BANNER		"PCS UUCP Daemon (PCSUUCPD)"

#define	DEFPATH		"/bin:/usr/sbin"

#define	PROG_SENDMAIL	"/usr/lib/sendmail"

#define	TO_SVC		60			/* service aquire timeout */
#define	TI_MARKTIME	(3600*12)

#define	LOGIN_TEXT	"RightCore network services\n"
#define	LOGIN_PROMPT	"UUCP login: "
#define	PASSWORD_PROMPT	"password: "

#define	VARDEBUGFD	"PCSUUCPD_DEBUGFD"



