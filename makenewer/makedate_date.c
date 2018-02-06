/* makedate_date */

/* subroutine to get the date out of the 'makedate' string */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* 'nprintf(3dam)' */


/* revision history:

	= 2000-03-01, David A­D­ Morano
	This code module was originally written.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine is a utility for extracting the date from a 'makedate'
        string (which just about ever program is compiled with).


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */


/* external routines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isdigitlatin(int) ;
extern int	isalphalatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


/* get the date out of the ID string */
int makedate_date(cchar *md,cchar **rpp)
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

	    while (*cp && (! CHAR_ISWHITE(*cp)))
	        cp += 1 ;

	    while (CHAR_ISWHITE(*cp))
	        cp += 1 ;

	} /* end if (skip over the name) */

	sp = cp ;
	if (rpp != NULL)
	    *rpp = cp ;

	while (*cp && (! CHAR_ISWHITE(*cp)))
	    cp += 1 ;

	return (cp - sp) ;
}
/* end subroutine (makedate_get) */



