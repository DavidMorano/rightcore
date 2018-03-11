/* cfdec */

/* convert a decimal digit string to its binary integer value */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-10-01, David A­D­ Morano
	This subroutine was written adapted from assembly.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Subroutines to convert decimal strings to binary integers.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define MAXDECDIG_I	10		/* decimal digits in 'int' */
#define MAXDECDIG_UI	10		/* decimal digits in 'uint' */
#define MAXDECDIG_L	10		/* decimal digits in 'long' */
#define MAXDECDIG_UL	10		/* decimal digits in 'ulong' */
#define MAXDECDIG_L64	19		/* decimal digits in 'long64' */
#define MAXDECDIG_UL64	20		/* decimal digits in 'ulong64' */

#undef	CFDEC_WEIGHT
#define	CFDEC_WEIGHT	10U


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */

int	cfdeci(cchar *,int,int *) ;


/* local variables */


/* exported subroutines */


int cfdec(s,len,rp)
cchar	s[] ;
int		len ;
int		*rp ;
{


	return cfdeci(s,len,rp) ;
}
/* end subroutine (cfdec) */


/* convert from a decimal string to a signed integer */
int cfdeci(s,len,rp)
cchar	s[] ;
int		len ;
int		*rp ;
{
	uint	val, weight ;

	const int	maxdigs = MAXDECDIG_I ;

	int	rs = SR_OK ;
	int	i, n ;


	val = 0 ;
	*rp = 0 ;
	if (len < 0)
	    len = strlen(s) ;

/* remove white space from the rear of the string */

	while ((len > 0) && isspace(s[len - 1]))
	    len -= 1 ;

	if (len == 0)
	    return SR_INVALID ;

/* convert possible digits */

	weight = 1 ;
	n = 0 ;
	for (i = len - 1 ; (rs >= 0) && (i >= 0) ; i -= 1) {

	    if ((s[i] == '-') || (s[i] == '+') || isspace(s[i]))
	        break ;

/* bad digits? */

	    if ((s[i] > '9') || (s[i] < '0'))
	        rs = SR_INVALID ;

	    if (rs >= 0) {

	        n += 1 ;
	        if ((n == maxdigs) && (s[i] > '2'))
	            rs = SR_OVERFLOW ;

	        if ((rs >= 0) && (n > maxdigs) && (s[i] != '0'))
	            rs = SR_OVERFLOW ;

	        if (rs >= 0) {

	            if (n == maxdigs) {

	                uint	adder ;

	                int	f_msb1, f_msb2 ;


	                adder = ((s[i] - '0') * weight) ;
	                f_msb1 = (val & (~ INT_MAX)) ? 1 : 0 ;
	                f_msb2 = (adder & (~ INT_MAX)) ? 1 : 0 ;
	                val += adder ;

	                if (f_msb1 || f_msb2 || (val & (~ INT_MAX)))
	                    rs = SR_OVERFLOW ;

	            } else
	                val += ((s[i] - '0') * weight) ;

	            if (rs >= 0)
	                weight *= CFDEC_WEIGHT ;

	        }

	    } /* end if */

	} /* end for (looping through digits, back to front) */

	if (rs >= 0) {

	    while ((i > 0) && isspace(s[i]))
	        i -= 1 ;

	    if ((i >= 0) && (s[i] == '-'))
	        val = - val ;

	} /* end if */

/* we are out of here! */
done:
	if (rs >= 0)
	    *rp = (int) val ;

	return rs ;
}
/* end subroutine (cfdeci) */


