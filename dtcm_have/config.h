/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)dtcm_have "
#define	SEARCHNAME	"dtcmhave"
#define	BANNER		"Desktop Calendar Have"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"DTCMHAVE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"DTCMHAVE_NAME"
#define	VAROPTS		"DTCMHAVE_OPTS"
#define	LOGHOSTVAR	"DTCMHAVE_LOGHOST"
#define	VARSVC		"DTCMHAVE_SVC"

#define	VARERRORFNAME	"DTCMHAVE_ERRORFILE"
#define	VARDEBUGFNAME	"DTCMHAVE_DEBUGFILE"
#define	VARDEBUGFD1	"DTCMHAVE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"
#define	VARDEBUGFD3	"ERROR_FD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SYSFNAME1	"sn"
#define	SYSFNAME2	"systems"

#define	TMPDNAME	"/tmp"
#define	CALDNAME	"/var/spool/calendar"

#define	CONFFNAME1	"etc/dtcmhave/dtcmhave.conf"
#define	CONFFNAME2	"etc/dtcmhave/conf"
#define	CONFFNAME3	"etc/dtcmhave.conf"
#define	LOGFNAME	"log/dtcmhave"

#define	HELPFNAME	"help"

#define	SVCSPEC_DTCMHAVE	"dtcmhave"

#ifndef	IPPORT_DTCMHAVE
#define	IPPORT_DTCMHAVE		5115
#endif

#define	MAILHOST	"www.rightcore.com"

#define	LOGHOST		"www.rightcore.com"		/* default LOGHOST */
#define	LOGPRIORITY	"user.info"
#define	LOGTAG		""

#define	LOCKTIMEOUT	(5 * 60)	/* lockfile timeout */



