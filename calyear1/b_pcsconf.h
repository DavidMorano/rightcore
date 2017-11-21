/* config */

/* last modified %G% version %I% */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0d"
#define	WHATINFO	"@(#)PCSCONF "
#define	BANNER		"PCS Configuration"
#define	SEARCHNAME	"pcsconf"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"PCSPOLL_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"PCSCONF_BANNER"
#define	VARSEARCHNAME	"PCSCONF_NAME"
#define	VARDEBUGLEVEL	"PCSCONF_DEBUGLEVEL"
#define	VAROPTS		"PCSCONF_OPTS"
#define	VARCFNAME	"PCSCONF_CF"
#define	VARAFNAME	"PCSCONF_AF"
#define	VAREFNAME	"PCSCONF_EF"
#define	VARERRORFNAME	"PCSCONF_ERRORFILE"

#define	VARDEBUGFNAME	"PCSCONF_DEBUGFILE"
#define	VARDEBUGFD1	"PCSCONF_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPCSORG	"PCSORG"
#define	VARPCSUSERORG	"PCSUSERORG"
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

#define	TMPDNAME	"/tmp"
#define	VCNAME		"var"
#define	LOGCNAME	"log"
#define	SERIALFNAME	"serial"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFILE1	"etc/pcs.conf"
#define	CONFIGFILE2	"etc/conf"

#define	LOGFNAME	"log/pcsconf"		/* activity log */
#define	HELPFNAME	"help"

#define	USERFSUF	"user"

#define	USAGECOLS	4

#define	LOGSIZE		(80*1024)


