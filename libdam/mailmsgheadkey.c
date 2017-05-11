/* msgheadkey */

/* get the next field in a white-space delineated record */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine extracts the key of a header key, if one is
	present in the supplied buffer.  A pointer to the key and the
	length of the key is returned (if a key was present).

	Synopsis:

	int msgheadkey(sp,sl,kpp)
	const char	*sp ;
	int		sl ;
	char		**kpp ;

	Arguments:

	sp		pointer to start of user supplied buffer
	sl		length of user supplied buffer
	kpp		pointer to pointer of the found field

	Returns:

	>0		length of found key field
	==0		an empty (zero-length) key was found
	<0		no key was found


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */

#define	ISSPACETAB(c)	(((c) == ' ') || ((c) == '\t'))
#define	ISKEYCHAR(c)	(isalnum(c) || ((c) == '-') || ((c) == '_'))


/* external subroutines */


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int mailmsgheadkey(sp,sl,kpp)
const char	*sp ;
int		sl ;
const char	**kpp ;
{
	int		kl ;
	int		f_len = (sl >= 0) ;

#if	CF_DEBUGS
	if (! f_len)
		sl = strlen(sp) ;
	debugprintf("msgheadkey: line=>%t<\n",
		sp,
		((sp[sl - 1] == '\n') ? (sl - 1) : sl)) ;
#endif /* CF_DEBUGS */

/* skip leading white space (not including NLs) */

	while (((! f_len) || (sl > 0)) && ISSPACETAB(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	*kpp = (char *) sp ;

/* skip the non-white space */

	while ((((! f_len) && *sp) || (sl > 0)) && ISKEYCHAR(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	kl = (sp - (*kpp)) ;

/* skip any trailing whitespace */

	while (((! f_len) || (sl > 0)) && ISSPACETAB(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

/* this character must be a colon (':') or else we didn't have a head-key */

#if	CF_DEBUGS
	debugprintf("msgheadkey: key=>%t<\n",
		(*kpp),
		(((*kpp)[kl - 1] == '\n') ? (kl - 1) : kl)) ;
#endif

	return (*sp == ':') ? kl : -1 ;
}
/* end subroutine (msgheadkey) */


