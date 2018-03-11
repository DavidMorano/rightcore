/* userattrent */

/* subroutines for simple USERATTR object (from UNIX® library-3c) management */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines manage some simple tasks for the USERATTR object,
        referenced as 'userattr_t'. This object is defined by UNIX® (really
        Solaris®) standards.

	Structures:

	typedef struct userattr_s {
		char   *name;		# user name
		char   *qualifier;	# reserved for future use
		char   *res1;		# reserved for future use
		char   *res2;		# reserved for future use
		kva_t  *attr;		# array of key-value pair attributes
	} userattr_t;

	typedef struct kva_s {
		int	length;		# array length
		kv_t    *data;		# array of key value pairs
	} kva_t;			# Key-value array

	typedef struct kv_s {
		char   *key;
		char   *value;
	} kv_t;				# a key-value pair


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>
#include	<user_attr.h>

#include	<vsystem.h>
#include	<storeitem.h>
#include	<sbuf.h>
#include	<vechand.h>
#include	<vecstr.h>
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

static int	userattrent_parseattr(userattr_t *,STOREITEM *,cchar *,int) ;
static int	userattrent_parseattrload(userattr_t *,STOREITEM *,vecstr *,
			int) ;

static int	storeitem_attrload(STOREITEM *,kv_t *,int,const char *) ;

static int	si_copystr(STOREITEM *,const char **,const char *) ;

static int	sbuf_fmtattrs(SBUF *,kva_t *) ;


/* local variables */


/* exported subroutines */


