/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)MKTMPUSER "
#define	BANNER		"Make Temporary User Directory"
#define	SEARCHNAME	"mktmpuser"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"MKTMPUSER_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MKTMPUSER_BANNER"
#define	VARSEARCHNAME	"MKTMPUSER_NAME"
#define	VAROPTS		"MKTMPUSER_OPTS"
#define	VARJOBDNAME	"MKTMPUSER_DNAME"
#define	VARAFNAME	"MKTMPUSER_AF"
#define	VAREFNAME	"MKTMPUSER_EF"
#define	VARERRORFNAME	"MKTMPUSER_ERRORFILE"

#define	VARDEBUGFNAME	"MKTMPUSER_DEBUGFILE"
#define	VARDEBUGFD1	"MKTMPUSER_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARHZ		"HZ"
#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARDOMAIN	"DOMAIN"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARTERM		"TERM"
#define	VARPRINTER	"PRINTER"
#define	VARLPDEST	"LPDEST"
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARNAME		"NAME"
#define	VARFULLNAME	"FULLNAME"
#define	VARTZ		"TZ"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	TMPDNAME	"/tmp"

#define	MKJOB1DNAME	"var/spool/locks"
#define	MKJOB2DNAME	"/tmp/pcs/locks"
#define	MKJOB3DNAME	"/tmp/locks"

#define	LOGFNAME	"log"
#define	HELPFNAME	"help"

#define	TRIES		2		/* tries of 'mkjobfile()' */
#define	WAITTIME	3		/* time to wait if 'mkjobfile' fails */
#define	JOBTIME		(5 * 60)	/* default time job file stays around */
#define	DMODE		0775		/* job file creation mode */



