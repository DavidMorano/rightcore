/* serialbuf */

/* serializing sbuffer object handling */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_STDORDER	1		/* use STDORDER? */


/* revision history:

	= 1998-11-04, David A­D­ Morano
	This object was first written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object provides a means by which data units can be serialized into
        a sbuffer and unserialized back again. The serialization into the
        sbuffer is done in a portable way using Network Byte Order.

        This whole function is similar to XDR in general but this object allows
        the details of the serial sbuffer to be known rather than opaque as in
        XDR!


*******************************************************************************/


#define	SERIALBUF_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<stdorder.h>
#include	<localmisc.h>

#include	"serialbuf.h"


/* extern subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */


/* exported subroutines */


int serialbuf_start(SERIALBUF *sbp,char *sbuf,int slen)
{

	sbp->bp = sbuf ;
	sbp->len = slen ;
	sbp->i = 0 ;
	return SR_OK ;
}
/* end subroutine (serialbuf_start) */


int serialbuf_finish(SERIALBUF *sbp)
{

	sbp->bp = NULL ;
	return sbp->i ;
}
/* end subroutine (serialbuf_finish) */


int serialbuf_getlen(SERIALBUF *sbp)
{

	return sbp->i ;
}
/* end subroutine (serialbuf_getlen) */


int serialbuf_robj(SERIALBUF *sbp,void *sbuf,int slen)
{

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= slen) {
	        memcpy(sbuf,sbp->bp,slen) ;
	        sbp->bp += slen ;
	        sbp->i += slen ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_robj) */


int serialbuf_wobj(SERIALBUF *sbp,const void *sbuf,int slen)
{

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= slen) {
	        memcpy(sbp->bp,sbuf,slen) ;
	        sbp->bp += slen ;
	        sbp->i += slen ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_wobj) */


/* "advance" the sbuffer as if we wrote something in there */
int serialbuf_adv(SERIALBUF *sbp,int size)
{

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_adv) */


int serialbuf_rchar(SERIALBUF *sbp,char *rp)
{
	const int	size = sizeof(char) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
	        *rp = *sbp->bp++ ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_rchar) */


int serialbuf_rshort(SERIALBUF *sbp,short *rp)
{
	const int	size = sizeof(ushort) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_rshort(sbp->bp,rp) ;
#else
	        netorder_rshort(sbp->bp,rp) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_rshort) */


int serialbuf_rint(SERIALBUF *sbp,int *rp)
{
	const int	size = sizeof(int) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_rint(sbp->bp,rp) ;
#else
	        netorder_rint(sbp->bp,rp) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_rint) */


int serialbuf_rinta(SERIALBUF *sbp,int *rp,int n)
{
	const int	size = sizeof(int) ;
	int		i ;

	for (i = 0 ; (sbp->i >= 0) && (i < n) ; i += 1) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_rint(sbp->bp,(rp + i)) ;
#else
	        netorder_rint(sbp->bp,(rp + i)) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	} /* end for */

	return sbp->i ;
}
/* end subroutine (serialbuf_rinta) */


int serialbuf_rlong(SERIALBUF *sbp,long *rp)
{
	const int	size = sizeof(long) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_rlong(sbp->bp,rp) ;
#else
	        netorder_rlong(sbp->bp,rp) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_rlong) */


int serialbuf_rlonga(SERIALBUF *sbp,long *rp,int n)
{
	const int	size = sizeof(long) ;
	int		i ;

	for (i = 0 ; (sbp->i >= 0) && (i < n) ; i += 1) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_rlong(sbp->bp,(rp + i)) ;
#else
	        netorder_rlong(sbp->bp,(rp + i)) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	} /* end for */

	return sbp->i ;
}
/* end subroutine (serialbuf_rlonga) */


int serialbuf_rll(SERIALBUF *sbp,LONG *rp)
{
	const int	size = sizeof(LONG) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_rll(sbp->bp,rp) ;
#else
	        netorder_rll(sbp->bp,rp) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_rll) */


