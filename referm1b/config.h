/* config */


#define	VERSION		"1b"
#define	WHATINFO	"@(#)REFERM "

#define	VARPROGRAMROOT1	"LOCAL"
#define	VARPROGRAMROOT2	"REFERM_PROGRAMROOT"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"referm"

#define	ETCDIR1		"etc/referm"
#define	ETCDIR2		"etc"
#define	CONFIGFILE1	"referm.conf"
#define	CONFIGFILE2	"conf"
#define	LOGFNAME	"log/referm"
#define	HELPFNAME	"lib/referm/helpfile"
#define	EIGENFNAME	"/usr/dict/eign"
#define	REFER		"/usr/lib/refer/papers"

#define	TMPDIR		"/tmp"
#define	LOGSIZE		(80*1024)

#define	BANNER		"Refer for MM Macros (REFERM)"


#define	MINWORDLEN	3
#define	MAXWORDLEN	6
#define	EIGENWORDS	1000
#define	KEYS		100
#define	IGNORECHARS	"XYZ"

#define	MACRONAME1	".IBR"
#define	MACRONAME2	".BK"

#define	PROG_LOOKBIB	"glookbib"

#define	DEFDATABASE	REFER

#define	MAXDB		5



