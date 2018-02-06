/* zoffparts */

/* manage time-zone offsets */


#define	CF_SAFE		0		/* run safer */


/* revision history:

	= 1995-08-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1995 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************
 
        This two small subroutine manipulate zone-offsets for use in time
        strings.

	Description:

        Set the value of the object from the number of seconds the current
        timezone is west of GMT.

	Symopsis:

	int zoffparts_set(zop,v)
	ZOFFPARTS	*zop ;
	int		v ;

	Arguments:

	aop		pointer to object
	v		offset from GMT (seconds west of GMT)

	Returns:

	0		always succeeds


	Description:

	Get the number of seconds that the current timezone is west of
	GMT from the object.  We do not care about whether the offset
	(from GMT) is positive or negative.  Someone else, someplace
	else, cares about that.

	Sysnopsis:

	int zoffparts_get(zop,vp)
	ZOFFPARTS	*zop ;
	int		*vp ;

	Arguments:

	zop		pointer to object
	vp		pointer to hold result (seoconds west of GMT)

	Returns:

	0		always succeeds


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>		/* for 'abs(3c)' */

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>

#include	"zoffparts.h"


/* forward references */

static int	storebuf_twodig(char *,int,int,int) ;


/* exported subroutines */


int zoffparts_set(ZOFFPARTS *zop,int v)
{

#if	CF_SAFE
	if (zop == NULL) return SR_FAULT ;
#endif

	zop->zoff = v ;
	v = abs(v) / 60 ;
	zop->hours = (v / 60) ;
	zop->mins = (v % 60) ;
	return SR_OK ;
}
/* end subroutine (zoffparts_set) */


int zoffparts_get(ZOFFPARTS *zop,int *vp)
{
	int		v ;

#if	CF_SAFE
	if (zop == NULL) return SR_FAULT ;
#endif

	v = ((zop->hours * 60) + zop->mins) * 60 ;
	if (zop->zoff < 0) v = (-v) ;
	if (vp != NULL) *vp = v ;

	v = abs(v) ;

	return v ;
}
/* end subroutine (zoffparts_get) */


int zoffparts_mkstr(ZOFFPARTS *zop,char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (zop == NULL) return SR_FAULT ;

	if (rs >= 0) {
	    int	ch = ((zop->zoff >= 0) ? '-' : '+') ;
	    rs = storebuf_char(rbuf,rlen,i,ch) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_twodig(rbuf,rlen,i,zop->hours) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_twodig(rbuf,rlen,i,zop->mins) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (zoffparts_mkstr) */


/* private subroutines */


static int storebuf_twodig(char *rbuf,int rlen,int i,int v)
{
	int		rs = SR_OK ;

	if ((i+2) <= rlen) {
	    rbuf[i++] = (v/10) + '0' ;
	    rbuf[i++] = (v%10) + '0' ;
	} else
	    rs = SR_OVERFLOW ;

	return rs ;
}
/* end subroutine (storebuf_twodig) */


