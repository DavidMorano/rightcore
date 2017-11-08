/* buffer */

/* string buffer object */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_SAFE		1		/* extra safe? */
#define	CF_BUFSTART	0		/* allcate buffer from start */
#define	CF_FASTGROW	1		/* grow (extend) faster */


/* revision history:

	= 1998-03-24, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module can be used to construct strings or messages in buffers
	WITHOUT using the 'sprint' subroutine.

	This module is useful when the user does NOT supply a buffer to be used
	as the working store.  Instead, a dynamically grown and managed buffer
	is maintained within the object.

	This module uses an object, that must be initialized and eventually
	freed, to track the state of the dynamically used internal buffer.  An
	exponential growth is used for increasing the buffer size as needed.

	Arguments:

	- bop		pointer to the buffer object
	- <others>

	Returns:

	>=0		the total length of the filled up buffer so far!
	<0		error

	Note:

	This module is not completely protected against calling methods when
	the object has not been initialized already.  This is a little hit for
	"performance reasons" but the benefits are really questionable given
	how cheap it is to check for an uninitialized object!


*******************************************************************************/


#define	BUFFER_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<format.h>
#include	<localmisc.h>

#include	"buffer.h"


/* local defines */

#define	BUFFER_STARTLEN	50		/* starting buffer length */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	MKCHAR
#define	MKCHAR(ch)	((ch) & UCHAR_MAX)
#endif


/* external subroutines */

extern int	ctdeci(char *,int,int) ;
extern int	ctdecui(char *,int,uint) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	nprintf(cchar *,cchar *,...) ;
extern int	strnnlen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

int		buffer_getbuf(BUFFER *,const char **) ;

static int	buffer_ext(BUFFER *,int) ;


/* local variables */


/* exported subroutines */


int buffer_start(BUFFER *op,int startlen)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	op->buf = NULL ;
	op->e = 0 ;
	op->len = 0 ;
	op->startlen = MAX(startlen,BUFFER_STARTLEN) ;

#if	CF_BUFSTART
	if ((rs = buffer_ext(op,-1)) >= 0) {
	    op->buf[0] = '\0' ;
	}
#endif

	return rs ;
}
/* end subroutine (buffer_start) */


/* free up this buffer object */
int buffer_finish(BUFFER *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		len = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

#if	CF_DEBUGS
	debugprintf("buffer_finish: ent\n") ;
#endif

	if (op->buf != NULL) {
	    rs1 = uc_free(op->buf) ;
	    if (rs >= 0) rs = rs1 ;
	    op->buf = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("buffer_finish: ret rs=%d len=%u\n",rs,op->len) ;
#endif

	len = op->len ;
	op->e = 0 ;
	op->startlen = 0 ;
	op->len = 0 ;
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (buffer_finish) */


int buffer_reset(BUFFER *op)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	rs = op->len ;
	if (rs >= 0) op->len = 0 ;
	return rs ;
}
/* end subroutine (buffer_reset) */


int buffer_adv(BUFFER *op,int advlen)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (op->len < 0)
	    return op->len ;

	if (advlen < 0)
	    return SR_INVALID ;

	if ((rs = buffer_ext(op,advlen)) >= 0)
	    op->len += advlen ;

	return (rs >= 0) ? advlen : rs ;
}
/* end subroutine (buffer_adv) */


/* add a character to the buffer object */
int buffer_char(BUFFER *op,int ch)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

#if	CF_DEBUGS
	debugprintf("buffer_char: ent\n") ;
	debugprintf("buffer_char: ch=>%c<\n",ch) ;
#endif

	if (op->len >= 0) {
	    if ((rs = buffer_ext(op,1)) >= 0) {
	        op->buf[(op->len)++] = ch ;
	        op->buf[op->len] = '\0' ;
	    }
	} else
	   rs = op->len ;

	return (rs >= 0) ? 1 : rs ;
}
/* end subroutine (buffer_char) */


/* add another byte array to this buffer object */
int buffer_buf(BUFFER *op,cchar *sbuf,int slen)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

#if	CF_DEBUGS
	debugprintf("buffer_buf: ent\n") ;
#endif

	if (op->len >= 0) {
	    if (slen < 0) slen = strlen(sbuf) ;
	    if ((rs = buffer_ext(op,slen)) >= 0) {
	        char	*bp = (op->buf + op->len) ;
	        memcpy(bp,sbuf,slen) ;
	        op->len += slen ;
	    }
	} else
	    rs = op->len ;

	return (rs >= 0) ? slen : rs ;
}
/* end subroutine (buffer_buf) */


