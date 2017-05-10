/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)MAILPOP "

#define	VARPROGRAMROOT1	"MAILPOP_PROGRAMROOT"
#define	VARPROGRAMROOT2	"PCS"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARDEBUGFD1	"MAILPOP_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	SEARCHNAME	"mailpop"

#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	REQFNAME	"req"
#define	MSGQFNAME	"msgq"

#define	LOGFNAME	"log/mailpop"		/* activity log */
#define	PIDFNAME	"spool/run/mailpop"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/mailpop"	/* lock mutex file */

#define	STAMPDIR	"spool/timestamps"	/* timestamp directory */
#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	BANNER		"Mail Popper"

#define	DEFPATH		"/bin:/usr/sbin"

#define	LOGSIZE		(80*1024)

#define	PORTSPEC_ECHO		"echo"
#define	PORTSPEC_MAILPOLL	"mailpoll"

#define	SVCSPEC_ECHO		"mailpoll"
#define	SVCSPEC_MAILPOLL	"mailpoll"

#define	PORT_ECHO		7
#define	PORT_MAILPOLL		5110

#define	DIALTIME	20



