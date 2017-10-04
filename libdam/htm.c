/* htm (HTML creation and output) */

/* hack to output HTML */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a hack to create and output some basic HTML.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<dlfcn.h>
#include	<time.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<shio.h>
#include	<sbuf.h>
#include	<buffer.h>
#include	<ascii.h>
#include	<linefold.h>
#include	<localmisc.h>

#include	"htm.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#define	ISCONT(b,bl)	\
	(((bl) >= 2) && ((b)[(bl) - 1] == '\n') && ((b)[(bl) - 2] == '\\'))


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	ctdeci(char *,int,int) ;
extern int	ctdecui(char *,int,uint) ;
extern int	msleep(int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* local structures */


/* forward references */

static int	htm_tagalone(HTM *,const char *,const char *,const char *) ;
static int	htm_printout(HTM *,int,const char *,int) ;


/* static writable data */


/* local variables */


/* exported subroutines */


int htm_start(HTM *op,SHIO *ofp,cchar *lang)
{
	int		rs ;
	int		wlen = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (ofp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("htm_start: lang=%s\n",lang) ;
#endif

	memset(op,0,sizeof(HTM)) ;
	op->ofp = ofp ;
	if ((rs = shio_print(op->ofp,"<!doctype html>",-1)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    char	lbuf[LINEBUFLEN+1] ;
	    wlen += rs ;
	    if ((lang != NULL) && (lang[0] != '\0')) {
	        rs = bufprintf(lbuf,llen,"<html lang=\"%s\">",lang) ;
	    } else {
	        rs = bufprintf(lbuf,llen,"<html>") ;
	    }
	    if (rs >= 0) {
	        rs = shio_print(op->ofp,lbuf,rs) ;
	        wlen += rs ;
	    }
	} /* end if (doctype) */
	if (rs >= 0) op->magic = HTM_MAGIC ;

#if	CF_DEBUGS
	debugprintf("htm_start: ret rs=%d\n",rs) ;
#endif

	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (htm_start) */


int htm_finish(HTM *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != HTM_MAGIC) return SR_NOTOPEN ;

	rs1 = htm_printline(op,"</html>",-1) ;
	if (rs >= 0) rs = rs1 ;

	wlen = op->wlen ;
	op->ofp = NULL ;
	op->magic = 0 ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (htm_finish) */


int htm_headbegin(HTM *op,const char *cfname)
{
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	const char	*sp = "<head>" ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != HTM_MAGIC) return SR_NOTOPEN ;
	if ((rs = shio_print(op->ofp,sp,-1)) >= 0) {
	    wlen += rs ;
	    if ((cfname != NULL) && (cfname[0] != '\0')) {
	        SHIO	cf ;
	        if ((rs = shio_open(&cf,cfname,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            char	lbuf[LINEBUFLEN+1] ;
	            while ((rs = shio_read(&cf,lbuf,llen)) > 0) {
	                rs = shio_write(op->ofp,lbuf,rs) ;
	                wlen += rs ;
	                if (rs < 0) break ;
	            } /* end while (reading lines) */
	            rs1 = shio_close(&cf) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (copy-content-file) */
	    } /* end if (have content-file) */
	} /* end if (shio_print) */

	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (htm_headbegin) */


int htm_headend(HTM *op)
{
	int		rs ;
	int		wlen = 0 ;
	const char	*sp = "</head>" ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != HTM_MAGIC) return SR_NOTOPEN ;
	rs = shio_print(op->ofp,sp,-1) ;
	wlen += rs ;
	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (htm_headend) */


int htm_bodybegin(HTM *op,cchar *cfname)
{
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	const char	*sp = "<body>" ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != HTM_MAGIC) return SR_NOTOPEN ;
	if ((rs = shio_print(op->ofp,sp,-1)) >= 0) {
	    wlen += rs ;
	    if ((cfname != NULL) && (cfname[0] != '\0')) {
	        SHIO	cf ;
	        if ((rs = shio_open(&cf,cfname,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            char	lbuf[LINEBUFLEN+1] ;
	            while ((rs = shio_read(&cf,lbuf,llen)) > 0) {
	                rs = shio_write(op->ofp,lbuf,rs) ;
	                wlen += rs ;
	                if (rs < 0) break ;
	            } /* end while (reading lines) */
	            rs1 = shio_close(&cf) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (copy-content-file) */
	    } /* end if (have content-file) */
	} /* end if (shio_print) */

	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (htm_bodybegin) */


int htm_bodyend(HTM *op)
{
	int		rs ;
	int		wlen = 0 ;
	const char	*sp = "</body>" ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != HTM_MAGIC) return SR_NOTOPEN ;
	rs = shio_print(op->ofp,sp,-1) ;
	wlen += rs ;
	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (htm_bodyend) */


int htm_tagbegin(HTM *op,cchar *tag,cchar *class,cchar *id,cchar *(*kv)[2])
{
	BUFFER		b ;
	const int	c = COLUMNS ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (tag == NULL) return SR_FAULT ;
	if (op->magic != HTM_MAGIC) return SR_NOTOPEN ;
	if (tag[0] == '\0') return SR_INVALID ;
	if ((rs = buffer_start(&b,c)) >= 0) {
	    int		i ;
	    const char	*k, *v ;
	    buffer_char(&b,CH_LANGLE) ;
	    buffer_strw(&b,tag,-1) ;
	    for (i = 0 ; i < 2 ; i += 1) {
	        v = NULL ;
	        switch (i) {
	        case 0:
	            k = "class" ;
	            v = class ;
	            break ;
	        case 1:
	            k = "id" ;
	            v = id ;
	            break ;
	        } /* end switch */
	        if ((v != NULL) && (v[0] != '\0')) {
	            rs = buffer_printf(&b," %s=\"%s\"",k,v) ;
	        }
	        if (rs < 0) break ;
	    } /* end for */
	    if ((rs >= 0) && (kv != NULL)) {
		cchar	*fmt ;
	        for (i = 0 ; kv[i][0] != NULL ; i += 1) {
		    if (kv[i][1] != NULL) {
			if (kv[i][1][0] != '\0') {
	                    fmt = " %s=\"%s\"" ;
			} else {
	                    fmt = " %s=" ;
			}
		    } else {
	                fmt = " %s" ;
		    }
	            rs = buffer_printf(&b,fmt,kv[i][0],kv[i][1]) ;
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (key-vals) */
	    buffer_char(&b,CH_RANGLE) ;
	    if (rs >= 0) {
		const char	*bp ;
	        if ((rs = buffer_get(&b,&bp)) >= 0) {
		    rs = htm_printout(op,c,bp,rs) ;
		    wlen += rs ;
	        } /* enbd if (buffer_get) */
	    } /* end if (ok) */
	    rs1 = buffer_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (buffer) */
	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (htm_tagbegin) */


int htm_tagend(HTM *op,const char *tag)
{
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		wlen = 0 ;
	char		lbuf[LINEBUFLEN+1] ;
	if (op == NULL) return SR_FAULT ;
	if (tag == NULL) return SR_FAULT ;
	if (op->magic != HTM_MAGIC) return SR_NOTOPEN ;
	if (tag[0] == '\0') return SR_INVALID ;
	if ((rs = bufprintf(lbuf,llen,"</%s>",tag)) >= 0) {
	    rs = shio_print(op->ofp,lbuf,rs) ;
	    wlen += rs ;
	}
	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (htm_tagend) */


int htm_textbegin(op,class,id,title,r,c,tkv)
HTM		*op ;
const char	*class ;
const char	*id ;
const char	*title ;
int		r,c ;
const char	*(*tkv)[2] ;
{
	const int	dlen = DIGBUFLEN ;
	int		rs ;
	int		size = 0 ;
	int		kvsize = 0 ;
	const char	*tag = "textarea" ;
	void		*p ;

	if (tkv != NULL) {
	    int	i ;
	    for (i = 0 ; tkv[i][0] != NULL ; i += 1) ;
	    kvsize = (i*(2*sizeof(cchar *))) ;
	}
	kvsize += (6*(2*sizeof(cchar *))) ;
	size += kvsize ;

	size += ((dlen+1)*2) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    const char	*(*kv)[2] ;
	    char	*bp = p ;
	    char	*d0, *d1 ;

	    kv = (cchar *(*)[2]) bp ;
	    bp += kvsize ;
	    d0 = bp ;
	    bp += (dlen+1) ;
	    d1 = bp ;
	    bp += (dlen+1) ;

	    if ((rs = ctdeci(d0,dlen,r)) >= 0) {
	        if ((rs = ctdeci(d1,dlen,c)) >= 0) {
		    int	i = 0 ;
	            kv[i][0] = "rows" ;
	            kv[i][1] = d0 ;
		    i += 1 ;
	            kv[i][0] = "cols" ;
	            kv[i][1] = d1 ;
		    i += 1 ;
		    if (title != NULL) {
	                kv[i][0] = "title" ;
	                kv[i][1] = title ;
		        i += 1 ;
		    }
		    if (tkv != NULL) {
			int	j ;
			for (j = 0 ; tkv[j][0] != NULL ; j += 1) {
			    kv[i][0] = tkv[j][0] ;
			    kv[i][1] = tkv[j][1] ;
			    i += 1 ;
			} /* end for */
		    } /* end if (have extras) */
	            kv[i][0] = NULL ;
	            kv[i][1] = NULL ;
	            rs = htm_tagbegin(op,tag,class,id,kv) ;
	        } /* end if (ctdeci) */
	    } /* end if (ctdeci) */

	    uc_free(p) ;
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (htm_textbegin) */


int htm_textend(HTM *op)
{
	cchar	*tag = "textarea" ;
	return htm_tagend(op,tag) ;
}
/* end subroutine (htm_textend) */


int htm_abegin(op,class,id,href,title)
HTM		*op ;
const char	*class ;
const char	*id ;
const char	*href ;
const char	*title ;
{
	SBUF		b ;
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	const char	*tag = "a" ;
	char		lbuf[LINEBUFLEN+1] ;
	if (op == NULL) return SR_FAULT ;
	if (href == NULL) return SR_FAULT ;
	if (op->magic != HTM_MAGIC) return SR_NOTOPEN ;
	if (href[0] == '\0') return SR_INVALID ;
	if ((rs = sbuf_start(&b,lbuf,llen)) >= 0) {
	    int		c ;
	    const char	*k, *v ;
	    sbuf_char(&b,CH_LANGLE) ;
	    sbuf_strw(&b,tag,-1) ;
	    for (c = 0 ; c < 4 ; c += 1) {
	        switch (c) {
	        case 0:
	            k = "class" ;
	            v = class ;
	            break ;
	        case 1:
	            k = "id" ;
	            v = id ;
	            break ;
	        case 2:
	            k = "href" ;
	            v = href ;
	            break ;
	        case 3:
	            k = "title" ;
	            v = title ;
	            break ;
	        } /* end switch */
	        if ((v != NULL) && (v[0] != '\0')) {
	            rs = sbuf_printf(&b,"\n %s=\"%s\"",k,v) ;
	        }
	        if (rs < 0) break ;
	    } /* end for */
	    sbuf_char(&b,CH_RANGLE) ;
	    if ((rs = sbuf_getlen(&b)) >= 0) {
	        rs = shio_print(op->ofp,lbuf,rs) ;
	        wlen += rs ;
	    }
	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */
	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (htm_abegin) */


int htm_aend(HTM *op)
{
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		wlen = 0 ;
	const char	*tag = "a" ;
	char		lbuf[LINEBUFLEN+1] ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != HTM_MAGIC) return SR_NOTOPEN ;
	if ((rs = bufprintf(lbuf,llen,"</%s>",tag)) >= 0) {
	    rs = shio_print(op->ofp,lbuf,rs) ;
	    wlen += rs ;
	}
	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (htm_aend) */


int htm_hr(HTM *op,cchar *class,cchar *id)
{
	return htm_tagalone(op,"hr",class,id) ;
}
/* end subroutine (htm_hr) */


int htm_br(HTM *op,cchar *class,cchar *id)
{
	return htm_tagalone(op,"br",class,id) ;
}
/* end subroutine (htm_br) */


int htm_img(op,class,id,src,title,alt,w,h)
HTM		*op ;
const char	*class ;
const char	*id ;
const char	*src ;
const char	*title ;
const char	*alt ;
int		w, h ;
{
	SBUF		b ;
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	const char	*tag = "img" ;
	char		lbuf[LINEBUFLEN+1] ;
	if (op == NULL) return SR_FAULT ;
	if (src == NULL) return SR_FAULT ;
	if (op->magic != HTM_MAGIC) return SR_NOTOPEN ;
	if (tag[0] == '\0') return SR_INVALID ;
	if ((rs = sbuf_start(&b,lbuf,llen)) >= 0) {
	    int		c ;
	    cchar	*k ;
	    cchar	*v ;
	    sbuf_char(&b,CH_LANGLE) ;
	    sbuf_strw(&b,tag,-1) ;
	    for (c = 0 ; c < 5 ; c += 1) {
	        v = NULL ;
	        switch (c) {
	        case 0:
	            k = "class" ;
	            v = class ;
	            break ;
	        case 1:
	            k = "id" ;
	            v = id ;
	            break ;
	        case 2:
	            k = "src" ;
	            v = src ;
	            break ;
	        case 3:
	            k = "title" ;
	            v = title ;
	            break ;
	        case 4:
	            k = "alt" ;
	            v = alt ;
	            break ;
	        } /* end switch */
	        if ((v != NULL) && (v[0] != '\0')) {
	            rs = sbuf_printf(&b,"\n %s=\"%s\"",k,v) ;
	        }
	        if (rs < 0) break ;
	    } /* end for */
	    if ((rs >= 0) && (w >= 0)) {
	        rs = sbuf_printf(&b," width=\"%d\"",w) ;
	    }
	    if ((rs >= 0) && (h >= 0)) {
	        rs = sbuf_printf(&b," height=\"%d\"",h) ;
	    }
	    sbuf_strw(&b," /",2) ;
	    sbuf_char(&b,CH_RANGLE) ;
	    if (rs >= 0) {
	        if ((rs = sbuf_getlen(&b)) >= 0) {
	            rs = shio_print(op->ofp,lbuf,rs) ;
	            wlen += rs ;
	        }
	    }
	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */
	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (htm_img) */


int htm_write(HTM *op,const void *lbuf,int llen)
{
	int		rs ;
	int		wlen = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;
	if (op->magic != HTM_MAGIC) return SR_NOTOPEN ;
	rs = shio_write(op->ofp,lbuf,llen) ;
	wlen += rs ;
	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (htm_write) */


int htm_printline(HTM *op,cchar *lbuf,int llen)
{
	int		rs ;
	int		wlen = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;
	if (op->magic != HTM_MAGIC) return SR_NOTOPEN ;
	rs = shio_print(op->ofp,lbuf,llen) ;
	wlen += rs ;
	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (htm_printline) */


int htm_printf(HTM *op,cchar *fmt,...)
{
	va_list		ap ;
	int		rs ;
	va_begin(ap,fmt) ;
	rs = htm_vprintf(op,fmt,ap) ;
	va_end(ap) ;
	return rs ;
}
/* end subroutine (htm_printf) */


int htm_vprintf(HTM *op,cchar *fmt,va_list ap)
{
	int		rs ;
	int		wlen = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != HTM_MAGIC) return SR_NOTOPEN ;
	rs = shio_vprintf(op->ofp,fmt,ap) ;
	wlen += rs ;
	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (htm_vprintf) */


int htm_putc(HTM *op,int ch)
{
	int		rs ;
	int		wlen = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != HTM_MAGIC) return SR_NOTOPEN ;
	rs = shio_putc(op->ofp,ch) ;
	wlen += rs ;
	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (htm_putc) */


/* private subroutines */


int htm_tagalone(HTM *op,cchar *tag,cchar *class,cchar *id)
{
	SBUF		b ;
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	char		lbuf[LINEBUFLEN+1] ;
	if (op == NULL) return SR_FAULT ;
	if (tag == NULL) return SR_FAULT ;
	if (op->magic != HTM_MAGIC) return SR_NOTOPEN ;
	if (tag[0] == '\0') return SR_INVALID ;
	if ((rs = sbuf_start(&b,lbuf,llen)) >= 0) {
	    int		c ;
	    cchar	*k ;
	    cchar	*v ;
	    sbuf_char(&b,CH_LANGLE) ;
	    sbuf_strw(&b,tag,-1) ;
	    for (c = 0 ; c < 2 ; c += 1) {
	        v = NULL ;
	        switch (c) {
	        case 0:
	            k = "class" ;
	            v = class ;
	            break ;
	        case 1:
	            k  = "id" ;
	            v  = id ;
	            break ;
	        } /* end switch */
	        if ((v != NULL) && (v[0] != '\0')) {
	            rs = sbuf_printf(&b," %s=\"%s\"",k,v) ;
	        }
	        if (rs < 0) break ;
	    } /* end for */
	    sbuf_strw(&b," /",2) ;
	    sbuf_char(&b,CH_RANGLE) ;
	    if (rs >= 0) {
	        if ((rs = sbuf_getlen(&b)) >= 0) {
	            rs = shio_print(op->ofp,lbuf,rs) ;
	            wlen += rs ;
	        }
	    }
	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */
	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (htm_tagalone) */


static int htm_printout(HTM *op,int c,cchar *bp,int bl)
{
	LINEFOLD	f ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs = linefold_start(&f,c,1,bp,bl)) >= 0) {
	    int		i ;
	    int		ll ;
	    cchar	*lp ;
	    for (i = 0 ; (ll = linefold_get(&f,i,&lp)) >= 0 ; i += 1) {
		if (i > 0) {
		    rs = shio_putc(op->ofp,CH_SP) ;
		    wlen += rs ;
		}
		if (rs >= 0) {
		    rs = shio_print(op->ofp,lp,ll) ;
		    wlen += rs ;
		} /* end if (ok) */
		if (rs < 0) break ;
	    } /* end for */
	    rs1 = linefold_finish(&f) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (linefold) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (htm_printout) */


