/* issamehostname */

/* rough equivalent host name check */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-21, David A­D­ Morano
	This program was started by copying from the RSLOW program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine can be used to determine if two hosts are really the
        same host because one may be qualified with the local domain and the
        other may not be.

	Synopsis:

	int issamehostname(h1,h2,localdomain)
	const char	h1[], h2[], localdomain[] ;

	Arguments:

	h1		one host name
	h2		another host name
	localdomain	the local domain name

	Returns:

	TRUE		the two host names are the same
	FALSE		the two host names are different


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stropts.h>
#include	<stdlib.h>
#include	<strings.h>		/* |strncasecmp(3c)| */
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LEQUIV
#define	LEQUIV(a,b)	(((a) && (b)) || ((! (a)) && (! (b))))
#endif


/* external subroutines */

#if	defined(BSD) && (! defined(EXTERN_STRNCASECMP))
extern int	strncasecmp(const char *,const char *,int) ;
#endif


/* external variables */


/* local structures */


/* forward subroutines */

int samehost(const char *,const char *,const char *) ;


/* local variables */


/* exported subroutines */


int issamehostname(h1,h2,localdomain)
const char	h1[], h2[] ;
const char	localdomain[] ;
{
	int		len1, len2 ;
	int		f_h1 = FALSE ;
	int		f_h2 = FALSE ;
	const char	*cp, *cp1, *cp2 ;

	if (((cp1 = strchr(h1,'.')) != NULL) && (cp1[1] != '\0'))
		f_h1 = TRUE ;

	if (((cp2 = strchr(h2,'.')) != NULL) && (cp2[1] != '\0'))
		f_h2 = TRUE ;

	if (LEQUIV(f_h1,f_h2))
	    return (! strcasecmp(h1,h2)) ;

	if (f_h1) {

	    len1 = cp1 - h1 ;
	    len2 = strlen(h2) ;

	    if (len1 != len2) 
		return FALSE ;

	    cp1 += 1 ;
	    if (strcasecmp(cp1,localdomain) != 0)
		return FALSE ;

	    return (strncasecmp(h1,h2,len1) == 0) ;

	} /* end if */

	len1 = strlen(h1) ;

	len2 = cp2 - h2 ;
	if (len1 != len2) 
		return FALSE ;

	cp2 += 1 ;
	if (strcasecmp(cp2,localdomain) != 0) 
		return FALSE ;

	return (strncasecmp(h1,h2,len2) == 0) ;
}
/* end subroutine (samehost) */

#if	CF_SAMEHOST

int samehostname(h1,h2,localdomain)
const char	h1[], h2[] ;
const char	localdomain[] ;
{

	return issamehostname(h1,h2,localdomain) ;
}

int samehost(h1,h2,localdomain)
const char	h1[], h2[] ;
const char	localdomain[] ;
{

	return issamehostname(h1,h2,localdomain) ;
}

#endif /* CF_SAMEHOST */


