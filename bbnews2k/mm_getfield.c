/* mm_getfield */

/* get the value of the specified header within specified message */


#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1995-05-01, David A­D­ Morano

	This code module was completely rewritten to 
	replace any original garbage that was here before.


*/

/* Copyright © 1995,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These are subroutines to aid in handling parts of a message.

	IMPORTANT NOTE:
	This subroutine is NOT RFC-822 compliant !!

	This routine searches a mail message (MM) for a given header
	and returns the value of the header to the caller.
	If the header appears in the message more than once, all found
	header value strings are returned separated by a comma.


	Arguments:

		mfp	Basic I/O file pointer to mail message file
		offset	byte offset from the beginning of the mail message
			file to the start of the message.
		mlen	message length (length of message itself in file)
		h	header string to search for
		fvalue	user specified buffer to hold the resulting header
			value (if found)
		buflen	user specifled length of supplied buffer
	

	return 'length-of-field'
		if header was found ('fvalue' is the returned field)

	return 'BAD' 
		if that header is not found in the message


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<char.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* external subroutines */

extern int	hmatch(cchar *,cchar *) ;


/* external variables */

#if	CF_DEBUG
extern struct proginfo	g ;
#endif


/* forward references */


/* local variables */


/* exported subroutines */


int mm_getfield(mfp,offset,mlen,h,fvalue,buflen)
bfile		*mfp ;
offset_t	offset ;
int		mlen ;
const char	h[] ;
char		fvalue[] ;
int		buflen ;
{
	offset_t	moff ;
	const int	llen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		i, l, ml, len ;
	int		flen = 0 ;
	int		f_bol, f_eol ;
	int		f_lookmore = FALSE ;
	int		f_boh = FALSE ;
	const char	*cp ;
	char		lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUG
	if (BATST(g.uo,UOV_CF_DEBUG)) 
		errprintf(
	    "mm_getfield: trying %s\n",h) ;
#endif

	moff = offset ;
	if (bseek(mfp,moff,SEEK_SET) < 0) 
		return BAD ;

	buflen -= 3 ;
	fvalue[0] = '\0' ;
	len = 0 ;
	f_bol = TRUE ;
	while ((len < mlen) && (flen < buflen) &&
	    ((l = breadline(mfp,lbuf,LINEBUFLEN)) > 0)) {

#if	CF_DEBUG
	    if (BATST(g.uo,UOV_CF_DEBUG)) 
		errprintf(
	        "mm_getfield: got line\n%W",lbuf,l) ;
#endif

	    len += l ;
	    if (lbuf[0] == '\n') 
		break ;

	    f_eol = FALSE ;
	    if (lbuf[l - 1] == '\n')
	        ((f_eol = TRUE), (l -= 1)) ;

	    lbuf[l] = '\0' ;

	    ml = 0 ;
	    if (f_bol) {

	        if (f_lookmore && CHAR_ISWHITE(lbuf[0])) {

	            cp = lbuf ;
	            while (CHAR_ISWHITE(*cp)) 
			cp += 1 ;

	            ml = MIN((lbuf + l - cp),(buflen - flen)) ;

	        } else if ((i = hmatch(h,lbuf)) > 0) {

	            f_lookmore = TRUE ;
	            f_boh = TRUE ;
	            cp = lbuf + i ;
	            while (CHAR_ISWHITE(*cp)) 
			cp += 1 ;

	            ml = MIN((lbuf + l - cp),(buflen - flen)) ;

	        } else {

	            f_lookmore = FALSE ;
	            f_boh = FALSE ;
	        }

	    } else if (f_lookmore) {

	        cp = lbuf ;
	        ml = MIN(l,(buflen - flen)) ;

	    }

	    if (ml > 0) {

	        if (flen > 0) {

	            if (f_boh) 
			fvalue[flen++] = ',' ;

	            fvalue[flen++] = ' ' ;
	        }

	        f_boh = FALSE ;
	        strncpy(fvalue + flen,cp,ml) ;

	        flen += ml ;
	        fvalue[flen] = '\0' ;

	    } /* end if */

	    f_bol = f_eol ;
	} /* end while */

#if	CF_DEBUG
	if (BATST(g.uo,UOV_CF_DEBUG))
	    errprintf("mm_getfield: returning OK w/ field \n\"%s\"\n",
	        fvalue) ;
#endif

	fvalue[flen] = '\0' ;
	return ((flen > 0) ? flen : BAD) ;
}
/* end subroutine (mm_getfield) */


