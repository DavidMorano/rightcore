/* fbreadline */

/* read a coded line from the STDIO stream */


#define	CF_FGETS	1		/* faster or not? */


/* revision history:

	= 1998-08-17, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This routine will only read at most 'len' number of bytes
	from the file.

	Note that the sematics of this call are not the same as
	'fgets(3c)'.  This call will write a NULLCHAR into the user
	buffer after the supplied length of the buffer is used up.
	With 'fgets(3c)', it will never write more than the user's
	supplied length of bytes.

	Notes:

	The IRIX operating system is messed up somehow.  An attempt
	to correct for this is below.


******************************************************************************/


#include	<envstandards.h>

#include	<stdio.h>

#include	<vsystem.h>


/* local defines */

#if	defined(IRIX)		/* IRIX screws up! (what else is new?) */
#define	mygetc(p)	fgetc(p)
#else
#define	mygetc(p)	getc(p)
#endif


/* exported subroutines */


int fbreadline(fp,lbuf,llen)
FILE	*fp ;
char	lbuf[] ;
int	llen ;
{
	int	i = 0 ;


	if (fp == NULL)
	    return SR_FAULT ;

	if (lbuf == NULL)
	    return SR_FAULT ;

	if (llen == 0)
	    return 0 ;

#if	CF_FGETS
	{
	    char	*bp = fgets(lbuf,(llen + 1),fp) ;
	    i = (bp != NULL) ? strlen(bp) : 0 ;
	} /* end block */
#else /* CF_FGETS */
	{
	    int	ch = 0 ;

	    char	*bp = lbuf ;


	    for (i = 0 ; (i < llen) && (ch != '\n') ; i += 1) {

	        if ((ch = mygetc(fp)) == EOF)
	            break ;

	        *bp++ = ch ;

	    } /* end for */

	    *bp++ = '\0' ;

	} /* end block */

#endif /* CF_FGETS */

	if ((i == 0) && ferror(fp)) {
	   clearerr(fp) ;
	   i = SR_IO ;
	}

	return i ;
}
/* end subroutine (fbreadline) */



