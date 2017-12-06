/* main */

/* grab the mailbody of a message file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0
#define	CF_DEBUG	0


/* revision history: 

	= 1986-07-01, David A­D­ Morano
	This subroiutine was originally written.

*/

/* Copyright © 1986 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program will act like a filter and remove the mail message
	body and instead write out all of the mail headers only.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<fcntl.h>

#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	NPARG		1


/* external subroutines */


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile		errfile, *efp = &errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;

	int		rs = SR_OK ;
	int	argl, aol ;
	int	pan, i ;
	int	len, line ;
	int	f_header ;
	int	f_dash = FALSE ;
	int	f_usage = FALSE ;

	cchar		*argp, *aop ;
	cchar		*progname ;
	cchar		*ifname, *ofname ;
	char		*bp
	char		buf[LINEBUFLEN] ;


	progname = argv[0] ;

	if (bopen(efp,BIO_STDERR,"dwca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;

	ifname = BIO_STDIN ;
	ofname = BIO_STDOUT ;


	pan = 0 ;			/* number of positional so far */
	i = 1 ;
	argc -= 1 ;

	while (argc > 0) {

	    argp = argv[i++] ;
	    argc -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            aop = argp ;
	            aol = argl ;

	            while (--aol) {

	                aop += 1 ;
	                switch (tolower(*aop)) {

	                case 'V':
	                    bprintf(efp,"%s: version %s\n",progname,VERSION) ;

	                    break ;

	                case '?':
	                    f_usage = TRUE ;
	                    break ;

	                default:
	                    printf("%s: unknown option - %c\n",progname,
	                        *aop) ;

	                    goto usage ;

	                } /* end switch */

	            }

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


	if (f_usage) goto usage ;


/* check the arguments */

	if (f_dash)
		rs = bopen(ifp,BIO_STDIN,"dr",0666) ;

	else
		rs = bopen(ifp,ifname,"r",0666) ;

	if (rs < 0)
		goto badin ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) < 0)
		goto badout ;


/* do program */

	while ((len = breadline(ifp,buf,LINEBUFLEN)) > 0) {
	        if (buf[0] == '\n') break ;
		bwrite(ofp,buf,len) ;
	} /* end while */


	bclose(ifp) ;

	bclose(ofp) ;

	bclose(efp) ;

	return OK ;

/* bad things come here */
badin:
	bprintf(efp,"%s: can't open infile (%d)\n",progname,rs) ;

	goto badret ;

badout:
	bprintf(efp,"%s: can't open outfile (%d)\n",progname,rs) ;

	goto badret ;

notenough:
	printf(efp,"%s: not enough arguments given\n",progname) ;

	goto badret ;

badparam:
	bprintf(efp,"%s: bad parameter specified\n",progname) ;

	goto badret ;

usage:
	bprintf(efp,"usage: %s [-|file] [-s]\n",
	    progname) ;

badret:
	fclose(efp) ;

	return BAD ;
}
/* end subroutine (main) */



