/* program to convert a file to PC format */

/*
	David A.D. Morano
	December 1990
*/


#define		VERSION		"0"

#define		DPRINT		0


#include	<bfile.h>

#include	"localmisc.h"


#define		LINELEN		200
#define		NPARG		2


extern int	bopen(), bclose() ;
extern int	bread(), bwrite() ;
extern int	breadline(), bprintf() ;



int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile	in, *ifp = &in ;
	bfile	out, *ofp = &out ;
	bfile	err, *efp = &err ;

	long	argl, aol ;

	int	pan, i ;
	int	rs, len ;
	int	f_dash = FALSE ;
	int	f_bin = FALSE, f_to = FALSE, f_from = FALSE ;

	char	*argp, *aop ;
	char	*progname ;
	char	buf[LINELEN] ;
	char	*infname, *outfname ;


	progname = argv[0] ;

	if ((rs = bopen(efp,BERR,"wca",0666)) < 0) return rs ;

	infname = (char *) 0 ;
	outfname = (char *) 1 ;

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
	                switch ((int) *aop) {

	                case 'b':
	                    f_bin = TRUE ;
	                    break ;

			case 'd':
	                case 'f':
	                    f_from = TRUE ;
	                    break ;

			case 'u':
	                case 't':
	                    f_to = TRUE ;
	                    break ;

	                case 'V':
	                    bprintf(efp,"%s version: %s\n",
				progname,VERSION) ;

	                    break ;

	                default:
	                    bprintf(efp,"%s: unknown option - %c\n",
	                        progname,*argp) ;

			case '?':
	                    goto usage ;

	                } /* end switch */

	            }

	        } else {

	            f_dash = TRUE ;
	            pan += 1 ;

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {

	            case 0:
	                infname = argp ;
	                break ;

	            case 1:
	                outfname = argp ;
	                break ;

	            default:
	                break ;

	            }

	            pan += 1 ;

	        } else {

	            bprintf(efp,"%s: extra arguments ignored\n",progname) ;

	        }

	    } /* end if */

	} /* end while */


/* check arguments */

	if (f_dash) infname = (char *) 0 ;

	if ((! f_from) && (! f_to)) f_to = TRUE ;

	if ((f_from) && (f_to)) {

	    bprintf(efp,"%s: ambiguous conversion request\n") ;

	    goto usage ;
	}

	rs = bopen(ifp,infname,"r",0666) ;
	if (rs < 0) goto bad_open ;

	rs = bopen(ofp,outfname,"wct",0666) ;
	if (rs < 0) goto bad_open ;

	if (f_bin) {

	    while ((len = bread(ifp,buf,LINELEN)) > 0) 
		bwrite(ofp,buf,len) ;

	} else if (f_to) {

#if	DPRINT
	    bprintf(efp,"converting to\n") ;
#endif

/* convert file to PC format */

	    while ((len = breadline(ifp,buf,LINELEN)) > 0) {

	        if (buf[len - 1] == '\n') {

	            rs = bwrite(ofp,buf,len - 1) ;
	            if (rs < 0) goto bad_out ;

	            rs = bwrite(ofp,"\r\n",2) ;
	            if (rs < 0) goto bad_out ;

	        } else {

	            rs = bwrite(ofp,buf,len) ;
	            if (rs < 0) goto bad_out ;

	        }

	    }

/* put the control Z at the end */

	    bputc(ofp,26) ;

	} else {

#if	DPRINT
	    bprintf(efp,"converting from\n") ;
#endif

/* convert file from PC format */

	    while ((len = breadline(ifp,buf,LINELEN)) > 0) {

#if	DPRINT
	        bprintf(efp,"len = %d\n",len) ;
#endif

	        if (len >= 2) {

#if	DPRINT
	            bprintf(efp,"last character - %02X\n",
	                buf[len - 1]) ;

	            bprintf(efp,"second to last character - %02X\n",
	                buf[len - 2]) ;

	            if (len >= 3)

	                bprintf(efp,"third to last character - %02X\n",
	                    buf[len - 3]) ;
#endif

	            if (buf[len - 2] == '\r') {

	                bwrite(ofp,buf,(len - 2)) ;

	                bputc(ofp,'\n') ;

	            } else bwrite(ofp,buf,len) ;

	        } else if (buf[0] != 26) bwrite(ofp,buf,len) ;

	    } /* end while */

	} /* end if */


/* finish up and get out */
done:
	bclose(ifp) ;

	bclose(ofp) ;

early:
	bclose(efp) ;

	return OK ;

not_enough:
	bprintf(efp,"%s: not enough arguments given\n",progname) ;

	goto bad_ret ;

usage:
	bprintf(efp,
		"%s usage: pcconv [-t | -f] [infile [outfile]]\n",
		progname) ;

	bprintf(efp,
		"\t-t\t- convert file to DOS format from UNIX format\n%s",
		"\t-f\t- convert file from DOS format to UNIX format\n") ;

	goto bad_ret ;

bad_out:
	bprintf(efp,"%s: bad return after write to out file - rs %d\n",
	    progname,rs) ;

	goto bad_ret ;

bad_open:
	bprintf(efp,"%s: could not open file - (rs %d)\n",
	    progname,rs) ;

bad_ret:
	bclose(ifp) ;

	bclose(ofp) ;

	bclose(efp) ;

	return BAD ;
}


