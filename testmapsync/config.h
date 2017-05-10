/* config */


/****************************************************************************

	this is the global header file which is included into every 
	source file.   the declarations here are defined in "defs.c".


******************************************************************************/


#define	VERSION		"0"
#define	WHATINFO	"@(#)TESTMAPSYNC "

#define	VARPROGRAMROOT1	"TESTMAPSYNC_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARDEBUGFD1	"TESTMAPSYNC_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testmapsync"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	TMPDIR		"/tmp"




