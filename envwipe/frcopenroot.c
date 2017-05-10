/* frcopenroot */

/* open a file name according to rules */
/* version %I% last modified %G% */


#define	F_DEBUGS	0


/* revision history :

	= 94/09/01, David A­D­ Morano
	This program was originally written.

	= 01/04/11, David A­D­ Morano
	This was straightforwardly adapted from 'bopenroot()'.


*/



/*****************************************************************************

	This subroutine will form a file name according to some
	rules.

	The rules are roughly :

	+ attempt to open it directly if it is already rooted
	+ open it if it is already in the root area
	+ attempt to open it as it is if it already exists
	+ attempt to open or create it located in the root area
	+ attempt to open or create it as it is


	Synopsis :

		rs = fopenroot(fpp,programroot,fname,outname,mode)
		FILE	**fpp ;
		char	programroot[], fname[], outname[] ;
		char	mode[] ;


	Arguments :
	+ fp		pointer to 'bfile' object
	+ programroot	path of program root directory
	+ fname		fname to find and open
	+ outname	user supplied buffer to hold possible resulting name
	+ mode		file open mode

	Returns :
	<0		error (same as 'bopen()')
	>=0		success (same as 'bopen()')

	outname		1) zero length string if no new name was needed
			2) will contain the path of the file that was opened

	

*****************************************************************************/



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

#include	<vsystem.h>

#include	"misc.h"
#include	"outbuf.h"


/* local defines */


/* external subroutines */

extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;

extern char	*strwcpy(char *,char *,int) ;


/* external variables */


/* forward references */

static int	frcopen(FILE **,char *,char *) ;


/* local global variabes */


/* local structures */


/* local variables */






int frcopenroot(fpp,programroot,fname,outname,mode)
FILE	**fpp ;
char	programroot[], fname[], outname[] ;
char	mode[] ;
{
	OUTBUF	ob ;

	FILE	*fp ;

	int	rs ;
	int	sl ;
	int	imode ;
	int	f_outbuf ;

	char	*mp, *onp = NULL ;


#if	F_DEBUGS
	    eprintf("bopenroot: entered fname=%s\n",fname) ;
#endif

	if ((fname == NULL) || (fname[0] == '\0'))
	    return SR_NOEXIST ;

	if (fpp == NULL)
		return SR_FAULT ;

	f_outbuf = (outname != NULL) ;

	imode = 0 ;
	for (mp = mode ; *mp ; mp += 1) {

	    switch ((int) *mp) {

	    case 'r':
	        imode += R_OK ;
	        break ;

	    case '+':
	    case 'w':
	        imode += W_OK ;
	        break ;

	    case 'x':
	        imode += X_OK ;
	        break ;

	    } /* end switch */

	} /* end for */


	if (fname[0] == '/') {

		if (f_outbuf)
			outname[0] = '\0' ;

	    rs = frcopen(fpp,fname,mode) ;

		return rs ;
	}

	outbuf_init(&ob,outname,-1) ;


	if (programroot != NULL) {

#if	F_DEBUGS
	    eprintf("bopenroot: about to alloc\n") ;
#endif

	    if ((rs = outbuf_get(&ob,&onp)) < 0)
	        return rs ;

	    sl = bufprintf(onp,MAXPATHLEN,"%s/%s",
	        programroot,fname) ;

	    if (perm(onp,-1,-1,NULL,imode) >= 0) {

	        rs = frcopen(fpp,onp,mode) ;

		if (rs >= 0)
	        	goto done ;

		}

	}

	if (perm(fname,-1,-1,NULL,imode) >= 0) {

	    rs = frcopen(fpp,fname,mode) ;

		if (rs >= 0) {

		if (f_outbuf)
			outname[0] = '\0' ;

	    		goto done ;
		}
	}

	if ((programroot != NULL) &&
	    (strchr(fname,'/') != NULL)) {

	    rs = frcopen(fpp,onp,mode) ;

		if (rs >= 0)
	        goto done ;

	}

		if (f_outbuf)
			outname[0] = '\0' ;

	rs = frcopen(fpp,fname,mode) ;

done:
	outbuf_free(&ob) ;

	return rs ;
}
/* end subroutine (frcopenroot) */



/* LOCAL SUBROUTINES */



/* file open w/ return code */
static int frcopen(fpp,fname,mode)
FILE	**fpp ;
char	fname[] ;
char	mode[] ;
{


	*fpp = fopen(fname,mode) ;

	return ((*fpp != NULL) ? SR_OK : SR_NOTOPEN) ;
}
/* end subroutine (frcopen) */