/* add a character string to this buffer object */
int buffer_strw(BUFFER *op,cchar *sbuf,int slen)
{
	int		rs ;
	int		len = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

#if	CF_DEBUGS
	debugprintf("buffer_strw: ent s=>%t<\n",
	    sbuf,strnlen(sbuf,slen)) ;
#endif

	if (op->len >= 0) {
	    if (slen < 0) slen = strlen(sbuf) ;
	    if ((rs = buffer_ext(op,slen)) >= 0) {
	        char	*bp = (op->buf + op->len) ;
	        len = strwcpy(bp,sbuf,slen) - bp ;
	        op->len += len ;
	    }
	} else
	    rs = op->len ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (buffer_strw) */


#ifdef	COMMENT /* never used */

/* add a character string to this buffer object */
int buffer_strn(BUFFER *op,cchar *mbuf,int mlen)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

#if	CF_DEBUGS
	debugprintf("buffer_strn: ent\n") ;
#endif

	if (op->len >= 0) {
	    if (mlen < 0) mlen = strlen(mbuf) ;
	    if ((rs = buffer_ext(op,mlen)) >= 0) {
	        char	*bp = (op->buf + op->len) ;
	        strncpy(bp,mbuf,mlen) ;
	        op->buf[op->len] = '\0' ;
	        op->len += mlen ;
	    }
	} else
	    rs = op->len ;

	return (rs >= 0) ? op->len : rs ;
}
/* end subroutine (buffer_strn) */

#endif /* COMMENT */


int buffer_deci(BUFFER *op,int v)
{
	const int	dlen = DIGBUFLEN ;
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if ((rs = buffer_ext(op,dlen)) >= 0) {
	    char	*bp = (op->buf + op->len) ;
	    rs = ctdeci(bp,dlen,v) ;
	    op->len += rs ;
	}

	return rs ;
}
/* end subroutine (buffer_deci) */


int buffer_decui(BUFFER *op,uint v)
{
	const int	dlen = DIGBUFLEN ;
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if ((rs = buffer_ext(op,dlen)) >= 0) {
	    char	*bp = (op->buf + op->len) ;
	    rs = ctdecui(bp,dlen,v) ;
	    op->len += rs ;
	}

	return rs ;
}
/* end subroutine (buffer_decui) */


/* vprintf-like thing */
int buffer_vprintf(BUFFER *op,cchar *fmt,va_list ap)
{
	const int	flen = LINEBUFLEN ;
	int		rs ;
	char		fbuf[LINEBUFLEN+1] ;

	if (op == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if ((rs = format(fbuf,flen,0x01,fmt,ap)) >= 0) {
	    rs = buffer_strw(op,fbuf,rs) ;
	}

	return rs ;
}
/* end subroutine (buffer_vprintf) */


/* PRINTFLIKE2 */
int buffer_printf(BUFFER *op,cchar *fmt,...)
{
	int		rs ;
	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = buffer_vprintf(op,fmt,ap) ;
	    va_end(ap) ;
	}
	return rs ;
}
/* end subroutine (buffer_printf) */


/* get the address of the byte array from the buffer object */
int buffer_get(BUFFER *op,cchar **spp)
{
	int		rs = SR_OK ;
	int		len ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	len = op->len ;
	if (spp != NULL) {
	    if ((rs = buffer_ext(op,1)) >= 0) {
	        *spp = (rs >= 0) ? op->buf : NULL ;
	    }
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (buffer_get) */


int buffer_getprev(BUFFER *op)
{
	int		rs = SR_NOENT ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (op->len > 0) {
	    if ((rs = buffer_ext(op,1)) >= 0) {
	        rs = MKCHAR(op->buf[op->len-1]) ;
	    }
	}

	return rs ;
}
/* end subroutine (buffer_getprev) */


/* private subroutines */


/* extend the object buffer */
static int buffer_ext(BUFFER *op,int req)
{
	int		rs = SR_OK ;
	int		need ;
	int		ne ;

#if	CF_DEBUGS
	debugprintf("buffer_ext: request=%d\n",req) ;
	debugprintf("buffer_ext: e=%u l=%d\n",op->e,op->len) ;
#endif

	if (req < 0) req = op->startlen ;

	need = ((op->len + (req+1)) - op->e) ;
	if (need > 0) {
	    char	*buf ;

	    if (op->buf == NULL) {

	        ne = MAX(op->startlen,need) ;
	        if ((rs = uc_malloc(ne,&buf)) >= 0) {
	            op->buf = buf ;
		    op->e = ne ;
	        } else {
	            op->len = rs ;
	        }

	    } else {

		ne = op->e ;
	        while ((op->len + (req+1)) > ne) {
#if	CF_FASTGROW
	            ne = ((ne + 1) * 2) ;
#else
	            ne = (ne + BUFFER_STARTLEN) ;
#endif /* CF_FASTGROW */
	        } /* end while */

	        if ((rs = uc_realloc(op->buf,ne,&buf)) >= 0) {
	            op->buf = buf ;
		    op->e = ne ;
	        } else {
	            op->len = rs ;
		}

	    } /* end if */

	} /* end if (extension needed) */

#if	CF_DEBUGS
	debugprintf("buffer_ext: ret rs=%d e=%u\n",rs,op->e) ;
#endif

	return rs ;
}
/* end subroutine (buffer_ext) */


