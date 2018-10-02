/* filebinary */

/* determine if named file is an binary file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

	= 2018-10-01, David A. Morano
	I refactored for some clarity.

*/

/* Copyright © 1998,2018 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	Given a file-name we determine if it is an binary file. In other words,
	we detemine if the passed file (reference by filename) contains binary
	data or not. Binary data is any data that is not a normal character in
	each byte position of the file. Non-normal characters are any
	characters lying within the control-0 or control-1 range of the
	ISO-8859-1 (otherwise know as ISO Latin-1) character set. We also
	recognize as normal characters all of the old C-language escape
	characters (not escaping an existing normal character). The present
	C-language escapes which we recognize are:
	
	\a
	\b
	\f
	\n
	\r
	\t
	\v

	Enjoy.
	
	
	Synopsis:

	int filebinary(fname)
	const char	fname[] ;

	Arguments:

	fname		file-path to check

	Returns:

	<0		error
	==0		not an object file
	>0		is an object file


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */


/* local structures */


/* forward references */

static int	hasbinary(cchar *,int) ;
static int	isbinary(int) ;
static int	isc0(int) ;
static int	isc1(int) ;


/* local variables */

static cchar	allowed[] = "\a\b\f\n\r\t\v" ;


/* exported subroutines */


int filebinary(cchar *fname)
{
	int		rs ;
	int		f = FALSE ;

	if (fname == NULL) return SR_FAULT ;
	if (fname[0] == '\0') return SR_INVALID ;

	if ((rs = uc_open(fname,O_RDONLY,0666)) >= 0) {
	    USTAT	sb ;
	    const int	fd = rs ;

	    if (((rs = u_fstat(fd,&sb)) >= 0) && S_ISREG(sb.st_mode)) {
		const int	llen = LINEBUFLEN ;
		char		lbuf[LINEBUFLEN+1] ;
		while ((rs = u_read(fd,lbuf,llen)) > 0) {
		    f = hasbinary(lbuf,rs) ;
		    if (f) break ;
		} /* end while */
	    } /* end if (stat) */

	    u_close(fd) ;
	} /* end if (open) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (filebinary) */


/* local subroutines */


static int hasbinary(cchar *sp,int sl)
{
	int	f = FALSE ;
	while (sl-- && sp[0]) {
	    f = isbinary(sp[0]) ;
	    if (f) break ;
	    sp += 1 ;
	} /* end while */
#if	CF_DEBUGS
	if (f) debugprintf("filebinary: ch=%02x\n",MKCHAR(sp[0])) ;
#endif
	return f ;
}
/* end subroutine (hasbinary) */


static int isbinary(int ch)
{
	int		f = FALSE ;
	ch &= UCHAR_MAX ;
	if (isc0(ch)) {
	    f = (ch != '\n') && (strchr(allowed,ch) == NULL) ;
	} else if (isc1(ch)) {
	    f = TRUE ;
	}
	return f ;
}
/* end subroutine (isbinary) */


static int isc0(int ch)
{
	return (ch < 0x20) ;
}
/* end subroutine (isc0) */


static int isc1(int ch)
{
	return ((ch >= 0x80) && (ch < 0xA0)) ;
}
/* end subroutine (isc1) */

