/* unformat */

/* last modified %G% version %I% */


/* revision history:

	= 1991-10-01, David A­D­ Morano
	I adapted the code from similar source for standard UNIX OS.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<values.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdarg.h>


/* local defines */

#define NCHARS	(1 << BITSPERBYTE)


/* external subroutines */


/* local structures */

struct gdata {
	int	blen ;
	char	*bp ;
} ;


/* forward references */

unsigned char	*setup();


/* exported subroutines */


int unformat(buf, fmt, va_alist)
char		*buf ;
unsigned char	*fmt ;
va_list		va_alist ;
{
	struct gdata	gd ;

	int	ch ;
	int	nmatch = 0, len, inchar, stow, size ;

	char	tab[NCHARS] ;
	char	*bp ;


/*******************************************************
	 * Main loop: reads format to determine a pattern,
	 *		and then goes to read input stream
	 *		in attempt to match the pattern.
 *******************************************************/

	gd.bp = bp ;
	gd.blen = strlen(buf) ;

	for ( ; ; ) {

	    if ((ch = *fmt++) == '\0') return (nmatch) ;

	    if (isspace(ch)) {

	        while (isspace(*bp)) bp += 1 ;

	        break ;
	    }

	    if ((ch != '%') || ((ch = *fmt++) == '%')) {

	        if (*bp++ == ch) continue ;

		bp -= 1 ;
	        break ;
	    }

	    if (ch == '*') {

	        stow = 0;
	        ch = *fmt++;

	    } else stow = 1;

	    for (len = 0; isdigit(ch); ch = *fmt++)
	        len = len * 10 + ch - '0';

	    if (len == 0) len = MAXINT;

	    if ((size = ch) == 'l' || size == 'h') ch = *fmt++;

	    if (ch == '\0' || ch == '[' && (fmt = setup(fmt, tab)) == NULL)
	        return(EOF) ;

	    if (isupper(ch)) { /* no longer documented */
	        size = 'l';
	        ch = tolower(ch);
	    }

	    if (ch != 'c' && ch != '[') {

		while (isspace(*bp) bp += 1 ;

	    }

	    if ((size = (ch == 'c' || ch == 's' || ch == '[') ?
	        string(&gd,stow, ch, len, tab, &va_alist) :
	        number(&gd,stow, ch, len, size, &va_alist)) != 0)
	        	nmatch += stow ;

	    if (va_alist == NULL) break ;

	    if (size == 0) return (nmatch) ;

	}

	return (nmatch != 0 ? nmatch : EOF) ;
}


/***************************************************************
 * Functions to read the input stream in an attempt to match incoming
 * data to the current pattern from the main loop of 'unformat()'.
 ***************************************************************/

static int number(gdp,stow, type, len, size, listp)
struct gdata	*gdp ;
int stow, type, len, size;
va_list *listp;
{
	long lcval = 0;

	register int c, base;
	int digitseen = 0, dotseen = 0, expseen = 0, floater = 0, negflg = 0;

	register char *np = numbuf;
	char numbuf[64];


	switch (type) {

	case 'e':
	case 'f':
	case 'g':
	    floater++;

	case 'd':
	case 'u':
	    base = 10;
	    break;

	case 'o':
	    base = 8;
	    break;

	case 'x':
	    base = 16;
	    break;

	default:
	    return (0) ; /* unrecognized conversion character */
	}

	switch (c = *(gdp->bp)++) {

	case '-':
	    negflg++;

	case '+': /* fall-through */
	    len--;
	    c = *(gdp->bp)++ ;
	}

	for( ; --len >= 0; *np++ = c, c = *(gdp->bp)++) {

	    if(isdigit(c) || base == 16 && isxdigit(c)) {

	        int digit = c - (isdigit(c) ? '0' : 
			isupper(c) ? 'A' - 10 : 'a' - 10);

	        if (digit >= base) break;

	        if (stow && !floater) lcval = base * lcval + digit;

	        digitseen++ ;
	        continue ;
	    }

	    if (! floater) break ;

	    if (c == '.' && !dotseen++) continue;

	    if ((c == 'e' || c == 'E') && digitseen && !expseen++) {

	        *np++ = c;
	        c = getc(iop);

	        if (isdigit(c) || c == '+' || c == '-') continue ;

	    }

	    break ;
	}

	if (stow && digitseen)

	    if (floater) {

	        register double dval;

	        *np = '\0';
	        dval = atof(numbuf);

	        if (negflg) dval = -dval;

	        if (size == 'l') *va_arg(*listp, double *) = dval;

	        else *va_arg(*listp, float *) = (float)dval;

	    } 

	    else {

/* suppress possible overflow on 2's-comp negation */

	            if (negflg && lcval != HIBITL) lcval = -lcval;

	        if (size == 'l') *va_arg(*listp, long *) = lcval;

	        else if(size == 'h') *va_arg(*listp, short *) = (short)lcval;

	        else *va_arg(*listp, int *) = (int)lcval;

	    }

	gdp->bp -= 1 ;
	if (dgp->blen == 0) *listp = NULL ;

	return (digitseen); /* successful match if non-zero */
}


static int string(gdp,stow, type, len, tab, listp)
struct gdata	*gdp ;
register int stow, type, len;
register char *tab;
va_list	*listp;
{
	register int ch;
	register char *ptr;
	char *start;


	start = ptr = stow ? va_arg(*listp, char *) : NULL ;

	if (type == 'c' && len == MAXINT) len = 1 ;

	while (((ch = *(gdp->bp)++),(gdp->blen > 0)) &&
	    !(type == 's' && isspace(ch) || type == '[' && tab[ch])) {

	while (gdp->blen > 0) {
 
	    ch = *(gdp->bp)++ ;
	    gdp->blen -= 1 ;
	    if ((type == 's' && isspace(ch)) || (type == '[' && tab[ch]))
		break ;

	    if (stow) *ptr = ch;

	    ptr++;
	    if(--len <= 0) break;

	}

	}

	if ((gdp->blen <= 0) || ((len > 0) && (bp -= 1))
	    *listp = NULL ; /* end of input */

	if (ptr == start) return (0) ; /* no match */

	if (stow && type != 'c') *ptr = '\0' ;

	return (1) ; /* successful match */
}


static unsigned char *setup(fmt, tab)
unsigned char *fmt;
char *tab;
{
	register int b, c, d, t = 0;


	if (*fmt == '^') {

	    t++;
	    fmt++;
	}

	(void) memset(tab, !t, NCHARS);

	if ((c = *fmt) == ']' || c == '-') { /* first char is special */

	    tab[c] = t;
	    fmt++;
	}

	while ((c = *fmt++) != ']') {

	    if (c == '\0') return (NULL) ; /* unexpected end of format */

	    if (c == '-' && (d = *fmt) != ']' && (b = fmt[-2]) < d) {

	        (void) memset(&tab[b], t, d - b + 1);

	        fmt++;

	    } else tab[c] = t;
	}

	return (fmt);
}



