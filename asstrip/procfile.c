/* procfile */

/* process an input file to the output */


#define	CF_DEBUG	1		/* run-time debugging */


/* revistion history:

	= 87/09/10, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine processes a file by writing its contents out to an
        output file with the correct pagination.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<time.h>
#include	<ctype.h>
#include	<string.h>
#include	<stdlib.h>

#include	<bfile.h>
#include	<baops.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	BUFLEN		(LINEBUFLEN + LINEBUFLEN)


/* external subroutines */


/* forward references */


/* local structures */


/* local data */


/* exported subroutines */


int procfile(gp,infname,fn)
struct global	*gp ;
char		infname[] ;
int		fn ;
{
	bfile	infile, *ifp = &infile ;

	int	rs ;
	int	line, len ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	*cp ;


	line = 0 ;

/* check the arguments */

	if ((infname == NULL) || (infname[0] == '-'))
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	    else
	    rs = bopen(ifp,infname,"r",0666) ;

	if (rs < 0)
	    goto badinfile ;

/* process the input source */

	while ((rs = breadline(ifp,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    linebuf[len] = '\0' ;
	    if ((cp = strchr(linebuf,';')) != NULL) {

	        if ((cp == linebuf) || (cp[-1] != '\'')) {

	            *cp++ = '\n' ;
	            len = cp - linebuf ;

	        } /* end if */

	    } /* end if */

	    if ((! gp->f.noblanks) || (len > 1))
	        bwrite(gp->ofp,linebuf,len) ;

	    line += 1 ;

	} /* end while (main file line reading loop) */


done:
	bclose(ifp) ;

badinfile:
	return (rs >= 0) ? line : rs ;
}
/* end subroutine (procfile) */



