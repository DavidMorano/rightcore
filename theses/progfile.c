/* progfile */

/* process an input file to the output */


#define	F_DEBUG		1
#define	F_FONT		1		/* change the font INSIDE display */


/* revistion history :

	= 1987-09-10, David A.D. Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************

	This subroutine processes a file by writing its contents out
	to an output file with the correct pagination.


*********************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	MAX_LINE
#define	LINEBUFLEN	MAX(MAX_LINE,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	BUFLEN
#define	BUFLEN		(LINEBUFLEN + LINEBUFLEN)
#endif


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;


/* forward references */


/* local structures */


/* local variables */


/* external subroutines */


int progfile(pip,infname,fn,f_eject)
struct proginfo	*pip ;
const char	infname[] ;
int		fn ;
int		f_eject ;
{
	bfile	infile, *ifp = &infile ;

	int	rs = SR_OK ;
	int	blurb ;
	int	len ;
	int	sl ;
	int	f_exit ;

	const char	*lbp ;

	char	linebuf[LINEBUFLEN + 1] ;


/* check the arguments */

	if ((infname == NULL) || (infname[0] == '-')) {
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;
	} else
	    rs = bopen(ifp,infname,"r",0666) ;

	if (rs < 0)
	    goto ret0 ;

/* output the pages */

	blurb = 0 ;
	f_exit = FALSE ;
	while (! f_exit) {

/* skip leading blank lines */

	    while ((rs = breadline(ifp,linebuf,LINEBUFLEN)) > 0) {
		len = rs ;

	        sl = sfshrink(linebuf,len,&lbp) ;

	        if (sl > 0) 
	            break ;

	    } /* end while (skipping blank lines) */

	    if (rs <= 0)
	        break ;

/* put out the stuff for this next blurb */

	    blurb += 1 ;
	    bprintf(pip->ofp,".SP 2\n") ;

	    bprintf(pip->ofp,".DS L F\n") ;

#if	F_FONT
	    bprintf(pip->ofp,".ft ZI\n") ;
#endif

/* copy over the first line */

	    bprintf(pip->ofp,"%w\n",lbp,sl) ;

/* copy over the lines */

	    while ((rs = breadline(ifp,linebuf,LINEBUFLEN)) > 0) {
	        len = rs ;

	        sl = sfshrink(linebuf,len,&lbp) ;

	        if (sl <= 0)
	            break ;

	        rs = bprintf(pip->ofp,"%w\n",lbp,sl) ;
		if (rs < 0) break ;

	    } /* end while (skipping blank lines) */

	    if (rs <= 0)
	        f_exit = TRUE ;

/* finish the last blurb */

#if	F_FONT
	    bprintf(pip->ofp,".ft\n") ;
#endif
	    bprintf(pip->ofp,".DE\n") ;

	} /* end while (main file line reading loop) */

done:
	bclose(ifp) ;

ret0:
	return (rs >= 0) ? blurb : rs ;
}
/* end subroutine (progfile) */



