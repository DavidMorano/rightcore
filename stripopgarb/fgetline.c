/* fgetline */

/* read a coded line from the STDIO stream */


#define	F_FGETS		0	/* faster or not ? */


/* revision history :

	= 86/01/17, David A­D­ Morano

	This subroutine was originally written.


	= 01/09/10, David A­D­ Morano

	I discovered that on the SGI Irix systems, 'getc()' does
	not seem to work properly so I hacked it out for that
	system.


*/


/******************************************************************************

	This routine will only read at most 'len' number of bytes
	from the file.

	Note that the sematics of this call are not the same as
	'fgets(3c)'.  This call will write a NULLCHAR into the user
	buffer after the supplied length of the buffer is used up.
	With 'fgets(3c)', it will never write more than the user's
	supplied length of bytes.


******************************************************************************/



#include	<stdio.h>



/* local defines */

#if	defined(IRIX)		/* IRIX screws up ! (what else is new) */
#define	mygetc(p)	fgetc(p)
#else
#define	mygetc(p)	getc(p)
#endif




int fgetline(fp,buf,len)
FILE	*fp ;
char	buf[] ;
int	len ;
{
	int	i ;


	if (len == 0)
	    return 0 ;

	if (len < 1)
	    return -1 ;

#if	F_FGETS

	{
	    char	*bp ;


	    bp = fgets(buf,(len + 1),fp) ;

	    i = (bp != NULL) ? strlen(bp) : 0 ;

	} /* end block */

#else /* F_FGETS */

	{
	    int	c ;

	    char	*bp = buf ;


	    c = 0 ;
	    for (i = 0 ; (i < len) && (c != '\n') ; i += 1) {

	        if ((c = mygetc(fp)) == EOF)
	            break ;

	        *bp++ = c ;

	    } /* end for */

	    *bp++ = '\0' ;

	} /* end block */

#endif /* F_FGETS */

	return i ;
}
/* end subroutine (fgetline) */



