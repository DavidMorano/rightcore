/* optvalue */

/* option-value */


/* revision history:

	- 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Get (divine) a numeric value option. The numeric value can be
        represented in many forms: decimal, hexadecial, octal, other.

	Synopsis:

	int optvalue(sp,sl)
	const char	*sp ;
	int		sl ;

	Arguments:

	sp		pointer to string
	sl		length of string

	Returns:

	<0		error
	>=0		value


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	matostr(const char **,int,const char *,int) ;
extern int	matocasestr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfnumi(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfnumui(const char *,int,uint *) ;
extern int	tolc(int) ;


/* local variables */

static cchar	*resps[] = {
	"no",
	"yes",
	"false",
	"true",
	NULL
} ;


/* exported subroutines */


int optvalue(cchar *sp,int sl)
{
	int		rs ;
	int		i ;

	if (sp == NULL) return SR_NOENT ;

	if (sl < 0)
	    sl = strlen(sp) ;

	if ((i = matocasestr(resps,1,sp,sl)) >= 0) {
	    rs = (i&1) ;
	} else {
	    int	v ;
	    rs = cfnumi(sp,sl,&v) ;
	    if (rs >= 0) rs = (v&INT_MAX) ;
	}

	return rs ;
}
/* end subroutine (optvalue) */


