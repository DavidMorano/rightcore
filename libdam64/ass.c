/* ass */

/* experimenal string manipulation object */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>
#include	<vsystem.h>

#include	"ass.h"


/* local defines */

#define	ASS_ADDLEN	100


/* exported subroutines */


int ass_start(asp)
ASS		*asp ;
{


	if (asp == NULL) 
	    return SR_FAULT ;

	asp->len = 0 ;
	asp->e = 0 ;
	asp->s = NULL ;
	return SR_OK ;
}


int ass_add(asp,c)
ASS		*asp ;
int		c ;
{
	int	rs = 0 ;
	int	size ;

	caddr_t	p ;


	if (asp->s == NULL) {

	    asp->len = 0 ;
	    asp->e = ASS_ADDLEN ;
	    size = asp->e ;
	    rs = uc_malloc(size,&p) ;

	} else if (asp->e == asp->len) {

	    asp->e += ASS_ADDLEN ;
	    size = asp->e ;
	    rs = uc_realloc(asp->s,size,&p) ;

	}

	if (rs >= 0) {
	    asp->s = p ;
	    asp->s[(asp->len)++] = c ;
	}

	return (rs >= 0) ? asp->len : rs ;
}


int ass_finish(asp)
ASS		*asp ;
{


	if (asp == NULL) 
	    return SR_FAULT ;

	if (asp->s != NULL) {
	    uc_free(asp->s) ;
	    asp->s = NULL ;
	}

	asp->len = 0 ;
	asp->e = 0 ;
	return SR_OK ;
}



