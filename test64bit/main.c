/* main (test64bit) */

/* test 64 bit integers */


#define	CF_DEBUGS	1
#define	CF_TEST2	0


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<bfile.h>

#include	"localmisc.h"
#include	"defs.h"



/* local defines */

#define	LINELEN		100



/* external subroutines */

extern char	*malloc_str(char *) ;

void		sub() ;


/* local structures */

struct proginfo {
	bfile	*ofp ;
} ;


/* forward references */






int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		outfile, *ofp = &outfile ;

	LONG		i64 = 0 ;

	int	es = 0, rs, i ;
	int	err_fd ;

	char	*cp, *cp1, *cp2 ;
	char	*dirname ;
	char	buf[MAXPATHLEN + 1] ;
	char	*progname ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    debugsetfd(err_fd) ;


	progname = argv[0] ;
	if (bopen(ofp,BFILE_STDOUT,"wct",0666) >= 0)
	    bcontrol(ofp,BC_LINEBUF,0) ;


	g.ofp = ofp ;


#if	CF_DEBUGS
	debugprintf("main: entered\n") ;
#endif


	if ((argc >= 2) && (argv[1] != NULL))
	    dirname = argv[1] ;

	else
	    dirname = "q" ;


	rs = 0 ;
	i64 = 0x123456789abcdefL ;

#if	CF_TEST2

	for (i = 0 ; i < 8 ; i += 1) {

		int	v ;


		v = (int) (i64 & 0xFF) ;
		bprintf(ofp,"32  %02X\n",v) ;

		bprintf(ofp,"64  %02llX\n",(i64 & 0xFF)) ;

		i64 = i64 >> 8 ;

	} /* end for */

#else

#if	CF_DEBUGS
	debugprintf("main: calling 'sub'\n") ;
#endif

	sub(i64,1,2,3,4,5,6,7) ;

#if	CF_DEBUGS
	debugprintf("main: called 'sub'\n") ;
#endif

#endif /* CF_TEST2 */


	es = 0 ;

done:

#if	CF_DEBUGS
	debugprintf("main: exiting\n") ;
#endif


	bclose(ofp) ;

	return es ;


}
/* end subroutine (main) */



