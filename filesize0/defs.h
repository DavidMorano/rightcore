/* defs */

/* rmlinks header */



#include	<sys/types.h>
#include	<sys/stat.h>
#include	<setjmp.h>
#include	<signal.h>
#include	<pwd.h>
#include	<ctype.h>
#include	<dirent.h>

#ifndef	INC_BIO
#include	<bfile.h>
#endif



#ifndef	BUFLEN
#define	BUFLEN	MAXPATHLEN
#endif

#define	MEGABYTE	(1024 * 1024)



struct gflags {
	uint	verbose : 1 ;
	uint	nochange : 1 ;
	uint	quiet : 1 ;
} ;

struct global {
	bfile		*efp ;
	bfile		*ofp ;
	struct gflags	f ;
	long		bytes ;
	long		megabytes ;
	int		debuglevel ;
	int		namelen ;
	int		suffixlen ;
	char		*progname ;
	char		*tmpdir ;
	char		*suffix ;
} ;



