/* sprintf */

/* subroutine to format string output */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SPRINTF	1		/* include 'sprintf()' */
#define	CF_VSNPRINTF	1		/* include 'vsnprintf()' */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This was written from the original 'bprintf()' subroutine.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used by 'printf' type routines to format an output
        string from a format specification. This routine has no support for
        floating point conversion since floating point formats are not general
        enough for the most portable applications.

	The following nonstandard additions are supported :

	%W		counted byte string output like 'write(2)'
	%x.xH		terminal cursor positioning, 2 integer arguments (r.c)
	%xA		the usual cursor movements
	%xB
	%xC
	%xD
	%xK		x=0,1,2
	%xJ		x=0,1,2

	Not implemented yet :

	%xM		delete line, x=number of lines 
	%S		save cursor position
	%R		restore cursor position


*******************************************************************************/


#define	PWD_INCLUDE		0
#define	GRP_INCLUDE		0
#define	SHADOW_INCLUDE		0

#define	_STDIO_H		1
#define	_ISO_STDIO_ISO__H	1
#define	_PWD_H			1
#define	_GRP_H			1
#define	_SHADOW_H		1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	MAXLEN		(MAXPATHLEN + 40)


/* external subroutines */

extern int	format(char *,int,int,const char *,va_list) ;


/* forward references */


/* structure definitions */


/* standard routine to format a string to a memory buffer */

#if	CF_SPRINTF

int sprintf(char buf[],const char fmt[],...)
{
	int		len = 0 ;

	{
	va_list	ap ;
	va_begin(ap,fmt) ;
	len = format(buf,MAXLEN,0,fmt,ap) ;
	va_end(ap) ;
	}

	return (len < 0) ? (- len) : len ;
}
/* end subroutine (sprintf) */

#endif /* CF_SPRINTF */


int vsprintf(char buf[],const char fmt[],va_list ap)
{
	int		len ;

	len = format(buf,MAXLEN,0,fmt,ap) ;

	return (len < 0) ? (- len) : len ;
}
/* end subroutine (vsprintf) */


/* standard routine to format a string to a memory buffer */
int snprintf(char buf,int buflen,const char fmt[],...)
{
	int		rs = SR_FAULT ;
	int		len = 0 ;

	if (buf != NULL) {
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    len = format(buf,buflen,0,fmt,ap) ;
	    va_end(ap) ;
	}

	return (len >= 0) ? len : (- len) ;
}
/* end subroutine (snprintf) */


#if	CF_VSNPRINTF

int vsnprintf(buf,buflen,fmt,ap)
char	buf[] ;
int	buflen ;
char	fmt[] ;
va_list	ap ;
{
	int	len ;


#if	CF_DEBUGS
	{
	int	fl = strlen(fmt) ;
	while (fl && (fmt[fl - 1] == '\n')) {
		fl -= 1 ;
	}
	debugprintf("vsnprintf: fmt=>%t<\n",fmt,fl) ;
	}
#endif /* CF_DEBUGS */

	d.rs = OK ;
	len = format(buf,buflen,0,fmt,ap) ;

	return (len < 0) ? (- len) : len ;
}
/* end subroutine (vsnprintf) */

#endif /* CF_VSNPRINTF */