/* read a fixed length string (possibly not NUL-terminated) */
int serialbuf_rstrn(SERIALBUF *sbp,char *sbuf,int slen)
{

#if	CF_DEBUGS
	int		rs1 ;
	char		*cp ;
#endif

#if	CF_DEBUGS
	debugprintf("serialbuf_rstrn: len=%d i=%d sbuf=%p slen=%d\n",
	    sbp->len,sbp->i,sbuf,slen) ;
#endif

	if (slen < 0) return SR_INVALID ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= slen) {
#if	CF_DEBUGS
	        cp = strwcpy(sbuf,sbp->bp,slen) ;
	        debugprintf("serialbuf_rstrn: sbuf=%p cp=%p diff=%d\n",
	            sbuf,cp,(cp - sbuf)) ;
#else
	        strwcpy(sbuf,sbp->bp,slen) ;
#endif
	        sbp->bp += slen ;
	        sbp->i += slen ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_rstrn) */


/* read a NUL-terminated variable length string */
int serialbuf_rstrw(SERIALBUF *sbp,char *sbuf,int slen)
{

	if (sbp->i >= 0) {
	    int		sl ;
	    int		cl, rl ;
	    int		i ;
	    char		*cp ;

	    if (slen >= 0) {

#ifdef	COMMENT
	        sl = strlcpy(sbuf,sbp->bp,(size_t) (slen + 1)) ;
	        sbp->bp += (sl + 1) ;
	        if (sl > slen) {
	            sbp->i = SR_TOOBIG ;
	        } else
	            sbp->i += (sl + 1) ;
#else /* COMMENT */

	        i = 0 ;
	        cp = sbp->bp + (sbp->len - sbp->i) ;
	        while ((i < slen) && (sbp->bp < cp) && 
	            (sbp->bp[0] != '\0')) {

	            sbuf[i] = *sbp->bp++ ;
	            i += 1 ;

	        } /* end while */

	        sbuf[i] = '\0' ;
	        if ((i <= slen) && (sbp->bp == cp)) {

	            sbp->i = SR_TOOBIG ;

	        } else if ((i == slen) && (sbp->bp[0] != '\0')) {

	            rl = strlen((char *) sbp->bp) ;

	            sbp->bp += (rl + 1) ;
	            sbp->i = SR_TOOBIG ;

	        } else {

	            sbp->bp += 1 ;
	            sbp->i += (i + 1) ;

	        } /* end if */

#endif /* COMMENT */

	    } else {

	        cl = sbp->len - sbp->i ;
	        sl = strwcpy(sbuf,sbp->bp,cl) - sbuf ;

	        if (sl == cl) {
	            sbp->bp += sl ;
	            sbp->i = SR_TOOBIG ;
	        } else {
	            sbp->bp += (sl + 1) ;
	            sbp->i += (sl + 1) ;
	        }

	    } /* end if */

	} /* end if (ok) */

	return sbp->i ;
}
/* end subroutine (serialbuf_rstrw) */


int serialbuf_rbuf(SERIALBUF *sbp,char *sbuf,int slen)
{

	if (slen < 0) return SR_INVALID ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= slen) {
	        memcpy(sbuf,sbp->bp,slen) ;
	        sbp->bp += slen ;
	        sbp->i += slen ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_rbuf) */


int serialbuf_ruchar(SERIALBUF *sbp,uchar *rp)
{
	const int	size = sizeof(uchar) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
	        *rp = (uchar) *sbp->bp++ ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_ruchar) */


int serialbuf_rushort(SERIALBUF *sbp,ushort *rp)
{
	const int	size = sizeof(ushort) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_rushort(sbp->bp,rp) ;
#else
	        netorder_rushort(sbp->bp,rp) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_rushort) */


