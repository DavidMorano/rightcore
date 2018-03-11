/* groupent */

/* subroutines for simple GROUP object (from UNIX® library-3c) management */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines manage some simple tasks for the GROUP object,
        referenced as 'struct group'. This object is defined by UNIX® standards.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<storeitem.h>
#include	<sbuf.h>
#include	<vechand.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	sichr(const char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	iceil(int,int) ;

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int groupent_parseusers(struct group *,STOREITEM *,const char *,int) ;
static int groupent_formatusers(struct group *,SBUF *) ;

static int si_copystr(STOREITEM *,char **,const char *) ;

static int storeitem_loadusers(STOREITEM *,vechand *,const char *,int) ;
static int storeitem_loaduser(STOREITEM *,vechand *,const char *,int) ;


/* local variables */


/* exported subroutines */


int groupent_parse(grp,grbuf,grlen,sp,sl)
struct group	*grp ;
char		grbuf[] ;
int		grlen ;
const char	*sp ;
int		sl ;
{
	STOREITEM	ib, *ibp = &ib ;
	int		rs ;
	int		rs1 ;

	if (grp == NULL) return SR_FAULT ;
	if (grbuf == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	memset(grp,0,sizeof(struct group)) ;
	if ((rs = storeitem_start(ibp,grbuf,grlen)) >= 0) {
	    int		fi = 0 ;
	    int		v ;
	    const char	*tp ;
	    const char	**vpp ;
	    while ((tp = strnchr(sp,sl,':')) != NULL) {
		vpp = NULL ;
	        switch (fi++) {
	        case 0:
	            vpp = (const char **) &grp->gr_name ;
	            break ;
	        case 1:
	            vpp = (const char **) &grp->gr_passwd ;
	            break ;
	        case 2:
	            rs = cfdeci(sp,(tp-sp),&v) ;
	            grp->gr_gid = v ;
	            break ;
	        case 3:
	            rs = groupent_parseusers(grp,ibp,sp,(tp-sp)) ;
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
	    if (rs >= 0) {
		if ((fi == 3) && sl && sp[0]) {
		    fi += 1 ;
		    rs = groupent_parseusers(grp,ibp,sp,sl) ;
		}
		if ((rs >= 0) && (fi < 3)) rs = SR_BADFMT ;
	    }
	    rs1 = storeitem_finish(ibp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (storeitem) */

	return rs ;
}
/* end subroutine (groupent_parse) */


int groupent_load(grp,grbuf,grlen,sgrp)
struct group	*grp ;
char		grbuf[] ;
int		grlen ;
const struct group	*sgrp ;
{
	STOREITEM	ib ;
	int		rs ;
	int		rs1 ;

	if (grp == NULL) return SR_FAULT ;
	if (grp == NULL) return SR_FAULT ;
	if (sgrp == NULL) return SR_FAULT ;

	memcpy(grp,sgrp,sizeof(struct group)) ;

	if ((rs = storeitem_start(&ib,grbuf,grlen)) >= 0) {

	    if (sgrp->gr_mem != NULL) {
	        int	n ;
	        void	**ptab ;

	        for (n = 0 ; sgrp->gr_mem[n] != NULL ; n += 1) ;

	        if ((rs = storeitem_ptab(&ib,n,&ptab)) >= 0) {
	            int		i = 0 ;
	            char	**tab = (char **) ptab ;

	            grp->gr_mem = tab ;
	            for (i = 0 ; sgrp->gr_mem[i] != NULL ; i += 1) {
	                rs = si_copystr(&ib,(grp->gr_mem + i),sgrp->gr_mem[i]) ;
	                if (rs < 0) break ;
	            } /* end for */
	            grp->gr_mem[i] = NULL ;

	        } /* end if (storeitem-ptab) */

	    } else
	        grp->gr_mem = NULL ;

	    si_copystr(&ib,&grp->gr_name,sgrp->gr_name) ;

	    si_copystr(&ib,&grp->gr_passwd,sgrp->gr_passwd) ;

	    rs1 = storeitem_finish(&ib) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (storeitem) */

	return rs ;
}
/* end subroutine (groupent_load) */


int groupent_format(grp,rbuf,rlen)
struct group	*grp ;
char		rbuf[] ;
int		rlen ;
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;

	if (grp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    int	i ;
	    int	v ;
	    for (i = 0 ; i < 4 ; i += 1) {
	        if (i > 0) rs = sbuf_char(&b,':') ;
	        if (rs >= 0) {
	            switch (i) {
	            case 0:
	                rs = sbuf_strw(&b,grp->gr_name,-1) ;
	                break ;
	            case 1:
	                rs = sbuf_strw(&b,grp->gr_passwd,-1) ;
	                break ;
	            case 2:
	                v = grp->gr_gid ;
	                rs = sbuf_deci(&b,v) ;
	                break ;
	            case 3:
	                rs = groupent_formatusers(grp,&b) ;
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
/* end subroutine (groupent_format) */


int groupent_size(grp)
const struct group	*grp ;
{
	int		size = 1 ;
	int		i ;
	if (grp->gr_name != NULL) {
	    size += (strlen(grp->gr_name)+1) ;
	}
	if (grp->gr_passwd != NULL) {
	    size += (strlen(grp->gr_passwd)+1) ;
	}
	if (grp->gr_mem != NULL) {
	    for (i = 0 ; grp->gr_mem[i] != NULL ; i += 1) {
	        size += (strlen(grp->gr_mem[i])+1) ;
	    } /* end for */
	    size += (i*sizeof(const char *)) ;
	} /* end if (group members) */
	size = iceil(size,sizeof(const char *)) ;
	return size ;
}
/* end subroutine (groupent_size) */


/* local subroutines */


static int groupent_parseusers(grp,ibp,sp,sl)
struct group	*grp ;
STOREITEM	*ibp ;
const char	*sp ;
int		sl ;
{
	vechand		u ;
	int		rs ;

	if ((rs = vechand_start(&u,8,0)) >= 0) {
	    if ((rs = storeitem_loadusers(ibp,&u,sp,sl)) > 0) {
	        int	n = rs ;
	        void	**ptab ;

	        if ((rs = storeitem_ptab(ibp,n,&ptab)) >= 0) {
		    int		i ;
	            const char	*cp ;

	            grp->gr_mem = (char **) ptab ;
		    for (i = 0 ; vechand_get(&u,i,&cp) >= 0 ; i += 1) {
	                grp->gr_mem[i] = (char *) cp ;
	            } /* end for */
	            grp->gr_mem[i] = NULL ;

	        } /* end if (storeitem-ptab) */

	    } else
	        grp->gr_mem = NULL ;
	    vechand_finish(&u) ;
	} /* end if (vechand) */

	return rs ;
}
/* end subroutine (groupent_parseusers) */


static int groupent_formatusers(struct group *grp,SBUF *bp)
{
	int		rs = SR_OK ;

	if (grp->gr_mem != NULL) {
	    int	i ;
	    const char	*un ;
	    for (i = 0 ; grp->gr_mem[i] != NULL ; i += 1) {
	        un = grp->gr_mem[i] ;
		if (un[0]) {
	            if (i > 0) rs = sbuf_char(bp,',') ;
		    if (rs >= 0) rs = sbuf_strw(bp,un,-1) ;
		}
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (non-null members) */

	return rs ;
}
/* end subroutine (groupent_formatusers) */


static int storeitem_loadusers(ibp,ulp,sp,sl)
STOREITEM	*ibp ;
vechand		*ulp ;
const char	*sp ;
int		sl ;
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*tp, *cp ;

	while ((tp = strnchr(sp,sl,',')) != NULL) {
	    if ((tp-sp) > 0) {
		    c += 1 ;
		    rs = storeitem_loaduser(ibp,ulp,sp,(tp-sp)) ;
	    } /* end if (non-zero) */
	    sl -= ((tp+1)-sp) ;
	    sp = (tp+1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && sl && sp[0]) {
		c += 1 ;
		rs = storeitem_loaduser(ibp,ulp,sp,sl) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (storeitem_loadusers) */


static int storeitem_loaduser(ibp,ulp,sp,sl)
STOREITEM	*ibp ;
vechand		*ulp ;
const char	*sp ;
int		sl ;
{
	int		rs ;
	const char	*cp ;
	if ((rs = storeitem_strw(ibp,sp,sl,&cp)) >= 0) {
	    rs = vechand_add(ulp,cp) ;
	}
	return rs ;
}
/* end subroutine (storeitem_loaduser) */


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


