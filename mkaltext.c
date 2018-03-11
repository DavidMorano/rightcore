/* mkaltext */

/* make a file-name w/ an alternate extension */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	The subroutine was written from scratch.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We take a file-name and create a new file-name replacing the
	existing extension with the new (alternative) extension supplied.

	Synopsis:

	int mkaltext(dbuf,name,ext)
	char		*debug ;
	const char	name[] ;
	const char	ext[] ;

	Returns:

	>=0		length of result string
	<0		error:
			    INVALID


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfsub(const char *,int,const char *,const char **) ;

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* global variables */


/* local variables */


/* exported subroutines */


int mkaltext(char *dbuf,cchar *name,cchar *ext)
{
	int		rs = SR_INVALID ;
	int		len = 0 ;
	const char	*tp ;

	if (dbuf == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;
	if (ext == NULL) return SR_FAULT ;

	if ((tp = strrchr(name,'.')) != NULL) {
	    if (tp[1] != '\0') {
	        SBUF		alt ;
	        const int	dlen = MAXPATHLEN ;
	        if ((rs = sbuf_start(&alt,dbuf,dlen)) >= 0) {
	            sbuf_strw(&alt,name,(tp - name)) ;
	            sbuf_char(&alt,'.') ;
	            sbuf_strw(&alt,ext,-1) ;
	            len = sbuf_finish(&alt) ;
	            if (rs >= 0) rs = len ;
	        } /* end if (SBUF) */
	    }
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkaltext) */


