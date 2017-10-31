/* fbprintf */

/* write binary data to a STDIO stream */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-08-17, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a knock-off of the 'fwrite(3)' subroutine, only sensible!


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdarg.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	bufprintf(char *,int,const char *,...) ;
extern int	vbufprintf(char *,int,const char *,va_list) ;

extern int	fbwrite(FILE *,cchar *,int) ;


/* exported subroutines */


int fbprintf(FILE *fp,cchar *fmt,...)
{
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		flen ;
	int		wlen = 0 ;
	char		lbuf[LINEBUFLEN + 1] ;

	if (fp == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = vbufprintf(lbuf,llen,fmt,ap) ;
	    flen = rs ;
	    va_end(ap) ;
	}

	if ((rs >= 0) && (flen > 0)) {
	    rs = fbwrite(fp,lbuf,flen) ;
	    wlen = rs ;
	}

	if ((rs == 0) && ferror(fp)) {
	   clearerr(fp) ;
	   rs = SR_IO ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (fbprintf) */


