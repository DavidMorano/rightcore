/* sfioext */

/* Safe-Fast I-O Extension(s) */


#define	CF_DEBUGS	1		/* compile-time debug print-outs */
#define	CF_FLUSHPART	0		/* partial flush */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a hack to add some normalcy to the SFIO interface.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<ast.h>			/* configures other stuff also */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<dlfcn.h>
#include	<time.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"sfioext.h"


/* local defines */

#define	SFIO_MODESTRLEN	20

#ifndef	SFIO_OFF
#define	SFIO_OFF	0
#endif

#ifndef	SFIO_ON
#define	SFIO_ON		1
#endif

#define	SFIO_MAXREADLEN	100

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	msleep(int) ;

extern int	sfreadline(Sfio_t *,char *,int) ;
extern int	sfreadlinetimed(Sfio_t *,char *,int,int) ;

extern int	format(char *,int,int,const char *,va_list) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* forward references */

static int	hasnl(const char *,int) ;


/* local variables */

static const char	*stdfnames[] = {
	STDINFNAME,
	STDOUTFNAME,
	STDERRFNAME,
	STDNULLFNAME,
	"*NULL*",
	NULL
} ;

enum stdfnames {
	stdfname_stdin,
	stdfname_stdout,
	stdfname_stderr,
	stdfname_stdnull,
	stdfname_oldnull,
	stdfname_overlast
} ;


/* exported subroutines */