int serialbuf_ruint(SERIALBUF *sbp,uint *rp)
{
	const int	size = sizeof(uint) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_ruint(sbp->bp,rp) ;
#else
	        netorder_ruint(sbp->bp,rp) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_ruint) */


int serialbuf_ruinta(SERIALBUF *sbp,uint *rp,int n)
{
	const int	size = sizeof(uint) ;
	int		i ;

	for (i = 0 ; (sbp->i >= 0) && (i < n) ; i += 1) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_ruint(sbp->bp,(rp + i)) ;
#else
	        netorder_ruint(sbp->bp,(rp + i)) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	} /* end for */

	return sbp->i ;
}
/* end subroutine (serialbuf_ruinta) */


int serialbuf_rulong(SERIALBUF *sbp,ulong *rp)
{
	const int	size = sizeof(ulong) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_rulong(sbp->bp,rp) ;
#else
	        netorder_rulong(sbp->bp,rp) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_rulong) */


int serialbuf_rulonga(SERIALBUF *sbp,ulong *rp,int n)
{
	const int	size = sizeof(ulong) ;
	int		i ;

	for (i = 0 ; (sbp->i >= 0) && (i < n) ; i += 1) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_rulong(sbp->bp,(rp + i)) ;
#else
	        netorder_rulong(sbp->bp,(rp + i)) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	} /* end for */

	return sbp->i ;
}
/* end subroutine (serialbuf_rulonga) */


int serialbuf_rull(SERIALBUF *sbp,ULONG *rp)
{
	const int	size = sizeof(ULONG) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_rull(sbp->bp,rp) ;
#else
	        netorder_rull(sbp->bp,rp) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_rull) */


int serialbuf_rustrn(SERIALBUF *sbp,uchar *usbuf,int slen)
{
	char		*sbuf = (char *) usbuf ;

	if (slen < 0) return SR_INVALID ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= slen) {
	        strwcpy(sbuf,sbp->bp,slen) ;
	        sbp->bp += slen ;
	        sbp->i += slen ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_rustrn) */


int serialbuf_rustrw(SERIALBUF *sbp,uchar *usbuf,int slen)
{

	if (sbp->i >= 0) {
	    int		sl ;
	    int		cl, rl ;
	    int		i ;
	    char	*cp ;
	    char	*sbuf = (char *) usbuf ;

	    if (slen >= 0) {

#ifdef	COMMENT
	        sl = strlcpy(sbuf,sbp->bp,(size_t) (slen + 1)) ;
	        sbp->bp += (sl + 1) ;
	        if (sl > slen) {
	            sbp->i = SR_TOOBIG ;
	        } else
	            sbp->i += (sl + 1) ;
#else /* COMMENT */

	        i = 0 ;
	        cp = sbp->bp + (sbp->len - sbp->i) ;
	        while ((i < slen) && (sbp->bp < cp) && (sbp->bp[0] != '\0')) {
	            sbuf[i] = *sbp->bp++ ;
	            i += 1 ;
	        }

	        sbuf[i] = '\0' ;
	        if ((i <= slen) && (sbp->bp == cp)) {
	            sbp->i = SR_TOOBIG ;
	        } else if ((i == slen) && (sbp->bp[0] != '\0')) {
	            rl = strlen((char *) sbp->bp) ;
	            sbp->bp += (rl + 1) ;
	            sbp->i = SR_TOOBIG ;
	        } else {
	            sbp->bp += 1 ;
	            sbp->i += (i + 1) ;
	        }

#endif /* COMMENT */

	    } else {

	        cl = sbp->len - sbp->i ;
	        sl = strwcpy(sbuf,sbp->bp,cl) - sbuf ;

	        if (sl == cl) {
	            sbp->bp += sl ;
	            sbp->i = SR_TOOBIG ;
	        } else {
	            sbp->bp += (sl + 1) ;
	            sbp->i += (sl + 1) ;
	        }

	    } /* end if */

	} /* end if (ok) */

	return sbp->i ;
}
/* end subroutine (serialbuf_rustrw) */


