/* fill */

/* Fill out text to 70 columns */


/* revision history:

	= 1988-06-01, David A­D­ Morano


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This program will fill out a file containing text in the form of
	words.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<fcntl.h>

#include	<bfile.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	bfile	in, out, err ;

	int	rs ;
	int	c, c2 ;
	int	exit = FALSE, comment = FALSE, leading = FALSE ;
	int	inside = 0 ;
	int	tabcount ;
	int	i, len ;
	int	inlen, outlen ;

	char	inbuf[LINEBUFLEN], outbuf[LINEBUFLEN * 4] ;


	rs = bopen(&err,2,"w",0666) ;
	if (rs < 0) return (rs) ;


	switch (argc) {

	case 1:
	    rs = bopen(&in,0,"r",0666) ;
	    if (rs < 0) return (rs) ;

	    rs = bopen(&out,1,"w",0666) ;
	    if (rs < 0) return (rs) ;

	    break ;

	case 2:
	    rs = bopen(&in,argv[1],"r",0666) ;
	    if (rs < 0) return (rs) ;

	    rs = bopen(&out,1,"w",0666) ;
	    if (rs < 0) return (rs) ;

	    break ;

	case 3:
	default:
	    rs = bopen(&in,argv[1],"r",0666) ;
	    if (rs < 0) return (rs) ;

	    rs = bopen(&out,argv[2],"w",0666) ;
	    if (rs < 0) return (rs) ;

	    break ;

	} ;  /* end switch */



	while ((inlen = breadline(&in,inbuf,LINEBUFLEN)) > 0) {







	} ; /* end outer while */


	bclose(&in) ;

	bclose(&out) ;

	bclose(&err) ;

	return OK ;
}
/* end subroutine (main) */


struct wordstat {
	char	*cp ;
	int	lenr ;
	char	*fp ;
	int	flen ;
} ;


int word(wsbp)
struct wordstat	*wsbp ;
{
	int	pos = wsbp->lenr, lenr ;

	char	*cp ;


	while (! isword(*cp)) {
		lenr -= 1 ;
		cp += 1 ;
	}

	wsb->fp = cp ;

	while (isword(*cp)) {
		lenr -= 1 ;
		cp += 1 ;
	}

	wsbp->lenr = lenr ;
	wsbp->flen = cp - wsbp->fp ;
	return (pos - wsbp->lenr) ;
}


int isword(c)
char	c ;
{
	int	sc = c | 0x20 ;

	if ((sc >= 'a' && sc <= 'z') || (c >= '0' && c <= '9') ||
		(c == '-') || (c == '_') || (c == "'")) return YES ;

	else return NO ;
}



