/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)TESTUUCP "

#define	PROGRAMROOTVAR1	"TESTUUCP_PROGRAMROOT"
#define	PROGRAMROOTVAR2	"LOCAL"
#define	PROGRAMROOTVAR3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testuucp"

#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	REQFNAME	"req"
#define	MSGQFNAME	"msgq"

#define	LOGFNAME	"log/testuucp"		/* activity log */
#define	PIDFNAME	"spool/run/testuucp"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/testuucp"	/* lock mutex file */

#define	STAMPDIR	"spool/timestamps"	/* timestamp directory */
#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	LOGSTDINFNAME	"/etc/default/login"
#define	INITFNAME	"/etc/default/init"

#define	BANNER		"Test UUCP"

#define	DEFPATH		"/bin:/usr/sbin"

#define	LOGSIZE		(80*1024)

#define	DEBUGFDVAR1	"TESTUUCP_DEBUGFD"
#define	DEBUGFDVAR2	"DEBUGFD"



