/* mailmsg */

/* message parsing object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_PEDANTIC	0		/* extra precautions */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

	= 2004-10-13, David A­D­ Morano
	Removed VECSTR as an optional storage object.

	= 2017-09-06, David A­D­ Morano
	Removed some unused subroutine at the end.

*/

/* Copyright © 1998,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module operates on mail messages. It is expected that a file be
        opened and fed to this object one line at a time. The order of
        processing should proceed roughly as follows by the user:

	call	mailmsg_start()

	load lines of message:
		while (read_line) {
		    mailmsg_loadline()
		}

	call any of:
		mailmsg_envaddress()
		mailmsg_hdrval()
		mailmsg_hdrival()
		...()

	finally call:
		mailmsg_finish()

	Notes:

        Our basic algoirthm is to read all message-header lines and to store
        them in allocated space. We then construct structures that contain
        elements that point to strings in the stored lines (previously read). We
        had (at least) two options for which object to use to store the lines:
        STRPACK or VECSTR. Either is useful and probably fairly efficient. The
        STRPACK is more space efficient but VECSTR is probably more speed
        efficient. I originally had both coded in and available with a
        compile-time switch, but I removed that option. We now only use STRPACK.
        If you want VECSTR or something else, just replace calls to STRPACK with
        your new thing. There are only a few places where these are.

        This object is pretty kick-butt fast! We also use late processing and
        caching almost throughout, so we don't waste time on things the caller
        doesn't need.


*******************************************************************************/


#define	MAILMSG_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<strings.h>		/* |strncasecmp(3c)| */

#include	<vsystem.h>
#include	<estrings.h>
#include	<vechand.h>
#include	<vecobj.h>
#include	<mailmsgmatenv.h>
#include	<strpack.h>
#include	<localmisc.h>

#include	"mailmsg.h"


/* local defines */

#ifndef	MAILMSG_HDR
#define	MAILMSG_HDR	struct mailmsg_hdr
#endif /* MAILMSG_HDR */
#ifndef	MAILMSG_ENV
#define	MAILMSG_ENV	struct mailmsg_env
#endif /* MAILMSG_ENV */

#define	MAILMSGHDR	MAILMSG_HDR
#define	MAILMSGHDRVAL	struct msghdrval
#define	MAILMSGHDRINST	struct msghdrinst
#define	MAILMSGHDRNAME	struct msghdrname

#define	DEFHEADERS	25

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	HDRNAMELEN	80

#define	ISHDRCONT(c)	(((c) == ' ') || ((c) == '\t'))
#define	ISEND(c)	(((c) == '\n') || ((c) == '\r'))


/* external subroutines */

#if	defined(BSD) && (! defined(EXTERN_STRNCASECMP))
extern int	strncasecmp(const char *,const char *,int) ;
#endif

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	mailmsgmathdr(cchar *,int,int *) ;
extern int	hasuc(cchar *,int) ;
extern int	tolc(int) ;
extern int	touc(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;


/* external variables */


/* local structures */

struct msghdrval {
	const char	*vp ;
	int		vl ;
} ;

struct msghdrinst {
	const char	*vp ;
	VECOBJ		vals ;
	int		vl ;
	int		f_alloc ;
} ;

struct msghdrname {
	const char	*name ;
	const char	*vp ;
	VECOBJ		insts ;
	int		vl ;
	int		f_alloc ;
	int		namelen ;
	int		lastinst ;	/* index of last INST */
} ;

enum msgstates {
	msgstate_env,
	msgstate_hdr,
	msgstate_overlast
} ;


/* forward references */

static int	mailmsg_procline(MAILMSG *,const char *,int) ;

static int	mailmsg_envbegin(MAILMSG *) ;
static int	mailmsg_envadd(MAILMSG *,MAILMSGMATENV *) ;
static int	mailmsg_envend(MAILMSG *) ;

static int	mailmsg_hdrbegin(MAILMSG *) ;
static int	mailmsg_hdrend(MAILMSG *) ;
static int	mailmsg_hdraddnew(MAILMSG *,cchar *,int,cchar *,int) ;
static int	mailmsg_hdraddcont(MAILMSG *,cchar *,int) ;
static int	mailmsg_hdrmatch(MAILMSG *,MAILMSGHDRNAME **,cchar *,int) ;

static int	msghdrname_start(MAILMSGHDRNAME *,cchar *,int,cchar *,int) ;
static int	msghdrname_match(MAILMSGHDRNAME *,const char *,int) ;
static int	msghdrname_addnew(MAILMSGHDRNAME *,const char *,int) ;
static int	msghdrname_addcont(MAILMSGHDRNAME *,const char *,int) ;
static int	msghdrname_iline(MAILMSGHDRNAME *,int,int,const char **) ;
static int	msghdrname_ival(MAILMSGHDRNAME *,int,const char **) ;
static int	msghdrname_val(MAILMSGHDRNAME *,const char **) ;
static int	msghdrname_count(MAILMSGHDRNAME *) ;
static int	msghdrname_finish(MAILMSGHDRNAME *) ;

static int	msghdrinst_start(MAILMSGHDRINST *,const char *,int) ;
static int	msghdrinst_add(MAILMSGHDRINST *,const char *,int) ;
static int	msghdrinst_ival(MAILMSGHDRINST *,int,const char **) ;
static int	msghdrinst_val(MAILMSGHDRINST *,const char **) ;
static int	msghdrinst_finish(MAILMSGHDRINST *) ;


/* local variables */


/* exported subroutines */


int mailmsg_start(MAILMSG *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(MAILMSG)) ;

	if ((rs = strpack_start(&op->stores,2000)) >= 0) {
	    if ((rs = mailmsg_envbegin(op)) >= 0) {
		if ((rs = mailmsg_hdrbegin(op)) >= 0) {
		    op->magic = MAILMSG_MAGIC ;
		}
		if (rs < 0)
		    mailmsg_envend(op) ;
	    }
	    if (rs < 0)
		strpack_finish(&op->stores) ;
	} /* end if */

	return rs ;
}
/* end subroutine (mailmsg_start) */


