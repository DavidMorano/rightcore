/* userlines */

/* FIFO-String-Interlocked management */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1994-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/***********************************************************************

	This object manages interlocked FIFO-string operations.


***********************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<pthread.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<fifostr.h>
#include	<ptm.h>
#include	<localmisc.h>

#include	"userlines.h"


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	strpcmp(const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isalnumlatin(int) ;
extern int	haslc(const char *,int) ;
extern int	hasuc(const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int userlines_start(op)
USERLINES	*op ;
{
	int	rs = SR_OK ;


	rs = fifostr_start(&op->q) ;
	if (rs < 0)
	    goto ret0 ;

	rs = ptm_init(&op->m,NULL) ;
	if (rs < 0)
	    fifostr_finish(&op->q) ;

ret0:
	return rs ;
}
/* end subroutine (userlines_start) */


int userlines_finish(op)
USERLINES	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	rs1 = ptm_destroy(&op->m) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = fifostr_finish(&op->q) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (userlines_finish) */


int userlines_add(op,s,slen)
USERLINES	*op ;
const char	s[] ;
int		slen ;
{
	int	rs ;


	if ((rs = ptm_lock(&op->m)) >= 0) {

	    rs = fifostr_add(&op->q,s,slen) ;

	    ptm_unlock(&op->m) ;
	} /* end if */

	return rs ;
}
/* end subroutine (userlines_add) */


int userlines_remove(op,s,slen)
USERLINES	*op ;
char		s[] ;
int		slen ;
{
	int	rs ;


	if ((rs = ptm_lock(&op->m)) >= 0) {

	    rs = fifostr_remove(&op->q,s,slen) ;

	    ptm_unlock(&op->m) ;
	} /* end if */

	return rs ;
}
/* end subroutine (userlines_remove) */



