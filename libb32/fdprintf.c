/* fdprintf */

/* 'FileDescriptor' printf subroutine */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_EPRINT	1		/* link in '??' support */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine performs a 'printf' like function but to the named file
        which is passed as the first argument.

	Synopsis:

	int fdprintf(fd,fmt,ap)
	int		fd ;
	const char	fmt[] ;
	va_alist	ap ;

	Arguments:

	fd		file descriptor to write to
	format		standard format string
	...		enverything else

	Returns:

	>=0		length of data (in bytes) written to file
	<0		failure


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#undef	BUFLEN
#define	BUFLEN		512


/* external subroutines */

extern int	format(char *,int,int,const char *,va_list) ;


/* local variables */


/* exported subroutines */


int fdprintf(int fd,const char *fmt,...)
{
	va_list	ap ;

	int	rs = SR_OK ;
	int	len = 0 ;

	char	buf[BUFLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("fdprintf: fd=%d\n",fd) ;
#endif

	if (fd < 0) {

#if	CF_EPRINT
		fd = debuggetfd() ;
#else
		return SR_NOTOPEN ;
#endif

	} /* end if */

#if	CF_DEBUGS
	debugprintf("fdprintf: fd=%d\n",fd) ;
#endif

#if	CF_DEBUGS
	debugprintf("fdprintf: fmt=%s\n",fmt) ;
#endif

	va_begin(ap,fmt) ;

	len = format(buf,BUFLEN,1,fmt,ap) ;

	va_end(ap) ;

#if	CF_DEBUGS
	    debugprintf("fdprintf: len=%d\n",len) ;
#endif

	if (len > 0)
	    rs = uc_writen(fd,buf,len) ;

#if	CF_DEBUGS
	debugprintf("fdprintf: rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (fdprintf) */



