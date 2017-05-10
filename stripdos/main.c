/* main */

/* remove the stupid DOS carriage return characters dor DOS files */
/* last modified %G% version %I% */


/* revision history:

	= 92/03/01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Execute as :

	$ stripdos [infile]


**************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"


/* local defines */

#define	NPARG		1

#define	MINCHUNK	4096
#define	DEFCHUNK	8192
#define	BUFLEN		MINCHUNK


/* external subroutines */

extern char	*strbasename() ;


/* external variables */


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile	errfile, *efp = &errfile ;
	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;

	int	argl, aol ;
	int	pan, i ;
	int	len, rs ;
	int	count_m = 0, count_z = 0 ;
	int	count_d = 0 ;
	int	f_usage = FALSE ;
	int	f_debug = FALSE ;
	int	f_dash = FALSE ;
	int	f_verbose = FALSE ;
	int	f_control = FALSE ;
	int	f_bol = TRUE ;
	int	f_eol = FALSE ; 

	char	*argp, *aop ;
	char	*progname ;

	char	buf[BUFLEN + 1], *bp ;
	char	*infname = NULL ;


	progname = strbasename(argv[0]) ;

	if (bopen(efp,BERR,"wca",0666) >= 0) 
		bcontrol(efp,BC_LINEBUF,0) ;


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

	                case 'D':
	                    f_debug = TRUE ;
	                    break ;

	                case 'V':
	                    bprintf(efp,"%s: version %s\n",
	                        progname,VERSION) ;

	                    break ;

	                case 'v':
	                    f_verbose = TRUE ;
	                    break ;

	                case 'd':
	                    f_control = TRUE ;
	                    break ;

	                default:
	                    bprintf(efp,"%s: unknown option - %c\n",
	                        progname,*aop) ;

	                case '?':
	                    f_usage = TRUE ;

	                } /* end switch */

	            } /* end while */

	        } else {

	            pan += 1 ;	/* increment position count */
	            f_dash = TRUE ;

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {

	            case 0:
	                if (argl > 0) infname = argp ;

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

	if (f_debug)
	    bprintf(efp,"finished parsing arguments\n") ;

	if (f_usage) goto usage ;

/* check arguments */


/* open files */

	if (infname == NULL) infname = (char *) 0 ;

	if ((rs = bopen(ifp,infname,"r",0666)) < 0)
	    goto badinfile ;

	if ((rs = bopen(ofp,(char *) 1,"wct",0666)) < 0)
	    goto badoutopen ;

/* go through the loops */

	while ((len = breadline(ifp,buf,BUFLEN)) > 0) {

	    f_eol = FALSE ;
	    if (buf[len - 1] == '\n') f_eol = TRUE ;

	    bp = buf ;
	    if (f_bol) {

	        if (*bp == 4) {

	            bp += 1 ;
	            len -= 1 ;
	            count_d += 1 ;

	        }

	        if ((len == 1) && (bp[0] == 26)) {

	            len = 0 ;
	            count_z += 1 ;

	        }

	    }

	    if (f_eol) {

	        if ((len > 1) && (bp[len - 2] == '\r')) {

	            count_m += 1 ;
	            bprintf(ofp,"%W\n",bp,len - 2) ;

	        } else
	            bwrite(ofp,bp,len) ;

	    } else 
	        bwrite(ofp,bp,len) ;

	    f_bol = f_eol ;

	} /* end while */

	if (f_debug || f_verbose) {

	    bprintf(efp,"control-M %d\ncontrol-Z %d\n",
	        count_m,count_z) ;

	    bprintf(efp,"control-D %d\n",
	        count_d) ;

	}

	bclose(ofp) ;

	bclose(ifp) ;

done:
	bclose(efp) ;

	return OK ;

badret:
	bclose(efp) ;

	return BAD ;

usage:
	bprintf(efp,
	    "usage: %s [-v] [-d] [infile] [-?VD]\n",
	    progname) ;

	bprintf(efp,
	    "\t-v	turn on \"verbose\" mode\n") ;

	bprintf(efp,
	    "\t-d	selectively strip DOS control-D characters\n") ;

	bprintf(efp,
	    "\tinfile	input source file\n") ;

	goto badret ;

badarg:
	bprintf(efp,"%s: bad argument given\n",progname) ;

	goto badret ;

badinfile:
	bprintf(efp,"%s: cannot open the input file (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badoutopen:
	bprintf(efp,"%s: could not open standard output (rs %d)\n",
	    progname,rs) ;

	goto badret ;

}
/* end subroutine (main) */



