/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)PCSUSERDOMAINS "
#define	BANNER		"PCS User Domains"
#define	SEARCHNAME	"pcsuserdomains"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"PCSUSERDOMAINS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"PCSUSERDOMAINS_BANNER"
#define	VARSEARCHNAME	"PCSUSERDOMAINS_NAME"
#define	NEWOPTS		"PCSUSERDOMAINS_OPTS"
#define	NEWSDIRVAR	"PCSUSERDOMAINS_NEWSDIR"

#define	VARAFNAME	"PCSUSERDOMAINS_AF"
#define	VAREFNAME	"PCSUSERDOMAINS_EF"
#define	VARDEBUGFD1	"PCSUSERDOMAINS_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	NEWSDNAME	"var/spool/boards"		/* news directory */
#define	STAMPDNAME	"var/timestamps"		/* timestamps */
#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"

#define	LOGFNAME	"log/pcsuserdomains"		/* activity log */
#define	PIDFNAME	"var/run/pcsuserdomains"
#define	SERIALFNAME	"var/serial"			/* serial file */
#define	LOCKFNAME	"var/spool/locks/pcsuserdomains"

#define	HELPFNAME	"help"
#define	CONFFNAME	"conf"
#define	STAMPFNAME	"pcsuserdomains"
#define	SRVFNAME	"srvtab"
#define	ACCFNAME	"acctab"
#define	ENVFNAME	"env"
#define	PATHFNAME	"path"
#define	XENVFNAME	"xenv"
#define	XPATHFNAME	"xpath"
#define	USERFNAME	"pcsuserdomains.users"
#define	DIRCACHEFNAME	".dircache"

#define	DEFPATH		"/bin:/usr/sbin"

#define	LOGSIZE		(80*1024)	/* nominal log file length */
#define	MAXJOBS		4		/* maximum jobs at once */

#define	TI_POLLSVC	(5 * 60)	/* default interval (minutes) */
#define	TI_MINCHECK	(1 * 60)	/* minimal check interval */

#define	TO_PIDLOCK	5

#define	PROG_SENDMAIL	"/usr/lib/sendmail"


