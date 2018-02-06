/* cgi (CGI creation and output) */

/* hack to output the HTTP headers */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a hack to create and output some basic CGIL.


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
#include	<time.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<shio.h>
#include	<sbuf.h>
#include	<ascii.h>
#include	<tmtime.h>
#include	<localmisc.h>

#include	"cgi.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	KBUFLEN
#define	KBUFLEN		MAX(TIMEBUFLEN,60)
#endif

#define	ISCONT(b,bl)	\
	(((bl) >= 2) && ((b)[(bl) - 1] == '\n') && ((b)[(bl) - 2] == '\\'))


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	msleep(int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(cchar *,int,int) ;
extern char	*strnrchr(cchar *,int,int) ;
extern char	*timestr_std(time_t,char *) ;
extern char	*timestr_msg(time_t,char *) ;


/* local structures */


/* forward references */


/* static writable data */


/* local variables */


/* exported subroutines */


int cgi_start(CGI *op,SHIO *ofp)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (ofp == NULL) return SR_FAULT ;

	memset(op,0,sizeof(CGI)) ;
	op->ofp = ofp ;
	if (rs >= 0) op->magic = CGI_MAGIC ;

#if	CF_DEBUGS
	debugprintf("cgi_start: ret rs=%d\n",rs) ;
#endif

	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (cgi_start) */


int cgi_finish(CGI *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != CGI_MAGIC) return SR_NOTOPEN ;

	op->ofp = NULL ;
	op->magic = 0 ;
	return rs ;
}
/* end subroutine (cgi_finish) */


int cgi_eoh(CGI *op)
{
	int		rs ;
	int		wlen = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != CGI_MAGIC) return SR_NOTOPEN ;
	rs = shio_putc(op->ofp,CH_NL) ;
	wlen += rs ;
	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (cgi_eof) */


int cgi_hdrdate(CGI *op,time_t t)
{
	SHIO		*ofp ;
	const int	hlen = KBUFLEN ;
	int		rs ;
	int		wlen = 0 ;
	const char	*hdr = "date" ;
	char		tbuf[TIMEBUFLEN+1] ;
	char		hbuf[TIMEBUFLEN+1] ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != CGI_MAGIC) return SR_NOTOPEN ;
	ofp = op->ofp ;
	timestr_msg(t,tbuf) ;
	if ((rs = bufprintf(hbuf,hlen,"%s: %s",hdr,tbuf)) >= 0) {
	    if ((rs = shio_print(ofp,hbuf,rs)) >= 0) {
	        wlen += rs ;
	    } /* end if (shio_write) */
	} /* end if (bufprintf) */
	op->wlen += wlen ;
#if	CF_DEBUGS
	debugprintf("cgi_hdrdate: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (cgi_hdrdate) */


int cgi_hdr(CGI *op,cchar *kp,cchar *vp,int vl)
{
	SHIO		*ofp ;
	const int	klen = KBUFLEN ;
	int		rs ;
	int		wlen = 0 ;
	char		kbuf[KBUFLEN+1] ;
#if	CF_DEBUGS
	debugprintf("cgi_hdr: ent k=%s v=>%t<\n",kp,vp,vl) ;
#endif
	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;
	if (op->magic != CGI_MAGIC) return SR_NOTOPEN ;
	ofp = op->ofp ;
	if ((rs = bufprintf(kbuf,klen,"%s: ",kp)) >= 0) {
	    if ((rs = shio_write(ofp,kbuf,rs)) >= 0) {
	        wlen += rs ;
	        rs = shio_print(ofp,vp,vl) ;
	        wlen += rs ;
	    } /* end if (shio_write) */
	} /* end if (bufprintf) */
	op->wlen += wlen ;
#if	CF_DEBUGS
	debugprintf("cgi_hdr: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (cgi_hdr) */


int cgi_write(CGI *op,const void *lbuf,int llen)
{
	int		rs ;
	int		wlen = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;
	if (op->magic != CGI_MAGIC) return SR_NOTOPEN ;
	rs = shio_write(op->ofp,lbuf,llen) ;
	wlen += rs ;
	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (cgi_write) */


int cgi_printline(CGI *op,cchar *lbuf,int llen)
{
	int		rs ;
	int		wlen = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;
	if (op->magic != CGI_MAGIC) return SR_NOTOPEN ;
	rs = shio_print(op->ofp,lbuf,llen) ;
	wlen += rs ;
	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (cgi_printline) */


int cgi_printf(CGI *op,cchar *fmt,...)
{
	va_list		ap ;
	int		rs ;
	va_begin(ap,fmt) ;
	rs = cgi_vprintf(op,fmt,ap) ;
	va_end(ap) ;
	return rs ;
}
/* end subroutine (cgi_printf) */


int cgi_vprintf(CGI *op,cchar *fmt,va_list ap)
{
	int		rs ;
	int		wlen = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != CGI_MAGIC) return SR_NOTOPEN ;
	rs = shio_vprintf(op->ofp,fmt,ap) ;
	wlen += rs ;
	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (cgi_vprintf) */


int cgi_putc(CGI *op,int ch)
{
	int		rs ;
	int		wlen = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != CGI_MAGIC) return SR_NOTOPEN ;
	rs = shio_putc(op->ofp,ch) ;
	wlen += rs ;
	op->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (cgi_putc) */


/* private subroutines */


