/* main (testdw) */

/* directory watching testing */


#define	CF_DEBUGS	1


/******************************************************************************

	This program tests the DW object.


******************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<dirent.h>
#include	<string.h>
#include	<stdlib.h>
#include	<ftw.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"dw.h"



/* local defines */

#define	NLOOPS		100
#define	SLEEPTIME	3



/* external subroutines */


/* forward references */








int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	DW		dir ;

	DW_CUR	cur ;

	bfile		outfile, *ofp = &outfile ;

	int	ex = EX_INFO ;
	int	rs, rs1, i, j, len ;
	int	fd_debug ;
	int	cl ;

	char	buf[MAXPATHLEN + 1] ;
	char	*cp, *cp1, *cp2 ;
	char	*dirname ;
	char	*pr = NULL ;
	char	*progname ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	progname = argv[0] ;
	if (bopen(ofp,BFILE_STDOUT,"wct",0666) >= 0)
	    bcontrol(ofp,BC_LINEBUF,0) ;


#if	CF_DEBUGS
	debugprintf("main: entered\n") ;
#endif


	if ((argc >= 2) && (argv[1] != NULL))
	    dirname = argv[1] ;

	    else
	    dirname = "q" ;

#if	CF_DEBUGS
	debugprintf("main: dirname=%s\n",dirname) ;
#endif


	rs = dw_start(&dir,dirname) ;

#if	CF_DEBUGS
	debugprintf("main: dw_start() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    DW_ENT	e ;

	    time_t	daytime ;

	    int		jid ;


	    for (i = 0 ; i < NLOOPS ; i += 1) {

	        sleep(SLEEPTIME) ;

	        daytime = time(NULL) ;

	        if (dw_check(&dir,daytime) > 0) {

	            dw_curbegin(&dir,&cur) ;

	            while (TRUE) {

	                rs1 = dw_enumcheckable(&dir,&cur,&e) ;

#if	CF_DEBUGS
	debugprintf("main: dw_enumcheckable() rs=%d\n",rs1) ;
#endif

	                jid = rs1 ;
	                if (rs1 < 0) break ;

	                bprintf(ofp,"checkable jid=%d %s\n",
	                    jid,e.name) ;

	            } /* end while */

	            dw_curend(&dir,&cur) ;

	        } /* end if */

	    } /* end while */

	    dw_finish(&dir) ;

	} else
	    bprintf(ofp,"couldn't initialize (%d)\n",rs) ;


	ex = EX_OK ;


done:

#if	CF_DEBUGS
	debugprintf("main: return ex=%d\n",ex) ;
#endif

	bclose(ofp) ;

	return ex ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */




