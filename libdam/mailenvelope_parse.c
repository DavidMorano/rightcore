/* mailenvelope_parse */

/* parse out a mail message envelope */


#define	CF_DEBUGS	0		/* compile-time debugging */


/*revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	I don't know!

	Synopsis:

	int mailenvelope_parse(mep,linebuf)
	struct mailenvelope	*mep ;
	char			linebuf[] ;

	Arguments:

	mep		object pointer
	linebuf		buffer containing data to parse

	Returns:



******************************************************************************/


#define	MAILENVELOPE_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<strings.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>

#include	"mailenvelope.h"


/* local defines */

#define	SPACETAB(c)	(((c) == ' ') || ((c) == '\t'))


/* external subroutines */

extern int	sfsub(const char *,int,const char *,const char **) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mailenvelope_parse(mep,linebuf)
MAILENVELOPE	*mep ;
char		linebuf[] ;
{
	int	i ;
	int	len ;

	const char	*rp ;
	const char	*cp, *cp1 ;


#if	CF_DEBUGS
	debugprintf("mailenvelope_parse: ent> %s\n",linebuf) ;
#endif

	if (linebuf == NULL)
	    return BAD ;

	if (mep != NULL) {

	    mep->f_escape = FALSE ;
	    mep->address[0] = '\0' ;
	    mep->date[0] = '\0' ;
	    mep->remote[0] = '\0' ;

	}

	cp = linebuf ;
	if (*cp == '>') {

	    cp += 1 ;
	    if (mep != NULL) 
		mep->f_escape = TRUE ;

	}

	if ((strncmp(cp,"From",4) != 0) || (! SPACETAB(cp[4])))
	    return BAD ;

#if	CF_DEBUGS
	debugprintf("mailenvelope_parse: got an envelope match\n") ;
#endif

	if (mep == NULL) 
		return OK ;

/* search for the address */

	cp = linebuf + 5 ;
	while (CHAR_ISWHITE(*cp)) 
		cp += 1 ;

/* is it a super short envelope? */

	if (*cp == '\0') 
		return OK ;

	cp1 = cp ;
	while (*cp && (! CHAR_ISWHITE(*cp))) 
		cp += 1 ;

	len = MIN((cp - cp1),MAILENVELOPE_ADDRESSLEN) ;
	strwcpy(mep->address,cp1,len) ;

/* search for the start of a date */

	while (CHAR_ISWHITE(*cp)) 
		cp += 1 ;

	if (*cp == '\0') 
		return OK ;

	if ((i = sfsub(cp,-1,"remote from",&rp)) >= 0) {

	    len = MIN(i,MAILENVELOPE_DATELEN) ;
	    strwcpy(mep->date,cp,len) ;

	    cp += (i + 11) ;
	    while (CHAR_ISWHITE(*cp)) 
		cp += 1 ;

	    if (*cp) {

	        cp1 = cp ;
	        while (*cp && (! CHAR_ISWHITE(*cp))) 
			cp += 1 ;

	        len = MIN((cp - cp1),MAILENVELOPE_REMOTELEN) ;
	        strwcpy(mep->remote,cp1,len) ;

	    }

	} else
	    strwcpy(mep->date,cp,MAILENVELOPE_DATELEN) ;

	return OK ;
}
/* end subroutine (mailenvelope_parse) */



