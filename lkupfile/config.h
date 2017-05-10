/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)lkupfile "
#define	BANNER		"Look Up File"

#define	VARPROGRAMROOT1	"LKUPFILE_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARDBFILE	"LKUPFILE_FILE"
#define	VARLPGET	"LKUPFILE_LPGET"
#define	VARBANNER	"LKUPFILE_BANNER"
#define	VARSEARCHNAME	"LKUPFILE_NAME"

#define	VARFILEROOT	"LKUPFILE_FILEROOT"
#define	VARLOGTAB	"LKUPFILE_LOGTAB"

#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARPRINTER	"PRINTER"
#define	VARLPDEST	"LPDEST"

#define	VARDEBUGFD1	"LKUPFILE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"lkupfile"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	PIDDNAME	"var/run"
#define	LOGDNAME	"var/log"
#define	LOCKDNAME	"var/spool/locks"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	PDBFNAME	"pdb"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	LOGFNAME	"lkupfile"			/* activity log */
#define	PIDFNAME	"lkupfile"			/* mutex PID file */
#define	LOCKFNAME	"lkupfile"			/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	DEFPRINTER	"_default"
#define	DEFPRINTERKEY	"use"

#define	PO_OPTION	"option"
#define	PO_SUFFIX	"suffix"


