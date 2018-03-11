/* convdec */

/* subroutine to convert integers to strings */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0	/* turn on debugging */
#define	CF_LLTOSTR	0	/* use Solaris 'lltostr(3c)' subroutine */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        This subroutine was adapted from an old version that was used in the old
        days at AT&T Bell Laboratores (known as Bell Telephone Laboratories back
        then). In those days, processors did not have native hardware support
        (or "good" native hardware support) for 64-bit (or sometimes not even
        32-bit) division.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        The 'convdec()' subroutine converts the unsigned long (64-bit) integer
        "lval" to printable decimal and places it in a buffer identified as
        relative to 'endptr' (a pointer to a char buffer). The value returned is
        the address of the first non-zero character.

        Short of assembly language magical smart stuff (which I have coded
        elsewhere), this implementation of decimal conversion should be among
        the fastest (despite its simplicity).

	Synopsis:

	char *convdecu(lval,endptr)
	ULONG		lval ;
	char		*endptr ;

	Arguments:

	lval		value (unsigned) to be converted
	endptr		pointer to one byte beyond the end of a result buffer
	
	Returns:

	-		pointer to start of convered string
	
	Implementation note:

        We develop the decimal digits in two stages. Since speed counts here we
        do it in two loops. The first loop gets "lval" ("long value") down until
        it is no larger than INT_MAX. The second loop uses integer divides
        rather than long divides to speed it up.

        Also note that we do not use *both* a division and a modulus operation
        to develop the digits (as some people do). Instead we use a single
        division and a multiplication to develop the digits. If we did this in
        assembly language on some architectures we could get a division and a
        modulus result with a single combined division-modulus instruction. But
        in C language (a high-level language) we do not have this luxury, unless
        a compiler is super smart and can somehow figure out that a single
        devision-modulus instruction can provide both results at different
        points in a a loop but each using the same operands. Some compilers have
        claimed to be able to do this, but in general we do not want to rely on
        that (rather remote) possibility here.

        Of course, we assume that doing both a single division and a
        multiplication operation per loop iteration is over all faster than
        doing both a division and a modulus operation per loop iteration. This
        seems to be a reasonable assumption. On platform architectures that do
        not have a good hardware division instruction (performance), this can be
        a huge win.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */

#define MAXDECDIG_I	10		/* decimal digits in 'int' */
#define MAXDECDIG_UI	10		/* decimal digits in 'uint' */
#define MAXDECDIG_L	10		/* decimal digits in 'long' */
#define MAXDECDIG_UL	10		/* decimal digits in 'ulong' */
#define MAXDECDIG_L64	19		/* decimal digits in 'long64' */
#define MAXDECDIG_UL64	20		/* decimal digits in 'ulong64' */

#ifndef	DIGBUFLEN
#define DIGBUFLEN	40		/* can hold int128_t */
#endif

#ifndef	ULONG_MAXPOW10
#define	ULONG_MAXPOW10	(MAXDECDIG_UL64-1)
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strdcpy1(char *,int,const char *) ;


/* local structures */


/* forward references */

char	*convdecu(ULONG,char *) ;


/* local variables */


/* exported subroutines */


#if	CF_LLTOSTR

char *convdecu(ULONG unum,char *endptr)
{
	char		*bp ;

	*endptr = '\0' ;
	bp = ulltostr(unum,endptr) ;

	return bp ;
}
/* end subroutine (convdecu) */

#else /* CF_LLTOSTR */

char *convdecu(ULONG lval,char *endptr)
{
	char		*bp = endptr ;

	*endptr = '\0' ;		/* 'ulltostr()' does not do this */

/* zero is a special case */

	if (lval > 0) {

/* execute the first loop to get value down to <= INT_MAX */

	    {
	        ULONG	nv ;
	        while (lval > INT_MAX) {
	            nv = lval / 10 ;
	            *--bp = (char) ((lval - (nv * 10)) + '0') ;
	            lval = nv ;
	        } /* end while */
	    }

/* this activity does not lose precision since 'sval' is <= INT_MAX */

	    {
	        uint	sval = (uint) lval ;
	        uint	nv ;
	        while (sval > 0) {
	            nv = sval / 10 ;
	            *--bp = (char) ((sval - (nv * 10)) + '0') ;
	            sval = nv ;
	        } /* end while */
	    }

	} else
	    *--bp = '0' ;

	return bp ;
}
/* end subroutine (convdecu) */

#endif /* CF_LLTOSTR */

char *convdecs(LONG num,char *endptr)
{
	ULONG	unum = (ULONG) num ;
	char	*bp ;

	if (num < 0) unum = (- unum) ;
	bp = convdecu(unum,endptr) ;
	if (num < 0) *--bp = '-' ;

	return bp ;
}
/* end subroutine (convdecs) */


