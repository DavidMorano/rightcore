/* dstr */

/* string object */



/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This little object tries to mimic a dynamic-length string.



******************************************************************************/


#define	DSTR_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"dstr.h"


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* exported subroutines */


int dstr_start(sop,s,slen)
DSTR	*sop ;
char	s[] ;
int	slen ;
{
	int	rs = SR_NOEXIST ;


	sop->s = NULL ;
	sop->slen = 0 ;
	if (s != NULL) {

	    sop->slen = (slen < 0) ? strlen(s) : slen ;

	    if ((rs = uc_malloc(sop->slen,&sop->s)) >= 0) {

	        rs = sop->slen ;
	        strwcpy(sop->s,s,sop->slen) ;

	    }

	}

	return rs ;
}


int dstr_finish(sop)
DSTR	*sop ;
{


	if (sop->s != NULL) {
	    uc_free(sop->s) ;
	    sop->s = NULL ;
	}

	sop->slen = 0 ;
	return SR_OK ;
}


int dstr_reinit(sop,s,slen)
DSTR	*sop ;
char	s[] ;
int	slen ;
{


	if (sop->s != NULL)
	    uc_free(sop->s) ;

	return dstr_start(sop,s,slen) ;
}


int dstr_assign(sop,sop2)
DSTR	*sop, *sop2 ;
{


	return dstr_reinit(sop,sop2->s,sop2->slen) ;
}