/* convert from a decimal number that will yield an unsigned integer */
int cfdecui(s,len,rp)
cchar	s[] ;
int		len ;
uint		*rp ;
{
	uint	val, weight ;

	const int	maxdigs = MAXDECDIG_UI ;

	int	rs = SR_OK ;
	int	i, n ;


	val = 0 ;
	*rp = 0 ;
	if (len < 0)
	    len = strlen(s) ;

/* remove white space from the rear of the string */

	while ((len > 0) && isspace(s[len - 1]))
	    len -= 1 ;

	if (len == 0)
	    return SR_INVALID ;

/* convert possible digits */

	weight = 1 ;
	n = 0 ;
	for (i = (len - 1) ; (rs >= 0) && (i >= 0) ; i -= 1) {

	    if ((s[i] == '-') || (s[i] == '+') || isspace(s[i]))
	        break ;

/* bad digits? */

	    if ((s[i] > '9') || (s[i] < '0'))
	        rs = SR_INVALID ;

	    if (rs >= 0) {

	        n += 1 ;
	        if ((n == maxdigs) && (s[i] > '4'))
	            rs = SR_OVERFLOW ;

	        if ((rs >= 0) && (n > maxdigs) && (s[i] != '0'))
	            rs = SR_OVERFLOW ;

	        if (rs >= 0) {

	            if (n == maxdigs) {

	                uint	adder ;

	                int	f_msb1, f_msb2, f_msbr ;


	                adder = ((s[i] - '0') * weight) ;
	                f_msb1 = (val & (~ INT_MAX)) ? 1 : 0 ;
	                f_msb2 = (adder & (~ INT_MAX)) ? 1 : 0 ;
	                val += adder ;
	                f_msbr = (val & (~ INT_MAX)) ? 1 : 0 ;

	                if ((f_msb1 && f_msb2) ||
	                    ((f_msb1 || f_msb2) && (! f_msbr)))
	                    rs = SR_OVERFLOW ;

	            } else
	                val += ((s[i] - '0') * weight) ;

	            if (rs >= 0)
	                weight *= CFDEC_WEIGHT ;

	        }

	    } /* end if */

	} /* end for */

	if (rs >= 0) {

	    while ((i > 0) && isspace(s[i]))
	        i -= 1 ;

	    if ((i >= 0) && (s[i] == '-'))
	        val = - val ;

	} /* end if */

done:
	if (rs >= 0)
	    *rp = (uint) val ;

	return rs ;
}
/* end subroutine (cfdecui) */


/* convert from a decimal number that will yield a long integer */
int cfdecl(s,len,rp)
cchar	s[] ;
int		len ;
long		*rp ;
{
	ulong	val, weight ;

	int	rs = SR_OK ;
	int	i, n ;
	int	maxdigs = MAXDECDIG_L ;


	if (sizeof(long) > 4)
	    maxdigs = MAXDECDIG_L64 ;

	val = 0 ;
	*rp = 0 ;
	if (len < 0)
	    len = strlen(s) ;

/* remove white space from the rear of the string */

	while ((len > 0) && isspace(s[len - 1]))
	    len -= 1 ;

	if (len == 0)
	    return SR_INVALID ;

/* convert possible digits */

	weight = 1 ;
	n = 0 ;
	for (i = (len - 1) ; (rs >= 0) && (i >= 0) ; i -= 1) {

	    if ((s[i] == '-') || (s[i] == '+') || isspace(s[i]))
	        break ;

/* bad digits? */

	    if ((s[i] > '9') || (s[i] < '0'))
	        rs = SR_INVALID ;

	    if (rs >= 0) {

	        n += 1 ;
	        if ((n > maxdigs) && (s[i] != '0'))
	            rs = SR_OVERFLOW ;

	        if (rs >= 0) {

	            if (n == maxdigs) {

	                ulong	adder ;

	                int	f_msb1, f_msb2 ;


	                adder = ((s[i] - '0') * weight) ;
	                f_msb1 = (val & (~ LONG_MAX)) ? 1 : 0 ;
	                f_msb2 = (adder & (~ LONG_MAX)) ? 1 : 0 ;
	                val += adder ;

	                if (f_msb1 || f_msb2 || (val & (~ LONG_MAX)))
	                    rs = SR_OVERFLOW ;

	            } else
	                val += ((s[i] - '0') * weight) ;

	            if (rs >= 0)
	                weight *= CFDEC_WEIGHT ;

	        }

	    } /* end if */

	} /* end for */

	if (rs >= 0) {

	    while ((i > 0) && isspace(s[i]))
	        i -= 1 ;

	    if ((i >= 0) && (s[i] == '-'))
	        val = - val ;

	} /* end if */

done:
	if (rs >= 0)
	    *rp = (long) val ;

	return rs ;
}
/* end subroutine (cfdecl) */


