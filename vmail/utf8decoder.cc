/* utf8decoder */
/* lang=C++98 */

/* UTF-8 decoding */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2016-06-29, David A­D­ Morano
	This was really made from scratch.

*/

/* Copyright © 2016 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We facilitate the decoding of UTF-8 byte streams into UNICODE®
        characters.

		utf8decoder_start
		utf8decoder_load
		utf8decoder_read
		utf8decoder_finish


*******************************************************************************/


#define	UTF8DECODER_MASTER	0	/* necessary for proper symbol names */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<string.h>

#include	<vector>
#include	<new>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"utf8decoder.h"


/* local defines */


/* namespaces */

using namespace		std ;		/* yes, we want punishment! */


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */

class widebuf {
	std::vector<int>	b ;
	int			oi ;		/* output index */
public:
	widebuf() : oi(0) { 
	} ;
	widebuf(cchar *sbuf) : oi(0) { 
	    int	i ;
	    for (i = 0 ; sbuf[i] ; i += 1) {
		b.push_back(sbuf[i]) ;
	    }
	} ;
	widebuf(cchar *sbuf,int slen) : oi(0) {
	    int	i ;
	    if (slen < 0) slen = strlen(sbuf) ;
	    for (i = 0 ; sbuf[i] ; i += 1) {
		b.push_back(sbuf[i]) ;
	    }
	} ;
	int operator [] (int i) const {
	    const int	n = b.size() ;
	    int		rch = 0 ;
	    if ((oi+i) < n) rch = b[oi+i] ;
	    return rch ;
	} ;
	widebuf &operator += (int ch) {
	    b.push_back(ch) ;
	    return *this ;
	} ;
	int add(int ch) {
	    b.push_back(ch) ;
	    return (b.size() - oi) ;
	} ;
	int add(cchar *sp,int sl = -1) {
	    int	i ;
	    if (sl < 0) sl = strlen(sp) ;
	    for (i = 0 ; i < sl ; i += 1) {
		b.push_back(sp[i]) ;
	    }
	    return (b.size() - oi) ;
	} ;
	int count() const {
	    return (b.size() - oi) ;
	} ;
	int len() const {
	    return (b.size() - oi) ;
	} ;
	int at(int i) const {
	    const int	n = b.size() ;
	    int		rch = 0 ;
	    if ((oi+i) < n) rch = b[oi+i] ;
	    return rch ;
	} ;
	int adv(int al) ;
} ; /* end structure (widebuf) */


/* forward references */


/* local variables */


/* exported subroutines */


int utf8decoder_start(UTF8DECODER *op)
{
	int		rs = SR_OK ;
	widebuf		*wbp ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(UTF8DECODER)) ;

	if ((wbp = new(nothrow) widebuf) != NULL) {
	    op->outbuf = (void *) wbp ;
	    op->magic = UTF8DECODER_MAGIC ;
	} else {
	    rs = SR_NOMEM ;
	}

	return rs ;
}
/* end subroutine (utf8decoder_start) */


int utf8decoder_finish(UTF8DECODER *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UTF8DECODER_MAGIC) return SR_NOTOPEN ;

	if (op->outbuf != NULL) {
	    widebuf *wbp = (widebuf *) op->outbuf ;
	    delete wbp ;
	    op->outbuf = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("utf8decoder_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (utf8decoder_finish) */


int utf8decoder_load(UTF8DECODER *op,cchar *sp,int sl)
{
	widebuf		*wbp ;
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("utf8decoder_load: ent sl=%d\n",sl) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != UTF8DECODER_MAGIC) return SR_NOTOPEN ;

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("utf8decoder_load: ent\n") ;
#endif

	if ((wbp = ((widebuf *) op->outbuf)) != NULL) {
	    while (sl-- > 0) {
		const uint	uch = *sp++ ;
#if	CF_DEBUGS
		debugprintf("utf8decoder_load: sch=%08ß\n",uch) ;
#endif
		if ((uch & 0x80) == 0) {
	            wbp->add(uch) ;
		    c += 1 ;
		} else {
		    if ((uch & 0xE0) == 0xC0) {
		        op->rem = 1 ;
		        op->code = ((uch & 0x1F) << 6) ;
		    } else if ((uch & 0xF0) == 0xE0) {
		        op->rem = 2 ;
		        op->code = ((uch & 0x0F) << 12) ;
		    } else if ((uch & 0xF8) == 0xF0) {
		        op->rem = 3 ;
		        op->code = ((uch & 0x07) << 18) ;
		    } else if ((uch & 0xC0) == 0x80) {
		        if (op->rem > 0) {
			    op->rem -= 1 ;
			    op->code |= ((uch & 0x3F) << (op->rem*6)) ;
			    if (op->rem == 0) {
	            	        wbp->add(op->code) ;
		    	        c += 1 ;
		            }
		        } /* end if (process continuation portion) */
		    } /* end if (multi type) */
		} /* end if (single or multi) */
	   } /* end while */
	} else {
	    rs = SR_BUGCHECK ;
	}

#if	CF_DEBUGS
	debugprintf("utf8decoder_load: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (utf8decoder_load) */


int utf8decoder_read(UTF8DECODER *op,wchar_t *rbuf,int rlen)
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != UTF8DECODER_MAGIC) return SR_NOTOPEN ;

	if (rlen < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("utf8decoder_read: ent rlen=%d\n",rlen) ;
#endif

	rbuf[0] = '\0' ;
	if (rlen > 0) {
	    widebuf	*wbp ;
	    if ((wbp = ((widebuf *) op->outbuf)) != NULL) {
	        const int	len = wbp->len() ;
	        int		ml ;
	        ml = MIN(len,rlen) ;
	        for (i = 0 ; i < ml ; i += 1) {
		    rbuf[i] = wbp->at(i) ;
	        }
	        rbuf[i] = '\0' ;
	        rs = wbp->adv(i) ;
	    } else {
	        rs = SR_BUGCHECK ;
	    }
	} /* end if (positive) */

#if	CF_DEBUGS
	debugprintf("utf8decoder_read: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (utf8decoder_read) */


/* private subroutines */


int widebuf::adv(int al) 
{
	const int	sl = b.size() ;
	int		rl = 0 ;
	if (al > 0) {
	    if (sl > (oi+al)) {
		rl = (sl - oi) ;
		oi += rl ;
	    } else {
		rl = (sl - oi) ;
		oi += rl ;
		if (rl == 0) {
		    b.clear() ;
		    oi = 0 ;
		}
	    }
	} else if (al < 0) {
	    if (sl > oi) {
		rl = (sl - oi) ;
		oi += rl ;
	    } else {
		b.clear() ;
		oi = 0 ;
	    }
	}
	return rl ;
}
/* end subroutine (widebuf::adv) */


