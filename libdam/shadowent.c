/* shadowent */

/* SHADOW structure management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_EXTRAS	0		/* extra SHADOW entries */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines perform some SHADOW-structure management functions.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>
#include	<shadow.h>

#include	<vsystem.h>
#include	<storeitem.h>
#include	<sbuf.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	sichr(const char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecl(const char *,int,long *) ;
extern int	cfdecul(const char *,int,ulong *) ;

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	shadowent_parseone(struct spwd *,STOREITEM *,int,
const char *,int) ;
static int	shadowent_parsedefs(struct spwd *,STOREITEM *,int) ;

static int	si_copystr(STOREITEM *,char **,const char *) ;


/* local variables */


/* exported subroutines */


int shadowent_parse(spp,spbuf,splen,sp,sl)
struct spwd	*spp ;
char		spbuf[] ;
int		splen ;
const char	*sp ;
int		sl ;
{
	STOREITEM	ib, *ibp = &ib ;
	int		rs ;
	int		rs1 ;

	if (spp == NULL) return SR_FAULT ;
	if (spbuf == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	memset(spp,0,sizeof(struct spwd)) ;
	if ((rs = storeitem_start(ibp,spbuf,splen)) >= 0) {
	    int		fi = 0 ;
	    int		si ;
	    while ((si = sichr(sp,sl,':')) >= 0) {
	        rs = shadowent_parseone(spp,ibp,fi++,sp,si) ;
	        sl -= (si+1) ;
	        sp += (si+1) ;
	        if (rs < 0) break ;
	    } /* end while */
	    if ((rs >= 0) && sl && sp[0]) {
	        rs = shadowent_parseone(spp,ibp,fi++,sp,sl) ;
	    }
	    if (rs >= 0) {
	        rs = shadowent_parsedefs(spp,ibp,fi) ;
	        fi = rs ;
	    }
	    if ((rs >= 0) && (fi < 6)) rs = SR_BADFMT ;
	    rs1 = storeitem_finish(ibp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (storeitem) */

	return rs ;
}
/* end subroutine (shadowent_parse) */


int shadowent_load(spp,spbuf,splen,sspp)
struct spwd	*spp ;
char		spbuf[] ;
int		splen ;
const struct spwd	*sspp ;
{
	STOREITEM	ib ;
	int		rs ;
	int		rs1 ;

	if (spp == NULL) return SR_FAULT ;
	if (spbuf == NULL) return SR_FAULT ;
	if (sspp == NULL) return SR_FAULT ;

	memcpy(spp,sspp,sizeof(struct spwd)) ;

	if ((rs = storeitem_start(&ib,spbuf,splen)) >= 0) {

	    si_copystr(&ib,&spp->sp_namp,sspp->sp_namp) ;

	    si_copystr(&ib,&spp->sp_pwdp,sspp->sp_pwdp) ;

	    rs1 = storeitem_finish(&ib) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (storeitem) */

	return rs ;
}
/* end subroutine (shadowent_load) */


int shadowent_format(spp,rbuf,rlen)
struct spwd	*spp ;
char		rbuf[] ;
int		rlen ;
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;

	if (spp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    int		i ;
	    long	v ;
	    for (i = 0 ; i < 9 ; i += 1) {
	        if (i > 0) rs = sbuf_char(&b,':') ;
	        if (rs >= 0) {
	            switch (i) {
	            case 0:
	                rs = sbuf_strw(&b,spp->sp_namp,-1) ;
	                break ;
	            case 1:
	                rs = sbuf_strw(&b,spp->sp_pwdp,-1) ;
	                break ;
	            case 2:
	            case 3:
	            case 4:
	            case 5:
	            case 6:
	            case 7:
	                switch (i) {
	                case 2:
	                    v = spp->sp_lstchg ;
	                    break ;
	                case 3:
	                    v = spp->sp_min ;
	                    break ;
	                case 4:
	                    v = spp->sp_max ;
	                    break ;
	                case 5:
	                    v = spp->sp_warn ;
	                    break ;
	                case 6:
	                    v = spp->sp_inact ;
	                    break ;
	                case 7:
	                    v = spp->sp_expire ;
	                    break ;
	                } /* end switch */
	                if (v != -1) {
	                    rs = sbuf_decl(&b,v) ;
	                }
	                break ;
	            case 8:
	                {
	                    ulong	uv = spp->sp_flag ;
	                    if (uv != 0) {
	                        rs = sbuf_decul(&b,uv) ;
	                    }
	                }
	                break ;
	            } /* end switch */
	        } /* end if (ok) */
	        if (rs < 0) break ;
	    } /* end for */
	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return rs ;
}
/* end subroutine (shadowent_format) */


int shadowent_size(pp)
const struct spwd	*pp ;
{
	int		size = 1 ;
	if (pp->sp_namp != NULL) {
	    size += (strlen(pp->sp_namp)+1) ;
	}
	if (pp->sp_pwdp != NULL) {
	    size += (strlen(pp->sp_pwdp)+1) ;
	}
	return size ;
}
/* end subroutine (shadowent_size) */


/* local subroutines */


static int shadowent_parseone(spp,ibp,fi,vp,vl)
struct spwd	*spp ;
STOREITEM	*ibp ;
int		fi ;
const char	*vp ;
int		vl ;
{
	int		rs = SR_OK ;
	long		v ;
	const char	**vpp = NULL ;

	switch (fi) {
	case 0:
	    vpp = (const char **) &spp->sp_namp ;
	    break ;
	case 1:
	    vpp = (const char **) &spp->sp_pwdp ;
	    break ;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	    if ((vl > 0) && ((rs = cfdecl(vp,vl,&v)) >= 0)) {
	        switch (fi) {
	        case 2:
	            spp->sp_lstchg = v ;
	            break ;
	        case 3:
	            spp->sp_min = v ;
	            break ;
	        case 4:
	            spp->sp_max = v ;
	            break ;
	        case 5:
	            spp->sp_warn = v ;
	            break ;
	        case 6:
	            spp->sp_inact = v ;
	            break ;
	        case 7:
	            spp->sp_expire = v ;
	            break ;
	        } /* end switch */
	    } /* end if (cfdecl) */
	    break ;
	case 8:
	    if (vl > 0) {
	        ulong	uv ;
	        rs = cfdecul(vp,vl,&uv) ;
	        spp->sp_flag = v ;
	    }
	    break ;
	} /* end switch */
	if ((rs >= 0) && (vpp != NULL)) {
	    int		cl ;
	    const char	*cp ;
	    if ((cl = sfshrink(vp,vl,&cp)) >= 0) {
	        rs = storeitem_strw(ibp,cp,cl,vpp) ;
	    }
	}

	return rs ;
}
/* end subroutine (shadowent_parseone) */


/* ARGSUSED */
static int shadowent_parsedefs(spp,ibp,sfi)
struct spwd	*spp ;
STOREITEM	*ibp ;
int		sfi ;
{
	int		rs = SR_OK ;
	if (sfi == 1) {
	    const char	**vpp = (const char **) &spp->sp_pwdp ;
	    const char	*np = spp->sp_namp ;
	    const char	*vp ;
	    vp = (np + strlen(np)) ;
	    sfi += 1 ;
	    rs = storeitem_strw(ibp,vp,0,vpp) ;
	}
	return (rs >= 0) ? sfi : rs ;
}
/* end subroutine (shadowent_parsedefs) */


static int si_copystr(ibp,pp,p1)
STOREITEM	*ibp ;
char		**pp ;
const char	*p1 ;
{
	int		rs = SR_OK ;
	const char	**cpp = (const char **) pp ;

	*cpp = NULL ;
	if (p1 != NULL)
	    rs = storeitem_strw(ibp,p1,-1,cpp) ;

	return rs ;
}
/* end subroutine (si_copystr) */


