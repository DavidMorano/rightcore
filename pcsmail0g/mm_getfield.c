/* mm_getfield */


#define		DEBUG	0



#include	<sys/types.h>
#include	<sys/unistd.h>
#include	<sys/stat.h>
#include	<sys/errno.h>
#include	<sys/param.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<baops.h>

#include	"config.h"
#include	"localmisc.h"

#ifdef	DEBUG
#include	<stdio.h>
#include	"smail.h"
#endif



/* external variables */

#if	DEBUG
extern struct global	g ;
#endif


/* forward references */

int	hmatch() ;




/****************************************************************************

	get the value of the specified header within specified message

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


#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	    "mm_getfield: trying %s\n",h) ;
#endif

	if (bseek(mfp,offset,SEEK_SET) < 0) return BAD ;

	buflen -= 3 ;
	fvalue[0] = '\0' ;
	len = 0 ;
	f_bol = TRUE ;
	while ((len < mlen) && (flen < buflen) &&
	    ((l = breadline(mfp,linebuf,LINELEN)) > 0)) {

#if	DEBUG
	    if (BATST(g.uo,UOV_DEBUG)) errprintf(
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

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG))
	    errprintf("mm_getfield: returning OK w/ field \n\"%s\"\n",
	        fvalue) ;
#endif

	fvalue[flen] = '\0' ;
	if (flen > 0) return flen ;

	return BAD ;
}
/* end subroutine (mm_getfield) */


/***************************************************************************

	Is the initial substring of 'ts' the specified 'hs' string ?  
	Return 0 if there is no match, else we return the
	character position of the header value string.
	The match is case independent.

*/

int hmatch(hs,ts)
char	hs[], ts[] ;
{
	char	*tp = ts, *hp = hs ;


	while (*hp && (*hp != ':')) {

	    if (LOWER(*tp) != LOWER(*hp)) return 0 ;

	    tp += 1 ;
	    hp += 1 ;
	}

	while (ISWHITE(*tp)) tp += 1 ;

	if (*tp != ':') return 0 ;

	return (tp + 1 - ts) ;
}
/* end subroutine (hmatch) */



