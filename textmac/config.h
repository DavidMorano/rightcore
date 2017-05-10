/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)textmac "
#define	BANNER		"Text Macintosh"
#define	SEARCHNAME	"textmac"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"TEXTMAC_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TEXTMAC_BANNER"
#define	VARSEARCHNAME	"TEXTMAC_NAME"

#define	VARFILEROOT	"TEXTMAC_FILEROOT"
#define	VARLOGTAB	"TEXTMAC_LOGTAB"

#define	NODEVAR		"NODE"
#define	CLUSTERVAR	"CLUSTER"
#define	SYSTEMVAR	"SYSTEM"
#define	PRINTERVAR	"PRINTER"

#define	VARDEBUGFD1	"TEXTMAC_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/textmac"		/* mutex PID file */
#define	LOGFNAME	"var/log/textmac"	/* activity log */
#define	LOCKFNAME	"spool/locks/textmac"	/* lock mutex file */

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	PO_OPTION	"option"


