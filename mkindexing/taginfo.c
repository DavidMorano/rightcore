/* taginfo */

/* parse a tag (given a string) */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_STRNLEN	0		/* call |strnlen(3c)| */


/* revision history:

	= 1994-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1994 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine parses a tag (given in a string) into its parts.

	Synopsis:

	int taginfo_parse(op,sp,sl)
	struct taginfo	*op ;
	const char	sp[] ;
	int		sl ;

	Arguments:

	op		object pointer
	sp		tag string
	sl		tag string length

	Returns:

	<0		error
	>=0		key-index (indenx to key beginning)


*******************************************************************************/


#define	TAGINFO_MASTER	0


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"taginfo.h"


/* local defines */

#ifndef	WORDBUFLEN
#define	WORDBUFLEN	100
#endif


/* external subroutines */

extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecul(const char *,int,ulong *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int taginfo_parse(op,sp,sl)
struct taginfo	*op ;
const char	sp[] ;
int		sl ;
{
	int	rs = SR_OK ;
	int	ki = 0 ;

	const char	*ssp = sp ;
	const char	*tp ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("taginfo_parse: ent sl=%d tag=>%s<\n",sl,sp,sl) ;
#endif

/* initialize */

	op->fnamelen = 0 ;
	op->recoff = 0 ;
	op->reclen = 0 ;

#if	CF_STRNLEN
	sl = strnlen(sp,sl) ;
#else
	if (sl < 0) sl = strlen(sp) ;
#endif

#if	CF_DEBUGS
	debugprintf("taginfo_parse: sl=%d\n",sl) ;
#endif

/* get the filename */

	if ((tp = strnchr(sp,sl,':')) != NULL) {
	    op->fnamelen = (tp - sp) ;
	    sl -= ((tp + 1) - sp) ;
	    sp = (tp + 1) ;
/* get the tag offset */
	    if ((tp = strnchr(sp,sl,',')) != NULL) {
		ulong	vv ;
	        if ((rs = cfdecul(sp,(tp - sp),&vv)) >= 0) {
	            op->recoff = vv ;
	            sl -= ((tp + 1) - sp) ;
	            sp = (tp + 1) ;
#if	CF_DEBUGS
	            debugprintf("taginfo_parse: reclen sl=%d s=>%s<\n",sl,sp) ;
#endif
	            if ((tp = strnchr(sp,sl,'\t')) != NULL) {
	                sl = (tp - sp) ;
	                ki = ((tp + 1) - ssp) ;
	            }
#if	CF_DEBUGS
	            debugprintf("taginfo_parse: sl=%d gos=>%t<\n",sl,sp,sl) ;
#endif
	            rs = cfdecul(sp,sl,&vv) ;
	            op->reclen = vv ;
	        } /* end if */
	    } else
	        rs = SR_INVALID ;
	} else
	    rs = SR_INVALID ;

	return (rs >= 0) ? ki : rs ;
}
/* end subroutine (taginfo_parse) */



