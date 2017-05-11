/* projectent */

/* subroutines for simple PROJECT object (from UNIX® library-3c) management */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines manage some simple tasks for the PROJECT object,
        referenced as 'struct project'. This object is defined by UNIX® (really
        Solaris®) standards.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>
#include	<project.h>

#include	<vsystem.h>
#include	<storeitem.h>
#include	<sbuf.h>
#include	<vechand.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	iceil(int,int) ;

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	storeitem_storestrs(STOREITEM *,int,const char *,int,char ***) ;
static int	storeitem_loadstrs(STOREITEM *,vechand *,int,const char *,int) ;

static int	si_copystr(STOREITEM *,char **,const char *) ;

static int	sbuf_fmtstrs(SBUF *,int,char **) ;


/* local variables */


/* exported subroutines */


int projectent_parse(pjp,pjbuf,pjlen,sp,sl)
struct project	*pjp ;
char		pjbuf[] ;
int		pjlen ;
const char	*sp ;
int		sl ;
{
	STOREITEM	ib, *ibp = &ib ;
	int		rs ;
	int		rs1 ;

	if (pjp == NULL) return SR_FAULT ;
	if (pjbuf == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	memset(pjp,0,sizeof(struct project)) ;
	if ((rs = storeitem_start(ibp,pjbuf,pjlen)) >= 0) {
	    int		fi = 0 ;
	    int		v ;
	    char	**sv ;
	    const char	*tp ;
	    const char	**vpp ;
	    while ((tp = strnchr(sp,sl,':')) != NULL) {
		vpp = NULL ;
	        switch (fi++) {
	        case 0:
	            vpp = (const char **) &pjp->pj_name ;
	            break ;
	        case 1:
	            rs = cfdeci(sp,(tp-sp),&v) ;
	            pjp->pj_projid = v ;
	            break ;
	        case 2:
	            vpp = (const char **) &pjp->pj_comment ;
	            break ;
		case 3:
	            rs = storeitem_storestrs(ibp,',',sp,(tp-sp),&sv) ;
		    pjp->pj_users = sv ;
	            break ;
	        case 4:
	            rs = storeitem_storestrs(ibp,',',sp,(tp-sp),&sv) ;
		    pjp->pj_groups = sv ;
	            break ;
	        case 5:
	            vpp = (const char **) &pjp->pj_attr ;
	            break ;
	        } /* end switch */
	        if ((rs >= 0) && (vpp != NULL)) {
	            int		cl ;
	            const char	*cp ;
	            if ((cl = sfshrink(sp,(tp-sp),&cp)) >= 0) {
	                rs = storeitem_strw(ibp,cp,cl,vpp) ;
	            }
	        }
	        sl -= ((tp+1)-sp) ;
	        sp = (tp+1) ;
	        if (rs < 0) break ;
	    } /* end while */
	    if ((rs >= 0) && (fi == 5) && sl && sp[0]) {
	        int		cl ;
	        const char	*cp ;
		vpp = (const char **) &pjp->pj_attr ;
		fi += 1 ;
	        if ((cl = sfshrink(sp,sl,&cp)) >= 0) {
	            rs = storeitem_strw(ibp,cp,cl,vpp) ;
	        }
	    }
	    rs1 = storeitem_finish(ibp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (storeitem) */

	return rs ;
}
/* end subroutine (projectent_parse) */


int projectent_load(pjp,pjbuf,pjlen,spjp)
struct project	*pjp ;
char		pjbuf[] ;
int		pjlen ;
const struct project	*spjp ;
{
	STOREITEM	ib ;
	int		rs ;
	int		rs1 ;

	if (pjp == NULL) return SR_FAULT ;
	if (pjbuf == NULL) return SR_FAULT ;
	if (spjp == NULL) return SR_FAULT ;

	memcpy(pjp,spjp,sizeof(struct project)) ;

	if ((rs = storeitem_start(&ib,pjbuf,pjlen)) >= 0) {

	    if (spjp->pj_users != NULL) {
	        int	n ;
	        int	i = 0 ;
	        void	**ptab ;

	        for (n = 0 ; spjp->pj_users[n] != NULL ; n += 1) ;

	        if ((rs = storeitem_ptab(&ib,n,&ptab)) >= 0) {
	            char	**tab = (char **) ptab ;

	            pjp->pj_users = tab ;
	            for (i = 0 ; spjp->pj_users[i] != NULL ; i += 1) {
	                rs = si_copystr(&ib,(tab + i),spjp->pj_users[i]) ;
	                if (rs < 0) break ;
	            } /* end for */
	            pjp->pj_users[i] = NULL ;

	        } /* end if (storeitem-ptab) */

	    } /* end if (users) */

	    if (spjp->pj_groups != NULL) {
	        int	n ;
	        int	i = 0 ;
	        void	**ptab ;

	        for (n = 0 ; spjp->pj_groups[n] != NULL ; n += 1) ;

	        if ((rs = storeitem_ptab(&ib,n,&ptab)) >= 0) {
	            char	**tab = (char **) ptab ;

	            pjp->pj_groups = tab ;
	            for (i = 0 ; spjp->pj_groups[i] != NULL ; i += 1) {
	                rs = si_copystr(&ib,(tab + i),spjp->pj_groups[i]) ;
	                if (rs < 0) break ;
	            } /* end for */
	            pjp->pj_groups[i] = NULL ;

	        } /* end if (storeitem-ptab) */

	    } /* end if (groups) */

	    si_copystr(&ib,&pjp->pj_name,spjp->pj_name) ;

	    si_copystr(&ib,&pjp->pj_comment,spjp->pj_comment) ;

	    si_copystr(&ib,&pjp->pj_attr,spjp->pj_attr) ;

	    rs1 = storeitem_finish(&ib) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return rs ;
}
/* end subroutine (projectent_load) */


int projectent_format(pjp,rbuf,rlen)
struct project	*pjp ;
char		rbuf[] ;
int		rlen ;
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;

	if (pjp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    int	i ;
	    int	v ;
	    for (i = 0 ; i < 6 ; i += 1) {
	        if (i > 0) rs = sbuf_char(&b,':') ;
	        if (rs >= 0) {
	            switch (i) {
	            case 0:
	                rs = sbuf_strw(&b,pjp->pj_name,-1) ;
	                break ;
	            case 1:
	                v = pjp->pj_projid ;
	                rs = sbuf_deci(&b,v) ;
	                break ;
	            case 2:
	                rs = sbuf_strw(&b,pjp->pj_comment,-1) ;
	                break ;
	            case 3:
	                rs = sbuf_fmtstrs(&b,',',pjp->pj_users) ;
	                break ;
	            case 4:
	                rs = sbuf_fmtstrs(&b,',',pjp->pj_groups) ;
	                break ;
	            case 5:
	                rs = sbuf_strw(&b,pjp->pj_attr,-1) ;
	                break ;
	            } /* end switch */
	        } /* end if */
	        if (rs < 0) break ;
	    } /* end for */
	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return rs ;
}
/* end subroutine (projectent_format) */


int projectent_size(pjp)
const struct project	*pjp ;
{
	int		size = 1 ;
	int		i ;
	if (pjp->pj_name != NULL) {
	    size += (strlen(pjp->pj_name)+1) ;
	}
	if (pjp->pj_comment != NULL) {
	    size += (strlen(pjp->pj_comment)+1) ;
	}
	if (pjp->pj_attr != NULL) {
	    size += (strlen(pjp->pj_attr)+1) ;
	}
	if (pjp->pj_users != NULL) {
	    for (i = 0 ; pjp->pj_users[i] != NULL ; i += 1) {
	        size += (strlen(pjp->pj_users[i])+1) ;
	    } /* end for */
	    size += (i*sizeof(const char *)) ;
	} /* end if */
	if (pjp->pj_groups != NULL) {
	    for (i = 0 ; pjp->pj_groups[i] != NULL ; i += 1) {
	        size += (strlen(pjp->pj_groups[i])+1) ;
	    } /* end for */
	    size += (i*sizeof(const char *)) ;
	} /* end if */
	size = iceil(size,sizeof(const char *)) ;
	return size ;
}
/* end subroutine (projectent_size) */


/* local subroutines */


static int storeitem_storestrs(ibp,sch,sp,sl,svp)
STOREITEM	*ibp ;
int		sch ;
const char	*sp ;
int		sl ;
char		***svp ;
{
	vechand		u ;
	int		rs ;

	if ((rs = vechand_start(&u,8,0)) >= 0) {
	    if ((rs = storeitem_loadstrs(ibp,&u,sch,sp,sl)) > 0) {
	        int	n = rs ;
	        void	**ptab ;

	        if ((rs = storeitem_ptab(ibp,n,&ptab)) >= 0) {
		    int		i ;
	            const char	*cp ;

	            *svp = (char **) ptab ;
		    for (i = 0 ; vechand_get(&u,i,&cp) >= 0 ; i += 1) {
	                (*svp)[i] = (char *) cp ;
	            } /* end for */
	            (*svp)[i] = NULL ;

	        } /* end if (storeitem_ptab) */

	    } else
	        (*svp) = NULL ;
	    vechand_finish(&u) ;
	} /* end if (vechand) */

	return rs ;
}
/* end subroutine (storeitem_storestrs) */


static int storeitem_loadstrs(ibp,ulp,sch,sp,sl)
STOREITEM	*ibp ;
vechand		*ulp ;
int		sch ;
const char	*sp ;
int		sl ;
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*tp, *cp ;

	while ((tp = strnchr(sp,sl,sch)) != NULL) {
	    if ((tp-sp) > 0) {
	        if ((rs = storeitem_strw(ibp,sp,(tp-sp),&cp)) >= 0) {
		    c += 1 ;
		    rs = vechand_add(ulp,cp) ;
	        }
	    } /* end if (non-zero) */
	    sl -= ((tp+1)-sp) ;
	    sp = (tp+1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && sl && sp[0]) {
	    if ((rs = storeitem_strw(ibp,sp,sl,&cp)) >= 0) {
		c += 1 ;
		rs = vechand_add(ulp,cp) ;
	    }
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (storeitem_loadstrs) */


static int si_copystr(ibp,pp,p1)
STOREITEM	*ibp ;
const char	*p1 ;
char		**pp ;
{
	int		rs = SR_OK ;
	const char	**cpp = (const char **) pp ;

	*cpp = NULL ;
	if (p1 != NULL)
	    rs = storeitem_strw(ibp,p1,-1,cpp) ;

	return rs ;
}
/* end subroutine (si_copystr) */


static int sbuf_fmtstrs(SBUF *bp,int sch,char **sv)
{
	int		rs = SR_OK ;

	if (sv != NULL) {
	    int	i ;
	    const char	*sp ;
	    for (i = 0 ; sv[i] != NULL ; i += 1) {
	        sp = sv[i] ;
		if (sp[0]) {
	            if (i > 0) rs = sbuf_char(bp,sch) ;
	            if (rs >= 0) rs = sbuf_strw(bp,sp,-1) ;
		}
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (non-null vector) */

	return rs ;
}
/* end subroutine (sbuf_fmtstrs) */


