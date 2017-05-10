/* main (testmimetypes) */

/* test the MIMETYPES object */


#define	CF_DEBUGS	1
#define	CF_SIMULATE	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<vechand.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"mimetypes.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MIMEFNAME	"mimetypes"


/* external subroutines */






int main()
{
	bfile		errfile, *efp = &errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		tmpfile, *tfp = &tmpfile ;

	MIMETYPES		obj ;

	MIMETYPES_CUR	cur ;

	int	rs, i, j, len ;
	int	fd_debug ;

	char	tmpfname_buf[MAXPATHLEN + 1] ;
	char	keybuf[MIMETYPES_TYPELEN + 1] ;
	char	valuebuf[MIMETYPES_TYPELEN + 1] ;
	char	*cp, *cp1, *cp2 ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
		cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	bopen(efp,BFILE_STDERR,"dwca",0666) ;



	bopen(ofp,BFILE_STDOUT,"wct",0666) ;

#if	CF_DEBUGS
	debugprintf("main: entered\n\n") ;
#endif


	rs = mimetypes_start(&obj) ;

#if	CF_DEBUGS
	debugprintf("main: mimetypes_start() rs=%d\n",rs) ;
#endif

	rs = mimetypes_file(&obj,MIMEFNAME) ;

#if	CF_DEBUGS
	debugprintf("main: mimetypes_file() rs=%d\n",rs) ;
#endif


	if ((rs = mimetypes_curbegin(&obj,&cur)) >= 0) {
	    int	vl ;

	    while (rs >= 0) {

		vl = mimetypes_enum(&obj,&cur,keybuf,valuebuf) ;
		if (vl == SR_NOTFOUND) break ;

		rs = vl ;

#if	CF_DEBUGS
	debugprintf("main: mimetypes_enum() rs=%d\n",rs) ;
#endif

		if (rs >= 0)
		    bprintf(ofp,"%-30s %s\n",
			keybuf,valuebuf) ;

	} /* end while */

	mimetypes_curend(&obj,&cur) ;
	} /* end if */

	mimetypes_finish(&obj) ;

	bclose(ofp) ;

	bclose(efp) ;

	return EX_OK ;
}
/* end subroutine (main) */



