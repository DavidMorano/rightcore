/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	BANNER		"Remote Slow Service Daemon (RSLOWD)"

#define	VARPROGRAMROOT	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	DEFCONFIGFILE1	"rslowd.conf"
#define	DEFCONFIGFILE2	"conf"
#define	DEFDIRECTORY	"q"
#define	DEFINTERRUPT	"i"
#define	DEFSRVFILE1	"rslowd.srvtab"
#define	DEFSRVFILE2	"srvtab"
#define	DEFLOGFILE	"log/rslowd"	/* activity log */
#define	DEFPIDFILE	"rslowd.pid"	/* mutex lock file */
#define	DEFWORKDIR	"/tmp"
#define	DEFTMPDIR	"/tmp"
#define	DEFPOLLTIME	250		/* in seconds */
#define	JOBIDLETIME	20		/* seconds */
#define	SRVIDLETIME	7		/* seconds */
#define	DEFLOGID	"*"
#define	DEFMAXJOBS	10		/* default maximum number of jobs */



