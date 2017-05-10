/* filebinary */

/* determine if named file is an object file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	Given a file-name we determine if it is an object file (or core file).

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

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;


/* local structures */


/* forward references */

static int	hasbinary(const char *,int) ;
static int	isbinary(int) ;
static int	iseol(int) ;


/* local variables */


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


static int hasbinary(const char *sp,int sl)
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
	int	f = FALSE ;
	ch &= UCHAR_MAX ;
	if (! f) {
	    f = TRUE ;
	    f = f && (ch < 0x20) ;
	    f = f && (ch != CH_TAB) ;
	    f = f && (ch != CH_BS) ;
	    f = f && (ch != CH_FF) ;
	    f = f && (! iseol(ch)) ;
	}
	f = f || ((ch > 0x80) && ((ch&0x7F) < 0x20)) ;
	return f ;
}
/* end subroutine (isbinary) */


static int iseol(int ch)
{
	int	f = FALSE ;
	f = f || (ch == CH_NL) ;
	f = f || (ch == CH_CR) ;
	return f ;
}
/* end subroutine (iseol) */


