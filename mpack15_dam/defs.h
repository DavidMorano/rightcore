/* defs */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<netdb.h>
#include	<signal.h>
#include	<ctype.h>

#include	<system.h>
#include	<bfile.h>


#ifndef	BUFLEN
#define	BUFLEN		200
#endif



struct gflags {
	uint		verbose : 1 ;
	uint		quiet : 1 ;
	uint		noinput : 1 ;
} ;

struct global {
	char		*progname ;
	char		*version ;
	char		*tmpdir ;
	char		*programroot ;
	char		*nodename ;
	char		*domainname ;
	char		*hdr_subject ;
	char		*hdr_mailer ;
	bfile		*efp ;
	bfile		*ofp ;
	struct gflags	f ;
	int		debuglevel ;
} ;



