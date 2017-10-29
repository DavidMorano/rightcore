/* defs */



#include	<sys/types.h>

#include	<bfile.h>
#include	<paramopt.h>
#include	<logfile.h>

#include	"misc.h"


#ifndef	BUFLEN
#define	BUFLEN	MAXPATHLEN
#endif


#define	MEGABYTE	(1024 * 1024)
#define	UNIXBLOCK	512

#define	SUFFIX		"suffix"
#define	OPTION		"option"




struct proginfo_flags {
	uint	log : 1 ;
	uint	nochange : 1 ;
	uint	quiet : 1 ;
	uint	suffix : 1 ;
	uint	follow : 1 ;		/* follow symbolic links */
} ;

struct proginfo {
	struct proginfo_flags	f ;
	logfile		lh ;
	bfile		*efp ;
	bfile		*ofp ;
	char		*progname ;
	char		*version ;
	char		*searchname ;
	char		*programroot ;
	char		*tmpdir ;
	uint		bytes ;
	uint		megabytes ;
	int		debuglevel ;
	int		verboselevel ;
	int		namelen ;
} ;


struct checkparams {
	struct proginfo	*pip ;
	PARAMOPT	*pp ;
} ;




