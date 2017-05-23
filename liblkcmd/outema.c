/* outema */

/* manage printing lines */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

	= 1999-02-01, David A­D­ Morano
	I added a little code to "post" articles that do not have a valid
	newsgroup to a special "dead article" directory in the BB spool area.

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object deals with printing lines.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<buffer.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"outema.h"


/* local defines */

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#undef	BUFLEN
#define	BUFLEN		(2 * 1024)

#ifndef	MAXMSGLINELEN
#define	MAXMSGLINELEN	76
#endif

#ifndef	NTABCOLS
#define	NTABCOLS	8
#endif

#define	BASE64LINELEN	72
#define	BASE64BUFLEN	((BASE64LINELEN / 4) * 3)

/* types of "content encodings" */
#define	CE_7BIT		0
#define	CE_8BIT		1
#define	CE_BINARY	2
#define	CE_BASE64	3

#ifndef	FROM_ESCAPE
#define	FROM_ESCAPE	'\b'
#endif

#undef	CHAR_TOKSEP
#define	CHAR_TOKSEP(c)	(CHAR_ISWHITE(c) || (! isprintlatin(c)))


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpycompact(char *,int,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nextqtoken(const char *,int,const char **) ;
extern int	vbufprintf(char *,int,const char *,va_list) ;
extern int	haswhite(const char *,int) ;
extern int	isprintlatin(int) ;

extern int	buffer_strquote(BUFFER *,const char *,int) ;
extern int	buffer_stropaque(BUFFER *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

int		outema_item(OUTEMA *,const char *,int) ;

static int	filebuf_outpart(FILEBUF *,int,const char *,int) ;


/* local variables */


/* exported subroutines */


int outema_start(OUTEMA *op,FILEBUF *ofp,int maxlen)
{

	if (op == NULL)
	    return SR_FAULT ;

	memset(op,0,sizeof(OUTEMA)) ;
	op->maxlen = maxlen ;
	op->rlen = maxlen ;
	op->ofp = ofp ;

	return SR_OK ;
}
/* end subroutine (outema_start) */


int outema_finish(OUTEMA *ldp)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (ldp == NULL) return SR_FAULT ;
	if (ldp->ofp == NULL) return SR_FAULT ;

	if (ldp->llen > 0) {
	    rs = filebuf_print(ldp->ofp,NULL,0) ;
	    ldp->wlen += rs ;
	    ldp->rlen = ldp->maxlen ;
	    ldp->llen = 0 ;
	}

	wlen = ldp->wlen ;
	ldp->ofp = NULL ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outema_finish) */


int outema_ent(OUTEMA *ldp,EMA_ENT *ep)
{
	BUFFER		b, *bufp = &b ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((rs = buffer_start(bufp,80)) >= 0) {
	    const char	*bp ;
	    int		bl ;
	    int		c = 0 ;

	    if ((rs >= 0) && (ep->ap != NULL) && (ep->al > 0)) {
	        if (c++ > 0) rs = buffer_char(bufp,CH_SP) ;
	        if (rs >= 0)
	            rs = buffer_strquote(bufp,ep->ap,ep->al) ;
	    }

	    if ((rs >= 0) && (ep->rp != NULL) && (ep->rl > 0)) {
	        if (c++ > 0) rs = buffer_char(bufp,CH_SP) ;
	        if (rs >= 0)
	            rs = buffer_char(bufp,CH_LANGLE) ;
	        if (rs >= 0)
	            rs = buffer_stropaque(bufp,ep->rp,ep->rl) ;
	        if (rs >= 0)
	            rs = buffer_char(bufp,CH_RANGLE) ;
	    }

#if	CF_DEBUGS
	    debugprintf("outema_ent: c=>%t<\n",ep->cp,ep->cl) ;
#endif

	    if ((rs >= 0) && (ep->cp != NULL) && (ep->cl > 0)) {
	        const char	*cp ;
	        int		cl ;
	        if ((cl = sfshrink(ep->cp,ep->cl,&cp)) > 0) {
	            if (c++ > 0) rs = buffer_char(bufp,CH_SP) ;
	            if (rs >= 0) {
	                const int	size = (cl+2+1) ;
	                char		*ap ;
	                if ((rs = uc_malloc(size,&ap)) >= 0) {
	                    char	*bp = ap ;
	                    *bp++ = CH_LPAREN ;
	                    if ((rs = snwcpycompact(bp,cl,cp,cl)) >= 0) {
	                        bp += rs ;
	                        *bp++ = CH_RPAREN ;
	                        rs = buffer_strw(bufp,ap,(bp-ap)) ;
	                    } /* end if (snwcpycompact) */
	                    uc_free(ap) ;
	                } /* end if (m-a-f) */
	            }
	        } /* end if (shrink) */
	    } /* end if (comment) */

	    if (rs >= 0) {
	        if ((rs = buffer_get(bufp,&bp)) > 0) {
	            bl = rs ;
	            rs = outema_item(ldp,bp,bl) ;
	            wlen += rs ;
	        }
	    }

	    rs1 = buffer_finish(bufp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (buffer-ret) */

#if	CF_DEBUGS
	debugprintf("outema_ent: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outema_ent) */


int outema_item(OUTEMA *ldp,cchar vp[],int vl)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (vl < 0) vl = strlen(vp) ;

	if (vl > 0) {
	    int	f_prevcomma = ldp->f.comma ;

	    ldp->f.comma = TRUE ;
	    rs = outema_value(ldp,vp,vl) ;
	    wlen += rs ;
	    ldp->c_items += 1 ;

	    ldp->f.comma = f_prevcomma ;
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outema_item) */


/* output a single value for a header (folding lines as needed) */
int outema_value(OUTEMA *ldp,cchar vp[],int vl)
{
	int		rs = SR_OK ;
	int		nlen ;
	int		cl, cl2 ;
	int		wlen = 0 ;
	int		f_comma = FALSE ;
	const char	*fmt ;
	const char	*tp, *cp ;

	if (ldp == NULL)
	    return SR_INVALID ;

	if ((vp == NULL) || (vp[0] == '\0'))
	    return SR_OK ;

	if (vl < 0) vl = strlen(vp) ;

	ldp->c_values = 0 ;
	while ((rs >= 0) && (vl > 0)) {

	    if ((cl = nextqtoken(vp,vl,&cp)) > 0) {

#if	CF_DEBUGS
	        debugprintf("outema_value: qt=>%t<\n",cp,cl) ;
#endif

	        f_comma = (ldp->f.comma && (ldp->c_items > 0)) ;
	        nlen = outema_needlength(ldp,cl) ;

	        if (nlen > ldp->rlen) {

	            if (ldp->llen > 0) {
	                fmt = "\n" ;
	                if (f_comma) {
	                    f_comma = FALSE ;
	                    ldp->f.comma = FALSE ;
	                    fmt = ",\n" ;
	                }
	                rs = filebuf_write(ldp->ofp,fmt,-1) ;
	                wlen += rs ;
	            }

	            ldp->rlen = ldp->maxlen ;
	            ldp->llen = 0 ;
	            ldp->c_values = 0 ;

	        } /* end if (overflow) */

	        if (rs >= 0) {
	            if (f_comma) {
	                ldp->f.comma = FALSE ;
	            }
	            rs = filebuf_outpart(ldp->ofp,f_comma,cp,cl) ;
	            wlen += rs ;
	            ldp->llen += rs ;
	            ldp->rlen -= rs ;
	            f_comma = FALSE ;
	        }

	        ldp->c_values += 1 ;
	        cl2 = (cp + cl - vp) ;
	        vp += cl2 ;
	        vl -= cl2 ;

	    } else if ((tp = strnchr(vp,vl,'\n')) != NULL) {
	        vl -= ((tp + 1) - vp) ;
	        vp = (tp + 1) ;
	    } else
	        vl = 0 ;

	} /* end while */

	ldp->wlen += wlen ;

#if	CF_DEBUGS
	debugprintf("outema_value: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outema_value) */


int outema_write(OUTEMA *ldp,cchar v[],int vlen)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (vlen < 0)
	    vlen = strlen(v) ;

	if (vlen > 0) {
	    rs = filebuf_write(ldp->ofp,v,vlen) ;
	    wlen += rs ;
	    ldp->llen += rs ;
	    ldp->rlen -= rs ;
	}

	ldp->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outema_write) */


#ifdef	COMMENT

int outema_printf(OUTEMA *ldp,const char *fmt,...)
{
	int		rs = SR_OK ;
	int		len = 0 ;
	int		wlen = 0 ;
	char		buf[BUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("outema_printf: ent fmt=>%s<\n",fmt) ;
#endif

	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = vbufprintf(buf,BUFLEN,fmt,ap) ;
	    len = rs ;
	    va_end(ap) ;
	}

	if (rs >= 0) {
	    rs = filebuf_write(ldp->ofp,buf,len) ;
	    wlen += rs ;
	    ldp->wlen += rs ;
	    ldp->llen += rs ;
	    ldp->rlen -= rs ;
	}

#if	CF_DEBUGS
	debugprintf("outema_printf: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outema_printf) */

#endif /* COMMENT */


int outema_hdrkey(OUTEMA *ldp,cchar kname[])
{
	int		rs = SR_OK ;
	int		nlen = 0 ;
	int		wlen = 0 ;

	if (ldp == NULL) return SR_FAULT ;
	if (kname == NULL) return SR_FAULT ;

	if (kname[0] == '\0') return SR_INVALID ;

	if ((rs >= 0) && (ldp->llen > 0)) {
	    rs = filebuf_print(ldp->ofp,kname,0) ;
	    wlen += rs ;
	    ldp->llen = 0 ;
	    ldp->rlen = ldp->maxlen ;
	}

	if (rs >= 0) {
	    rs = filebuf_write(ldp->ofp,kname,-1) ;
	    wlen += rs ;
	    nlen += rs ;
	}

	if (rs >= 0) {
	    char	buf[2] ;
	    buf[0] = ':' ;
	    buf[1] = '\0' ;
	    rs = filebuf_write(ldp->ofp,buf,1) ;
	    wlen += rs ;
	    nlen += rs ;
	}

	if ((rs >= 0) && (nlen > 0)) {
	    ldp->llen += nlen ;
	    ldp->rlen -= nlen ;
	}

	ldp->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outema_hdrkey) */


int outema_needlength(OUTEMA *ldp,int cl)
{
	int		nlen = (cl + 1) ;

	if (ldp->llen == 0) {
	    nlen += 1 ;
	}

	if (ldp->f.comma && (ldp->c_items > 0)) {
	    nlen += 1 ;
	}

	return nlen ;
}
/* end subroutine (outema_needlength) */


/* private subroutines */


static int filebuf_outpart(FILEBUF *fbp,int f_comma,cchar *cp,int cl)
{
	int		rs = SR_OK ;
	int		i ;
	int		wlen = 0 ;
	char		buf[3] ;

	if (fbp == NULL)
	    return SR_FAULT ;

	i = 0 ;
	if (f_comma) buf[i++] = CH_COMMA ;
	buf[i++] = ' ' ;
	buf[i] = '\0' ;
	if ((rs = filebuf_write(fbp,buf,i)) >= 0) {
	    wlen += rs ;
	    rs = filebuf_write(fbp,cp,cl) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_outpart) */


