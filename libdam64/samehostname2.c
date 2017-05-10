/* samehostname2 */

/* NOT DONE -- JUST AN IDEA! */
/* rough equivalent host check */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-21, David A­D­ Morano

	This program was started by copying from the RSLOW program.


	= 1999-03-24, David A­D­ Morano

	I started to think about enhancing this subroutine to better handle
	non-domain qualified host names that end in dot!


*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************


	Synopsis:

	int samehostname(h1,h2,localdomain)
	const char	h1[], h2[], localdomain[] ;

	Arguments:

	h1		one host name
	h2		another host name
	localdomain	the local domain name

	Returns:

	>=0	OK
	< 0	BAD


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<strings.h>
#include	<ctype.h>
#include	<netdb.h>

#include	<bfile.h>
#include	<logfile.h>
#include	<userinfo.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LEQUIV
#define	LEQUIV(a,b)	(((a) && (b)) || ((! (a)) && (! (b))))
#endif


/* external subroutines */


/* local structures */

struct breakname {
	char	*n, *d ;
	int	nlen, dlen ;
} ;


/* forward subroutines */

static int	breakname_parse() ;


/* external variables */


/* local variables */


/* exported subroutines */


/* compare host names */
int samehostname(h1,h2,localdomain)
char	h1[], h2[] ;
char	localdomain[] ;
{
	int	f_h1 = FALSE, f_h2 = FALSE ;
	int	len1, len2 ;

	char	*n1, *n2, *d1, *d2 ;
	char	*cp, *cp1, *cp2 ;


	splitname(h1,&n1,&d1)

	if (((cp1 = strchr(h1,'.')) != NULL) && (cp1[1] != '\0')) {

		f_h1 = TRUE ;

	} else {

		n1 = h1 ;
		d1 = "" ;

	}

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
	if (len1 != len2) return FALSE ;

	cp2 += 1 ;
	if (strcasecmp(cp2,localdomain) != 0) 
		return FALSE ;

	return (strncasecmp(h1,h2,len2) == 0) ;
}
/* end subroutine (samehostname) */


/* local subroutines */


static void breakname_parse(bnp,name)
struct breakname	*bnp ;
char			name[] ;
{
	int	hlen ;

	char	*cp, *cp1 ;


	if ((hlen = strlen(name)) == 0) {

	bnp->n = bnp->d = NULL ;
	bnp->nlen = bnp->dlen = 0 ;
	return ;
	}

	if (((cp1 = strchr(name,'.')) != NULL) && (cp1[1] != '\0')) {

		bnp->n = name ;
		bnp->nlen = cp1 - cp ;

	} else if (name[hlen - 1] == '.') {


	} else {


	}

}
/* end subroutine (breakname_parse) */


