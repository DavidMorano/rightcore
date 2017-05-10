/* mm_getfield */


#define	CF_DEBUG	0


/* get the value of the specified header within specified message */



#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<errno.h>
#include	<unistd.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<baops.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* external variables */

#if	CF_DEBUG
extern struct global	g ;
#endif


/* forward references */

int	hmatch() ;




/****************************************************************************

	This routine searches a mail message (MM) for a given header
	and returns the value of the header to the caller.
	If the header appears in the message more than once, all found
	header value strings are returned separated by a comma.

	Arguments:
		mfp	Basic I/O file pointer to mail message file
		offset	byte offset from the beginning of the mail message
			file to the start of the message
		mlen	message length (length of message itself in file)
		h	header string to search for
		fvalue	user specified buffer to hold the resulting header
			value (if found)
		buflen	user specifled length of supplied buffer
	

	return 'length-of-field'
		if header was found ('fvalue' is the returned field)

	return 'BAD' 
		if that header is not found in the message


****************************************************************************/



int mm_getfield(mfp,offset,mlen,h,fvalue,buflen)
bfile	*mfp ;
offset_t	offset ;
int	mlen ;
char	h[] ;
char	fvalue[] ;
int	buflen ;
{
	int	i, l, ml, flen = 0, len ;
	int	f_bol, f_eol ;
	int	f_lookmore = FALSE ;
	int	f_boh = FALSE ;

	char	linebuf[LINELEN + 1], *cp ;


#if	CF_DEBUG
	if (BATST(g.uo,UOV_CF_DEBUG)) errprintf(
	    "mm_getfield: trying %s\n",h) ;
#endif

	if (bseek(mfp,offset,SEEK_SET) < 0) return BAD ;

	buflen -= 3 ;
	fvalue[0] = '\0' ;
	len = 0 ;
	f_bol = TRUE ;
	while ((len < mlen) && (flen < buflen) &&
	    ((l = breadline(mfp,linebuf,LINELEN)) > 0)) {

#if	CF_DEBUG
	    if (BATST(g.uo,UOV_CF_DEBUG)) errprintf(
	        "mm_getfield: got line\n%W",linebuf,l) ;
#endif

	    len += l ;
	    if (linebuf[0] == '\n') break ;

	    f_eol = FALSE ;
	    if (linebuf[l - 1] == '\n')
	        ((f_eol = TRUE), (l -= 1)) ;

	    linebuf[l] = '\0' ;

	    ml = 0 ;
	    if (f_bol) {

	        if (f_lookmore && ISWHITE(linebuf[0])) {

	            cp = linebuf ;
	            while (ISWHITE(*cp)) cp += 1 ;

	            ml = MIN((linebuf + l - cp),(buflen - flen)) ;

	        } else if ((i = hmatch(h,linebuf)) > 0) {

	            f_lookmore = TRUE ;
	            f_boh = TRUE ;
	            cp = linebuf + i ;
	            while (ISWHITE(*cp)) cp += 1 ;

	            ml = MIN((linebuf + l - cp),(buflen - flen)) ;

	        } else {

	            f_lookmore = FALSE ;
	            f_boh = FALSE ;
	        }

	    } else if (f_lookmore) {

	        cp = linebuf ;
	        ml = MIN(l,(buflen - flen)) ;

	    }

	    if (ml > 0) {

	        if (flen > 0) {

	            if (f_boh) fvalue[flen++] = ',' ;

	            fvalue[flen++] = ' ' ;
	        }

	        f_boh = FALSE ;
	        strncpy(fvalue + flen,cp,ml) ;

	        flen += ml ;
	        fvalue[flen] = '\0' ;

	    }

	    f_bol = f_eol ;

	} /* end while */

#if	CF_DEBUG
	if (BATST(g.uo,UOV_CF_DEBUG))
	    errprintf("mm_getfield: returning OK w/ field \n\"%s\"\n",
	        fvalue) ;
#endif

	fvalue[flen] = '\0' ;
	if (flen > 0) return flen ;

	return BAD ;
}
/* end subroutine (mm_getfield) */



