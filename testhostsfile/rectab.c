/* rectab */

/* string rectab object */


#define	F_FASTGROW	1
#define	F_UCMALLOC	1


/* revision history:

	= 94/03/24, David A­D­ Morano

	This object module was morphed from some previous one.
	I do not remember what the previous one was.


*/



/******************************************************************************

	This object module creates and manages a hosts-file entry
	record table object.



*****************************************************************************/


#define	RECTAB_MASTER	1


#include	<sys/types.h>

#include	<vsystem.h>
#include	<vecitem.h>

#include	"localmisc.h"
#include	"rectab.h"




/* local defines */



/* external subroutines */


/* forward references */






int rectab_init(rtp,startlen)
RECTAB	*rtp ;
int	startlen ;
{
	int	rs ;


	if (rtp == NULL)
	    return SR_FAULT ;

	if (startlen < RECTAB_STARTLEN)
	    startlen = RECTAB_STARTLEN ;


	rs = vecitem_start(rtp,startlen,VECITEM_PNOHOLES) ;

	return rs ;
}
/* end subroutine (rectab_init) */


int rectab_free(rtp)
RECTAB	*rtp ;
{


	return vecitem_finish(rtp) ;
}


int rectab_add(rtp,ia,csi,si)
RECTAB	*rtp ;
uint	ia, csi, si ;
{
	RECTAB_ENT	e ;

	int	rs ;


	e.ia = ia ;
	e.csi = csi ;
	e.si = si ;
	rs = vecitem_add(rtp,&e,sizeof(RECTAB_ENT)) ;

	return rs ;
}
/* end subroutine (rectab_add) */