/* convert from a decimal number that will yield an unsigned long integer */
int cfdecul(s,len,rp)
cchar	s[] ;
int		len ;
ulong		*rp ;
{
	ulong	val, weight ;

	int	rs = SR_OK ;
	int	i, n ;
	int	maxdigs = MAXDECDIG_UL ;


	if (sizeof(long) > 4)
	    maxdigs = MAXDECDIG_UL64 ;

	val = 0 ;
	*rp = 0 ;
	if (len < 0)
	    len = strlen(s) ;

/* remove white space from the rear of the string */

	while ((len > 0) && isspace(s[len - 1]))
	    len -= 1 ;

	if (len == 0)
	    return SR_INVALID ;

/* convert possible digits */

	weight = 1 ;
	n = 0 ;
	for (i = (len - 1) ; (rs >= 0) && (i >= 0) ; i -= 1) {

	    if ((s[i] == '-') || (s[i] == '+') || isspace(s[i]))
	        break ;

	    if ((s[i] > '9') || (s[i] < '0'))
	        rs = SR_INVALID ;

	    if (rs >= 0) {

	        n += 1 ;
	        if ((n > maxdigs) && (s[i] != '0'))
	            rs = SR_OVERFLOW ;

	        if (rs >= 0) {

	            if (n == maxdigs) {

	                ulong	adder ;

	                int	f_msb1, f_msb2, f_msbr ;


	                adder = ((s[i] - '0') * weight) ;
	                f_msb1 = (val & (~ LONG_MAX)) ? 1 : 0 ;
	                f_msb2 = (adder & (~ LONG_MAX)) ? 1 : 0 ;
	                val += adder ;
	                f_msbr = (val & (~ LONG_MAX)) ? 1 : 0 ;

	                if ((f_msb1 && f_msb2) ||
	                    ((f_msb1 || f_msb2) && (! f_msbr)))
	                    rs = SR_OVERFLOW ;

	            } else
	                val += ((s[i] - '0') * weight) ;

	            if (rs >= 0)
	                weight *= CFDEC_WEIGHT ;

	        }

	    } /* end if */

	} /* end for */

	if (rs >= 0) {

	    while ((i > 0) && isspace(s[i]))
	        i -= 1 ;

	    if ((i >= 0) && (s[i] == '-'))
	        val = - val ;

	} /* end if */

done:
	if (rs >= 0)
	    *rp = (ulong) val ;

	return rs ;
}
/* end subroutine (cfdecul) */


