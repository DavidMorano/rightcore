/* config */


/****************************************************************************

	this is the global header file which is included into every 
	source file.   the declarations here are defined in "defs.c".


******************************************************************************/


#define	VERSION		"0"
#define	WHATINFO	"@(#)TESTFSPIPE "

#define	VARDEBUGFD1	"TESTFSPIPE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#ifndef	LOGFILE
#define	LOGFILE		"log/testfspipe"
#endif

#ifndef	USERFILE
#define	USERFILE	"log/testfspipe.users"
#endif

#ifndef	HELPFILE
#define	HELPFILE	"lib/testfspipe/help"
#endif



#define	TMPDIR		"/tmp"

 

