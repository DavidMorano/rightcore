/* qpdecoder */
/* lang=C++98 */

/* Quoted-Printable (QP) decoding */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2016-06-29, David A­D­ Morano
	This was really made from scratch.

*/

/* Copyright © 2016 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We facilitate the decoding of Quoted-Printable (QP) encoded data.

		qpdecoder_start
		qpdecoder_load
		qpdecoder_read
		qpdecoder_finish


*******************************************************************************/


#define	QPDECODER_MASTER	0	/* necessary for proper symbol names */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<string.h>

#include	<vector>
#include	<new>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>

#include	"qpdecoder.h"
#include	"obuf.hh"


/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(ch)	((ch) & UCHAR_MAX)
#endif


/* namespaces */

using namespace		std ;		/* yes, we want punishment! */


/* external subroutines */

extern "C" int	sichr(cchar *,int,int) ;
extern "C" int	nextfield(cchar *,int,cchar **) ;
extern "C" int	ifloor(int,int) ;
extern "C" int	hexval(int) ;
extern "C" int	isprintlatin(int) ;
extern "C" int	ishexlatin(int) ;

#if	CF_DEBUGS
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	debugprinthexblock(cchar *,int,cchar *,int) ;
extern "C" int	strlinelen(const char *,int,int) ;
#endif

extern "C" char	*strwcpy(char *,cchar *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	qpdecoder_loadspace(QPDECODER *,cchar *,int) ;
static int	qpdecoder_add(QPDECODER *,cchar *,int) ;
static int	qpdecoder_add(QPDECODER *,char) ;
static int	qpdecoder_cvt(QPDECODER *) ;


/* local variables */


/* exported subroutines */


int qpdecoder_start(QPDECODER *op,int f_space)
{
	int		rs = SR_OK ;
	obuf		*obp ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(QPDECODER)) ;
	op->f.space = f_space ;

#if	CF_DEBUGS
	debugprintf("qpdecoder_start: f_space=%u\n",f_space) ;
#endif

	if ((obp = new(nothrow) obuf) != NULL) {
	    op->outbuf = (void *) obp ;
	    op->magic = QPDECODER_MAGIC ;
	} else
	    rs = SR_NOMEM ;

	return rs ;
}
/* end subroutine (qpdecoder_start) */


