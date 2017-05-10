/* last modified %G% version %I% */

/* default options for the RSX daemon program */

/*
	David A.D. Morano
	November 1991
*/


#define		VERSION		"0"

#define		DEFCONFIGFILE	"sattab"
#define		DEFCONDIR	"con"
#define		DEFWORKDIR	"/usr/tmp"
#define		DEFLOGFILE	"satlog"
#define		DEFWAIT		250

#define		NAMELEN		32	/* maximum file name length */
#define		PATHLEN		150	/* maximum path_file_name length */

#define		LINELEN		100
#define		BUFLEN		200
#define		NLENV		40	/* number of "local" environment
					things */
#define		ENVLEN		2000	/* buffer space for environment */

#define		LOCKPREFIX	"LK_"	/* lock file prefix */
#define		TMPPREFIX	"sc_"	/* 'tmp' file prefix */
#define		TMPTRIES	100	/* N tries to get 'tmp' file */


/*************************************************************************

#	The following substitutions are made on command strings :
#		%a	general arguments
#		%c	control lock directory
#		%w	working directory
#		%m	mail address
#

**************************************************************************/

struct expand {
	char	*a ;
	char	*m ;
	char	*c ;
	char	*w ;
} ;