/* convert from a decimal number tp a 64 bit LONG integer */
int cfdecll(s,len,rp)
cchar	s[] ;
int		len ;
LONG		*rp ;
{
	ULONG	val, weight ;

	int	rs = SR_OK ;
	int	i, n ;
	int	maxdigs = MAXDECDIG_L64 ;


	val = 0 ;
	*rp = 0 ;
	if (len < 0)
	    len = strlen(s) ;

/* remove white space from the rear of the string */

	while ((len > 0) && isspace(s[len - 1]))
	    len -= 1 ;

	if (len == 0)
	    return SR_INVALID ;

/* convert possible digits */

	weight = 1 ;
	n = 0 ;
	for (i = (len - 1) ; (rs >= 0) && (i >= 0) ; i -= 1) {

	    if ((s[i] == '-') || (s[i] == '+') || isspace(s[i]))
	        break ;

/* bad digits? */

	    if ((s[i] > '9') || (s[i] < '0'))
	        rs = SR_INVALID ;

	    if (rs >= 0) {

	        n += 1 ;
	        if ((n > maxdigs) && (s[i] != '0'))
	            rs = SR_OVERFLOW ;

	        if (rs >= 0) {

	            if (n == maxdigs) {

	                ULONG	adder ;

	                int	f_msb1, f_msb2 ;


	                adder = ((s[i] - '0') * weight) ;
	                f_msb1 = (val & (~ LONG64_MAX)) ? 1 : 0 ;
	                f_msb2 = (adder & (~ LONG64_MAX)) ? 1 : 0 ;
	                val += adder ;

	                if (f_msb1 || f_msb2 || (val & (~ LONG64_MAX)))
	                    rs = SR_OVERFLOW ;

	            } else
	                val += ((s[i] - '0') * weight) ;

	            if (rs >= 0)
	                weight *= CFDEC_WEIGHT ;

	        }

	    } /* end if */

	} /* end for */

	if (rs >= 0) {

	    while ((i > 0) && isspace(s[i]))
	        i -= 1 ;

	    if ((i >= 0) && (s[i] == '-'))
	        val = - val ;

	} /* end if */

done:
	if (rs >= 0)
	    *rp = (LONG) val ;

	return rs ;
}
/* end subroutine (cfdecll) */


/* convert from a decimal number to an unsigned long (64 bit) integer */
int cfdecull(s,len,rp)
cchar	s[] ;
int		len ;
ULONG		*rp ;
{
	ULONG	val, weight ;

	int	rs = SR_OK ;
	int	i, n ;
	int	maxdigs = MAXDECDIG_UL64 ;


	val = 0 ;
	*rp = 0 ;
	if (len < 0)
	    len = strlen(s) ;

/* remove white space from the rear of the string */

	while ((len > 0) && isspace(s[len - 1]))
	    len -= 1 ;

	if (len == 0)
	    return SR_INVALID ;

/* convert possible digits */
convert:
	weight = 1 ;
	n = 0 ;
	for (i = (len - 1) ; (rs >= 0) && (i >= 0) ; i -= 1) {

	    if ((s[i] == '-') || (s[i] == '+') || isspace(s[i]))
	        break ;

/* bad digits? */

	    if ((s[i] > '9') || (s[i] < '0'))
	        rs = SR_INVALID ;

	    if (rs >= 0) {

	        n += 1 ;
	        if ((n > maxdigs) && (s[i] != '0'))
	            rs = SR_OVERFLOW ;

	        if (rs >= 0) {

	            if (n == maxdigs) {

	                ULONG	adder ;

	                int	f_msb1, f_msb2, f_msbr ;


	                adder = ((s[i] - '0') * weight) ;
	                f_msb1 = (val & (~ LONG64_MAX)) ? 1 : 0 ;
	                f_msb2 = (adder & (~ LONG64_MAX)) ? 1 : 0 ;
	                val += adder ;
	                f_msbr = (val & (~ LONG64_MAX)) ? 1 : 0 ;

	                if ((f_msb1 && f_msb2) ||
	                    ((f_msb1 || f_msb2) && (! f_msbr)))
	                    rs = SR_OVERFLOW ;

	            } else
	                val += ((s[i] - '0') * weight) ;

	            if (rs >= 0)
	                weight *= CFDEC_WEIGHT ;

	        }

	    } /* end if */

	} /* end for */

	if (rs >= 0) {

	    while ((i > 0) && isspace(s[i]))
	        i -= 1 ;

	    if ((i >= 0) && (s[i] == '-'))
	        val = - val ;

	} /* end if */

done:
	if (rs >= 0)
	    *rp = (ULONG) val ;

	return rs ;
}
/* end subroutine (cfdecull) */


