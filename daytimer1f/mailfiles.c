/* mailfiles */


#define	F_DEBUGS	1


/* revision history :

	= 88/02/01, David A­D­ Morano

	This module was originally written.


*/


/******************************************************************************

	This module contains the operations for managing the 'mailfiles'
	object.


******************************************************************************/



#define	MAILFILES_MASTER	1


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>

#include	<vsystem.h>
#include	<vecelem.h>
#include	<mallocstuff.h>

#include	"misc.h"
#include	"mailfiles.h"



/* external subroutines */


/* forward references */

static void	entry_init(MAILFILES_ENT *) ;
static void	entry_free(MAILFILES_ENT *) ;






int mailfiles_init(lp)
MAILFILES	*lp ;
{


	return vecelem_init(lp,5,VECELEM_PNOHOLES) ;
}


int mailfiles_add(lp,path,pathlen)
MAILFILES	*lp ;
char		path[] ;
int		pathlen ;
{
	MAILFILES_ENT		e ;

	struct ustat		sb ;


	if ((lp == NULL) || (path == NULL))
	    return SR_FAULT ;

	entry_init(&e) ;

	if ((e.mailfile = mallocstrn(path,pathlen)) == NULL)
	    return SR_NOMEM ;

	e.lasttime = -1 ;
	e.lastsize = -1 ;
	if (u_stat(path,&sb) >= 0) {

	    e.lasttime = sb.st_mtime ;
	    e.lastsize = sb.st_size ;

	}

	return vecelem_add(lp,&e,sizeof(MAILFILES_ENT)) ;
}


int mailfiles_get(lp,i,epp)
MAILFILES	*lp ;
int		i ;
MAILFILES_ENT	**epp ;
{


	return vecelem_get(lp,i,epp) ;
}


int mailfiles_free(lp)
MAILFILES	*lp ;
{
	MAILFILES_ENT	*ep ;

	int	i ;


	for (i = 0 ; vecelem_get(lp,i,&ep) >= 0 ; i += 1) {

	    if (ep == NULL) continue ;

	    entry_free(ep) ;

	} /* end for */

	return vecelem_free(lp) ;
}


int mailfiles_count(lp)
MAILFILES	*lp ;
{


	return vecelem_count(lp) ;
}


int mailfiles_check(lp)
MAILFILES	*lp ;
{
	MAILFILES_ENT	*ep ;

	struct ustat	sb ;

	int	rs, i ;
	int	changed = 0 ;


	if (lp == NULL)
	    return SR_FAULT ;

	for (i = 0 ; (rs = vecelem_get(lp,i,&ep)) >= 0 ; i += 1) {

	    if (ep == NULL) continue ;

	    if (u_stat(ep->mailfile,&sb) >= 0) {

	        if (! ep->f_changed) {

	            if (sb.st_size > ep->lastsize) {

	                ep->f_changed = TRUE ;
	                changed += 1 ;

	            }

	        } else {

	            if (sb.st_size < ep->lastsize)
	                ep->f_changed = FALSE ;

	        }

#ifdef	COMMENT
	        if ((changed == 0) && (sb.st_mtime > ep->lasttime))
	            f_changed = TRUE ;
#endif /* COMMENT */

	        ep->lasttime = sb.st_mtime ;
	        ep->lastsize = sb.st_size ;

	    } /* end if */

	} /* end for */

	if ((rs >= 0) && (changed > 0))
	    rs = changed ;

	return rs ;
}
/* end subroutine (mailfiles_check) */



/* INTERNAL SUBROUTINES */



static void entry_init(ep)
MAILFILES_ENT	*ep ;
{


	ep->mailfile = NULL ;
	ep->f_changed = FALSE ;
}

static void entry_free(ep)
MAILFILES_ENT	*ep ;
{


	if (ep->mailfile != NULL)
	    free(ep->mailfile) ;

}



