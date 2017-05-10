/* utmpent */

/* methods for the UTMPENT object */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_SAFE		0		/* safer */
#define	CF_DYNENTRIES	1		/* dynamic entries per read */


/* revision history:

	= 2000-07-19, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This code provides the methods for the UTMPENT object.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"utmpent.h"


/* local defines */


/* external subroutines */

extern char	*strwcpy(char *,char *,int) ;
extern char	*strnwcpy(char *,int,cchar *,int) ;


/* forward references */


/* local variables */


/* exported subroutines */


int utmpent_start(UTMPENT *op)
{
	if (op == NULL) return SR_FAULT ;
	memset(op,0,sizeof(UTMPENT)) ;
	return SR_OK ;
}
/* end subroutine (utmpent_start) */


int utmpent_finish(UTMPENT *op)
{
	if (op == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (utmpent_finish) */


int utmpent_settype(UTMPENT *op,int type)
{
	if (op == NULL) return SR_FAULT ;
	op->ut_type = type ;
	return SR_OK ;
}
/* end subroutine (utmpent_settype) */


int utmpent_setpid(UTMPENT *op,pid_t sid)
{
	if (op == NULL) return SR_FAULT ;
	op->ut_pid = sid ;
	return SR_OK ;
}
/* end subroutine (utmpent_setpid) */


int utmpent_setsession(UTMPENT *op,int lines)
{
	if (op == NULL) return SR_FAULT ;
	op->ut_session = lines ;
	return SR_OK ;
}
/* end subroutine (utmpent_setsession) */


int utmpent_setcols(UTMPENT *op,int cols)
{
	if (op == NULL) return SR_FAULT ;
	op->cols = cols ;
	return SR_OK ;
}
/* end subroutine (utmpent_setcols) */


int utmpent_setid(UTMPENT *op,cchar *sp,int sl)
{
	if (op == NULL) return SR_FAULT ;
	return (strnwcpy(op->ut_id,UTMPENT_LID,sp,sl) - op->ut_id) ;
}
/* end subroutine (utmpent_setid) */


int utmpent_setline(UTMPENT *op,cchar *sp,int sl)
{
	if (op == NULL) return SR_FAULT ;
	return (strnwcpy(op->ut_line,UTMPENT_LLINE,sp,sl) - op->ut_line) ;
}
/* end subroutine (utmpent_setline) */


int utmpent_setuser(UTMPENT *op,cchar *sp,int sl)
{
	if (op == NULL) return SR_FAULT ;
	return (strnwcpy(op->ut_user,UTMPENT_LUSER,sp,sl) - op->ut_user) ;
}
/* end subroutine (utmpent_setuser) */


int utmpent_sethost(UTMPENT *op,cchar *sp,int sl)
{
	int		rs ;
	if (op == NULL) return SR_FAULT ;
	rs = (strnwcpy(op->ut_host,UTMPENT_LHOST,sp,sl) - op->ut_host) ;
	op->ut_syslen = rs ;
	return rs ;
}
/* end subroutine (utmpent_sethost) */


int utmpent_gettype(UTMPENT *op)
{
	int	rs = MKCHAR(op->ut_type) ;
	return rs ;
}
/* end subroutine (utmpent_gettype) */


int utmpent_getpid(UTMPENT *op)
{
	int		rs = op->ut_pid ;
	return rs ;
}
/* end subroutine (utmpent_getpid) */


int utmpent_getsession(UTMPENT *op)
{
	int		rs = op->ut_session ;
	return rs ;
}
/* end subroutine (utmpent_getsession) */


int utmpent_getcols(UTMPENT *op)
{
	int		rs = op->cols ;
	return rs ;
}
/* end subroutine (utmpent_getcols) */


int utmpent_getid(UTMPENT *op,cchar **rpp)
{
	int		rs = strnlen(op->ut_id,UTMPENT_LID) ;
	if (rpp != NULL) *rpp = op->ut_id ;
	return rs ;
}
/* end subroutine (utmpent_getid) */


int utmpent_getline(UTMPENT *op,cchar **rpp)
{
	int		rs = strnlen(op->ut_line,UTMPENT_LLINE) ;
	if (rpp != NULL) *rpp = op->ut_line ;
	return rs ;
}
/* end subroutine (utmpent_getline) */


int utmpent_getuser(UTMPENT *op,cchar **rpp)
{
	int		rs = strnlen(op->ut_user,UTMPENT_LUSER) ;
	if (rpp != NULL) *rpp = op->ut_user ;
	return rs ;
}
/* end subroutine (utmpent_getuser) */


int utmpent_gethost(UTMPENT *op,cchar **rpp)
{
	int		rs = strnlen(op->ut_host,UTMPENT_LHOST) ;
	if (op->ut_syslen > 0) {
	    if (rs > op->ut_syslen) rs = op->ut_syslen ;
	}
	if (rpp != NULL) *rpp = op->ut_host ;
	return rs ;
}
/* end subroutine (utmpent_gethost) */


