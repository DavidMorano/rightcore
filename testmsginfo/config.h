/* config */



#define	VERSION		"0"
#define	WHATINFO	"@(#)testmsginfo "
#define	BANNER		"Test Message Information"

#define	VARPROGRAMROOT1	"TESTMSGINFO_PROGRAMROOT"
#define	VARPROGRAMROOT2	"PCS"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"TESTMSGINFO_NAME"
#define	VARPROGMODE	"TESTMSGINFO_MODE"
#define	VAROPTS		"TESTMSGINFO_OPTS"
#define	VARMAILBOX	"TESTMSGINFO_MAILBOX"

#define	VARDEBUGFD1	"TESTMSGINFO_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	SEARCHNAME	"testmsginfo"

#define	TMPDNAME	"/tmp"
#define SPOOLDNAME	"/var/mail"

#define	HELPFNAME	"help"
#define	MBFNAME		"mbtab"
#define	WLFNAME		"whitelist"
#define	BLFNAME		"blacklist"

#define	SERIALFNAME	"var/serial"
#define	COMSATFNAME	"etc/testmsginfo.nodes"
#define	SPAMFNAME	"etc/testmsginfo.spam"
#define	LOGFNAME	"log/testmsginfo"
#define	USERFNAME	"log/testmsginfo.users"
#define	LOGFNAME	"log/testmsginfo"
#define	LOGENVFNAME	"log/testmsginfo.env"
#define	LOGZONEFNAME	"log/testmsginfo.zones"
#define	RMTABFNAME	"/etc/rmtab"
#define	MSGIDDBNAME	"var/testmsginfo"

#define	MAILGROUP	"mail"

#define	MAILGID		6

#define	DIVERTUSER	"adm"

#define	LINELEN		256
#define	FIELDLEN	4096

#define	LOGSIZE		(80*1024)

#define	MAILLOCKAGE	(5 * 60)

#define	TO_LOCK		(10 * 60)
#define	TO_MSGREAD	10

#define	MAXMSGID	490

#define	PORTSPEC_COMSAT		"biff"
#define	PORTSPEC_MAILPOLL	"mailpoll"

#define	EMA_POSTMASTER	"postmaster"



