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

/**************************************************************************

	This program will act like a filter and remove the mail message
	body and instead write out all of the mail headers only.


***************************************************************************/


#Include	<envstandards.h>

#include	<sys/types.h>
#include	<fcntl.h>

#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	NPARG		1


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile		errfile, *efp = &errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;

	int	rs ;
	int	argl, aol ;
	int	pan, i ;
	int	len, line ;
	int	ex = EX_INFO ;
	int	f_header ;
	int	f_dash = FALSE ;
	int	f_usage = FALSE ;

	char	*argp, *aop ;
	char	*progname ;
	char	*ifname, *ofname ;
	char	*bp, buf[LINEBUFLEN] ;


	progname = argv[0] ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;


	ifname = BFILE_STDIN ;
	ofname = BFILE_STDOUT ;


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

	                akp += 1 ;
	                switch ((int) *aop) {

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
		rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	else
		rs = bopen(ifp,ifname,"r",0666) ;

	if (rs < 0)
		goto badin ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) < 0)
		goto badout ;


/* do program */

	while ((len = breadline(ifp,buf,LINEBUFLEN)) > 0) {

	        if (buf[0] == '\n')
			break ;

		bwrite(ofp,buf,len) ;

	} /* end while */


	bclose(ifp) ;

	bclose(ofp) ;

	bclose(efp) ;

	return OK ;

/* bad things come here */
badin:
	ex = EX_NOINPUT ;
	bprintf(efp,"%s: can't open infile (%d)\n",progname,rs) ;

	goto badret ;

badout:
	ex = EX_CANTCREAT ;
	bprintf(efp,"%s: can't open outfile (%d)\n",progname,rs) ;

	goto badret ;

notenough:
	ex = EX_USAGE ;
	bprintf(efp,"%s: not enough arguments given\n",progname) ;

	goto badret ;

badparam:
	ex = EX_USAGE ;
	bprintf(efp,"%s: bad parameter specified\n",progname) ;

	goto badret ;

usage:
	ex = EX_USAGE ;
	bprintf(efp,"usage: %s [-|file] [-s]\n",
	    progname) ;

badret:
	bclose(efp) ;

	return ex ;
}
/* end subroutine (main) */



