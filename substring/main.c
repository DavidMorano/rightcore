/* main */

/* substitute a string for another */


#define	CF_DEBUGS	0


/* revision history:

	= 86/07/10, David A­D­ Morano

	This subroutine was originally written.


*/


/***********************************************************************

	This program will replace one string in a file with another.



***********************************************************************/



#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"



/* local defines */

#define	STAGEBUFLEN	100



/* external subroutines */

extern char	*strbasename(char *) ;


/* local variables */







int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		infile, *ifp = &infile ;

	int	rs, i, j ;
	int	ex = EX_INFO ;
	int	sl, cl ;
	int	c ;
	int	slen, rlen ;
	int	offset ;
	int	fd_debug ;

	char	*progname ;
	char	*search, *replace ;
	char	*sp, *cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;



	if (argc < 3)
	    goto badargnum ;

	search = argv[1] ;
	replace = argv[2] ;

	slen = MIN(strlen(search), STAGEBUFLEN) ;

	rlen = MIN(strlen(replace),slen) ;


	if (bopen(ofp,BFILE_STDOUT,"dwct",0666) < 0)
	    goto badout ;


	if (bopen(ifp,BFILE_STDIN,"rd",0666) >= 0) {

	    char	stagebuf[STAGEBUFLEN + 1] ;


	    offset = 0 ;
	    while ((c = bgetc(ifp)) >= 0) {

	        sp = search ;
	        i = 0 ;
	        if (c == search[i]) {

#if	CF_DEBUGS
	            debugprintf("main: got a start c=%c\n",c) ;
#endif

	            stagebuf[i++] = c ;

	            while ((i < slen) &&
	                ((c = bgetc(ifp)) == search[i])) {

	                stagebuf[i++] = c ;

	            } /* end while */

#if	CF_DEBUGS
	            debugprintf("main: found end i=%d c=%d\n",i,c) ;
#endif

	            if (i == slen) {

#if	CF_DEBUGS
	                debugprintf("main: matched\n") ;
#endif

	                for (j = 0 ; j < rlen ; j += 1)
	                    bputc(ofp,replace[j]) ;

	                for ( ; j < slen ; j += 1)
	                    bputc(ofp,0) ;

	                offset += i ;

	            } else {

#if	CF_DEBUGS
	                debugprintf("main: didn't match\n") ;
#endif

	                for (j = 0 ; j < i ; j += 1)
	                    bputc(ofp,search[j]) ;

	                bputc(ofp,c) ;

	                offset += (i + 1) ;

	            }

	        } else {

	            bputc(ofp,c) ;

	            offset += 1 ;

	        }

	        if (c < 0)
	            break ;

	    } /* end while (reading characters) */

	    bclose(ifp) ;

#if	CF_DEBUGS
	    debugprintf("main: offset=%d\n",offset) ;
#endif

	} /* end if (opened the file) */

	bclose(ofp) ;

	ex = EX_OK ;

done:

badexit:
	bclose(efp) ;

	return ex ;

/* bad stuff */
badargnum:
	ex = EX_USAGE ;
	bprintf(efp,"%s: not enough arguments specified\n",
	    progname) ;

	goto badarg ;

badin:
	ex = EX_NOINPUT ;
	bprintf(efp,"%s: can't open input file (rs=%d)\n",
	    progname,rs) ;

	goto badarg ;

badout:
	ex = EX_CANTCREAT ;
	bprintf(efp,"%s: can't open output file (rs=%d)\n",
	    progname,rs) ;

	goto badarg ;

badarg:
	goto done ;

}
/* end subroutine (main) */



