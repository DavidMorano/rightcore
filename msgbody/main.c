/* main */

/* filter out all but the body of a mail message */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1986-07-01, David A­D­ Morano

	This subroutine was originally written.


*/


/**************************************************************************

	This program will act like a filter and remove mail headers
	from the front of the input file and only write to the output
	the mail body of the input.


***************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<bfile.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"



/* local defines */

#define	LINELEN		256
#define	NPARG		1



/* external subroutines */






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile	errfile, *efp = &errfile ;
	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;

	int	argl, aol ;
	int	pan ;
	int	rs, i ;
	int	len, line ;
	int	ex = EX_INFO ;
	int	f_header ;
	int	f_dash = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;

	char	*argp, *aop ;
	char	*progname ;
	char	*ifname, *ofname ;
	char	*bp, buf[LINELEN] ;


	progname = argv[0] ;
	if (bopen(efp,BFILE_STDERR,"wca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;


	ifname = ((char *) 0) ;
	ofname = ((char *) 1) ;


	rs = SR_OK ;

	pan = 0 ;			/* number of positional so far */
	i = 1 ;
	argc -= 1 ;
	while ((rs >= 0) && (argc > 0)) {

	    argp = argv[i++] ;
	    argc -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            aop = argp ;
	            aol = argl ;

	            while (--aol) {

	                akp += 1 ;
	                switch ((uint) *aop) {

	                case 'V':
				f_version = TRUE ;
	                    bprintf(efp,"%s: version %s\n",progname,VERSION) ;

	                    break ;

	                default:
	                    printf("%s: unknown option - %c\n",progname,
	                        *aop) ;

/* we fall through to the next case on purpose */

	                case '?':
	                    f_usage = TRUE ;
	                    break ;

	                } /* end switch */

	            } /* end while */

	        } else {

	            f_dash = TRUE ;
	            pan += 1 ;		/* increment position count */

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {

	            case 0:
	                ifname = argp ;
	                break ;

	            case 1:
	                ofname = argp ;
	                break ;

	            } /* end switch */

	            pan += 1 ;

	        } else {

	            bprintf(efp,"%s: extra arguments ignored\n",progname) ;

	        }

	    } /* end if */

	} /* end while */


	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto retearly ;


/* check the arguments */

	if (f_dash) 
		ifname = (char *) BFILE_STDIN ;

	if ((rs = bopen(ifp,ifname,"r",0666)) < 0) 
		goto badin ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) < 0) {

		bclose(ifp) ;

		goto badout ;
	}


/* do program */

	f_header = TRUE ;
	while ((rs = breadline(ifp,buf,LINELEN)) > 0) {

		len = rs ;
	    if (f_header == TRUE) {

	        if (buf[0] == '\n') 
			f_header = FALSE ;

	    } else 
		rs = bwrite(ofp,buf,len) ;

	    if (rs < 0)
		break ;

	} /* end while */

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

done:
	bclose(ifp) ;

	bclose(ofp) ;

retearly:
	bclose(efp) ;

	return ex ;

/* bad stuff */
badin:
	ex = EX_NOINPUT ;
	bprintf(efp,"%s: can't open infile (%d)\n",progname,rs) ;

	goto retearly ;

badout:
	bprintf(efp,"%s: can't open outfile (%d)\n",progname,rs) ;

	goto retearly ;

usage:
	bprintf(efp,"usage: %s [-|file] [-s]\n",
	    progname) ;

	goto retearly ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



