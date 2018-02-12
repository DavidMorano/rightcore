/* config (rbbpost) */


#ifndef	CONFIG_INCLUDE
#define	CONFIG_INCLUDE	1


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"1d"
#define	WHATINFO	"@(#)RBBPOST "
#define	BANNER		"Bulletin Board Post"
#define	SEARCHNAME	"rbbpost"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"RBBPOST_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"RBBPOST_BANNER"
#define	VARSEARCHNAME	"RBBPOST_NAME"
#define	VAROPTS		"RBBPOST_OPTS"
#define	VARPROGMAILFROM	"RBBPOST_MAILFROM"
#define	VARNEWSDNAME	"RBBPOST_NEWSDIR"
#define	VARAFNAME	"RBBPOST_AF"
#define	VAREFNAME	"RBBPOST_EF"
#define	VARIFNAME	"RBBPOST_IF"
#define	VARCFNAME	"RBBPOST_CF"
#define	VARLFNAME	"RBBPOST_LF"

#define	VARDEBUGFNAME	"RBBPOST_DEBUGFILE"
#define	VARDEBUGFD1	"RBBPOST_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"
#define	VARBBOPTS	"RBBOPTS"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARUSERNAME	"USERNAME"
#define	VARFULLNAME	"FULLNAME"
#define	VARNAME		"NAME"
#define	VARTERM		"TERM"
#define	VARPRINTER	"PRINTER"
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARMAILFROM	"MAILFROM"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	MKJOBDIR1	"/tmp/locks"
#define	MKJOBDIR2	"/tmp/jobs"
#define	STAMPDNAME	"var/timestamps"		/* timestamps */
#define	NEWSDNAME	"spool/boards"			/* news directory */
#define	BBNEWSDNAME	"spool/boards"
#define	DEADDNAME	"spool/bb/dead"
#define	FORWARDDNAME	"spool/bb/forward"		/* outgoing */
#define	CONTROLDNAME	"spool/bb/control"		/* incoming */

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	VCNAME		"var"
#define	LOGCNAME	"log"
#define	ARTCNAME	".arts"

#define	LOCKFNAME	"spool/locks/bbdircache"	/* lock mutex file */
#define	PIDFNAME	"var/run/bbdircache"		/* mutex PID file */
#define	SERIALFNAME	"var/serial"			/* serial file */
#define	LOGFNAME	"log/bbdircache"		/* activity log */
#define	BBHOSTSFNAME	"etc/bb/bbhosts"
#define	BBNAMESFNAME	"etc/bb/bbnames"

#define	HELPFNAME	"help"
#define	CONFFNAME	"conf"
#define	STAMPFNAME	"bbdircache"
#define	SRVFNAME	"srvtab"
#define	ACCFNAME	"acctab"
#define	ENVFNAME	"env"
#define	PATHFNAME	"path"
#define	XENVFNAME	"xenv"
#define	XPATHFNAME	"xpath"
#define	USERFNAME	"bbdircache"
#define	DIRCACHEFNAME	".dircache"

#define	USERFSUF	"users"
#define	DEFNEWSGROUP	"misc"
#define	DEADNG		"dead"
#define	SERVICE		"bbpost"	/* for dialing out */
#define	MAILHOST	"mailhost"

#define	PROG_MAILER	"pcsmail"
#define	PROG_RSLOW	"rslow"
#define	PROG_UNLINKD	"unlinkd"

#define	LOGSIZE		(80*1024)	/* nominal log file length */

#define	TI_POLLSVC	(5 * 60)	/* default interval (minutes) */
#define	TI_MINCHECK	(1 * 60)	/* minimal check interval */

#define	TO_PIDLOCK	5

#endif /* CONFIG_INCLUDE */



