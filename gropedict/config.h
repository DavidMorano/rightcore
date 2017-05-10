/* config */
/* ancillary support program for the EXPTOOLS GROPE program */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)GROPEDICT "
#define	BANNER		"Grope Dictionary Updater"
#define	SEARCHNAME	"gropedict"
#define	VARPRNAME	"TOOLS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/exptools"
#endif

#define	VARPROGRAMROOT1	"GROPEDICT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"GROPEDICT_NAME"
#define	VARBANNER	"GROPEDICT_BANNER"
#define	VAROPTS		"GROPEDICT_OPTS"
#define	VARDICTDNAME	"GROPEDICT_DICTDIR"
#define	VARERRORFNAME	"GROPEDICT_ERRORFILE"

#define	VARDEBUGFNAME	"GROPEDICT_DEBUGFILE"
#define	VARDEBUGFD1	"GROPEDICT_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARTMPDNAME	"TMPDIR"

#define	TMPDNAME	"/tmp"
#define	LOGCNAME	"log"
#define	DICTDNAME	"lib/grope"

#define	ETCDIR1		"etc/grope"
#define	ETCDIR2		"etc"
#define	CONFIGFNAME1	"grope.conf"
#define	CONFIGFNAME2	"conf"
#define	LOGFNAME	"log/grope-dict"
#define	HELPFNAME	"grope-dict.help"

#define	LOGSIZE		(80*1024)

#define	PREFIX		"dic_"

#define	NOUTFILES	10		/* number of output files */
#define	MAXWORDLEN	70		/* maximum word length */
#define	NOOUTFILES	30

#define	PO_SUFFIX	"suffix"
#define	PO_OPTION	"option"


