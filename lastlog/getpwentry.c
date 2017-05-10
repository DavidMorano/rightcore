/* getpwentry */

/* subroutines to access the 'passwd' and 'group' databases */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine was written so that we could use a single interface to
	access the 'passwd' database on all UNIX® platforms.  This code module
	provides a platform independent implementation of UNIX® 'passwd'
	database access subroutines.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>

#if	SYSHAS_SHADOW
#include	<shadow.h>
#endif

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<storeitem.h>
#include	<gecos.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<localmisc.h>

#include	"pwentry.h"


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#define	GETPW_UID	ugetpw_uid
#else
#define	GETPW_NAME	getpw_name
#define	GETPW_UID	getpw_uid
#endif /* CF_UGETPW */

#define	TO_AGAIN	10


/* external subroutines */

extern int	snwcpyhyphen(char *,int,const char *,int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward subroutines */

static int	getpwentry_load(PWENTRY *,char *,int,struct passwd *) ;
static int	getpwentry_gecos(PWENTRY *,STOREITEM *,const char *) ;
static int	getpwentry_shadow(PWENTRY *,STOREITEM *,struct passwd *) ;
static int	getpwentry_setnuls(PWENTRY *,const char *) ;

static int	isNoEntry(int) ;

static int	checknul(const char *,const char **) ;


/* local variables */

static const int	rsents[] = {
	    SR_NOTFOUND,
	    SR_ACCESS,
	    SR_NOSYS,
	    0
} ;


/* exported subroutines */


int getpwentry_name(PWENTRY *uep,char ebuf[],int elen,cchar name[])
{
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	char		*pwbuf ;

#if	CF_DEBUGS
	debugprintf("getpwentry_name: ent u=%s\n",name) ;
#endif

	if (uep == NULL) return SR_FAULT ;
	if (ebuf == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,name)) >= 0) {
	        rs = getpwentry_load(uep,ebuf,elen,&pw) ;
	    }
	    uc_free(pwbuf) ;
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("getpwentry_name: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (getpwentry_name) */


int getpwentry_uid(PWENTRY *uep,char *ebuf,int elen,uid_t uid)
{
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	char		*pwbuf ;

#if	CF_DEBUGS
	debugprintf("getpwentry_uid: ent uid=%d\n",uid) ;
#endif

	if (uep == NULL) return SR_FAULT ;
	if (ebuf == NULL) return SR_FAULT ;

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    if ((rs = getpwusername(&pw,pwbuf,pwlen,uid)) >= 0) {
	        rs = getpwentry_load(uep,ebuf,elen,&pw) ;
	    }
	    uc_free(pwbuf) ;
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (getpwentry_uid) */


/* local subroutines */


/* load a user entry from an internal one */
static int getpwentry_load(PWENTRY *uep,char *ebuf,int elen,struct passwd *pep)
{
	int		rs ;
	int		rs1 ;

	if (uep == NULL) return SR_FAULT ;
	if (ebuf == NULL) return SR_FAULT ;
	if (pep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("getpwentry_load: ent name=%s\n",pep->pw_name) ;
#endif

	memset(uep,0,sizeof(PWENTRY)) ;

	if (pep->pw_name != NULL) {
	    STOREITEM	ubuf ;
	    const char	*emptyp = NULL ;
	    if ((rs = storeitem_start(&ubuf,ebuf,elen)) >= 0) {
		cchar	**vpp ;

/* fill in the stuff that we got from the system */

		vpp = &uep->username ;
	        rs = storeitem_strw(&ubuf,pep->pw_name,-1,vpp) ;
	        emptyp = (uep->username + rs) ;

#if	(! SYSHAS_SHADOW)
	        if ((rs >= 0) && (pep->pw_passwd != NULL)) {
		    vpp = &uep->password ;
	            storeitem_strw(&ubuf,pep->pw_passwd,-1,vpp) ;
	        }
#endif /* SYSHAS_SHADOW */

	        uep->uid = pep->pw_uid ;
	        uep->gid = pep->pw_gid ;

	        if ((rs >= 0) && (pep->pw_gecos != NULL)) {
		    vpp = &uep->gecos ;
	            storeitem_strw(&ubuf,pep->pw_gecos,-1,vpp) ;
		}

/* break up the GECOS field further */

		if (rs >= 0) {
		    cchar	*gecos = pep->pw_gecos ;
	            if ((rs = getpwentry_gecos(uep,&ubuf,gecos)) >= 0) {

	                if (pep->pw_dir != NULL) {
			    vpp = &uep->dir ;
	                    storeitem_strw(&ubuf,pep->pw_dir,-1,vpp) ;
		        }

	                if (pep->pw_shell != NULL) {
			    vpp = &uep->shell ;
	                    storeitem_strw(&ubuf,pep->pw_shell,-1,vpp) ;
		        }

	                if ((rs = getpwentry_shadow(uep,&ubuf,pep)) >= 0) {
	                    rs = 0 ;
	                } else if (isNoEntry(rs)) {
	                    rs = SR_OK ;
	                }

	            } /* end if (gecos) */
		} /* end if (ok) */

#if	CF_DEBUGS
	        debugprintf("getpwentry_load: gecos-out rs=%d\n",rs) ;
#endif

	        getpwentry_setnuls(uep,emptyp) ;

	        rs1 = storeitem_finish(&ubuf) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (storeitem) */
	} else
	    rs = SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("getpwentry_load: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (getpwentry_load) */


static int getpwentry_gecos(PWENTRY *uep,STOREITEM *bp,cchar *gecosdata)
{
	GECOS		g ;
	int		rs ;
	int		rs1 ;

	if ((rs = gecos_start(&g,gecosdata,-1)) >= 0) {
	    int		i ;
	    int		vl ;
	    const char	*vp ;
	    for (i = 0 ; i < gecosval_overlast ; i += 1) {
	        if ((vl = gecos_getval(&g,i,&vp)) >= 0) {
	            void	*p ;
		    cchar	**vpp = NULL ;
	            switch (i) {
	            case gecosval_organization:
	                vpp = &uep->organization ;
	                break ;
	            case gecosval_realname:
	                if ((rs = uc_malloc((vl+1),&p)) >= 0) {
	                    char	*nbuf = p ;
	                    if (strnchr(vp,vl,'_') != NULL) {
	                        rs = snwcpyhyphen(nbuf,-1,vp,vl) ;
	                        vp = nbuf ;
	                    }
	                    if (rs >= 0) {
	                        vpp = &uep->realname ;
	                        rs = storeitem_strw(bp,vp,vl,vpp) ;
				vpp = NULL ;
	                    }
	                    uc_free(p) ;
	                } /* end if (memory_allocation) */
	                break ;
	            case gecosval_account:
	                vpp = &uep->account ;
	                break ;
	            case gecosval_bin:
	                vpp = &uep->bin ;
	                break ;
	            case gecosval_office:
	                vpp = &uep->office ;
	                break ;
	            case gecosval_wphone:
	                vpp = &uep->wphone ;
	                break ;
	            case gecosval_hphone:
	                vpp = &uep->hphone ;
	                break ;
	            case gecosval_printer:
	                vpp = &uep->printer ;
	                break ;
	            } /* end switch */
		    if ((rs >= 0) && (vpp != NULL)) {
	                rs = storeitem_strw(bp,vp,vl,vpp) ;
		    }
	        } /* end if (gecos_getval) */
	        if (rs < 0) break ;
	    } /* end for */
	    rs1 = gecos_finish(&g) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (gecos) */

	return rs ;
}
/* end subroutine (getpwentry_gecos) */


static int getpwentry_shadow(PWENTRY *uep,STOREITEM *sip,struct passwd *pep)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("getpwentry_shadow: ent name=%s\n",pep->pw_name) ;
#endif

#if	SYSHAS_SHADOW
	{
	    struct spwd	sd ;
	    const int	splen = getbufsize(getbufsize_sp) ;
	    char	*spbuf ;
#if	CF_DEBUGS
	    debugprintf("getpwentry_shadow: rocking and rolling\n") ;
#endif
	    if ((rs = uc_malloc((splen+1),&spbuf)) >= 0) {
	        const char	*pn = pep->pw_name ;
	        const char	**vpp = &uep->password ;
#if	CF_DEBUGS
	        debugprintf("getpwentry_shadow: pn=%s\n",pn) ;
#endif
	        if ((rs = getsp_name(&sd,spbuf,splen,pn)) >= 0) {
	            uep->lstchg = sd.sp_lstchg ;
	            uep->min = sd.sp_min ;
	            uep->max = sd.sp_max ;
	            uep->warn = sd.sp_warn ;
	            uep->inact = sd.sp_inact ;
	            uep->expire = sd.sp_expire ;
	            uep->flag = sd.sp_flag ;
	            if (pep->pw_passwd != NULL) {
	                if ((strcmp(pep->pw_passwd,"*NP*") == 0) ||
	                    (strcmp(pep->pw_passwd,"x") == 0)) {
	                    storeitem_strw(sip,sd.sp_pwdp,-1,vpp) ;
	                } else {
	                    const char	*passwd = pep->pw_passwd ;
	                    storeitem_strw(sip,passwd,-1,vpp) ;
	                }
	            } else {
	                storeitem_strw(sip,sd.sp_pwdp,-1,vpp) ;
	            }
	        } else if (rs == SR_ACCESS) {
#if	CF_DEBUGS
	            debugprintf("getpwentry_shadow: access problem\n") ;
#endif
	            rs = SR_OK ;
	            if (pep->pw_passwd != NULL) {
	                rs = storeitem_strw(sip,pep->pw_passwd,-1,vpp) ;
	            }
	        }
	        uc_free(spbuf) ;
	    } /* end if (m-a) */
	} /* end block */
#else /* SYSHAS_SHADOW */
	{
#if	CF_DEBUGS
	    debugprintf("getpwentry_shadow: party pooper\n") ;
#endif
	    rs = SR_NOSYS ;
	    uep->lstchg = 1 ;
	    uep->min = -1 ;
	    uep->max = -1 ;
	    uep->warn = -1 ;
	    uep->inact = -1 ;
#ifdef	OPTIONAL
	    uep->expire = 0 ;
	    uep->flag = 0 ;
#endif
	}
#endif /* SYSHAS_SHADOW */

#if	CF_DEBUGS
	debugprintf("getpwentry_shadow: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (getpwentry_shadow) */


static int getpwentry_setnuls(PWENTRY *uep,cchar *emptyp)
{

	checknul(emptyp,&uep->username) ;
	checknul(emptyp,&uep->password) ;
	checknul(emptyp,&uep->gecos) ;
	checknul(emptyp,&uep->dir) ;
	checknul(emptyp,&uep->shell) ;
	checknul(emptyp,&uep->organization) ;
	checknul(emptyp,&uep->realname) ;
	checknul(emptyp,&uep->account) ;
	checknul(emptyp,&uep->bin) ;
	checknul(emptyp,&uep->name_m1) ;
	checknul(emptyp,&uep->name_m2) ;
	checknul(emptyp,&uep->name_l) ;
	checknul(emptyp,&uep->office) ;
	checknul(emptyp,&uep->wphone) ;
	checknul(emptyp,&uep->hphone) ;
	checknul(emptyp,&uep->printer) ;

	return 0 ;
}
/* end subroutine (getpwentry_setnuls) */


static int checknul(cchar *emptyp,cchar **epp)
{
	if (*epp == NULL) {
	    *epp = emptyp ;
	}
	return 0 ;
}
/* end subroutine (checknul) */


static int isNoEntry(int rs)
{
	return isOneOf(rsents,rs) ;
}
/* end subroutine (isNoEntry) */


