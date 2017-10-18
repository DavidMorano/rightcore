/* config */


/* revision history:

	= 1998-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)cksumpass "
#define	BANNER		"CheckSum Pass"
#define	SEARCHNAME	"cksumpass"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"CKSUMPASS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"CKSUMPASS_BANNER"
#define	VARSEARCHNAME	"CKSUMPASS_NAME"
#define	VAROPTS		"CKSUMPASS_OPTS"
#define	VARFILEROOT	"CKSUMPASS_FILEROOT"
#define	VARRFNAME	"CKSUMPASS_RF"
#define	VARLFNAME	"CKSUMPASS_LF"
#define	VARAFNAME	"CKSUMPASS_AF"
#define	VAREFNAME	"CKSUMPASS_EF"

#define	VARDEBUGFNAME	"CKSUMPASS_DEBUGFILE"
#define	VARDEBUGFD1	"CKSUMPASS_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARTMPDNAME	"TMPDIR"
#define	NODEVAR		"NODE"
#define	CLUSTERVAR	"CLUSTER"
#define	SYSTEMVAR	"SYSTEM"
#define	PRINTERVAR	"PRINTER"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/cksumpass"		/* mutex PID file */
#define	LOGFNAME	"var/log/cksumpass"	/* activity log */
#define	LOCKFNAME	"spool/locks/cksumpass"	/* lock mutex file */

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	MAXNREC		100000000

#ifndef	BLOCKSIZE
#define	BLOCKSIZE	512	/* ALWAYS -- UNIX standard */
#endif

#ifndef	MEGABYTE
#define	MEGABYTE	(1024 * 1024)
#endif


