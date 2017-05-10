/* main (testrealname) */

/* test the REALNAME object */


#define	CF_DEBUGS	0



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<ftw.h>
#include	<dirent.h>
#include	<string.h>
#include	<stdlib.h>
#include	<errno.h>

#include	<bfile.h>
#include	<mallocstuff.h>

#include	"localmisc.h"
#include	"realname.h"



/* local defines */

#define	LINELEN		100



/* external subroutines */


/* forward references */






int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;

	REALNAME	rn ;

	int	rs, i, j, sl, len ;
	int	err_fd ;

	char	*cp ;
	char	linebuf[MAXPATHLEN + 1] ;
	char	*progname ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    debugsetfd(err_fd) ;


	progname = argv[0] ;

	if (bopen(ofp,BFILE_STDOUT,"dwct",0666) >= 0)
	    bcontrol(ofp,BC_LINEBUF,0) ;

	bopen(ifp,BFILE_STDIN,"dr",0666) ;


#if	CF_DEBUGS
	debugprintf("main: entered\n") ;
#endif


	while (TRUE) {

	    bprintf(ofp,"name> ") ;

	    if ((len = breadline(ifp,linebuf,LINELEN)) <= 0)
	        break ;

	    if (linebuf[len - 1] == '\n')
	        len -= 1 ;

	    linebuf[len] = '\0' ;
	    bprintf(ofp,"> %s\n",linebuf,len) ;

	    realname_start(&rn,linebuf,len) ;

	    bprintf(ofp,"first>\t\t%s %c\n",
	        rn.first,
	        (rn.f.abv_first) ? '*' : ' ') ;

	    bprintf(ofp,"middle1>\t%s %c\n",
	        rn.m1,
	        (rn.f.abv_m1) ? '*' : ' ') ;

	    bprintf(ofp,"middle2>\t%s %c\n",
	        rn.m2,
	        (rn.f.abv_m2) ? '*' : ' ') ;

	    bprintf(ofp,"last>\t\t%s %c\n",
	        rn.last,
	        (rn.f.abv_last) ? '*' : ' ') ;


	    sl = realname_fullname(&rn,linebuf,LINELEN) ;

	    bprintf(ofp,"fullname=%s\n",linebuf) ;


	    sl = realname_name(&rn,linebuf,LINELEN) ;

	    bprintf(ofp,"name=%s\n",linebuf) ;


	    sl = realname_mailname(&rn,linebuf,LINELEN) ;

	    bprintf(ofp,"mailname=%s\n",linebuf) ;



	    realname_finish(&rn) ;

	} /* end while */



#if	CF_DEBUGS
	debugprintf("main: done\n") ;
#endif


	rs = 0 ;

done:
	bclose(ifp) ;

	bclose(ofp) ;

	return rs ;
}
/* end subroutine (main) */



