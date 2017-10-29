/* varsub_dumpfd */

/* dump the substitution variables of the object */


#define	F_DEBUGS	0


/* revision history :

	= 92/05/23, David Morano

	This subroutine was written to provide some helper support
	for substitution setup.


*/



#include	<sys/types.h>

#include	<vsystem.h>
#include	<varsub.h>

#include	"misc.h"



/* external subroutines */

extern int	fdprintf(int,...) ;





#if	F_DEBUGS

int varsub_dumpfd(vshp,fd)
varsub	*vshp ;
int	fd ;
{
	varsub_entry	*vsa ;

	int	i ;


	if (vshp->vsa == NULL)
		return ;

	if (fd < 0)
		fd = egetfd() ;

	vsa = vshp->vsa ;
	for (i = 0 ; i < vshp->i ; i += 1) {

	    if (vsa[i].kp == NULL) {

	        fdprintf(fd,"varsub_dumpfd NULL\n") ;

	    } else {

	        fdprintf(fd,"varsub_dumpfd key=%W value=%W\n",
	            vsa[i].kp,vsa[i].klen,
	            vsa[i].vp,vsa[i].vlen) ;

	}

	} /* end for */

}
/* end subroutine (varsub_dumpfd) */

#else

int varsub_dumpfd(vsp,fd)
varsub	*vsp ;
int	fd ;
{


	return SR_OK ;
}

#endif /* F_DEBUGS */



