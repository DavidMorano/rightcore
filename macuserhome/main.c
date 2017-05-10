/* main */

/* generic front-end for SHELL built-ins */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1989-03-01, David A.D. Morano

	This subroutine was originally written.


*/

/* Copyright © 1989 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ userhome [<username(s)>|-] [-af <afile>]


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	matstr(const char **,const char *,int) ;

extern int	p_userhome(int,const char **,const char **,void *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	int	ex ;


	ex = p_userhome(argc,argv,envv,NULL) ;

	return ex ;
}
/* end subroutine (main) */