int mailmsg_finish(MAILMSG *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILMSG_MAGIC) return SR_NOTOPEN ;

	rs1 = mailmsg_hdrend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = mailmsg_envend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = strpack_finish(&op->stores) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (mailmsg_finish) */


int mailmsg_loadline(MAILMSG *op,cchar *lp,int ll)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("mailmsg_loadline: ent l=>%t<\n",
	    lp,strlinelen(lp,ll,40)) ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILMSG_MAGIC) return SR_NOTOPEN ;

	if (ll < 0)
	    ll = strlen(lp) ;

	while ((ll > 0) && ISEND(lp[ll-1])) {
	    ll -= 1 ;
	}

	if (ll > 0) {
	    int		sl ;
	    const char	*sp ;
	    if ((rs = strpack_store(&op->stores,lp,ll,&sp)) >= 0) {
		sl = ll ;
	        rs = mailmsg_procline(op,sp,sl) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("mailmsg_loadline: ret rs=%d ll=%u\n",rs,ll) ;
#endif

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (mailmsg_loadline) */


int mailmsg_envcount(MAILMSG *op)
{
	MAILMSG_ENV	*oep ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILMSG_MAGIC) return SR_NOTOPEN ;

	oep = &op->envs ;
	rs = vecobj_count(&oep->insts) ;

	return rs ;
}
/* end subroutine (mailmsg_envcount) */


int mailmsg_envaddress(MAILMSG *op,int i,cchar **rpp)
{
	MAILMSG_ENV	*oep = &op->envs ;
	MAILMSGMATENV	*ep ;
	int		rs ;
	int		el = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILMSG_MAGIC) return SR_NOTOPEN ;

	if ((rs = vecobj_get(&oep->insts,i,&ep)) >= 0) {
	    if (ep != NULL) el = ep->a.el ;
	}

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? ep->a.ep : NULL ;
	}

	return (rs >= 0) ? el : rs ;
}
/* end subroutine (mailmsg_envaddress) */


int mailmsg_envdate(MAILMSG *op,int i,cchar **rpp)
{
	MAILMSG_ENV	*oep = &op->envs ;
	MAILMSGMATENV	*ep ;
	int		rs ;
	int		el = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILMSG_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("mailmsg_envdate: i=%d\n",i) ;
#endif

	if ((rs = vecobj_get(&oep->insts,i,&ep)) >= 0) {
	    if (ep != NULL) el = ep->d.el ;
	}

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? ep->d.ep : NULL ;
	}

#if	CF_DEBUGS
	debugprintf("mailmsg_envdate: ret rs=%d len=%u\n",rs,el) ;