int serialbuf_rubuf(SERIALBUF *sbp,uchar *sbuf,int slen)
{

	if (slen < 0) return SR_INVALID ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= slen) {
	        memcpy(sbuf,sbp->bp,slen) ;
	        sbp->bp += slen ;
	        sbp->i += slen ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_rubuf) */


int serialbuf_wchar(SERIALBUF *sbp,int ch)
{
	const int	size = sizeof(char) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
	        *sbp->bp++ = (char) ch ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_wchar) */


int serialbuf_wshort(SERIALBUF *sbp,int sw)
{
	const int	size = sizeof(short) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_wshort(sbp->bp,sw) ;
#else
	        netorder_wshort(sbp->bp,sw) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_wshort) */


int serialbuf_wint(SERIALBUF *sbp,int iw)
{
	const int	size = sizeof(int) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_wint(sbp->bp,iw) ;
#else
	        netorder_wint(sbp->bp,iw) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_wint) */


int serialbuf_winta(SERIALBUF *sbp,int *iwa,int n)
{
	const int	size = sizeof(int) ;
	int		i ;

	for (i = 0 ; (sbp->i >= 0) && (i < n) ; i += 1) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_wint(sbp->bp,iwa[i]) ;
#else
	        netorder_wint(sbp->bp,iwa[i]) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	} /* end for */

	return sbp->i ;
}
/* end subroutine (serialbuf_winta) */


int serialbuf_wlong(SERIALBUF *sbp,long lw)
{
	const int	size = sizeof(long) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_wlong(sbp->bp,lw) ;
#else
	        netorder_wlong(sbp->bp,lw) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_wlong) */


int serialbuf_wlonga(SERIALBUF *sbp,long *lwa,int n)
{
	const int	size = sizeof(long) ;
	int		i ;

	for (i = 0 ; (sbp->i >= 0) && (i < n) ; i += 1) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_wlong(sbp->bp,lwa[i]) ;
#else
	        netorder_wlong(sbp->bp,lwa[i]) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	} /* end for */

	return sbp->i ;
}
/* end subroutine (serialbuf_wlonga) */


int serialbuf_wll(SERIALBUF *sbp,LONG lw)
{
	const int	size = sizeof(LONG) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_wll(sbp->bp,lw) ;
#else
	        netorder_wll(sbp->bp,lw) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_wll) */


/* write a fixed length string (possibly not NUL-terminated) */
int serialbuf_wstrn(SERIALBUF *sbp,cchar *s,int slen)
{

	if (slen < 0) slen = strlen(s) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= slen) {
	        strncpy(sbp->bp,s,slen) ;
	        sbp->bp += slen ;
	        sbp->i += slen ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_wstrn) */


