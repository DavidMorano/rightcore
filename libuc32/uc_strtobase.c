/* uc_strtobase */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int uc_strtoi(cchar *startp,char **endpp,int b,int *rp)
{
	int		rs ;
	char		*endp = NULL ;

	if (rp == NULL) return SR_FAULT ;
	errno = 0 ;
	*rp = (int) strtol(startp,&endp,b) ;
	if (endpp != NULL) *endpp = endp ;
	rs = (errno != 0) ? (- errno) : (endp-startp) ;

	return rs ;
}
/* end subroutine (uc_strtoi) */


int uc_strtol(cchar *startp,char **endpp,int b,long *rp)
{
	int		rs ;
	char		*endp = NULL ;

	if (rp == NULL) return SR_FAULT ;
	errno = 0 ;
	*rp = strtol(startp,&endp,b) ;
	if (endpp != NULL) *endpp = endp ;
	rs = (errno != 0) ? (- errno) : (endp-startp) ;

	return rs ;
}
/* end subroutine (uc_strtol) */


int uc_strtoll(cchar *startp,char **endpp,int b,LONG *rp)
{
	int		rs ;
	char		*endp = NULL ;

	if (rp == NULL) return SR_FAULT ;
	errno = 0 ;
	*rp = strtoll(startp,&endp,b) ;
	if (endpp != NULL) *endpp = endp ;
	rs = (errno != 0) ? (- errno) : (endp-startp) ;

	return rs ;
}
/* end subroutine (uc_strtoll) */


int uc_strtoui(cchar *startp,char **endpp,int b,uint *rp)
{
	int		rs ;
	char		*endp = NULL ;

	if (rp == NULL) return SR_FAULT ;
	errno = 0 ;
	*rp = (uint) strtoul(startp,&endp,b) ;
	if (endpp != NULL) *endpp = endp ;
	rs = (errno != 0) ? (- errno) : (endp-startp) ;

	return rs ;
}
/* end subroutine (uc_strtoui) */


int uc_strtoul(cchar *startp,char **endpp,int b,ulong *rp)
{
	int		rs ;
	char		*endp = NULL ;

	if (rp == NULL) return SR_FAULT ;
	errno = 0 ;
	*rp = strtoul(startp,&endp,b) ;
	if (endpp != NULL) *endpp = endp ;
	rs = (errno != 0) ? (- errno) : (endp-startp) ;

	return rs ;
}
/* end subroutine (uc_strtoul) */


int uc_strtoull(cchar *startp,char **endpp,int b,ULONG *rp)
{
	int		rs ;
	char		*endp = NULL ;

	if (rp == NULL) return SR_FAULT ;
	errno = 0 ;
	*rp = strtoull(startp,&endp,b) ;
	if (endpp != NULL) *endpp = endp ;
	rs = (errno != 0) ? (- errno) : (endp-startp) ;

	return rs ;
}
/* end subroutine (uc_strtoull) */


