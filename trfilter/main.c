/* main */

/* program to filter a TARTS/Traffic report file */
/* last modified %G% version %I% */


#define	VERSION		"0"


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ trfilter <keys> [infile [outfile]]


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#define		NPARG		3

#define		NULL		((char *) 0)
#define		LINELEN		400
#define		NBITBYTES	32


/* external variables */


/* forward references */

static int	makeline() ;


/* exported subroutines */


int main(int argc,cchar *argv,cchar *envv)
{
	bfile		errfile, *efp = &errfile ;
	bfile		infile, *ifp = &infile ;
	int		argl, aol ;
	int		pan, i ;
	int		len, rs ;
	int		ofd ;
	int		f_debug = FALSE ;
	int		f_got ;
	unsigned char	linebuf[LINELEN] ;
	unsigned char	bitmap[NBITBYTES] ;
	unsigned char	*keyp = ((unsigned char *) 0) ;
	const char	*argp, *aop ;
	const char	*progname ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;


	if (bopen(efp,BERR,"wca",0666) < 0) return BAD ;

	progname = argv[0] ;

	pan = 0 ;			/* number of positional so far */
	i = 1 ;
	argc -= 1 ;

	while (argc > 0) {

	    argp = argv[i++] ;
	    argc -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

	                if (cfdec(argp + 1,argl - 1,&rs)) goto badarg ;

	                bprintf(efp,"%s: numeric option ignored\n",
	                    progname) ;

	            } else {

	                aop = argp ;
	                aol = argl ;
	                while (--aol) {

	                    akp += 1 ;
	                    switch ((int) *aop) {

	                    case 'D':
	                        f_debug = TRUE ;
	                        break ;

	                    case 'V':
	                        bprintf(efp,"%s version: %s\n",
	                            progname,VERSION) ;

	                        break ;

	                    default:
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            progname,*aop) ;

	                    case '?':
	                        goto usage ;

	                    } ; /* end switch */

	                } /* end while */
	            }

	        } else {

	            pan += 1 ;	/* increment position count */

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {

	            case 0:
	                if (argl > 0) keyp = (unsigned char *) argp ;

	                break ;

	            case 1:
	                if (argl > 0) ifname = argp ;

	                break ;

	            case 2:
	                if (argl > 0) ofname = argp ;

	                break ;

	            default:
	                break ;
	            }

	            pan += 1 ;

	        } else {

	            bprintf(efp,"%s: extra arguments ignored\n",
	                progname) ;

	        }

	    } /* end if */

	} /* end while */


/* check arguments */

	if (keyp == ((unsigned char *) 0)) goto nokey ;


/* open the input file using Basic I/O */

	if (ifname == NULL) ifname = BFILE_STDIN ;

	if ((rs = bopen(ifp,ifname,"r",0666)) < 0) goto badinfile ;


/* open the output file using raw UNIX interface */

	if (ofname == NULL) {
	    ofd = 1 ;
	} else {
	    ofd = open(ofname,O_WRONLY | O_CREAT | O_APPEND,0664) ;
	}
	if (ofd < 0)
	    goto badoutopen ;


/* prepare a bit map */

	for (i = 0 ; i < NBITBYTES ; i += 1) bitmap[i] = 0 ;

	while (*keyp) {
	    BASET(*keyp,bitmap) ;
	    keyp += 1 ;
	}

	if (f_debug) {
	    bprintf(efp,"%s: bitmap generated\n",progname) ;
	    for (i = 0 ; i < NBITBYTES ; i += 1) {
	        bprintf(efp,"  %02X",bitmap[i]) ;
	        if ((i % 8) == 7) bprintf(efp,"\n") ;
	    }
	}

/* go through the loops */

	while ((len = makeline(ifp,linebuf,LINELEN)) > 0) {

	    f_got = FALSE ;
	    if (len > 2) {
	        if ((linebuf[0] == '@') && BATST(linebuf[1],bitmap)) {
	            f_got = TRUE ;
	            write(ofd,linebuf,len) ;
	        }
	    }

/* if we maxed out on our line buffer, take action */

	    if ((len == LINELEN) && (linebuf[len - 1] != '\n')) {
	        while ((len = makeline(ifp,linebuf,LINELEN)) > 0) {
	            if (f_got) write(ofd,linebuf,len) ;
	            if (linebuf[len - 1] == '\n') break ;
	        }
	    }
	}

	bclose(ifp) ;

	close(ofd) ;

done:
	bclose(efp) ;

	return OK ;

badret:
	bclose(efp) ;

	return BAD ;

usage:
	bprintf(efp,
	    "usage: %s keys [-?] [-V] [-D] [infile [outfile]]\n",
	    progname) ;

	bprintf(efp,"\tkeys	TARTS/Traffic record key letters\n") ;

	bprintf(efp,"\t-?	print this message\n") ;

	bprintf(efp,"\t-V	print program version\n") ;

	bprintf(efp,"\t-D	turn on debug output\n") ;

	bprintf(efp,"\tinfile	input file\n") ;

	bprintf(efp,"\toutfile	output file\n") ;

	goto badret ;

badnumarg:
	bprintf(efp,"%s: not enough arguments specified\n",progname) ;

	goto badret ;

badarg:
	bprintf(efp,"%s: bad argument given\n",progname) ;

	goto badret ;

nokey:
	bprintf(efp,"%s: no key letter were specified\n",
	    progname) ;

	goto badret ;

badinfile:
	bprintf(efp,"%s: cannot open the input file (%d)\n",
	    progname,rs) ;

	goto badret ;

badoutopen:
	bprintf(efp,"%s: cannot open the output file (%d)\n",
	    progname,-errno) ;

	goto badret ;
}
/* end subroutine (main) */


/* local subroutines */


static int makeline(fp,buf,buflen)
bfile	*fp ;
char	*buf ;
int	buflen ;
{
	int		l, ll ;

	if ((ll = breadline(fp,buf,buflen)) <= 0) return ll ;

	while ((buf[ll - 1] != '\n') && (ll < buflen)) {

	    if ((l = breadline(fp,buf + ll,buflen - ll)) < 0)
	        return l ;

	    if (l == 0) return ll ;

	    ll += l ;
	}

	return ll ;
}
/* end subroutine (makeline) */


