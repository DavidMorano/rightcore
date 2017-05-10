/* print out the check sum of a PLD JEDEC file a JEDEC */


/* 
	$ pld_sum file1 file2 .. 

*/

#include	<envstandards.h>

#include	<sys/types.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<localmisc.h>


#define		VERSION		"1"
#define		LINELEN		100
#define		NBUF		3
#define		NFILES		100


/* exported subroutines */


int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		infile, *ifp = &infile ;

	int		argl, aol ;
	int		npa, i ;
	int		f_usage = FALSE ;
	int		f_debug = FALSE ;
	int		f_dash = FALSE ;
	int		len ;
	int		bn, fn, rs ;

	char		*argp, *aop ;
	char		*progname ;
	char		*filename[NFILES+1] ;
	char		linebuf0[LINELEN + 1] ;
	char		linebuf1[LINELEN + 1] ;
	char		linebuf2[LINELEN + 1] ;
	char		device[LINELEN + 1] ;
	char		*which[NBUF] ;
	char		*lp ;


	progname = argv[0] ;
	if (bopen(efp,BERR,"wca",0666) < 0) return BAD ;

	for (i = 0 ; i < NFILES ; i += 1) filename[i] = ((char *) 0) ;

	npa = 0 ;			/* number of positional so far */
	i = 1 ;
	argc -= 1 ;

	while (argc > 0) {

	    argp = argv[i++] ;
	    argc -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

	                bprintf(efp,"%s: illegal option \"%s\" ignored\n",
				progname,argp) ;

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
	                        bprintf(efp,"%s: version %s\n",
	                            progname,VERSION) ;

	                        break ;

	                    default:
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            progname,*aop) ;

	                    case '?':
	                        f_usage = TRUE ;

	                    } /* end switch */

	                } /* end while */
	            }

	        } else {

	            npa += 1 ;	/* increment position count */
		    f_dash = TRUE ;

	        } /* end if */

	    } else {

	        if (npa < NFILES) {

		    if (strlen(argp) > 0) filename[npa++] = argp ;

	        } else {

	            bprintf(efp,"%s: extra arguments ignored\n",
	                progname) ;

	        }

	    } /* end if */

	} /* end while */

	if (f_debug) bprintf(efp,"finished parsing arguments\n") ;

	if (f_usage) goto usage ;

	bflush(efp) ;

	which[0] = linebuf0 ;
	which[1] = linebuf1 ;
	which[2] = linebuf2 ;

	if ((rs = bopen(ofp,BOUT,"wct",0666)) < 0) goto badoutopen ;

	if (f_dash) {

		filename[0] = ((char *) 0) ;
		npa = 1 ;
	}

	fn = 0 ;
	while (fn < npa) {

	    if (filename[fn] == ((char *) 0)) {

		if (fn != 0) continue ;

	        if ((rs = bopen(ifp,filename[fn],"r",0666)) < 0) 
			goto badinfile ;

		filename[0] = "*std_input*" ;

	    } else if ((rs = bopen(ifp,filename[fn],"r",0666)) < 0) 
		goto badinfile ;

	    len = breadline(ifp,device,LINELEN) ;

	    len = breadline(ifp,device,LINELEN) ;

	    device[len - 2] = '\0' ;

	    bn = 0 ;
	    lp = linebuf0 ;
	    while ((len = breadline(ifp,lp,LINELEN)) > 0) {

	        bn = (bn + 1) % NBUF ;
	        lp = which[bn] ;

	    }

	    bclose(ifp) ;

	    bn = (bn + 1) % NBUF ;
	    lp = which[bn] ;

	    bprintf(ofp,"%-32s - %W - %s\n",
	        filename[fn],lp + 1,4,device) ;

	    fn += 1 ;
	}

done:
	bclose(ofp) ;

	bclose(efp) ;

	return OK ;

badoutopen:
	bprintf(efp,"%s: can't open output file\n",
	    progname) ;

badinfile:
	bprintf(efp,"%s: can't open input file \"%s\" - rs (%d)\n",
	    progname,filename[fn],rs) ;

badret:
	bclose(ofp) ;

	bclose(efp) ;

	return BAD ;

usage:
	bprintf(efp,
	    "usage: %s [-?] [-V] - | jedec_file [more files ...]\n",
	    progname) ;

	goto badret ;
}