/* write a NUL-terminated variable length string */
int serialbuf_wstrw(SERIALBUF *sbp,cchar *sbuf,int slen)
{

	slen = strnlen(sbuf,slen) ;

	if (sbp->i >= 0) {
	    int		cl ;
	    if ((sbp->len - sbp->i) >= (slen + 1)) {
	        cl = strwcpy(sbp->bp,sbuf,slen) - sbp->bp ;
	        sbp->bp += (cl + 1) ;
	        sbp->i += (cl + 1) ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_wstrw) */


int serialbuf_wbuf(SERIALBUF *sbp,cchar *sbuf,int slen)
{

	if (slen < 0) return SR_INVALID ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= slen) {
	        memcpy(sbp->bp,sbuf,slen) ;
	        sbp->bp += slen ;
	        sbp->i += slen ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_wbuf) */


int serialbuf_wuchar(SERIALBUF *sbp,uint ch)
{
	const int	size = sizeof(uchar) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
	        *sbp->bp++ = (char) ch ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_wuchar) */


int serialbuf_wushort(SERIALBUF *sbp,uint sw)
{
	const int	size = sizeof(ushort) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_wushort(sbp->bp,sw) ;
#else
	        netorder_wushort(sbp->bp,sw) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_wushort) */


int serialbuf_wuint(SERIALBUF *sbp,uint iw)
{
	const int	size = sizeof(uint) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_wuint(sbp->bp,iw) ;
#else
	        netorder_wuint(sbp->bp,iw) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_wuint) */


int serialbuf_wuinta(SERIALBUF *sbp,uint *iwa,int n)
{
	const int	size = sizeof(uint) ;
	int		i ;

	for (i = 0 ; (sbp->i >= 0) && (i < n) ; i += 1) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_wuint(sbp->bp,iwa[i]) ;
#else
	        netorder_wuint(sbp->bp,iwa[i]) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	} /* end for */

	return sbp->i ;
}
/* end subroutine (serialbuf_wuinta) */


int serialbuf_wulong(SERIALBUF *sbp,ulong lw)
{
	const int	size = sizeof(ulong) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_wulong(sbp->bp,lw) ;
#else
	        netorder_wulong(sbp->bp,lw) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_wulong) */


int serialbuf_wulonga(SERIALBUF *sbp,ulong *lwa,int n)
{
	const int	size = sizeof(ulong) ;
	int		i ;

	for (i = 0 ; (sbp->i >= 0) && (i < n) ; i += 1) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_wulong(sbp->bp,lwa[i]) ;
#else
	        netorder_wulong(sbp->bp,lwa[i]) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	} /* end for */

	return sbp->i ;
}
/* end subroutine (serialbuf_wulonga) */


int serialbuf_wull(SERIALBUF *sbp,ULONG lw)
{
	const int	size = sizeof(ULONG) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= size) {
#if	CF_STDORDER
	        stdorder_wull(sbp->bp,lw) ;
#else
	        netorder_wull(sbp->bp,lw) ;
#endif
	        sbp->bp += size ;
	        sbp->i += size ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_wull) */


int serialbuf_wustrn(SERIALBUF *sbp,const uchar *usbuf,int slen)
{
	char		*sbuf = (char *) usbuf ;

	if (slen < 0) slen = strlen(sbuf) ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= slen) {
	        strncpy(sbp->bp,sbuf,slen) ;
	        sbp->bp += slen ;
	        sbp->i += slen ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_wustrn) */


int serialbuf_wustrw(SERIALBUF *sbp,const uchar *usbuf,int slen)
{
	char		*sbuf = (char *) usbuf ;

	if (slen < 0) slen = strlen(sbuf) ;

	if (sbp->i >= 0) {
	    int		cl ;
	    if ((sbp->len - sbp->i) >= (slen + 1)) {
	        cl = strwcpy(sbp->bp,sbuf,slen) - sbp->bp ;
	        sbp->bp += (cl + 1) ;
	        sbp->i += (cl + 1) ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

	return sbp->i ;
}
/* end subroutine (serialbuf_wustrw) */


int serialbuf_wubuf(SERIALBUF *sbp,const uchar *sbuf,int slen)
{

#if	CF_DEBUGS
	debugprintf("serialbuf_wubuf: slen=%d\n",slen) ;
	debugprintf("serialbuf_wubuf: len=%d i=%d\n",sbp->len,sbp->i) ;
#endif

	if (slen < 0) return SR_INVALID ;

	if (sbp->i >= 0) {
	    if ((sbp->len - sbp->i) >= slen) {
	        memcpy(sbp->bp,sbuf,slen) ;
	        sbp->bp += slen ;
	        sbp->i += slen ;
	    } else {
	        sbp->i = SR_TOOBIG ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("serialbuf_wubuf: ret sbp->i=%d\n",sbp->i) ;
#endif

	return sbp->i ;
}
/* end subroutine (serialbuf_wubuf) */