int qpdecoder_finish(QPDECODER *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != QPDECODER_MAGIC) return SR_NOTOPEN ;

	if (op->outbuf != NULL) {
	    obuf *obp = (obuf *) op->outbuf ;
	    delete obp ;
	    op->outbuf = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("qpdecoder_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (qpdecoder_finish) */


int qpdecoder_load(QPDECODER *op,cchar *sp,int sl)
{
	obuf		*obp ;
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("qpdecoder_load: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != QPDECODER_MAGIC) return SR_NOTOPEN ;

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("qpdecoder_load: s=>%t<\n",
	    sp,strlinelen(sp,sl,50)) ;
#endif

	if ((obp = ((obuf *) op->outbuf)) != NULL) {
	    if (op->f.space) {
	        rs = qpdecoder_loadspace(op,sp,sl) ;
	        c += rs ;
	    } else {
	        const int	nl = 2 ;
	        while ((rs >= 0) && (sl > 0)) {
#if	CF_DEBUGS
	            debugprintf("qpdecoder_load: f_esc=%u\n",op->f.esc) ;
#endif
	            if (op->f.esc) {
	                int		ml = MIN(sl,(nl-op->rl)) ;
	                const int	rl = op->rl ;
	                char	*rb = op->rb ;
	                strwcpy((rb+rl),sp,ml) ;
	                op->rl += ml ;
	                sp += ml ;
	                sl -= ml ;
	                if (rl == nl) {
	                    rs = qpdecoder_cvt(op) ;
	                    c += rs ;
	                    op->rl = 0 ;
	                    op->f.esc = FALSE ;
	                }
	            } else {
	                int		si ;
	                while ((si = sichr(sp,sl,'=')) >= 0) {
	                    op->f.esc = TRUE ;
	                    if (si > 0) {
	                        rs = qpdecoder_add(op,sp,si) ;
	                        c += rs ;
	                        sp += si ;
	                        sl -= si ;
	                    }
	                    sp += 1 ;
	                    sl -= 1 ;
	                    if (sl > 0) {
	                        int		ml = MIN(sl,(nl-op->rl)) ;
	                        const int	rl = op->rl ;
	                        char	*rb = op->rb ;
	                        strwcpy((rb+rl),sp,ml) ;
	                        op->rl += ml ;
	                        sp += ml ;
	                        sl -= ml ;
	                        if (op->rl == nl) {
	                            rs = qpdecoder_cvt(op) ;
	                            c += rs ;
	                            op->rl = 0 ;
	                            op->f.esc = FALSE ;
	                        }
	                    } /* end if */
	                } /* end while */
	                if ((rs >= 0) && (sl > 0)) {
	                    rs = qpdecoder_add(op,sp,sl) ;
	                    c += rs ;
	                    sl = 0 ;
	                } /* end if (remaining source) */
	                if (rs < 0) break ;
	            } /* end if (escape or not) */
	        } /* end while */
	    } /* end if (space or not) */
	} else
	    rs = SR_BUGCHECK ;

#if	CF_DEBUGS
	debugprintf("qpdecoder_load: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (qpdecoder_load) */


int qpdecoder_read(QPDECODER *op,char *rbuf,int rlen)
{
	obuf		*obp ;
	int		rs = SR_OK ;
	int		i = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != QPDECODER_MAGIC) return SR_NOTOPEN ;

	if (rlen < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("qpdecoder_read: ent rlen=%d\n",rlen) ;
#endif

	rbuf[0] = '\0' ;
	if (rlen > 0) {
	    if ((obp = ((obuf *) op->outbuf)) != NULL) {
	        const int	len = obp->len() ;
	        int		ml ;
	        ml = MIN(len,rlen) ;
	        for (i = 0 ; i < ml ; i += 1) {
	            rbuf[i] = obp->at(i) ;
	        }
	        rbuf[i] = '\0' ;
	        rs = obp->adv(i) ;
	    } else
	        rs = SR_BUGCHECK ;
	} /* end if (positive) */

#if	CF_DEBUGS
	debugprintf("qpdecoder_read: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (qpdecoder_read) */


/* private subroutines */


/* load source while ignoring spaces (for mail headers) */
static int qpdecoder_loadspace(QPDECODER *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	while ((rs >= 0) && (sl-- > 0)) {
	    const int	ch = MKCHAR(*sp) ;
	    if (! CHAR_ISWHITE(ch)) {
	        if (op->f.esc) {
	            op->rb[op->rl++] = ch ;
	            if (op->rl == 2) {
	                rs = qpdecoder_cvt(op) ;
	                c += rs ;
	                op->rl = 0 ;
	                op->f.esc = FALSE ;
	            }
	        } else if (ch == '=') {
	            op->f.esc = TRUE ;
	        } else {
	            rs = qpdecoder_add(op,ch) ;
	            c += rs ;
	        }
	    } /* end if (not white-space) */
	    sp += 1 ;
	} /* end while */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (qpdecoder_readspace) */


static int qpdecoder_add(QPDECODER *op,cchar *vp,int vl = -1)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	while ((rs >= 0) && vl-- && *vp) {
	    rs = qpdecoder_add(op,*vp++) ;
	    c += rs ;
	}
	return c ;
}
/* end subrolutine (qpdecoder_add) */


static int qpdecoder_add(QPDECODER *op,char v)
{
	obuf 	*obp = (obuf *) op->outbuf ;
	if (op->f.space && (v == '_')) v = ' ' ;
#if	CF_DEBUGS
	debugprintf("qpdecoder_add: v=%02x\n",v) ;
#endif
	obp->add(v) ;
	return 1 ;
}
/* end subrolutine (qpdecoder_add) */


static int qpdecoder_cvt(QPDECODER *op)
{
	int		rs = SR_OK ;
	char		*rb = op->rb ;
	if (op->outbuf != NULL) {
	    obuf 	*obp = (obuf *) op->outbuf ;
	    int		v = 0 ;
	    int		ch0 = MKCHAR(rb[0]) ;
	    int		ch1 = MKCHAR(rb[1]) ;
	    if (ishexlatin(ch0) && ishexlatin(ch1)) {
		v |= (hexval(ch0)<<4) ;
		v |= (hexval(ch1)<<0) ;
	    } else {
	        v = '¿' ;
	    } 
	    obp->add(v) ;
	} else
	    rs = SR_BUGCHECK ;
#if	CF_DEBUGS
	debugprintf("qpdecoder_cvt: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? 1 : rs ;
}
/* end subroutine (qpdecoder_cvt) */


