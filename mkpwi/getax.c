/* getax */

/* subroutines to access the PWD, GRP, and SHADOW databases */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines were written so that we could use a single interface
        to access the 'passwd' database on all UNIX® platforms. This code module
        provides a platform independent implementation of UNIX® 'passwd'
        database access subroutines.

	These are the preferred interfaces:

	preferred interfaces: getpw_name(), getpw_uid() ;
	preferred interfaces: getgr_name(), getgr_gid() ;


*******************************************************************************/


#define	GETAX_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<passwdent.h>
#include	<localmisc.h>

#include	"getax.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getpw_begin()
{
	return uc_setpwent() ;
}
/* end subroutine (getpw_begin) */


int getpw_end()
{
	return uc_endpwent() ;
}
/* end subroutine (getpw_end) */


int getpw_ent(struct passwd *pwp,char *pwbuf,int pwlen)
{
	int		rs ;
	if ((rs = uc_getpwent(pwp,pwbuf,pwlen)) == SR_NOTFOUND) rs = SR_OK ;
#if	CF_DEBUGS
	debugprintf("getpw_ent: uc_getpwent() rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (getpw_ent) */


int getpw_name(struct passwd *pwp,char *pwbuf,int pwlen,cchar *name)
{
	return uc_getpwnam(name,pwp,pwbuf,pwlen) ;
}
/* end subroutine (getpw_name) */


int getpw_uid(struct passwd *pwp,char *pwbuf,int pwlen,uid_t uid)
{
	return uc_getpwuid(uid,pwp,pwbuf,pwlen) ;
}
/* end subroutine (getpw_uid) */


int getsp_begin()
{
	return uc_setspent() ;
}
/* end subroutine (getsp_begin) */


int getsp_end()
{
	return uc_endspent() ;
}
/* end subroutine (getsp_end) */


int getsp_ent(struct spwd *sp,char *rbuf,int rlen)
{
	int		rs ;
	if ((rs = uc_getspent(sp,rbuf,rlen)) == SR_NOTFOUND) rs = SR_OK ;
#if	CF_DEBUGS
	debugprintf("getsp_ent: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (getsp_ent) */


int getsp_name(struct spwd *sp,char *rbuf,int rlen,cchar *name)
{
	int		rs ;
#if	CF_DEBUGS
	debugprintf("getsp_name: ent name=%s\n",name) ;
#endif
	rs = uc_getspnam(name,sp,rbuf,rlen) ;
#if	CF_DEBUGS
	debugprintf("getsp_name: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (getsp_name) */


int getgr_begin()
{
	return uc_setgrent() ;
}
/* end subroutine (getgr_begin) */


int getgr_end()
{
	return uc_endgrent() ;
}
/* end subroutine (getgr_end) */


int getgr_ent(struct group *grp,char *grbuf,int grlen)
{
	int		rs ;
	if ((rs = uc_getgrent(grp,grbuf,grlen)) == SR_NOTFOUND) rs = SR_OK ;
	return rs ;
}
/* end subroutine (getgr_ent) */


int getgr_name(struct group *grp,char *rbuf,int rlen,cchar *name)
{
	return uc_getgrnam(name,grp,rbuf,rlen) ;
}
/* end subroutine (getgr_name) */


int getgr_gid(struct group *grp,char *rbuf,int rlen,gid_t gid)
{
	return uc_getgrgid(gid,grp,rbuf,rlen) ;
}
/* end subroutine (getgr_gid) */


int getpj_begin()
{
	return uc_setpjent() ;
}
/* end subroutine (getpj_begin) */


int getpj_end()
{
	return uc_endpjent() ;
}
/* end subroutine (getpj_end) */


int getpj_ent(struct project *pjp,char *rbuf,int rlen)
{
	int		rs ;
	if ((rs = uc_getpjent(pjp,rbuf,rlen)) == SR_NOTFOUND) rs = SR_OK ;
	return rs ;
}
/* end subroutine (getpj_ent) */


int getpj_name(struct project *pjp,char *rbuf,int rlen,cchar *name)
{
	return uc_getpjbyname(name,pjp,rbuf,rlen) ;
}
/* end subroutine (getpj_name) */


int getpj_pjid(struct project *pjp,char *rbuf,int rlen,projid_t pjid)
{
	return uc_getpjbyid(pjid,pjp,rbuf,rlen) ;
}
/* end subroutine (getpj_pjid) */


