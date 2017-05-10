/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)MKKEY "
#define	SEARCHNAME	"mkkey"
#define	BANNER		"Make Keys (MKKEY)"

#define	PROGRAMROOTVAR1	"MKKEY_PROGRAMROOT"
#define	PROGRAMROOTVAR2	"LOCAL"
#define	PROGRAMROOTVAR3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	ETCDIR1		"etc/mkkey"
#define	ETCDIR2		"etc"
#define	CONFIGFILE1	"mkkey.conf"
#define	CONFIGFILE2	"conf"
#define	LOGFNAME	"log/mkkey"
#define	HELPFNAME	"help"
#define	EIGENFNAME	"/usr/dict/eign"

#define	TMPDIR		"/tmp"
#define	LOGSIZE		(80*1024)

/* tuning defaults */

#define	MINWORDLEN	3
#define	MAXWORDLEN	6
#define	EIGENWORDS	1000		/* default maximum eigenwords */
#define	KEYS		100		/* maximum keys per entry */
#define	IGNORECHARS	"XYZ"



