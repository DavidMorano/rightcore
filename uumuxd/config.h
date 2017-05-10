/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)UUMUXD "

#define	VARPROGRAMROOT1	"UUMUXD_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	CONFIGDIR1	"etc/uumuxd"
#define	CONFIGDIR2	"etc"
#define	CONFIGFILE1	"uumuxd.conf"
#define	CONFIGFILE2	"conf"

#define	SRVFILE1	"etc/uumuxd/uumuxd.srvtab"
#define	SRVFILE2	"etc/uumuxd/srvtab"
#define	LOGFILE		"log/uumuxd"		/* activity log */
#define	PIDFILE		"spool/run/uumuxd"	/* mutex PID file */
#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	LOGSIZE		(80*1024)

#define	BANNER		"UUMUX Daemon (UUMUXD)"

#define	SRVTO		60			/* service aquire timeout */

#define	PROG_SENDMAIL	"/usr/lib/sendmail"