#endif

	return (rs >= 0) ? el : rs ;
}
/* end subroutine (mailmsg_envdate) */


int mailmsg_envremote(MAILMSG *op,int i,cchar **rpp)
{
	MAILMSG_ENV	*oep = &op->envs ;
	MAILMSGMATENV	*ep ;
	int		rs ;
	int		el = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILMSG_MAGIC) return SR_NOTOPEN ;

	if ((rs = vecobj_get(&oep->insts,i,&ep)) >= 0) {
	    if (ep != NULL) el = ep->r.el ;
	}

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? ep->r.ep : NULL ;
	}

	return (rs >= 0) ? el : rs ;
}
/* end subroutine (mailmsg_envremote) */


int mailmsg_hdrcount(MAILMSG *op,cchar name[])
{
	MAILMSGHDRNAME	*hnp ;
	const int	hlen = HDRNAMELEN ;
	int		rs ;
	int		hl = -1 ;
	int		c = 0 ;
	const char	*hp = name ;
	char		hbuf[HDRNAMELEN + 1] ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILMSG_MAGIC) return SR_NOTOPEN ;

	if (hasuc(name,-1)) {
	    hl = strwcpylc(hbuf,name,hlen) - hbuf ;
	    hp = hbuf ;
	}

	if ((rs = mailmsg_hdrmatch(op,&hnp,hp,hl)) >= 0) {
	    if (hnp != NULL) {
	        rs = msghdrname_count(hnp) ;
	        c = rs ;
	    }
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mailmsg_hdrcount) */


int mailmsg_hdrikey(MAILMSG *op,int hi,cchar **rpp)
{
	MAILMSG_HDR	*ohp ;
	MAILMSGHDRNAME	*hnp ;
	int		rs ;
	int		nl = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILMSG_MAGIC) return SR_NOTOPEN ;

	ohp = &op->hdrs ;
	if ((rs = vecobj_get(&ohp->names,hi,&hnp)) >= 0) {
	    if (hnp != NULL) nl = hnp->namelen ;
	}

	if (rpp != NULL)
	    *rpp = (rs >= 0) ? hnp->name : NULL ;

	return (rs >= 0) ? nl : rs ;
}
/* end subroutine (mailmsg_hdrikey) */


