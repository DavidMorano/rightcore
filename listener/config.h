/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)listener "
#define	BANNER		"Listener"
#define	SEARCHNAME	"listener"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"LISTENER_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"LISTENER_NAME"
#define	CONFVAR		"LISTENER_CONF"
#define	ADDRVAR		"LISTENER_ADDR"
#define	PROGVAR		"LISTENER_PROGRAM"
#define	LOGVAR		"LISTENER_LOG"
#define	USERNAMEVAR	"LISTENER_USERNAME"
#define	PIDVAR		"LISTENER_PID"
#define	VARDEBUGFD1	"LISTENER_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	TMPDNAME	"/tmp"
#define	PIDDIR		"run/listener/"
#define	LOCKDIR		"/tmp/locks/listener/"

#define	CONFIGFNAME1	"etc/listener/listener.conf"
#define	CONFIGFNAME2	"etc/listener/conf"
#define	CONFIGFNAME3	"etc/listener.conf"

#define	LOGFNAME	"log/listener"
#define	HELPFNAME	"lib/listener/help"

#define	DEF_OFFSET	(5*60)		/* default offset 5 minutes */
#define	DEF_TIMEOUT	(8 * 60)	/* default screen blanking timeout */
#define	LOCKTIMEOUT	(5 * 60)	/* lockfile timeout */
#define	DEF_REFRESH	(1 * 60)	/* automatic refresh interval */
#define	DEF_MAILTIME	(1 * 60)	/* active mail display time */

#define	LOGSIZE		(80 * 1024)

#define	PROG_MAILPOLL	"/etc/bin/mailpoll"
#define	PROG_LPD	"/usr/lib/print/in.lpd"
#define	PROG_ECHOD	"/usr/net/servers/echod"

#define	DEFUSERNAME	"daemon"

#define	DEFUID		1
#define	DEFGID		1


