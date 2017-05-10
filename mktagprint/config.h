/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)mktagprint "
#define	BANNER		"Make Tag Print"
#define	SEARCHNAME	"mktagprint"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"MKTAGPRINT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MKTAGPRINT_BANNER"
#define	VARSEARCHNAME	"MKTAGPRINT_NAME"
#define	VAROPTS		"MKTAGPRINT_OPTS"
#define	VARLINELEN	"MKTAGPRINT_LINELEN"
#define	VARERRORFNAME	"MKTAGPRINT_ERRORFILE"

#define	VARDEBUGFNAME	"MKTAGPRINT_DEBUGFILE"
#define	VARDEBUGFD1	"MKTAGPRINT_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
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

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	TMPDNAME	"/tmp"
#define	LOGDNAME	"log"
#define	ETCDIR1		"etc/mktagprint"
#define	ETCDIR2		"etc"

#define	CONFIGFILE1	"mktagprint.conf"
#define	CONFIGFILE2	"conf"
#define	LOGFNAME	"log/mktagprint"
#define	HELPFNAME	"help"
#define	EIGENFNAME	"share/dict/eign"
#define	EIGENFNAME1	"/usr/add-on/ncmp/share/dict/eign"
#define	EIGENFNAME2	"/usr/dict/eign"
#define	BIBLEBOOKS1	"share/misc/biblebooks"
#define	BIBLEBOOKS2	"/usr/add-on/local/share/misc/biblebooks"

#define	LOGSIZE		(80*1024)

/* tuning defaults */

#define	MINWORDLEN	3
#define	MAXWORDLEN	6
#define	EIGENWORDS	10000		/* default maximum eigenwords */
#define	MAXKEYS		100000		/* maximum keys per entry */
#define	NATURALWORDLEN	50
#define	IGNORECHARS	"XYZ"

#define	TABLEN		(2 * 1024)

#define	FE_HASH		"hash"
#define	FE_TAG		"tag"

#ifndef	COLUMNS
#define	COLUMNS		80
#endif



