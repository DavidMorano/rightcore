/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)CEX "
#define	BANNER		"Cluster Execution"
#define	SEARCHNAME	"cex"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"CEX_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"CEX_BANNER"
#define	VARSEARCHNAME	"CEX_NAME"
#define	VAROPTS		"CEX_OPTS"
#define	VARMODE		"CEX_MODE"
#define	VARAFNAME	"CEX_AF"
#define	VAREFNAME	"CEX_EF"

#define	VARDEBUGFNAME	"CEX_DEBUGFILE"
#define	VARDEBUGFD1	"CEX_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"
#define	VARPAGER	"PAGER"
#define	VARSHELL	"SHELL"
#define	VARRXPORT	"RXPORT"

#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	LOGCNAME	"log"

#define	NETFILE1	"etc/netrc"
#define	NETFILE2	"etc/cex/netrc"
#define	CONFIGFNAME	"etc/cex/conf"		/* not currently used */
#define	LOGFNAME	"log/cex"
#define	HELPFNAME	"help"

#ifndef	NODEFNAME
#define	NODEFNAME	"etc/node"
#endif

#ifndef	CLUSTERFNAME
#define	CLUSTERFNAME	"etc/cluster"
#endif

#define	RXPORT		"DISPLAY,NAME,FULLNAME,PRINTER"

#define	DEFPROGSHELL	"/usr/bin/ksh"

#define	DEFINTKEEP	(3 * 60)	/* sanity check timeout */
#define	DEFEXECSERVICE	512
#define	BRAINDAMAGEWAIT	0		/* some parameter for braindamage */
#define	SANITYFAILURES	5		/* something w/ sanity? */

#define	LOGSIZE		400000

#define	TO_CONNECT	30		/* default connect timeout */
#define	TO_WORM		60		/* TTL for the worm file */


