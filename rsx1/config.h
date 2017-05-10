/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)REX"
#define	BANNER		"Remote EXecution"

#define	VARPROGRAMROOT1	"REX_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"rec"

#define	CONFIGDIR1	"etc/rec"
#define	CONFIGDIR2	"etc"

#define	CONFIGFILE1	"rec.conf"
#define	CONFIGFILE2	"conf"

#define	SRVFILE1	"etc/rec/rec.srvtab"
#define	SRVFILE2	"etc/rec/srvtab"

#define	ENVFILE		"environ"
#define	PATHSFILE	"paths"

#define	LOGFILE		"log/rec"		/* activity log */
#define	PIDFILE		"spool/run/rec"	/* mutex PID file */

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	LOGSTDINFNAME	"/etc/default/login"
#define	INITFNAME	"/etc/default/init"

#define	PORTNAME	"tcpmux"
#define	PORT		5108			/* default TCP port */

#define	LOGSIZE		(80*1024)

#define	SRVTO		60			/* service aquire timeout */

#define	PROG_SENDMAIL	"/usr/lib/sendmail"



