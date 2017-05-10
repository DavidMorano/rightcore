/* ffprintf */

/* knock off subroutine 'fprintf(3c)' */


#define	CF_DEBUGS	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdio.h>
#include	<errno.h>
#include	<stdarg.h>

#include	<vsystem.h>

#include	"ffile.h"


/* local defines */


/* external subroutines */


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int ffprintf(FFILE *fp,const char *fmt,...)
{
	FILE	*sfp ;

	va_list	ap ;

	int	rs ;


	if (fp == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	sfp = fp->sfp ;

	{
	    va_begin(ap,fmt) ;
	    rs = vfprintf(sfp,fmt,ap) ;
	    va_end(ap) ;
	}

	return rs ;
}
/* end subroutine (ffprintf) */



