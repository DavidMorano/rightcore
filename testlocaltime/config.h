/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)TESTLOCALTIME "

#define	VARPROGRAMROOT1	"TESTLOCALTIME_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	SEARCHNAME	"testlocaltime"

#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	REQFNAME	"req"
#define	MSGQFNAME	"msgq"

#define	LOGFNAME	"log/testlocaltime"		/* activity log */
#define	PIDFNAME	"spool/run/testlocaltime"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/testlocaltime"	/* lock mutex file */

#define	STAMPDIR	"spool/timestamps"
#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	BANNER		"Personal Communication Services (PCS) Poll Program"

#define	PROG_SENDMAIL	"/usr/lib/sendmail"

#define	DEFPATH		"/bin:/usr/sbin"

#define	LOGSIZE		(80*1024)
#define	DEFINTERVAL	5			/* default interval (minutes) */
#define	MAXJOBS		5			/* maximum jobs at once */

#define	ECHOPORT	517



