/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)TESTBFILE "
#define	SEARCHNAME	"testbfile"
#define	BANNER		"Test Bfile"

#define	PROGRAMROOTVAR1	"TESTBFILE_PROGRAMROOT"
#define	PROGRAMROOTVAR2	"LOCAL"
#define	PROGRAMROOTVAR3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARBANNER	"TESTBFILE_BANNER"
#define	VARSEARCHNAME	"TESTBFILE_NAME"
#define	VAROPTS		"TESTBFILE_OPTS"
#define	VARFILEROOT	"TESTBFILE_FILEROOT"
#define	VARLOGTAB	"TESTBFILE_LOGTAB"
#define	VARMSFNAME	"TESTBFILE_MSFILE"
#define	VARAFNAME	"TESTBFILE_AF"
#define	VAREFNAME	"TESTBFILE_EF"

#define	VARDEBUGFNAME	"TESTBFILE_DEBUGFILE"
#define	VARDEBUGFD1	"TESTBFILE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"

#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	REQFNAME	"req"
#define	MSGQFNAME	"msgq"

#define	LOGFNAME	"log/testbfile"		/* activity log */
#define	PIDFNAME	"spool/run/testbfile"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/testbfile"	/* lock mutex file */

#define	STAMPDIR	"spool/timestamps"	/* timestamp directory */
#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	INITFNAME	"/etc/default/init"
#define	LOGSTDINFNAME	"/etc/default/login"

#define	DEFPATH		"/bin:/usr/sbin"

#define	LOGSIZE		(80*1024)


