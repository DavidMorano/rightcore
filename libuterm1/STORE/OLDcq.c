/* cq */

/* character queue routines */


#define	F_DEBUG		0


/*
	= Dave Morano, February 85

*/



#include	"misc.h"

#include	"q.h"



/* character queue initialization */
int initc(qp,buf,len)
struct cq	*qp ;
char	buf[] ;
int	len ;
{

#if	F_DEBUG
	eprintf("initc: entered\n") ;
#endif

	qp->buf = buf ;

#if	F_DEBUG
	eprintf("initc: 1 \n") ;
#endif

	qp->len = len ;
	qp->count = 0 ;
	qp->ri = qp->wi = 0 ;
}
/* end subroutine */


/* routine to insert into FIFO */
int insc(fp,c)
struct cq	*fp ;
char		c ;
{

	if (fp->count >= fp->len) return Q_OVER ;

	(fp->buf)[fp->wi] = c ;
	fp->count += 1 ;

	fp->wi += 1 ;
	if (fp->wi == fp->len) fp->wi = 0 ;

	return Q_OK ;
}
/* end subroutine */


/* remove from FIFO */
int remc(fp,cp)
struct cq	*fp ;
char	*cp ;
{

	if (fp->count == 0) return Q_UNDER ;

	*cp = (fp->buf)[fp->ri] ;
	fp->count -= 1 ;

	fp->ri += 1 ;
	if (fp->ri == fp->len) fp->ri = 0 ;

	return Q_OK ;
}
/* end subroutine */


