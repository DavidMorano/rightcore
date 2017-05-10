/* address */

/* process mail address strings */



#include	<string.h>
#include	<stdio.h>

#include	"localmisc.h"
#include	"defs.h"



#define TO_LEN		4096
#define SUBJ_LEN	4096



/* external subroutines */

extern char	*strshrink() ;


/* forward */

char	*ma_path() ;




/* get the next address in the address string list */

char *ma_next(s)
char	s[] ;
{
	int	c_quote = 0 ;
	int	c_comment = FALSE ;
	int	f_end = FALSE ;
	int	f_allow = FALSE ;

	char	*cp, *cp1, *cp2 ;


	cp = s ;
	while (ISWHITE(*cp)) cp += 1 ;

	cp2 = cp ;

	c_comment = 0 ;
	c_quote = 0 ;
	f_end = FALSE ;
	while (*cp != '\0') {

	    switch (*cp) {

	    case '\\':
	        if ((c_quote > 0) && (cp[1] != '\0'))
	            cp += 1 ;

	        break ;

	    case '"':
	        if (c_quote == 1)
	            c_quote = 0 ;

	        else 
	            c_quote = 1 ;

	        break ;

	    case '(':
	        if (c_quote == 0) c_comment += 1 ;

	        break ;

	    case ')':
	        if ((c_quote == 0) && (c_comment > 0))
	            c_comment -= 1 ;

	        break ;

	    case ',':
	        if ((c_quote == 0) && (c_comment == 0))
	            f_end = TRUE ;

	        break ;

	    } /* end switch */

	    if (f_end) break ;

	    cp += 1 ;

	} /* end while */

	if (f_end) *cp = '\0' ;

	return cp2 ;
}
/* end subroutine (ma_next) */


/* extract a mail route path out of the address */
char *ma_path(p)
char	p[] ;
{
	int	c_quote = 0 ;
	int	c_comment = 0 ;
	int	f_white ;

	char	*cp, *cp1, *cp2 ;
	char	buf[4096], *bp = buf ;


	cp1 = ma_next(p) ;

	if ((cp = strchr(cp1,'<')) != NULL) {

	    cp += 1 ;
	    if ((cp2 = strchr(cp,'>')) != NULL) *cp2 = '\0' ;

	} else {

	    c_comment = 0 ;
	    cp = cp1 ;
	    while (*cp1 != '\0') {

	        f_white = FALSE ;
	        switch (*cp) {

	        case '\\':
	            if ((c_quote > 0) && (cp1[1] != '\0'))
	                *bp++ = *cp1++ ;

	            if (c_comment == 0)
	                *bp++ = *cp1++ ;

	            else
	                cp1 += 1 ;

	            break ;

	        case '"':
	            if (c_quote > 0) {

	                c_quote = 0 ;

	            } else {

	                c_quote = 1 ;

	            }

/* fall through to the next case */

		default:
	            if (c_comment == 0)
	                *bp++ = *cp1++ ;

	            else
	                cp1 += 1 ;

	            break ;

	        case '(':
	            if (c_quote == 0) {

	                c_comment += 1 ;
	                cp1 += 1 ;

	            } else {

	                if (c_comment == 0)
	                    *bp++ = *cp1++ ;

	                else
	                    cp1 += 1 ;

	            }
	            break ;

	        case ')':
	            if (c_comment == 0)
	                *bp++ = *cp1++ ;

	            else
	                cp1 += 1 ;

	            if ((c_quote == 0) && (c_comment > 0))
	                c_comment -= 1 ;

	            break ;

	        case ' ':
	        case '\t':
	        case '\n':
	        case '\v':
	        case '\r':
	            if ((c_quote > 0) && (c_comment == 0))
	                *cp++ = *cp1++ ;

	            else
	                cp1 += 1 ;

	        } /* end switch */

	        if ((c_comment == 0) && (! f_white))
	            *bp++ = *cp1 ;

	        cp1 += 1 ;

	    } /* end while */

	    *bp = '\0' ;
	    cp = p ;
	    strcpy(p,strshrink(buf)) ;

	} /* end if */

	return cp ;
}
/* end subroutine (ma_path) */


