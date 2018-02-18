/* expcook */

/* Expand-Cookie - creates the substitution variables for cookie escapes */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGBUF	0		/* debug 'expcook_expbuf()' */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little object is used to set up the substitution variables and to
	do the substitution expansions for string buffers with cookie escapes
	in it.

	Notes:

	The left brace character is HEX '\x7b', the right brace character is
	HEX '\x7d'.  Deal with it!

	Cookies take the form:

	$(<key>)

	Where:

	<key>		is the key to look-up


*******************************************************************************/


#define	EXPCOOK_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<hdbstr.h>
#include	<sbuf.h>
#include	<buffer.h>
#include	<localmisc.h>

#include	"expcook.h"


/* local defines */

#define	KEYBUFLEN	63


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;
extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

int		expcook_expbuf(EXPCOOK *,int,BUFFER *,const char *,int) ;

static int	expcook_prockey(EXPCOOK *,int,BUFFER *,const char *,int) ;

static int	mkcomposite(char *,int,const char *,int,const char *,int) ;

#if	CF_DEBUGS && CF_DEBUGBUF
static int	debugdump(HDBSTR *) ;
#endif


/* local variables */


/* exported subroutines */


int expcook_start(EXPCOOK *ecp)
{
	int		rs ;

	if (ecp == NULL) return SR_FAULT ;

	ecp->magic = 0 ;
	if ((rs = hdbstr_start(&ecp->subs,10)) >= 0) {
	    ecp->magic = EXPCOOK_MAGIC ;
	}

	return rs ;
}
/* end subroutine (expcook_start) */


int expcook_finish(EXPCOOK *ecp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ecp == NULL) return SR_FAULT ;
	if (ecp->magic != EXPCOOK_MAGIC) return SR_NOTOPEN ;

	rs1 = hdbstr_finish(&ecp->subs) ;
	if (rs >= 0) rs = rs1 ;
	ecp->magic = 0 ;

