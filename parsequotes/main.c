/* parsequotes */

/* remove the stupid DOS carriage return characters for DOS files */
/* last modified %G% version %I% */


/* revision history:

	= 92/03/01, David A­D­ Morano

	This program was originally written.


*/


/**************************************************************************

	Execute as :

		stripdos [infile]


	This program will also cleanup stupid old Macintosh text files.


**************************************************************************/



#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<ctype.h>
#include	<stdlib.h>

#include	<bfile.h>
#include	<ascii.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	NPARG		1

#ifndef	BUFLEN
#define	BUFLEN		4096
#endif

#define	S_SEARCH	0
#define	S_CR		1
#define	S_Z		2



/* external subroutines */

extern char	*strbasename(char *) ;


/* external variables */






int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		errfile, *efp = &errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;

	int	argr, argl, aol ;
	int	pan, i ;
	int	len, rs ;
	int	state ;
	int	count_m = 0, count_z = 0 ;
	int	count_d = 0 ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_debug = FALSE ;
	int	f_dash = FALSE ;
	int	f_verbose = FALSE ;
	int	f_bol = TRUE ;
	int	f_eol = FALSE ;
	int	f_double = FALSE ;

	char	*argp, *aop ;
	char	*progname ;
	char	buf[BUFLEN + 1], *bp ;
	char	*infname = NULL ;


	progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"wca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;


	pan = 0 ;			/* number of positional so far */
	i = 1 ;
	argr = argc - 1 ;
	while (argr > 0) {

	    argp = argv[i++] ;
	    argr -= 1 ;
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
	                    f_version = TRUE ;
	                    bprintf(efp,"%s: version %s\n",
	                        progname,VERSION) ;

	                    break ;

			case 'd':
				f_double = TRUE ;
				break ;

	                case 'v':
	                    f_verbose = TRUE ;
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
	    bprintf(efp,"%s: finished parsing arguments\n",
	        progname) ;

	if (f_usage) goto usage ;

	if (f_version) goto earlyret ;

/* check arguments */


/* open files */

	if (infname == NULL) infname = (char *) 0 ;

	if ((rs = bopen(ifp,infname,"r",0666)) < 0)
	    goto badinfile ;

	if ((rs = bopen(ofp,(char *) 1,"wct",0666)) < 0)
	    goto badoutopen ;


/* go through the loops */

	state = S_SEARCH ;
	while ((len = bread(ifp,buf,BUFLEN)) > 0) {

	    for (i = 0 ; i < len ; i += 1) {

	        switch (buf[i] & 0xFF) {

	        case '\n':
	            if (state == S_Z) bputc(ofp,C_US) ;

	            state = S_SEARCH ;
	            bputc(ofp,'\n') ;

		if (f_double) bputc(ofp,'\n') ;

	            break ;

	        case '\r':
	            if (state == S_Z) bputc(ofp,C_US) ;

	            else if (state == S_CR) bputc(ofp,'\n') ;

	            state = S_CR ;
	            break ;

	        case C_US:
	            if (state == S_CR) bputc(ofp,'\r') ;

	            state = S_Z ;
	            break ;

	        default:
	            switch (state) {

	            case S_Z:
	                state = S_SEARCH ;
	                bputc(ofp,C_US) ;

	                bputc(ofp,buf[i]) ;

	                break ;

	            case S_CR:
	                state = S_SEARCH ;
	                bputc(ofp,'\n') ;

	                bputc(ofp,buf[i]) ;

	                break ;

	            default:
	                bputc(ofp,buf[i]) ;

	            } /* end switch */

	        } /* end switch */

	    } /* end for */

	} /* end while */

	    if (state == S_CR) bputc(ofp,'\n') ;


done:
	bclose(ofp) ;

	bclose(ifp) ;

earlyret:
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



