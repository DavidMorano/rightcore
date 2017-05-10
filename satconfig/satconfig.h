/* last modified %G% version %I% */

/* default options for the SATconfig program */

/*
	David A.D. Morano
	November 1991
*/


#define		VERSION		"0"

#define		DEFCONFIGFILE	"sattab"
#define		DEFLIBDIR	"lib"
#define		DEFCONDIR	"con"
#define		DEFWORKDIR	"/usr/tmp"
#define		DEFRELEASE	"default"
#define		DEFLOGFILE	"satlog"
#define		DEFFILTFILE	"default.f"
#define		DEFMAPFILE	"default.m"
#define		DEFDICTFILE	"default.d"
#define		DEFWAIT		250

#define		NUMSCRIPTS	50	/* maximum number of script files */
#define		NSYSTEM		30

#define		NAMELEN		14	/* maximum file name length */
#define		PATHLEN		150	/* maximum path_file_name length */

#define		LINELEN		100
#define		BUFLEN		200
#define		NLENV		40
#define		ENVLEN		2000

#define		LOCKPREFIX	"LK_"
#define		TMPPREFIX	"sc_"
#define		TMPTRIES	100


/*************************************************************************

#	The following substitutions are made on command strings :
#		%a	file name argument
#		%s	system name
#		%f	SAT filter file
#		%m	SAT mapping file
#		%d	SAT dictionary file
#		%l	library directory
#		%c	control lock directory
#		%w	working directory
#		%r	SUT release
#

**************************************************************************/

struct expand {
	char	*a ;
	char	*s ;
	char	*f ;
	char	*m ;
	char	*d ;
	char	*l ;
	char	*c ;
	char	*w ;
	char	*r ;
} ;