#if	CF_DEBUGS
	debugprintf("expcook_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (expcook_finish) */


int expcook_add(EXPCOOK *ecp,cchar *kbuf,cchar *vbuf,int vlen)
{
	HDBSTR		*slp ;
	int		rs ;
	int		kl ;

	if (ecp == NULL) return SR_FAULT ;
	if (kbuf == NULL) return SR_FAULT ;

	if (ecp->magic != EXPCOOK_MAGIC) return SR_NOTOPEN ;

	slp = &ecp->subs ;
	kl = strlen(kbuf) ;

#if	CF_DEBUGS
	debugprintf("expcook_add: k=%s\n",kbuf) ;
	if (vbuf != NULL)
	    debugprintf("expcook_add: v=>%t<\n",vbuf,strlinelen(vbuf,vlen,40)) ;
#endif

	if ((rs = hdbstr_fetch(slp,kbuf,kl,NULL,NULL)) >= 0) {
	    rs = hdbstr_delkey(slp,kbuf,kl) ;
	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	}

	if ((rs >= 0) && (vbuf != NULL)) {
	    rs = hdbstr_add(slp,kbuf,kl,vbuf,vlen) ;
	}

#if	CF_DEBUGS
	debugprintf("expcook_add: ret rs=%d\n",rs) ;
	debugdump(slp) ;
#endif

	return rs ;
}
/* end subroutine (expcook_add) */


int expcook_curbegin(EXPCOOK *ecp,EXPCOOK_CUR *curp)
{
	int		rs ;

	if (ecp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (ecp->magic != EXPCOOK_MAGIC) return SR_NOTOPEN ;

	rs = hdbstr_curbegin(&ecp->subs,&curp->cur) ;

	return rs ;
}
/* end subroutine (expcook_curbegin) */


int expcook_curend(EXPCOOK *ecp,EXPCOOK_CUR *curp)
{
	int		rs ;

	if (ecp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (ecp->magic != EXPCOOK_MAGIC) return SR_NOTOPEN ;

	rs = hdbstr_curend(&ecp->subs,&curp->cur) ;

	return rs ;
}
/* end subroutine (expcook_curend) */


int expcook_enum(EXPCOOK *ecp,EXPCOOK_CUR *curp,char *rbuf,int rlen)
{
	int		rs ;
	int		vl ;
	int		bl = 0 ;
	const char	*kp, *vp ;

	if (ecp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (ecp->magic != EXPCOOK_MAGIC) return SR_NOTOPEN ;

	if ((rs = hdbstr_enum(&ecp->subs,&curp->cur,&kp,&vp,&vl)) >= 0) {
	    int	kl = rs ;
	    rs = mkcomposite(rbuf,rlen,kp,kl,vp,vl) ;
	    bl = rs ;
	} /* end if */

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (expcook_enum) */


int expcook_findkey(EXPCOOK *ecp,cchar *kp,int kl,cchar **rpp)
{
	int		rs ;

	if (ecp == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (ecp->magic != EXPCOOK_MAGIC) return SR_NOTOPEN ;

	rs = hdbstr_fetch(&ecp->subs,kp,kl,NULL,rpp) ;

	return rs ;
}
/* end subroutine (expcook_findkey) */


int expcook_delkey(EXPCOOK *ecp,cchar *key)
{
	int		rs ;

	if (ecp == NULL) return SR_FAULT ;

	if (ecp->magic != EXPCOOK_MAGIC) return SR_NOTOPEN ;

	rs = hdbstr_delkey(&ecp->subs,key,-1) ;

	return rs ;
}
/* end subroutine (expcook_delkey) */


int expcook_exp(EXPCOOK *ecp,int wch,char *rbuf,int rlen,cchar *sp,int sl)
{
	BUFFER		bo ;
	int		rs ;
	int		rs1 ;
	int		bl = 0 ;
	const char	*bp ;

	if (ecp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (ecp->magic != EXPCOOK_MAGIC) return SR_NOTOPEN ;

	if (sl < 0)
	    sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("expcook_exp: sl=%u s=>%t<\n",
	    sl,sp,strlinelen(sp,sl,50)) ;
	debugprintf("expcook_exp: rlen=%u\n",rlen) ;
#endif

	rbuf[0] = '\0' ;
	if (rlen > 0) {
	    if ((rs = buffer_start(&bo,rlen)) >= 0) {

	        if ((rs = expcook_expbuf(ecp,wch,&bo,sp,sl)) >= 0) {
	            bl = rs ;
	            if ((rs = buffer_get(&bo,&bp)) >= 0) {
	                if (bp != NULL) {
	                    rs = snwcpy(rbuf,rlen,bp,bl) ;
			}
	            }
	        }

	        rs1 = buffer_finish(&bo) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (buffer) */
	} else {
	    rs = SR_OVERFLOW ;
	}

#if	CF_DEBUGS
	debugprintf("expcook_exp: ret rs=%d bl=%u\n",rs,bl) ;
#endif

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (expcook_exp) */


int expcook_expbuf(EXPCOOK *ecp,int wch,BUFFER *bufp,cchar *sp,int sl)
{
	const int	sch = '%' ;
	int		rs = SR_OK ;
	int		kl ;
	int		len = 0 ;
	const char	*ss = "{}" ;
	const char	*kp ;
	const char	*tp ;

	if (ecp == NULL) return SR_FAULT ;
	if (bufp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (ecp->magic != EXPCOOK_MAGIC) return SR_NOTOPEN ;

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS && CF_DEBUGBUF
	debugprintf("expcook_expbuf: wch=%d sl=%u s=>%t<\n",
	    wch,sl,sp,strlinelen(sp,sl,50)) ;
#endif

	while ((tp = strnchr(sp,sl,sch)) != NULL) {

#if	CF_DEBUGS && CF_DEBUGBUF
	    debugprintf("expcook_expbuf: cycle si=%d part=>%t<\n",
	        (tp-sp),sp,(tp-sp)) ;
#endif

	    if ((rs = buffer_strw(bufp,sp,(tp-sp))) >= 0) {
		len += rs ;
	        kp = NULL ;
	        kl = -1 ;

	        sl -= ((tp+1)-sp) ;
	        sp = (tp+1) ;
	        if (sl > 0) {
	            if (sp[0] == sch) {
	                rs = buffer_char(bufp,sch) ;
		  	len += rs ;
	                sl -= 1 ;
	                sp += 1 ;
	            } else if (sp[0] == ss[0]) {
	                sl -= 1 ;
	                sp += 1 ;
	                if ((tp = strnchr(sp,sl,ss[1])) != NULL) {
	                    kp = sp ;
	                    kl = (tp-sp) ;
	                    sl -= ((tp+1)-sp) ;
	                    sp = (tp+1) ;
	                } else {
	                    kp = sp ;
	                    kl = 0 ;
	                    sl -= ((tp+1)-sp) ;
	                    sp = (tp+1) ;
	                } /* end if */
	            } else {
	                kp = sp ;
	                kl = 1 ;
	                sl -= 1 ;
	                sp += 1 ;
	            } /* end if (extracting the key) */
	        } else {
		    kp = sp ;
		    kl = 0 ;
		}

	        if ((rs >= 0) && (kp != NULL)) {
		    rs = expcook_prockey(ecp,wch,bufp,kp,kl) ;
		    len += rs ;
		}

	    } /* end if (buf-add leading part) */

	    if (rs < 0) break ;
	} /* end while (expanding) */

/* copy over any remainder (trailing part) */

	if ((rs >= 0) && (sl > 0)) {
	    rs = buffer_strw(bufp,sp,sl) ;
	    len += rs ;
	}

#if	CF_DEBUGS && CF_DEBUGBUF
	debugprintf("expcook_expbuf: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (expcook_expbuf) */


#if	CF_DEBUGS && CF_DEBUGBUF
int expcook_debugdump(EXPCOOK *ecp)
{
	HDBSTR		*slp ;
	int		rs ;
	if (ecp == NULL) return SR_FAULT ;
	if (ecp->magic != EXPCOOK_MAGIC) return SR_NOTOPEN ;
	slp = &ecp->subs ;
	rs = debugdump(slp) ;
	return rs ;
}
/* end subroutine (expcook_debugdump) */
#endif /* CF_DEBUGS && CF_DEBUGBUF */


/* private subroutines */


/* careful: we are *supposed* to return the accumulated length */
static int expcook_prockey(EXPCOOK *op,int wch,BUFFER *bufp,cchar *kp,int kl)
{
	HDBSTR		*slp = &op->subs ;
	int		rs = SR_OK ;
	int		vl = SR_NOTFOUND ;
	const char	*vp ;

	if (kl < 0) kl = strlen(kp) ;

#if	CF_DEBUGS && CF_DEBUGBUF
	debugprintf("expcook_prockey: kl=%d k=%t\n",kl,kp,kl) ;
	debugdump(slp) ;
#endif

	if (kl > 0) {
	    vl = hdbstr_fetch(slp,kp,kl,NULL,&vp) ;
	}

#if	CF_DEBUGS && CF_DEBUGBUF
	debugprintf("expcook_prockey: lookup rs=%d\n",vl) ;
#endif

	if (vl >= 0) {
	    if (vl > 0) {
	        rs = buffer_strw(bufp,vp,vl) ;
	    }
	} else if (vl == SR_NOTFOUND) {
	    if (wch < 0) {
	        rs = SR_NOTFOUND ;
	    } else if (wch > 0) {
		int	len ;
	        if ((rs = buffer_char(bufp,wch)) >= 0) {
		    len = rs ;
	            rs = buffer_strw(bufp,kp,kl) ;
		    len += rs ;
		}
		if (rs >= 0) {
	            rs = buffer_char(bufp,wch) ;
		    len += rs ;
		}
		if (rs >= 0)
		    rs = len ;
	    } /* end if (specified delimiter) */
	} else {
	    rs = vl ;
	}

	return rs ;
}
/* end subroutine (expcook_prockey) */


static int mkcomposite(char rbuf[],int rlen,cchar *kp,int kl,cchar *vp,int vl)
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {

	    sbuf_strw(&b,kp,kl) ;
	    sbuf_char(&b,'=') ;
	    if (vp != NULL) sbuf_strw(&b,vp,vl) ;

	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return rs ;
}
/* end subroutine (mkcomposite) */


#if	CF_DEBUGS && CF_DEBUGBUF
static int debugdump(HDBSTR *slp)
{
	HDBSTR_CUR	cur ;
	int		rs ;
	int		c = 0 ;
	debugprintf("debugdump: ent\n") ;
	if ((rs = hdbstr_curbegin(slp,&cur)) >= 0) {
	    int		kl ;
	    int		vl ;
	    const char	*kbuf ;
	    const char	*vbuf ;
	    while ((kl = hdbstr_enum(slp,&cur,&kbuf,&vbuf,&vl)) >= 0) {
		c += 1 ;
	        debugprintf("debugdump: kl=%d k=%s\n",kl,kbuf) ;
	        debugprintf("debugdump: vl=%d v=%s\n",vl,vbuf) ;
	    }
	    hdbstr_curend(slp,&cur) ;
	} /* end if (cursor) */
	debugprintf("debugdump: ret rs=%d c=%u\n",rs,c) ;
	return rs ;
}
/* end subroutine (debugdump) */
#endif /* CF_DEBUGS && CF_DEBUGBUF */


