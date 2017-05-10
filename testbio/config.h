/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)TESTBFILE "
#define	SEARCHNAME	"testbio"
#define	BANNER		"Test Bfile"

#define	PROGRAMROOTVAR1	"TESTBFILE_PROGRAMROOT"
#define	PROGRAMROOTVAR2	"LOCAL"
#define	PROGRAMROOTVAR3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	REQFNAME	"req"
#define	MSGQFNAME	"msgq"

#define	LOGFNAME	"log/testbio"		/* activity log */
#define	PIDFNAME	"spool/run/testbio"		/* mutex PID file */
#define	LOCKFNAME	"spool/locks/testbio"	/* lock mutex file */

#define	STAMPDIR	"spool/timestamps"	/* timestamp directory */
#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	LOGSTDINFNAME	"/etc/default/login"
#define	INITFNAME	"/etc/default/init"

#define	PROG_SENDMAIL	"/usr/lib/sendmail"

#define	DEFPATH		"/bin:/usr/sbin"

#define	LOGSIZE		(80*1024)
#define	DEFINTERVAL	5			/* default interval (minutes) */
#define	MAXJOBS		5			/* maximum jobs at once */