int mailmsg_hdriline(MAILMSG *op,cchar *name,int hi,int li,cchar **rpp)
{
	MAILMSGHDRNAME	*hnp = NULL ;
	const int	hlen = HDRNAMELEN ;
	int		rs ;
	int		hl = -1 ;
	int		vl = 0 ;
	const char	*hp = name ;
	char		hbuf[HDRNAMELEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;

	if (op->magic != MAILMSG_MAGIC) return SR_NOTOPEN ;

	if (hasuc(name,-1)) {
	    hl = strwcpylc(hbuf,name,hlen) - hbuf ;
	    hp = hbuf ;
	}

	if ((rs = mailmsg_hdrmatch(op,&hnp,hp,hl)) >= 0) {
	    if (hnp != NULL) {
	        rs = msghdrname_iline(hnp,hi,li,rpp) ;
	        vl = rs ;
#if	CF_DEBUGS
	debugprintf("mailmsg_hdriline: msghdrname_iline() rs=%d\n",rs) ;
#endif
	    }
	}

	if (rs < 0) {
	    if (rpp != NULL) *rpp = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("mailmsg_hdriline: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (mailmsg_hdriline) */


int mailmsg_hdrival(MAILMSG *op,cchar *name,int hi,cchar **rpp)
{
	MAILMSGHDRNAME	*hnp = NULL ;
	const int	hlen = HDRNAMELEN ;
	int		rs ;
	int		hl = -1 ;
	int		vl = 0 ;
	const char	*hp = name ;
	char		hbuf[HDRNAMELEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;

	if (op->magic != MAILMSG_MAGIC) return SR_NOTOPEN ;

	if (hasuc(name,-1)) {
	    hl = strwcpylc(hbuf,name,hlen) - hbuf ;
	    hp = hbuf ;
	}

	if ((rs = mailmsg_hdrmatch(op,&hnp,hp,hl)) >= 0) {
	    if (hnp != NULL) {
	        rs = msghdrname_ival(hnp,hi,rpp) ;
	        vl = rs ;
	    }
	}

	if (rs < 0) {
	    if (rpp != NULL) *rpp = NULL ;
	}

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (mailmsg_hdrival) */


int mailmsg_hdrval(MAILMSG *op,cchar *name,cchar **rpp)
{
	MAILMSGHDRNAME	*hnp = NULL ;
	const int	hlen = HDRNAMELEN ;
	int		rs ;
	int		hl = -1 ;
	int		vl = 0 ;
	const char	*hp = name ;
	char		hbuf[HDRNAMELEN + 1] ;

#if	CF_DEBUGS
	debugprintf("mailmsg_hdrval: ent\n") ;
	debugprintf("mailmsg_hdrval: name=%s\n",name) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;

	if (op->magic != MAILMSG_MAGIC) return SR_NOTOPEN ;

	if (hasuc(name,-1)) {
	    hl = strwcpylc(hbuf,name,hlen) - hbuf ;
	    hp = hbuf ;
	}

	if ((rs = mailmsg_hdrmatch(op,&hnp,hp,hl)) >= 0) {
	    if (hnp != NULL) {
	        rs = msghdrname_val(hnp,rpp) ;
	        vl = rs ;
	    }
	}

	
	if (rs < 0) {
	    if (rpp != NULL) *rpp = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("mailmsg_hdrval: ret rs=%d vl=%d\n",rs,vl) ;
	if (rpp != NULL)
	debugprintf("mailmsg_hdrval: ret v=>%t<\n",
		(*rpp),strlinelen((*rpp),vl,40)) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (mailmsg_hdrval) */


/* local subroutines */


static int mailmsg_procline(MAILMSG *op,cchar *lp,int ll)
{
	int		rs = SR_OK ;
	int		vl ;
	int		vi = 0 ;
	const char	*vp ;

#if	CF_DEBUGS
	debugprintf("mailmsg_procline: ent line=>%t<\n",
	    lp,strlinelen(lp,ll,40)) ;
#endif

	if (op->msgstate == msgstate_env) {
	    MAILMSGMATENV	es ;

	    if ((rs = mailmsgmatenv(&es,lp,ll)) > 0) {
	        rs = mailmsg_envadd(op,&es) ;
	    } else if (rs == 0) {
	        op->msgstate = msgstate_hdr ;
	    }
		
	} /* end if */

#if	CF_DEBUGS
	debugprintf("mailmsg_procline: mid2 rs=%d ll=%u\n",rs,ll) ;
#endif

	if ((rs >= 0) && (op->msgstate == msgstate_hdr)) {
	    if ((rs = mailmsgmathdr(lp,ll,&vi)) > 0) {
	        vp = (lp + vi) ;
	        vl = (ll - vi) ;
	        rs = mailmsg_hdraddnew(op,lp,rs,vp,vl) ;
	    } else if ((rs == 0) && (ll > 0) && ISHDRCONT(lp[0])) {
	        rs = mailmsg_hdraddcont(op,(lp+1),(ll-1)) ;
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("mailmsg_procline: ret rs=%d ll=%u\n",rs,ll) ;
#endif

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (mailmsg_procline) */


static int mailmsg_envbegin(MAILMSG *op)
{
	MAILMSG_ENV	*oep = &op->envs ;
	const int	size = sizeof(MAILMSGMATENV) ;
	int		rs ;

#if	CF_PEDANTIC
	memset(oep,0,sizeof(MAILMSG_ENV)) ;
#endif

	rs = vecobj_start(&oep->insts,size,4,0) ;

	return rs ;
}
/* end subroutine (mailmsg_envbegin) */


static int mailmsg_envend(MAILMSG *op)
{
	MAILMSG_ENV	*oep = &op->envs ;
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vecobj_finish(&oep->insts) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (mailmsg_envend) */


static int mailmsg_envadd(MAILMSG *op,MAILMSGMATENV *esp)
{
	MAILMSG_ENV	*oep = &op->envs ;
	int		rs ;

	rs = vecobj_add(&oep->insts,esp) ;

	return rs ;
}
/* end subroutine (mailmsg_envadd) */


static int mailmsg_hdrbegin(MAILMSG *op)
{
	MAILMSG_HDR	*ohp = &op->hdrs ;
	const int	size = sizeof(MAILMSGHDRNAME) ;
	int		rs ;

#if	CF_PEDANTIC
	memset(ohp,0,sizeof(MAILMSG_HDR)) ;
#endif

	ohp->lastname = -1 ;
	rs = vecobj_start(&ohp->names,size,10,0) ;

	return rs ;
}
/* end subroutine (mailmsg_hdrbegin) */


static int mailmsg_hdrend(MAILMSG *op)
{
	MAILMSG_HDR	*ohp = &op->hdrs ;
	MAILMSGHDRNAME	*hnp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_get(&ohp->names,i,&hnp) >= 0 ; i += 1) {
	    if (hnp != NULL) {
	        rs1 = msghdrname_finish(hnp) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	rs1 = vecobj_finish(&ohp->names) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (mailmsg_hdrend) */


static int mailmsg_hdraddnew(MAILMSG *op,cchar *hp,int hl,cchar *vp,int vl)
{
	MAILMSG_HDR	*ohp = &op->hdrs ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("mailmsg_hdraddnew: ent hl=%u h=%t\n",
		hl,hp,strlinelen(hp,hl,40)) ;
	debugprintf("mailmsg_hdraddnew: vl=%u v=>%t<\n",
		vl,vp,strlinelen(vp,vl,40)) ;
#endif

	if (hl != 0) {
	    const int	hlen = HDRNAMELEN ;
	    int		cl ;
	    cchar	*cp ;
	    char	hbuf[HDRNAMELEN + 1] ;

	    if (hasuc(hp,hl)) {
	        if ((hl < 0) || (hl > hlen)) hl = hlen ;
	        strwcpylc(hbuf,hp,hl) ;
	        hp = hbuf ;
	    }

	    if ((cl = sfshrink(vp,vl,&cp)) >= 0) {
	        MAILMSGHDRNAME	*hnp ;
	        if ((rs1 = mailmsg_hdrmatch(op,&hnp,hp,hl)) >= 0) {
	            ohp->lastname = rs1 ;
	            rs = msghdrname_addnew(hnp,cp,cl) ;
	        } else {
		    VECOBJ	*nlp = &ohp->names ;
	            void	*p ;
	            if ((rs = vecobj_addnew(nlp,&p)) >= 0) {
	                MAILMSGHDRNAME	*hnp = (MAILMSGHDRNAME *) p ;
		        const int	i = rs ;
	                if ((rs = msghdrname_start(hnp,hp,hl,cp,cl)) >= 0) {
	                    ohp->lastname = i ;
		        } else {
		            vecobj_del(nlp,i) ;
		        }
	            }
	        } /* end if */
	    } /* end if (shrink) */

	} else
	    ohp->lastname = -1 ;

#if	CF_DEBUGS
	debugprintf("mailmsg_hdraddnew: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mailmsg_hdraddnew) */


static int mailmsg_hdraddcont(MAILMSG *op,cchar *vp,int vl)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("mailmsg_hdraddcont: vl=%u v=>%t<\n",
		vl,vp,strlinelen(vp,vl,40)) ;
#endif

	if (vl > 0) {
	    MAILMSG_HDR	*ohp = &op->hdrs ;
	    int		cl ;
	    cchar	*cp ;
	    if ((cl = sfshrink(vp,vl,&cp)) > 0) {
		const int	ln = ohp->lastname ;
		if (ln >= 0) {
		    MAILMSGHDRNAME	*hnp ;
		    if ((rs = vecobj_get(&ohp->names,ln,&hnp)) >= 0) {
	    	        if (hnp != NULL) {
	        	    rs = msghdrname_addcont(hnp,cp,cl) ;
	    	        }
		    }
		}
	    } /* end if (positive) */
	} /* end if (positive) */

	return rs ;
}
/* end subroutine (mailmsg_hdraddcont) */


static int mailmsg_hdrmatch(MAILMSG *op,MAILMSGHDRNAME **hnpp,cchar *hp,int hl)
{
	MAILMSG_HDR	*ohp = &op->hdrs ;
	const int	rsn = SR_NOTFOUND ;
	int		rs ;
	int		i ;
	int		f = FALSE ;

	if (hl < 0) hl = strlen(hp) ;

#if	CF_DEBUGS
	debugprintf("mailmsg_hdrmatch: ent\n") ;
	debugprintf("mailmsg_hdrmatch: h=>%t<\n",hp,hl) ;
#endif

	*hnpp = NULL ;
	for (i = 0 ; (rs = vecobj_get(&ohp->names,i,hnpp)) >= 0 ; i += 1) {
	    if (*hnpp != NULL) {
	        if ((rs = msghdrname_match(*hnpp,hp,hl)) > 0) {
		    f = TRUE ;
		} else if (rs == rsn) {
		    rs = SR_OK ;
		}
	    }
	    if (f) break ;
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("mailmsg_hdrmatch: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mailmsg_hdrmatch) */


static int msghdrname_start(MAILMSGHDRNAME *hnp,cchar *hp,int hl,
		cchar *vp,int vl)
{
	VECOBJ		*ilp = &hnp->insts ;
	int		rs ;
	int		size ;

#if	CF_PEDANTIC
	memset(hnp,0,sizeof(MAILMSGHDRNAME)) ;
#endif

	hnp->vp = NULL ;
	hnp->vl = 0 ;
	hnp->f_alloc = FALSE ;
	hnp->name = NULL ;
	hnp->namelen = 0 ;
	hnp->lastinst = -1 ;

	size = sizeof(MAILMSGHDRINST) ;
	if ((rs = vecobj_start(ilp,size,2,0)) >= 0) {
	    cchar	*cp ;
	    if ((rs = uc_mallocstrw(hp,hl,&cp)) >= 0) {
	        hnp->name = cp ;
	        hnp->namelen = (rs-1) ;
	        rs = msghdrname_addnew(hnp,vp,vl) ;
		if (rs < 0) {
	    	    uc_free(hnp->name) ;
	    	    hnp->name = NULL ;
	    	    hnp->namelen = 0 ;
		}
	    } /* end if (m-a) */
	    if (rs < 0) {
		vecobj_finish(ilp) ;
	    }
	} /* end if (vecobj_start) */

#if	CF_DEBUGS
	debugprintf("msghdrname_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msghdrname_start) */


static int msghdrname_finish(MAILMSGHDRNAME *hnp)
{
	MAILMSGHDRINST	*hip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if ((hnp->vp != NULL) && hnp->f_alloc) {
	    rs1 = uc_free(hnp->vp) ;
	    if (rs >= 0) rs = rs1 ;
	    hnp->vp = NULL ;
	    hnp->vl = 0 ;
	    hnp->f_alloc = FALSE ;
	}

	if (hnp->name != NULL) {
	    rs1 = uc_free(hnp->name) ;
	    if (rs >= 0) rs = rs1 ;
	    hnp->name = NULL ;
	    hnp->namelen = 0 ;
	}

	for (i = 0 ; vecobj_get(&hnp->insts,i,&hip) >= 0 ; i += 1) {
	    if (hip != NULL) {
	        rs1 = msghdrinst_finish(hip) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	hnp->lastinst = -1 ;
	rs1 = vecobj_finish(&hnp->insts) ;
	if (rs >= 0) rs = rs1 ;

	hnp->vp = NULL ;
	hnp->f_alloc = FALSE ;
	return rs ;
}
/* end subroutine (msghdrname_finish) */


static int msghdrname_match(MAILMSGHDRNAME *hnp,cchar *hp,int hl)
{
	int		rs = SR_OK ;
	int		f ;

	if (hp == NULL) return SR_FAULT ;

	f = (hnp->namelen == hl) ;
	f = f && (hnp->name[0] == hp[0]) ;
	f = f && (strncmp(hnp->name,hp,hl) == 0) ;

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (msghdrname_match) */


static int msghdrname_addnew(MAILMSGHDRNAME *hnp,cchar *vp,int vl)
{
	VECOBJ		*ilp = &hnp->insts ;
	int		rs ;
	int		f = FALSE ;
	void		*p ;

#if	CF_DEBUGS
	debugprintf("msghdrname_addnew: ent v=>%t<\n",
		vp,strlinelen(vp,vl,40)) ;
#endif

	if ((rs = vecobj_addnew(ilp,&p)) >= 0) {
	    MAILMSGHDRINST	*instp = (MAILMSGHDRINST *) p ;
	    const int		i = rs ;
	    if ((rs = msghdrinst_start(instp,vp,vl)) >= 0) {
	        hnp->lastinst = i ;
		f = TRUE ;
	    } else {
		vecobj_del(ilp,i) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("msghdrname_addnew: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (msghdrname_addnew) */


static int msghdrname_addcont(MAILMSGHDRNAME *hnp,cchar *vp,int vl)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("msghdrname_addcont: vl=%d v=>%t<\n",
		vl,vp,strlinelen(vp,vl,40)) ;
#endif

	if (vl > 0) {
	    const char	*cp ;
	    int		cl ;
	    if ((cl = sfshrink(vp,vl,&cp)) > 0) {
	        const int	li = hnp->lastinst ;
		if (li >= 0) {
		    MAILMSGHDRINST	*hip ;
		    if ((rs = vecobj_get(&hnp->insts,li,&hip)) >= 0) {
		        if (hip != NULL) {
	    		    rs = msghdrinst_add(hip,cp,cl) ;
			}
		    }
		}
	    } /* end if (positive) */
	} /* end if (positive) */

	return rs ;
}
/* end subroutine (msghdrname_addcont) */


static int msghdrname_iline(MAILMSGHDRNAME *hnp,int hi,int li,cchar **rpp)
{
	MAILMSGHDRINST	*hip ;
	int		rs ;
	int		vl = 0 ;

#if	CF_DEBUGS
	debugprintf("msghdrname_iline: ent hi=%u li=%u\n",hi,li) ;
#endif

	if ((rs = vecobj_get(&hnp->insts,hi,&hip)) >= 0) {
#if	CF_DEBUGS
	debugprintf("msghdrname_iline: vecobj_get() rs=%d hip{%p}\n",rs,hip) ;
#endif
	    if (hip != NULL) {
	        rs = msghdrinst_ival(hip,li,rpp) ;
	        vl = rs ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("msghdrname_iline: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (msghdrname_iline) */


static int msghdrname_ival(MAILMSGHDRNAME *hnp,int hi,cchar **rpp)
{
	MAILMSGHDRINST	*hip ;
	int		rs ;
	int		vl = 0 ;

	if ((rs = vecobj_get(&hnp->insts,hi,&hip)) >= 0) {
	    if (hip != NULL) {
	        rs = msghdrinst_val(hip,rpp) ;
	        vl = rs ;
	    }
	}

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (msghdrname_ival) */


static int msghdrname_val(MAILMSGHDRNAME *hnp,cchar **rpp)
{
	int		rs = SR_OK ;
	int		vl = hnp->vl ;

#if	CF_DEBUGS
	debugprintf("msghdrname_val: ent\n") ;
#endif

	if (hnp->vp == NULL) {
	    VECOBJ		*ilp = &hnp->insts ;
	    MAILMSGHDRINST	*hip ;
	    int			i ;
	    int			size = 1 ;
	    const char		*hivp = NULL ;
	    char		*bp ;

	    for (i = 0 ; vecobj_get(ilp,i,&hip) >= 0 ; i += 1) {
	        if (hip != NULL) {
	            rs = msghdrinst_val(hip,&hivp) ;
	            size += (rs + 2) ;
		}
	        if (rs < 0) break ;
	    } /* end for */

#if	CF_DEBUGS
	    debugprintf("msghdrname_val: mid1 rs=%d size=%d\n",rs,size) ;
#endif

	    if ((rs >= 0) && ((rs = uc_malloc(size,&bp)) >= 0)) {
	            int		n = 0 ;
		    hnp->vp = bp ;
	            hnp->f_alloc = TRUE ;
	            for (i = 0 ; vecobj_get(ilp,i,&hip) >= 0 ; i += 1) {
	                if (hip != NULL) {
	                    if ((rs = msghdrinst_val(hip,&hivp)) > 0) {
	                        if (n++ > 0) {
	                            *bp++ = ',' ;
	                            *bp++ = ' ' ;
	                        }
	                        bp = strwcpy(bp,hivp,rs) ;
			    } /* end if (positive) */
	                } /* end if (non-null) */
			if (rs < 0) break ;
	            } /* end for */
	            *bp = '\0' ;
	            hnp->vl = (bp - hnp->vp) ;
	    } /* end if (m-a) */

	    vl = hnp->vl ;
	} /* end if (needed) */

	if (rpp != NULL)
	    *rpp = (rs >= 0) ? hnp->vp : NULL ;

#if	CF_DEBUGS
	debugprintf("msghdrname_val: ret rs=%d vl=%d\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (msghdrname_val) */


static int msghdrname_count(MAILMSGHDRNAME *hnp)
{
	int		rs ;

	rs = vecobj_count(&hnp->insts) ;

	return rs ;
}
/* end subroutine (msghdrname_count) */


static int msghdrinst_start(MAILMSGHDRINST *hip,cchar *vp,int vl)
{
	const int	size = sizeof(MAILMSGHDRVAL) ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("msghdrinst_start: ent v=>%t<\n",
		vp,strlinelen(vp,vl,50)) ;
#endif

#if	CF_PEDANTIC
	memset(hip,0,sizeof(MAILMSGHDRINST)) ;
#endif

	hip->vp = NULL ;
	hip->vl = 0 ;
	if ((rs = vecobj_start(&hip->vals,size,4,0)) >= 0) {
	    if ((vl > 0) && (vp != NULL)) {
	        rs = msghdrinst_add(hip,vp,vl) ;
	        if (rs < 0)
		    vecobj_finish(&hip->vals) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("msghdrinst_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msghdrinst_start) */


static int msghdrinst_add(MAILMSGHDRINST *hip,cchar *vp,int vl)
{
	int		rs = SR_OK ;

	if (vl > 0) {
	    VECOBJ	*vlp = &hip->vals ;
	    void	*p ;
	    if ((rs = vecobj_addnew(vlp,&p)) >= 0) {
	        MAILMSGHDRVAL	*valp = (MAILMSGHDRVAL *) p ;
		valp->vp = vp ;
		valp->vl = vl ;
	    }
	}

	return rs ;
}
/* end subroutine (msghdrinst_add) */


static int msghdrinst_ival(MAILMSGHDRINST *hip,int li,cchar **rpp)
{
	MAILMSGHDRVAL	*valp = NULL ;
	int		rs ;
	int		vl = 0 ;

#if	CF_DEBUGS
	debugprintf("mailmsg/msghdrinst_ival: ent li=%u\n",li) ;
#endif

	if ((rs = vecobj_get(&hip->vals,li,&valp)) >= 0) {
	    vl = valp->vl ;
	}

	if (rpp != NULL)
	    *rpp = (rs >= 0) ? valp->vp : NULL ;

#if	CF_DEBUGS
	debugprintf("mailmsg/msghdrinst_ival: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (msghdrinst_ival) */


static int msghdrinst_val(MAILMSGHDRINST *hip,cchar **rpp)
{
	int		rs = SR_OK ;
	int		vl = hip->vl ;

#if	CF_DEBUGS
	debugprintf("msghdrinst_val: ent vp{%p}\n",hip->vp) ;
#endif

	if (hip->vp == NULL) {
	    MAILMSGHDRVAL	*valp ;
	    int			i ;
	    int			size = 1 ;
	    char		*bp ;

	    for (i = 0 ; vecobj_get(&hip->vals,i,&valp) >= 0 ; i += 1) {
	        if (valp != NULL) {
	            size += (valp->vl + 1) ;
		}
	    } /* end for */

#if	CF_DEBUGS
	    debugprintf("msghdrinst_val: mid1 rs=%d size=%d\n",rs,size) ;
#endif

	    if ((rs = uc_malloc(size,&bp)) >= 0) {
		int	n = 0 ;
		hip->vp = bp ;
		hip->f_alloc = TRUE ;
		for (i = 0 ; vecobj_get(&hip->vals,i,&valp) >= 0 ; i += 1) {
		    if (valp != NULL) {
		        if (valp->vl > 0) {
			    if (n++ > 0) *bp++ = ' ' ;
			    bp = strwcpy(bp,valp->vp,valp->vl) ;
			}
		    }
		} /* end for */
		*bp = '\0' ;
		hip->vl = (bp - hip->vp) ;
	    } /* end if (m-a) */

	    vl = hip->vl ;
	} /* end if (needed) */

	if (rpp != NULL)
	    *rpp = (rs >= 0) ? hip->vp : NULL ;

#if	CF_DEBUGS
	debugprintf("msghdrinst_val: ret rs=%d vl=%d\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (msghdrinst_val) */


static int msghdrinst_finish(MAILMSGHDRINST *hip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if ((hip->vp != NULL) && hip->f_alloc) {
	    rs1 = uc_free(hip->vp) ;
	    if (rs >= 0) rs = rs1 ;
	    hip->vp = NULL ;
	    hip->vl = 0 ;
	    hip->f_alloc = FALSE ;
	}

	rs1 = vecobj_finish(&hip->vals) ;
	if (rs >= 0) rs = rs1 ;

	hip->vp = NULL ;
	hip->f_alloc = FALSE ;
	return rs ;
}
/* end subroutine (msghdrinst_finish) */