int sfio_reade(op,abuf,alen,to,opts)
Sfio_t		*op ;
void		*abuf ;
int		alen ;
int		to ;
int		opts ;
{
	int	rs = SR_OK ;
	int	rlen = alen ;

	char	*rbuf = (char *) abuf ;


	if (op == NULL) return SR_FAULT ;
	if (abuf == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("sfio_reade: ent\n") ;
#endif

	if (rlen == 0) goto ret0 ;

	    Sfio_t	*streams[2] ;
	    time_t	ti_now = time(NULL) ;
	    time_t	ti_start ;

	    rbuf[0] = '\0' ;
	    ti_start = ti_now ;
	    streams[0] = op ;
	    streams[1] = NULL ;
	    while (rs >= 0) {
		if ((rs = sfpoll(streams,1,1000)) < 0) rs = SR_INTR ;
#if	CF_DEBUGS
	debugprintf("sfio_reade: sfpoll() rs=%d\n",rs) ;
#endif
		if (rs > 0) {
		    int v = sfvalue(op) ;
#if	CF_DEBUGS
	debugprintf("sfio_reade: sfvalue() v=%04x\n",v) ;
#endif
		    if (v & SF_READ) {
			{
			    char	*p ;
		            p = sfgetr(op,'\n',0) ;
#if	CF_DEBUGS
	debugprintf("sfio_reade: sfgetr() p=%p\n",p) ;
#endif
			    if (p != NULL) {
				if ((v = sfvalue(op)) < 0) rs = SR_HANGUP ;
#if	CF_DEBUGS
	debugprintf("sfio_reade: sfvalue() v=%d\n",v) ;
#endif
				if (rs >= 0)
				    rs = snwcpy(rbuf,rlen,p,v) ;
			    } else {
		                p = sfgetr(op,'\n',SF_LASTR) ;
			        if (p != NULL) {
				    if ((v = sfvalue(op)) < 0) rs = SR_HANGUP ;
				    if (rs >= 0)
				        rs = snwcpy(rbuf,rlen,p,v) ;
				} else {
			            rs = SR_OK ;
				    break ;
				}
			    }
		        } /* end block */
		        break ;
		    } else
		        msleep(10) ;
		} else if (rs < 0)
		    break ;
		if (to >= 0) {
		    ti_now = time(NULL) ;
		    if ((ti_now - ti_start) >= to) {
			rs = SR_TIMEDOUT ;
			break ;
		    }
		}
	    } /* end while */

ret0:

#if	CF_DEBUGS
	debugprintf("sfio_reade: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sfio_reade) */


int sfio_read(op,ubuf,ubuflen)
Sfio_t		*op ;
void		*ubuf ;
int		ubuflen ;
{


	return sfio_reade(op,ubuf,ubuflen,-1,0) ;
}
/* end subroutine (sfio_read) */


int sfio_readlinetimed(op,lbuf,llen,to)
Sfio_t		*op ;
char		lbuf[] ;
int		llen ;
int		to ;
{
	int	rs = SR_OK ;
	int	rl = 0 ;


	if (op == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("sfio_readlinetimed: ent\n") ;
#endif

	    rl = sfreadlinetimed(op,lbuf,llen,to) ;
	    rs = (rl >= 0) ? rl : SR_HANGUP ;

#if	CF_DEBUGS
	debugprintf("sfio_readlinetimed: ret rs=%d rl=%u\n",rs,rl) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (sfio_readlinetimed) */


int sfio_readline(op,lbuf,llen)
Sfio_t		*op ;
char		lbuf[] ;
int		llen ;
{


	return sfio_readlinetimed(op,lbuf,llen,-1) ;
}
/* end subroutine (sfio_readline) */


int sfio_write(op,lbuf,llen)
Sfio_t		*op ;
const void	*lbuf ;
int		llen ;
{
	int	rs ;
	int	wlen = 0 ;


	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("sfio_write: ent\n") ;
	    if (llen < 0) llen = strlen(lbuf) ;
	debugprintf("sfio_write: wbuf=>%t<\n",
		lbuf,strlinelen(lbuf,llen,60)) ;
#endif

	    if (llen < 0) llen = strlen(lbuf) ;
	    if ((rs = sfwrite(op,lbuf,llen)) < 0) rs = SR_PIPE ;
	    wlen = rs ;

#if	CF_DEBUGS
	debugprintf("sfio_write: ret rs=%d\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (sfio_write) */


int sfio_printline(op,lbuf,llen)
Sfio_t		*op ;
const char	lbuf[] ;
int		llen ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;


	if (op == NULL)
	    return SR_FAULT ;

	    if (llen < 0)
	        llen = strlen(lbuf) ;

	    if (llen > 0) {
	        if ((rs = sfwrite(op,lbuf,llen)) < 0) rs = SR_PIPE ;
	        wlen += rs ;
	    }

	    if ((rs >= 0) && ((llen == 0) || (lbuf[llen-1] != '\n'))) {
	        char	eol[2] ;
	        eol[0] = '\n' ;
	        eol[1] = '\0' ;
	        if ((rs = sfwrite(op,eol,1)) < 0) rs = SR_PIPE ;
		wlen += rs ;
	    }

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (sfio_printline) */


int sfio_printf(Sfio_t *op,const char fmt[],...)
{
	va_list	ap ;

	const int	llen = LINEBUFLEN ;

	int	rs ;
	int	len ;
	int	wlen = 0 ;

	char	lbuf[LINEBUFLEN + 1] ;


	if (op == NULL)
	    return SR_FAULT ;

	va_begin(ap,fmt) ;
	rs = format(lbuf,llen,0,fmt,ap) ;
	len = rs ;
	va_end(ap) ;

#if	CF_DEBUGS
	debugprintf("sfio_printf: format() rs=%d\n",rs) ;
	if (rs >= 0)
	debugprintf("sfio_printf: r=>%t<\n",
		lbuf,strlinelen(lbuf,len,60)) ;
#endif

	if ((rs >= 0) && (len > 0)) {
		if ((rs = sfwrite(op,lbuf,len)) < 0) rs = SR_PIPE ;
		wlen = rs ;
	        if ((rs >= 0) && hasnl(lbuf,len))
		    sfsync(op) ;
	}

#if	CF_DEBUGS
	debugprintf("sfio_printf: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (sfio_printf) */


int sfio_putc(op,ch)
Sfio_t		*op ;
int		ch ;
{
	int	rs = SR_OK ;

	char	buf[2] ;


	if (op == NULL)
	    return SR_FAULT ;

	    buf[0] = ch ;
	    if ((rs = sfwrite(op,buf,1)) < 0)
	        rs = SR_PIPE ;
	    if ((rs >= 0) && (ch == '\n'))
		sfsync(op) ;

	return (rs >= 0) ? 1 : rs ;
}
/* end subroutine (sfio_putc) */


int sfio_seek(op,o,w)
Sfio_t		*op ;
Sfoff_t		o ;
int		w ;
{
	Sfoff_t	sfo ;
	int	rs = SR_OK ;


	if (op == NULL)
	    return SR_FAULT ;

	    sfo = sfseek(op,o,w) ;
	    rs = (sfo >= 0) ? (sfo&INT_MAX) : ((int) sfo) ;

	return rs ;
}
/* end subroutine (sfio_seek) */


int sfio_flush(op)
Sfio_t		*op ;
{
	int	rs = SR_OK ;


	if (op == NULL)
	    return SR_FAULT ;

	    rs = sfsync(op) ;

	return rs ;
}
/* end subroutine (sfio_flush) */


int sfio_control(Sfio_t *op,int cmd,...)
{
	va_list	ap ;
	int	rs = SR_OK ;
	int	sfcmd = 0 ;
	int	fd ;
	int	f ;


	if (op == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("sfio_control: ent cmd=%u\n",cmd) ;
#endif

	va_begin(ap,cmd) ;

	switch (cmd) {

	case SFIO_CNOP:
	    break ;

	case SFIO_CSETBUFWHOLE:
	    {
	        int	flags = SF_WHOLE ;
	        f = (int) va_arg(ap,int) ;
		sfcmd = (f) ? SFIO_ON : SFIO_OFF ;
	        rs = sfset(op,flags,sfcmd) ;
	    }
	    break ;

	case SFIO_CSETBUFLINE:
	    {
	        int	flags = SF_LINE ;
	        f = (int) va_arg(ap,int) ;
		sfcmd = (f) ? SFIO_ON : SFIO_OFF ;
	        rs = sfset(op,flags,sfcmd) ;
	    }
	    break ;

	case SFIO_CSETBUFUN:
	    {
	        int	flags = (SF_LINE | SF_WHOLE) ;
	        rs = sfset(op,flags,SFIO_OFF) ;
	    }
	    break ;

	case SFIO_CFD:
	        rs = sffileno(op) ;
	        if (rs < 0) rs = SR_NOTOPEN ;
	    {
		int	*aip = (int *) va_arg(ap,int *) ;
	        if (aip != NULL) *aip = rs ;
	    }
	    break ;

	case SFIO_CSETFLAGS:
	    {
	        int	flags = SF_LINE ;
	    f = (int) va_arg(ap,int) ;
		sfcmd = (f) ? SFIO_ON : SFIO_OFF ;
	        rs = sfset(op,flags,sfcmd) ;
	    }
	    break ;

	default:
	    rs = SR_INVALID ;
	    break ;

	} /* end switch */

	va_end(ap) ;

#if	CF_DEBUGS
	debugprintf("sfio_control: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sfio_control) */


int sfio_getfd(op)
Sfio_t		*op ;
{
	int		rs = SR_OK ;
	int		fd = -1 ;

	if (op == NULL) return SR_FAULT ;
	    rs = sffileno(op) ;
	    fd = rs ;
	    if (rs < 0) rs = SR_NOTOPEN ;
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (sfio_getfd) */


int sfio_readintr(op,ubuf,ulen,to,ip)
Sfio_t		*op ;
void		*ubuf ;
int		ulen ;
int		to ;
int		*ip ;
{
	int	rs = SR_TIMEDOUT ;


	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("sfio_readintr: ent to=%d\n",to) ;
#endif

	while ((rs == SR_TIMEDOUT) && ((to < 0) || (to-- >= 0))) {
	    if ((ip != NULL) && *ip) break ;

	    rs = sfio_reade(op,ubuf,ulen,1,FM_TIMED) ;

	} /* end while */

	if (rs == SR_TIMEDOUT) {
	    if ((ip != NULL) && *ip) rs = SR_INTR ;
	}

#if	CF_DEBUGS
	debugprintf("sfio_readintr: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sfio_readintr) */


static int hasnl(sp,sl)
const char	*sp ;
int		sl ;
{
	return (strnrchr(sp,sl,'\n') != NULL) ;
}
/* end subroutine (hasnl) */


