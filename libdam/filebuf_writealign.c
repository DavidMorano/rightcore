/* filebuf_writealign */

/* extra methods for the FILEBUF object */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Align the file-pointer to the specified alignment (zero-filling as
        needed).

	Synopsis:

	int filebuf_writealign(bp,align)
	FILEBUF		*bp ;
	int		align ;

	Arguments:

	bp		FILEBUF object pointer
	align		source buffer length

	Returns:

	<0		error
	>=0		number of bytes written


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<filebuf.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,const char *,const char *,const char *,
			const char *) ;
extern int	sncpy5(char *,int,const char *,const char *,const char *,
			const char *,const char *) ;
extern int	sncpy6(char *,int,const char *,const char *,const char *,
			const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	filebuf_writezero(FILEBUF *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* exported variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int filebuf_writealign(FILEBUF *bp,int asize)
{
	offset_t	foff ;
	int		rs ;
	int		len = 0 ;

	if ((rs = filebuf_tell(bp,&foff)) >= 0) {
	    int	r = (int) (foff & (asize - 1)) ;
	    if (r > 0) {
	        const int	nzero = (asize - r) ;
	        if (nzero > 0) {
	            rs = filebuf_writezero(bp,nzero) ;
	            len += rs ;
	        }
	    }
	} /* end if (filebuf_tell) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (filebuf_writeallign) */


