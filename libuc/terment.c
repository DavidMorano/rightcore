/* terment */

/* methods for the TERMENT object */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_SAFE		0		/* safer */
#define	CF_DYNENTRIES	1		/* dynamic entries per read */


/* revision history:

	= 2000-07-19, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This code provides the methods for the TERMENT object. The TERMENT
        object, while residing in a file, forms a single record in a
        file-database of records. This is similar to a raw UTMPX object when it
        is written out to the 'utmpx' file. This object is used by the TERMENQ
        object to represent a record in the TERMENQ database.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"terment.h"


/* local defines */


/* external subroutines */

extern char	*strwcpy(char *,char *,int) ;
extern char	*strnwcpy(char *,int,cchar *,int) ;


/* forward references */


/* local variables */


/* exported subroutines */


int terment_start(TERMENT *op)
{
	if (op == NULL) return SR_FAULT ;
	memset(op,0,sizeof(TERMENT)) ;
	return SR_OK ;
}
/* end subroutine (terment_start) */


int terment_finish(TERMENT *op)
{
	if (op == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (terment_finish) */


int terment_settype(TERMENT *op,int type)
{
	if (op == NULL) return SR_FAULT ;
	op->type = type ;
	return SR_OK ;
}
/* end subroutine (terment_settype) */


int terment_setsid(TERMENT *op,pid_t sid)
{
	if (op == NULL) return SR_FAULT ;
	op->sid = sid ;
	return SR_OK ;
}
/* end subroutine (terment_setsid) */


int terment_setlines(TERMENT *op,int lines)
{
	if (op == NULL) return SR_FAULT ;
	op->lines = lines ;
	return SR_OK ;
}
/* end subroutine (terment_setlines) */


int terment_setcols(TERMENT *op,int cols)
{
	if (op == NULL) return SR_FAULT ;
	op->cols = cols ;
	return SR_OK ;
}
/* end subroutine (terment_setcols) */


int terment_setid(TERMENT *op,cchar *sp,int sl)
{
	if (op == NULL) return SR_FAULT ;
	return (strnwcpy(op->id,TERMENT_LID,sp,sl) - op->id) ;
}
/* end subroutine (terment_setid) */


int terment_setline(TERMENT *op,cchar *sp,int sl)
{
	if (op == NULL) return SR_FAULT ;
	return (strnwcpy(op->line,TERMENT_LLINE,sp,sl) - op->line) ;
}
/* end subroutine (terment_setline) */


int terment_settermtype(TERMENT *op,cchar *sp,int sl)
{
	if (op == NULL) return SR_FAULT ;
	return (strnwcpy(op->termtype,TERMENT_LTERMTYPE,sp,sl) - op->termtype) ;
}
/* end subroutine (terment_settermtype) */


int terment_setanswer(TERMENT *op,cchar *sp,int sl)
{
	if (op == NULL) return SR_FAULT ;
	return (strnwcpy(op->answer,TERMENT_LANSWER,sp,sl) - op->answer) ;
}
/* end subroutine (terment_setanswer) */


int terment_gettype(TERMENT *op)
{
	int	rs = MKCHAR(op->type) ;
	return rs ;
}
/* end subroutine (terment_gettype) */


int terment_getsid(TERMENT *op)
{
	int		rs = op->sid ;
	return rs ;
}
/* end subroutine (terment_getsid) */


int terment_getlines(TERMENT *op)
{
	int		rs = op->lines ;
	return rs ;
}
/* end subroutine (terment_getlines) */


int terment_getcols(TERMENT *op)
{
	int		rs = op->cols ;
	return rs ;
}
/* end subroutine (terment_getcols) */


int terment_getid(TERMENT *op,cchar **rpp)
{
	int		rs = strnlen(op->id,TERMENT_LID) ;
	if (rpp != NULL) *rpp = op->id ;
	return rs ;
}
/* end subroutine (terment_getid) */


int terment_getline(TERMENT *op,cchar **rpp)
{
	int		rs = strnlen(op->line,TERMENT_LLINE) ;
	if (rpp != NULL) *rpp = op->line ;
	return rs ;
}
/* end subroutine (terment_getline) */


int terment_gettermtype(TERMENT *op,cchar **rpp)
{
	int		rs = strnlen(op->termtype,TERMENT_LTERMTYPE) ;
	if (rpp != NULL) *rpp = op->termtype ;
	return rs ;
}
/* end subroutine (terment_gettermtype) */


int terment_getanswer(TERMENT *op,cchar **rpp)
{
	int		rs = strnlen(op->answer,TERMENT_LANSWER) ;
	if (rpp != NULL) *rpp = op->answer ;
	return rs ;
}
/* end subroutine (terment_getanswer) */


