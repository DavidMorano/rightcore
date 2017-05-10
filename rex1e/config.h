/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Updated for for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"1e"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	NETFILE1	"etc/netrc"
#define	NETFILE2	"etc/rex/netrc"
#define	CONFIGFILE	"etc/rex/config"	/* not currently used */
#define	LOGFNAME	"log/rex"

#define	TMPDIR		"/tmp"
#define	RXPORT		"DISPLAY,NAME,FULLNAME,PRINTER"

#define	DEFKEEPTIME	(3 * 60)		/* sanity check timeout */
#define	DEFEXECSERVICE	512

#define	BRAINDAMAGEWAIT	3


