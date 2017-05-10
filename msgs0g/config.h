/* config */

/* MSGS configuration */


#define	VERSION		"0h"
#define	WHATINFO	"@(#)MSGS "
#define	BANNER		"Messages"
#define	SEARCHNAME	"msgs"
#define	VARPRNAME	"PCS"

#define	VARPROGRAMROOT1	"MSGS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MSGS_BANNER"
#define	VARNAME		"MSGS_NAME"
#define	VAROPTS		"MSGS_OPTS"
#define	VARAFNAME	"MSGS_AF"
#define	VAREFNAME	"MSGS_EF"
#define	VARERRORFNAME	"MSGS_ERRORFILE"

#define	VARDEBUGFNAME	"MSGS_DEBUGFILE"
#define	VARDEBUGFD1	"MSGS_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	LOGCNAME	"log"

#define	LOGFNAME	"log/msgs"
#define	USERFNAME	"log/msgs.users"
#define SPOOLDIR1	"var/spool/msgs"	/* alternate spool directory */
#define SPOOLDIR2	"/var/spool/msgs"	/* spool directory */
#define	CMDHELPFNAME	"lib/msgs/cmdhelp"

#define PROG_MAILER	"mailx"
#define PROG_PAGER	"more"			/* CRT screen paging program */
#define	PROG_METAMAIL	"metamail"

#define BOUNDS1		".msgs_bounds"		/* message bounds file */
#define BOUNDS2		"bounds"		/* message bounds file */
#define MSGSRC		".msgsrc"		/* user's RC file */
#define TEMP		"/tmp/msgXXXXXXXXXXXX"

#define	ORGANIZATION	"Lucent technologies"

#define NDAYS		90			/* default keep time */
#define NLINES		24			/* default CRT screen LINES */

#define	LOCKTIMEOUT	20			/* bounds file timeout (sec) */

#define	GROUP_ADMIN	"pcs"			/* administration group */