int userattrent_parse(uap,uabuf,ualen,sp,sl)
userattr_t	*uap ;
char		uabuf[] ;
int		ualen ;
const char	*sp ;
int		sl ;
{
	STOREITEM	ib, *ibp = &ib ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (uap == NULL) return SR_FAULT ;
	if (uabuf == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	memset(uap,0,sizeof(userattr_t)) ;
	if ((sl > 0) && (sp[0] != '#')) {
	    if ((rs = storeitem_start(ibp,uabuf,ualen)) >= 0) {
	        int		fi = 0 ;
	        const char	*tp ;
	        const char	**vpp ;
	        while ((tp = strnchr(sp,sl,':')) != NULL) {
	            vpp = NULL ;
	            switch (fi++) {
	            case 0:
	                vpp = (const char **) &uap->name ;
	                break ;
	            case 1:
	                vpp = (const char **) &uap->qualifier ;
	                break ;
	            case 2:
	                vpp = (const char **) &uap->res1 ;
	                break ;
	            case 3:
	                vpp = (const char **) &uap->res1 ;
	                break ;
	            case 4:
	                rs = userattrent_parseattr(uap,ibp,sp,(tp-sp)) ;
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
	        if ((rs >= 0) && (fi == 4) && sl && sp[0]) {
	            rs = userattrent_parseattr(uap,ibp,sp,sl) ;
	        }
	        rs1 = storeitem_finish(ibp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (storeitem) */
	} /* end if (non-zero non-comment) */

	return rs ;
}
/* end subroutine (userattrent_parse) */


int userattrent_load(uap,uabuf,ualen,suap)
userattr_t	*uap ;
char		uabuf[] ;
int		ualen ;
const userattr_t	*suap ;
{
	STOREITEM	ib, *ibp = &ib ;
	int		rs ;
	int		rs1 ;

	if (uap == NULL) return SR_FAULT ;
	if (uabuf == NULL) return SR_FAULT ;
	if (suap == NULL) return SR_FAULT ;

	memcpy(uap,suap,sizeof(userattr_t)) ;

	if ((rs = storeitem_start(ibp,uabuf,ualen)) >= 0) {

	    if (suap->attr != NULL) {
	        const int	ksize = sizeof(kva_t) ;
	        const int	al = sizeof(void *) ;
	        void		*p ;

	        if ((rs = storeitem_block(ibp,ksize,al,&p)) >= 0) {
	            kva_t	*kvap = p ;
	            const int	n = suap->attr->length ;
	            int		dsize ;
	            uap->attr = kvap ;
	            dsize = (n*sizeof(kv_t)) ;
	            if ((rs = storeitem_block(ibp,dsize,al,&p)) >= 0) {
	                kv_t	*kvp = p ;
	                int	i = 0 ;
	                cchar	*rp ;
	                cchar	*dp ;

	                uap->attr->length = n ;
	                uap->attr->data = kvp ;
	                for (i = 0 ; i < n ; i += 1) {
	                    dp = suap->attr->data[i].key ;
	                    if ((rs = si_copystr(ibp,&rp,dp)) >= 0) {
	                        kvp[i].key = (char *) rp ;
	                        dp = suap->attr->data[i].value ;
	                        if ((rs = si_copystr(ibp,&rp,dp)) >= 0) {
	                            kvp[i].value = (char *) rp ;
	                        }
	                    }
	                    if (rs < 0) break ;
	                } /* end for */

	            } /* end if (storeitem_block) */
	        } /* end if (storeitem_block) */
	    } /* end if (attr) */

	    {
	        cchar	*cp ;
	        si_copystr(ibp,&cp,suap->name) ;
	        uap->name = (char *) cp ;
	    }

	    rs1 = storeitem_finish(ibp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (storeitem) */

	return rs ;
}
/* end subroutine (userattrent_load) */


int userattrent_format(uap,rbuf,rlen)
userattr_t	*uap ;
char		rbuf[] ;
int		rlen ;
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("userattrent_format: ent\n") ;
#endif

	if (uap == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    int	i ;
	    int	v ;
	    for (i = 0 ; i < 5 ; i += 1) {
	        if (i > 0) rs = sbuf_char(&b,':') ;
	        if (rs >= 0) {
	            switch (i) {
	            case 0:
	                rs = sbuf_strw(&b,uap->name,-1) ;
	                break ;
	            case 1:
	                if (uap->qualifier != NULL) {
	                    rs = sbuf_strw(&b,uap->qualifier,-1) ;
	                }
	                break ;
	            case 2:
	                if (uap->res1 != NULL) {
	                    rs = sbuf_strw(&b,uap->res1,-1) ;
	                }
	                break ;
	            case 3:
	                if (uap->res2 != NULL) {
	                    rs = sbuf_strw(&b,uap->res2,-1) ;
	                }
	                break ;
	            case 4:
	                if (uap->attr != NULL) {
	                    rs = sbuf_fmtattrs(&b,uap->attr) ;
	                }
	                break ;
	            } /* end switch */
	        } /* end if */
	        if (rs < 0) break ;
	    } /* end for */
	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

#if	CF_DEBUGS
	debugprintf("userattrent_format: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (userattrent_format) */


int userattrent_size(uap)
const userattr_t	*uap ;
{
	int		size = 1 ;
	if (uap->name != NULL) {
	    size += (strlen(uap->name)+1) ;
	}
	if (uap->attr != NULL) {
	    kva_t	*kvap = uap->attr ;
	    kv_t	*kvp ;
	    int		n, i ;
	    size += sizeof(kva_t) ;
	    n = kvap->length ;
	    kvp = kvap->data ;
	    for (i = 0 ; i < n ; i += 1) {
	        size += (strlen(kvp[i].key)+1) ;
	        size += (strlen(kvp[i].value)+1) ;
	    } /* end for */
	    size += (n*sizeof(kv_t)) ;
	} /* end if */
	size = iceil(size,sizeof(const char *)) ;
	return size ;
}
/* end subroutine (userattrent_size) */


/* local subroutines */


static int userattrent_parseattr(uap,ibp,sp,sl)
userattr_t	*uap ;
STOREITEM	*ibp ;
const char	*sp ;
int		sl ;
{
	vecstr		attrs, *alp = &attrs ;
	int		rs ;
	int		c = 0 ;

	if ((rs = vecstr_start(alp,0,0)) >= 0) {
	    const int	sch = ';' ;
	    const char	*tp ;
	    while ((tp = strnchr(sp,sl,sch)) != NULL) {
	        if ((tp-sp) > 0) {
		    c += 1 ;
	            rs = vecstr_add(alp,sp,(tp-sp)) ;
	        }
	        sl -= ((tp+1)-sp) ;
	        sp = (tp+1) ;
	        if (rs < 0) break ;
	    } /* end while */
	    if ((rs >= 0) && (sl > 0)) {
		c += 1 ;
	        rs = vecstr_add(alp,sp,sl) ;
	    }
	    if ((rs >= 0) && ((rs = vecstr_count(alp)) > 0)) {
	        rs = userattrent_parseattrload(uap,ibp,alp,rs) ;
	    }
	    vecstr_finish(alp) ;
	} /* end if (vecstr) */

	return rs ;
}
/* end subroutine (userattrent_parseattr) */


static int userattrent_parseattrload(uap,ibp,alp,n)
userattr_t	*uap ;
STOREITEM	*ibp ;
vecstr		*alp ;
int		n ;
{
	const int	ksize = sizeof(kva_t) ;
	const int	al = sizeof(void *) ;
	int		rs ;
	void		*p ;

	if ((rs = storeitem_block(ibp,ksize,al,&p)) >= 0) {
	    kva_t	*kvap = p ;
	    int		dsize ;
	    uap->attr = kvap ;
	    dsize = (n*sizeof(kv_t)) ;
	    if ((rs = storeitem_block(ibp,dsize,al,&p)) >= 0) {
	        kv_t	*kvp = p ;
	        int	i = 0 ;
	        cchar	*ep ;
	        uap->attr->length = n ;
	        uap->attr->data = kvp ;
	        for (i = 0 ; vecstr_get(alp,i,&ep) >= 0 ; i += 1) {
	            if (ep != NULL) {
	                rs = storeitem_attrload(ibp,kvp,i,ep) ;
	            }
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (storeitem_block) */
	} /* end if (storeitem_block) */

	return rs ;
}
/* end subroutine (userattrent_parseattrload) */


static int storeitem_attrload(ibp,kvp,i,ep)
STOREITEM	*ibp ;
kv_t		*kvp ;
int		i ;
const char	*ep ;
{
	int		rs ;
	int		el = -1 ;
	const char	*tp ;
	const char	*rp ;
	const char	*vp ;
	if ((tp = strchr(ep,'=')) != NULL) {
	    vp = (tp+1) ;
	    el = (tp-ep) ;
	} else {
	    vp = (ep+strlen(ep)) ;
	}
	if ((rs = storeitem_strw(ibp,ep,el,&rp)) >= 0) {
	    kvp[i].key = (char *) rp ;
	    if ((rs = si_copystr(ibp,&rp,vp)) >= 0) {
	        kvp[i].value = (char *) rp ;
	    }
	}
	return rs ;
}
/* end subroutine (storeitem_attrload) */


static int si_copystr(ibp,pp,p1)
STOREITEM	*ibp ;
const char	**pp ;
const char	*p1 ;
{
	int		rs = SR_OK ;
	const char	**cpp = (const char **) pp ;

	*cpp = NULL ;
	if (p1 != NULL) {
	    rs = storeitem_strw(ibp,p1,-1,cpp) ;
	}

	return rs ;
}
/* end subroutine (si_copystr) */


static int sbuf_fmtattrs(SBUF *bp,kva_t *attr)
{
	int		rs = SR_OK ;

	if (attr != NULL) {
	    kv_t	*kv = attr->data ;
	    const int	n = attr->length ;
	    const int	sch = ';' ;
	    int		i ;
	    const char	*sp ;
	    for (i = 0 ; i < n ; i += 1) {
	        if (kv[i].key != NULL) {
	            if (i > 0) rs = sbuf_char(bp,sch) ;
	            if (rs >= 0) rs = sbuf_strw(bp,kv[i].key,-1) ;
	            if ((rs >= 0) && (kv[i].value != NULL)) {
	                rs = sbuf_char(bp,'=') ;
	                if (rs >= 0) rs = sbuf_strw(bp,kv[i].value,-1) ;
	            }
	        } /* end if */
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (non-null attr) */

	return rs ;
}
/* end subroutine (sbuf_fmtattrs) */


