/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)vcardadm "
#define	BANNER		"V-Card Administration"

#define	VARPROGRAMROOT1	"VCARDADM_PROGRAMROOT"
#define	VARPROGRAMROOT2	"PCS"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"VCARDADM_NAME"
#define	VARFILEROOT	"VCARDADM_FILEROOT"
#define	VARLOGTAB	"VCARDADM_LOGTAB"

#define	VARDEBUGFD1	"VCARDADM_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	SEARCHNAME	"vcardadm"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/vcardadm"		/* mutex PID file */
#define	LOGFNAME	"var/log/vcardadm"	/* activity log */
#define	LOCKFNAME	"spool/locks/vcardadm"	/* lock mutex file */

#define	VCARDADMDBNAME	"var/dmail"

#define	LOGSIZE		(80*1024)

#define	DEFNENTRIES	20



