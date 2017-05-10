/* config */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)MC "
#define	SEARCHNAME	"mc"
#define	BANNER		"Mail Check"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"MC_PROGRAMROOT"
#define	VARPROGRAMROOT2	"PCS"
#define	VARPROGRAMROOT3	"PROGRAMROOT"
#define	VARSEARCHNAME	"mc"
#define	VARDEBUGFD1	"MC_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	HELPFNAME	"help"
#define	CONFFNAME	"conf"
#define	STAMPFNAME	"mc"
#define	MBFNAME		"mbtab"
#define	SRVFNAME	"srvtab"
#define	ACCFNAME	"acctab"
#define	ENVFNAME	"env"
#define	PATHFNAME	"path"
#define	XENVFNAME	"xenv"
#define	XPATHFNAME	"xpath"
#define	USERFNAME	"mc.users"

#define	LOGFNAME	"log/mc"		/* activity log */
#define	PIDFNAME	"var/run/mc"		/* mutex PID file */
#define	SERIALFNAME	"var/serial"		/* serial file */
#define	LOCKFNAME	"spool/locks/mc"	/* lock mutex file */

#define	STAMPDNAME	"spool/timestamps"	/* timestamp directory */
#define	MAILDNAME	"/var/mail"
#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	DEFPATH		"/bin:/usr/sbin"

#define	LOGSIZE		(80*1024)	/* nominal log file length */
#define	MAXJOBS		4		/* maximum jobs at once */
#define	PORTSPEC	"5110"

#define	TI_POLLSVC	30		/* default interval (minutes) */
#define	TI_MINCHECK	(1 * 60)	/* minimal check interval */

#define	PROG_SENDMAIL	"/usr/lib/sendmail"


