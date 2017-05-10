/* main */

/* remove the stupid DOS carriage return characters dor DOS files */
/* last modified %G% version %I% */


/* revision history:

	= 96/09/09, David A­D­ Morano 

	This program was taken from the code of the 'stripdos'
	program.


*/


/**************************************************************************

	Execute as :

		psclean [infile]


**************************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"



/* local defines */

#define		NPARG		1
#define		BUFLEN		1024



/* external variables */







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
	int	rs, len ;
	int	count_m = 0, count_z = 0 ;
	int	count_d = 0 ;
	int	ex = EX_INFO ;
	int	f_usage = FALSE ;
	int	f_debug = FALSE ;
	int	f_dash = FALSE ;
	int	f_verbose = FALSE ;
	int	f_control = FALSE ;
	int	f_start = FALSE ;
	int	f_bol ;
	int	f_eol ;

	char	*argp, *aop ;
	char	*progname ;

	char	buf[BUFLEN + 1], *bp ;
	char	*infname = NULL ;


	if (bopen(efp,BERR,"wca",0666) < 0)
		efp = NULL ;


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

	f_start = FALSE ;
	f_bol = TRUE ;
	while ((rs = breadline(ifp,buf,BUFLEN)) > 0) {

	    len = rs ;
	    f_eol = FALSE ;
	    if (buf[len - 1] == '\n') 
		f_eol = TRUE ;

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

	    } /* end if (BOL) */

	    if (f_eol) {

	        if ((len > 1) && (bp[len - 2] == '\r')) {

	            count_m += 1 ;
	            bprintf(ofp,"%t\n",bp,len - 2) ;

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

	ex = EX_OK ;

ret3:
	bclose(ofp) ;

ret2:
	bclose(ifp) ;

done:
ret1:
	bclose(efp) ;

ret0:
	return ex ;

/* program usage */
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

	goto ret1 ;

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	bprintf(efp,"%s: bad argument given\n",progname) ;

	goto bad1 ;

badinfile:
	ex = EX_DATAERR ;
	bprintf(efp,"%s: cannot open the input file (%d)\n",
	    progname,rs) ;

	goto ret1 ;

badoutopen:
	ex = EX_DATAERR ;
	bprintf(efp,"%s: could not open standard output (%d)\n",
	    progname,rs) ;

	goto ret2 ;

}
/* end subroutine (main) */



