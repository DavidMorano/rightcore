/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)RANDOMD "

#define	VARPROGRAMROOT1	"RANDOMD_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"randomd"

#define	CONFIGFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"

#define	LOGFNAME	"log/randomd"		/* activity log */
#define	PIDFNAME	"spool/run/randomd"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/randomd"	/* lock file */

#define	DEVICEFNAME	"/dev/random"
#define	SEEDFNAME	"/var/tmp/random"

#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	BANNER		"Random Daemon (RANDOMD)"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	LOGSIZE		(80*1024)
#define	DEFPATH		"/bin:/usr/sbin"

#define	PROG_PS		"/bin/ps -ef"

#define	PORTNAME	"random"
#define	PORT		5112			/* default TCP port */



