/* nprintf */

/* 'Named File' printf subroutine */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine performs a 'printf' like function but to the named file
	which is passed as the first argument.

	Synopsis:

	int nprintf(fname,fmt,...)
	const char	fname[] ;
	const char	fmt[] ;
	...		

	Arguments:

	filename	file to print to
	format		standard format string
	...		enverything else

	Returns:

	>=0		length of data (in bytes) written to file
	<0		failure


	Notes:

	Q. Does this subroutine have to be multi-thread-safe?
	A. In short, of course!

	Q. What do we not hve to place a mutex lock around the
	|write(2)| subroutine?
	A. Because we think that because we open the file a-fresh,
	getting a unique file-pointer, we *think* that the |write(2)|
	shoule be atomic, thus making this subroutine multi-thread-safe.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"format.h"


/* local defines */

#define	FBUFLEN		512
#define	TO_LOCK		2		/* lock timeout */


/* external subroutines */


/* local variables */


/* exported subroutines */


int nprintf(const char fname[],const char fmt[],...)
{
	const int	oflags = (O_WRONLY | O_APPEND) ;
	int		rs ;
	int		len = 0 ;

	if (fname == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;
	if (fmt[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("nprintf: fname=%s\n",fname) ;
#endif

	if ((rs = u_open(fname,oflags,0666)) >= 0) {
	    const int	flen = FBUFLEN ;
	    const int	fd = rs ;
	    int		fl ;
	    char	fbuf[FBUFLEN + 1] ;

#if	CF_DEBUGS
	    debugprintf("nprintf: fmt=%s\n",fmt) ;
#endif

	    {
		va_list	ap ;
	        va_begin(ap,fmt) ;
	        fl = format(fbuf,flen,1,fmt,ap) ;
	        va_end(ap) ;
	    }

#if	CF_DEBUGS
	    debugprintf("nprintf: flen=%d\n",flen) ;
#endif

	    if (fl > 0) {
		const int	cmd = F_LOCK ;
		if ((rs = uc_lockf(fd,cmd,0L)) >= 0) {
	            rs = uc_writen(fd,fbuf,fl) ;
		    len = rs ;
		}
	    } /* end if */

	    u_close(fd) ;
	} /* end if (open) */

#if	CF_DEBUGS
	debugprintf("nprintf: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (nprintf) */


