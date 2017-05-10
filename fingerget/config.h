/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)fingerget "
#define	BANNER		"Remote Finger"

#define	VARPROGRAMROOT1	"FINGERGET_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"FINGERGET_NAME"
#define	VARTMPDNAME	"TMPDIR"

#define	VARDEBUGFD1	"FINGERGET_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"
#define	VARDEBUGFD3	"ERROR_FD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"fingerget"

#define	TMPDNAME	"/tmp"

#define	SYSFNAME1	"sn"
#define	SYSFNAME2	"systems"

#define	LOGDNAME	"log"
#define	CONFFNAME1	"etc/fingerget/fingerget.conf"
#define	CONFFNAME2	"etc/fingerget/conf"
#define	CONFFNAME3	"etc/fingerget.conf"

#define	LOGFNAME	"log/fingerget"
#define	HELPFNAME	"lib/fingerget/help"

#define	SVCSPEC_FINGERGET	"finger"

#define	TO_CONNECT	30
#define	TO_READ		30

#define	LOCALHOST	"localhost"



