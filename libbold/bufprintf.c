/* bufprintf */

/* subroutine to format "buffered" output */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revistion history:

	= 1998-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is used by 'printf' type routines to format an
	output "buffer" from a format specification.  This routine has
	no support for floating point conversion since floating point
	formats are not general enough for the most portable applications.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<format.h>
#include	<localmisc.h>


/* local defines */

#define	MAXLEN		(MAXPATHLEN + 40)


/* external references */


/* local structures */


/* forward references */


/* exported subroutines */


int bufprintf(char dbuf[],int dlen,const char fmt[],...)
{
	va_list	ap ;

	const int	m = 0 ;

	int	rs ;


	if (dbuf == NULL)
	    return SR_FAULT ;

	{
	    va_begin(ap,fmt) ;
	    rs = format(dbuf,dlen,m,fmt,ap) ;
	    va_end(ap) ;
	}

	return rs ;
}
/* end subroutine (bufprintf) */


int vbufprintf(dbuf,dlen,fmt,ap)
char		dbuf[] ;
int		dlen ;
const char	fmt[] ;
va_list		ap ;
{
	const int	m = 0 ;

	int	rs ;


	rs = format(dbuf,dlen,m,fmt,ap) ;

	return rs ;
}
/* end subroutine (vbufprintf) */


/* local subroutines */


