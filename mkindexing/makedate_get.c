/* makedate_get */

/* get the name on the MAKEDATE string */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1988-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We get the date out of the ID string.

	Synopsis:

	int makedate_get(makedate,rpp)

	Arguments:

	makedate	pointer to the MAKEDATE string
	rpp		pointer to pointer to hold result

	Returns:

	<0		error
	>=		length of result


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	snwcpyuc(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int makedate_get(cchar *md,cchar **rpp)
{
	int		ch ;
	const char	*sp ;
	const char	*cp ;

	if (rpp != NULL)
	    *rpp = NULL ;

	if ((cp = strchr(md,CH_RPAREN)) == NULL)
	    return SR_NOENT ;

	while (CHAR_ISWHITE(*cp))
	    cp += 1 ;

	ch = MKCHAR(*cp) ;
	if (! isdigitlatin(ch)) {

	    while (*cp && (! CHAR_ISWHITE(*cp))) cp += 1 ;

	    while (CHAR_ISWHITE(*cp)) cp += 1 ;

	} /* end if (skip over the name) */

	sp = cp ;
	if (rpp != NULL) *rpp = cp ;

	while (*cp && (! CHAR_ISWHITE(*cp))) cp += 1 ;

	return (cp - sp) ;
}
/* end subroutine (makedate_get) */


