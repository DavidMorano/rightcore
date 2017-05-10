/* main */

/* test the various email address objects */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revistion history:

	= 2003-05-08, David A­D­ Morano


*/



#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"address.h"
#include	"emainfo.h"

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2028
#endif

#ifdef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif


/* external subroutines */

extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local variables */

static unsigned char	tterms[] = {
	0x00, 0x1B, 0x00, 0x00,
	0x01, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static unsigned char	dterms[] = {
	0x00, 0x1B, 0x00, 0x00,
	0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static char	*const types[] = {
	"address",
	"emainfo",
	"mailaddr",
	"addrinfo",
	NULL
} ;

enum types {
	type_address,
	type_emainfo,
	type_mailaddr,
	type_addrinfo,
	type_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;

	EMAINFO	ei ;

	FIELD	f ;

	int	rs = SR_OK ;
	int	i, n ;
	int	len, sl ;
	int	itype, otype ;
	int	fd_debug ;

	const char	*cp ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	lineout[LINEBUFLEN + 1] ;
	char	mahost[MAILADDRLEN + 1] ;
	char	malocal[MAILADDRLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	bopen(ifp,BFILE_STDIN,"dr",0666) ;

	bopen(ofp,BFILE_STDOUT,"dwct",0666) ;


	bprintf(ofp,"email address test program\n") ;


/* loop stuff */

	while (TRUE) {

	    int		dlen ;

	    char	*dp ;


	    bprintf(ofp,"types :\n") ;

	    for (i = 0 ; i < type_overlast ; i += 1) {

	            bprintf(ofp,"\t%d\t%s\n",i,types[i]) ;

	    } /* end for */

	    bprintf(ofp,
	        "enter type and address> ") ;

	    if ((len = breadline(ifp,linebuf,LINEBUFLEN)) <= 0)
	        break ;

	while ((len > 0) && isspace(linebuf[len - 1]))
		len -= 1 ;

	linebuf[len] = '\0' ;
	    field_start(&f,linebuf,len) ;

/* get the type */

	    field_get(&f,tterms) ;

	    itype = f.fp[0] - '0' ;

	    if (f.term == ',') {

	        field_get(&f,tterms) ;

	        otype = f.fp[0] - '0' ;

	    } /* end if (getting output type) */

	    dp = f.lp ;
	dlen = f.rlen ;
	while ((dlen > 0) && isspace(dp[0])) {

		dp += 1 ;
		dlen -= 1 ;
	}

	    if (dlen > 0) {

	        bprintf(ofp,
	            "\rconverting %s=>%t<\n",types[itype],dp,dlen) ;

	        switch (itype) {

	        case type_address:
	            rs = addressparse(dp,dlen,mahost,malocal) ;

	            break ;

	        case type_emainfo:
		    rs = emainfo(&ei,dp,dlen) ;

	            break ;

	        case type_mailaddr:


	        case type_addrinfo:


	        default:
	            rs = SR_NOTSUP ;
	            bprintf(ofp,"unknown input type\n") ;

	        } /* end switch */


	        if (rs >= 0) {

			switch (itype) {

			case type_address:
	                bprintf(ofp,"type=%d\n",rs) ;

	                bprintf(ofp,"host=%s\n",mahost) ;

	                bprintf(ofp,"local=%s\n",malocal) ;

				break ;

			case type_emainfo:
	                bprintf(ofp,"type=%d\n",ei.type) ;

	                bprintf(ofp,"host=%t\n",ei.host,ei.hlen) ;

	                bprintf(ofp,"local=%t\n",ei.local,ei.llen) ;

			break ;

			} /* end switch */

	        } else
	            bprintf(ofp,"bad conversion (rs %d)\n",rs) ;

	    } else
	        bprintf(ofp,"no string given to convert\n") ;

	    field_finish(&f) ;

	} /* end while */


	bclose(ofp) ;

	bclose(ifp) ;

	return EX_OK ;
}
/* end subroutine (main) */



