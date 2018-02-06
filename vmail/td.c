/* td */

/* Terminal Display (TD) library */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUGPROC	0		/* ? */
#define	CF_SAVERESTORE	0		/* implementing cursor-save-restore? */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This object package is finally finished!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object module facilitates managing a terminal display.

        This object contains routines (not unlike in the UNIX 'curses' library)
        to display information in "windows" on a terminal display. The major
        difference between this terminal display library and the standard UNIX
        'curses' library is that THIS LIBRARY WORKS!

	Other notes:

        An 'xterm' terminal is almost en entire complete 'vt102' terminal. It
        pretty much only lacks the blinking character attribute and the double
        height-width characters.


*******************************************************************************/


#define	TD_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<vecitem.h>
#include	<buffer.h>
#include	<ascii.h>
#include	<ansigr.h>
#include	<termstr.h>
#include	<localmisc.h>

#include	"td.h"


/* local defines */

#define	TD_MAGIC	0x02652103
#define	TD_BUFLEN	(8 * 1024)

#define	TD_WIN		struct td_window

#define	TD_RESTORELEN	4
#define	TD_SCROLLBUFLEN	20

/* terminal capabilities */
#define	TD_TCSCROLL	0x0001		/* supports scroll regions */
#define	TD_TCCOLOR	0x0002		/* supports color */
#define	TD_TCEIGHT	0x0004		/* 8-bit controls */
#define	TD_TCIL		0x0008		/* insert line */

/* aid for graphic rendition operations */
#define	TD_GRMASK	(TD_GRBOLD|TD_GRUNDER|TD_GRBLINK|TD_GRREV)

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	TERMCONSEQLEN
#define	TERMCONSEQLEN	84
#endif

#undef	CBUFLEN
#define	CBUFLEN		((4*4)+3)

#define	HEXBUFLEN	100

#ifndef	VARTERM
#define	VARTERM		"TERM"
#endif

#ifndef	NTABCOLS
#define	NTABCOLS	8		/* maximum number columns in a TAB */
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	tabcols(int,int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	vbufprintf(char *,int,const char *,va_list) ;
extern int	ctdecui(char *,int,uint) ;
extern int	termconseq(char *,int,int,int,int,int,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct termtype {
	const char	*term ;
	int		termcap ;	/* capabilities */
} ;


/* forward references */

int 		td_vprintf(TD *,int,const char *,va_list) ;
int 		td_vpprintf(TD *,int,int,int,const char *,va_list) ;
int 		td_printf(TD *,int,const char *,...) ;
int 		td_pprintf(TD *,int,int,int,const char *,...) ;
int 		td_pwrite(TD *,int,int,int,const char *,int) ;
int 		td_pwritegr(TD *,int,int,int,int,const char *,int) ;
int		td_suspend(TD *,int,int) ;

static int	td_startwin(TD *,int,int,int,int) ;
static int	td_flushmove(TD *,TD_WIN *,int,int) ;
static int	td_iflush(TD *) ;
static int	td_procstr(TD *,TD_WIN *,int,const char *,int) ;
static int	td_termstrbegin(TD *) ;
static int	td_termstrend(TD *) ;
static int	td_store(TD *,const char *,int) ;

#if	CF_SAVERESTORE
static int	td_isave(TD *) ;
static int	td_irestore(TD *) ;
#endif /* CF_SAVERESTORE */

#ifdef	COMMENT
static int	termmatch(const struct termtype *,const char *) ;
#endif

#if	CF_DEBUGS
extern int	mkhexstr(char *,int,const void *,int) ;
#endif


/* local variables */

#ifdef	COMMENT
/* 	TYPE	CAPABILITY	COLOR */
static const struct termtype	terms[] = {
	    { "vt100",	(TD_TCSCROLL) },
	    { "ansi",	(TD_TCSCROLL | TD_TCIL) },
	    { "xterm",	(TD_TCSCROLL | TD_TCIL | TD_TCCOLOR) },
	    { "screen",	(TD_TCSCROLL | TD_TCIL) },
	    { "vt220",	(TD_TCSCROLL | TD_TCIL | TD_TCEIGHT) },
	    { "vt240",	(TD_TCSCROLL | TD_TCIL | TD_TCEIGHT | TD_TCCOLOR) },
	    { "vt320",	(TD_TCSCROLL | TD_TCIL | TD_TCEIGHT) },
	    { "vt340",	(TD_TCSCROLL | TD_TCIL | TD_TCEIGHT | TD_TCCOLOR) },
	    { "vt420",	(TD_TCSCROLL | TD_TCIL | TD_TCEIGHT) },
	    { "vt440",	(TD_TCSCROLL | TD_TCIL | TD_TCEIGHT | TD_TCCOLOR) },
	    { "vt520",	(TD_TCSCROLL | TD_TCIL | TD_TCEIGHT) },
	    { "vt540",	(TD_TCSCROLL | TD_TCIL | TD_TCEIGHT | TD_TCCOLOR) },
	    { "vt101",	(TD_TCSCROLL | TD_TCIL) },
	    { "vt102",	(TD_TCSCROLL | TD_TCIL) },
	    { NULL, 0 }
	} ;
#endif /* COMMENT */



/* exported subroutines */


int td_start(TD *tdp,int tfd,const char *termtype,int r,int c)
{
	struct ustat	sb ;

	int	rs = SR_OK ;

	if (tdp == NULL) return SR_FAULT ;
	if (termtype == NULL) return SR_FAULT ;

	if (tfd < 0) return SR_BADF ;

	if (termtype[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("td_start: r=%d c=%d\n",r,c) ;
#endif

	if ((r < 0) || (c < 0)) return SR_INVALID ;

/* initial checks */

	memset(tdp,0,sizeof(TD)) ;

	if ((rs = u_fstat(tfd,&sb)) >= 0) {
	    if (isatty(tfd)) {
	        struct termios	termconf ;
	        tdp->tfd = tfd ;
	        tdp->termcap = 0 ;
	        tdp->buflen = TD_BUFLEN ;
	        tdp->curlen = 0 ;
	        tdp->rows = r ;
	        tdp->cols = c ;
	        tdp->f.statusdisplay = FALSE ;
	        tdp->f.eol = FALSE ;
	        tdp->f.linebuf = FALSE ;
	        if ((rs = uc_tcgetattr(tfd,&termconf)) >= 0) {
	            const char	*cp ;
	            tdp->f.nlcr = (termconf.c_oflag & ONLCR) ? TRUE : FALSE ;
	            if ((rs = uc_mallocstrw(termtype,-1,&cp)) >= 0) {
	                char	*bp ;
	                tdp->termtype = cp ;
	                if ((rs = uc_malloc(tdp->buflen,&bp)) >= 0) {
	                    const char	*tt = termtype ;
	                    tdp->buf = bp ;
	                    if ((rs = termstr_start(&tdp->enter,tt)) >= 0) {
	                        VECITEM		*wlp = &tdp->wins ;
	                        const int	opts = VECITEM_PHOLES ;
	                        if ((rs = vecitem_start(wlp,5,opts)) >= 0) {
	                            if ((rs = td_startwin(tdp,0,0,r,c)) >= 0) {
	                                tdp->cur.row = -1 ;
	                                tdp->cur.col = -1 ;
	                                tdp->magic = TD_MAGIC ;
	                            }
	                            if (rs < 0)
	                                vecitem_finish(wlp) ;
	                        } /* end if (vecitem_start) */
	                        if (rs < 0)
	                            termstr_finish(&tdp->enter) ;
	                    } /* end if (termstr_start) */
	                    if (rs < 0) {
	                        uc_free(tdp->buf) ;
	                        tdp->buf = NULL ;
	                    }
	                } /* end if (m-a) */
	                if (rs < 0) {
	                    uc_free(tdp->termtype) ;
	                    tdp->termtype = NULL ;
	                }
	            } /* end if (m-a) */
	        } /* end if (tcgetattr) */
	    } else
	        rs = SR_NOTSOCK ;
	} /* end if (stat) */

#if	CF_DEBUGS
	debugprintf("td_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (td_start) */


/* free up this whole object */
int td_finish(TD *tdp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (tdp == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

/* reset the scroll region if necessary */

	rs1 = td_suspend(tdp,-1,-1) ;
	if (rs >= 0) rs = rs1 ;

/* do one final flush */

	rs1 = td_iflush(tdp) ;
	if (rs >= 0) rs = rs1 ;

/* free the internal stuff */

	rs1 = vecitem_finish(&tdp->wins) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = termstr_finish(&tdp->enter) ;
	if (rs >= 0) rs = rs1 ;

	if (tdp->buf != NULL) {
	    rs1 = uc_free(tdp->buf) ;
	    if (rs >= 0) rs = rs1 ;
	    tdp->buf = NULL ;
	}

	if (tdp->termtype != NULL) {
	    rs1 = uc_free(tdp->termtype) ;
	    if (rs >= 0) rs = rs1 ;
	    tdp->termtype = NULL ;
	}

	tdp->magic = 0 ;
	return rs ;
}
/* end subroutine (td_finish) */


/* suspend the windowing object (like for escape or something like that) */
int td_suspend(TD *tdp,int r,int c)
{
	TD_WIN		*wp ;
	int		rs ;
	int		len = 0 ;

	if (tdp == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

/* reset scroll region */

	if ((rs = vecitem_get(&tdp->wins,0,&wp)) >= 0) {
	    if ((rs = td_termstrbegin(tdp)) >= 0) {

	        if ((rs >= 0) && tdp->f.smallscroll) {
	            tdp->f.smallscroll = FALSE ;
	            rs = termstr_ssr(&tdp->enter,0,tdp->rows) ;
	        }

	        if (rs >= 0)
	            rs = td_flushmove(tdp,wp,r,c) ;

	        if (rs >= 0) {
	            rs = td_termstrend(tdp) ;
	            len = rs ;
	        }
	    } /* end if (transaction) */
	    if (rs >= 0) rs = td_iflush(tdp) ;
	} /* end if (getting window) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (td_suspend) */


/* create a new subwindow */
int td_subnew(TD *tdp,int srow,int scol,int rows,int cols)
{
	int		rs ;

	if (tdp == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

	if ((srow < 0) || (scol < 0)) return SR_INVALID ;
	if ((rows < 0) || (cols < 0)) return SR_INVALID ;

/* create a new subwindow */

	rs = td_startwin(tdp,srow,scol,rows,cols) ;

	return rs ;
}
/* end subroutine (td_subnew) */


/* delete a subwindow */
int td_subdel(TD *tdp,int wn)
{
	int		rs ;

	if (tdp == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

	if (wn == 0) return SR_INVALID ;	/* cannot delete win==0 */

	if ((rs = td_iflush(tdp)) >= 0) {
	    if (wn < 0) {
	        TD_WIN	*wp ;
	        int	i ;
	        for (i = 1 ; vecitem_get(&tdp->wins,i,&wp) >= 0 ; i += 1) {
	            if (wp == NULL) continue ;
	            rs = vecitem_del(&tdp->wins,i) ;
	            if (rs < 0) break ;
	        } /* end for */
	    } else {
	        rs = vecitem_del(&tdp->wins,wn) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (td_subdel) */


int td_getlines(TD *tdp,int w)
{
	TD_WIN		*wp ;
	int		rs ;
	int		lines = 0 ;

	if (tdp == NULL) return SR_FAULT ;
	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

	if ((rs = vecitem_get(&tdp->wins,w,&wp)) >= 0) {
	    lines = wp->rows ;
	}

	return (rs >= 0) ? lines : rs ;
}
/* end subroutine (td_getlines) */


int td_setlines(TD *tdp,int w,int nlines)
{
	TD_WIN		*wp ;
	int		rs ;
	int		olines = 0 ;

	if (tdp == NULL) return SR_FAULT ;
	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

	if ((rs = vecitem_get(&tdp->wins,w,&wp)) >= 0) {
	    olines = wp->rows ;
	    wp->rows = nlines ;
	}

	return (rs >= 0) ? olines : rs ;
}
/* end subroutine (td_setlines) */


/* execute a control action */
/* ARGSUSED */ /* future use? */
int td_control(TD *tdp,int cmd,int arg1,int arg2)
{
	int		rs = SR_OK ;

	if (tdp == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

	switch (cmd) {
	case TD_CCURSOR:
	case TD_CEOL:
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */

	return rs ;
}
/* end subroutine (td_control) */


/* make a cursor screen move */
int td_move(TD *tdp,int wn,int r,int c)
{
	TD_WIN		*wp ;
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (tdp == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

	if ((r >= 0) || (c >= 0)) {
	    if ((rs = vecitem_get(&tdp->wins,wn,&wp)) >= 0) {
	        if ((r < wp->rows) && (c < wp->cols)) {

	            if (r >= 0) {
	                f = TRUE ;
	                wp->move.row = r ;
	            }
	            if (c >= 0) {
	                f = TRUE ;
	                wp->move.col = c ;
	            }
	            if (f)
	                wp->move.timecount = tdp->timecounter++ ;

#if	CF_DEBUGS
	            debugprintf("td_move: w=%u saved move r=%d c=%d\n",
	                wn,wp->move.row,wp->move.col) ;
#endif

	        } /* end if (ok) */
	    } /* end if (get-window-pointer) */
	} /* end if (need something) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (td_move) */


/* scroll a window ( (n>0)==UP, (n<0)==DOWN ) */
int td_scroll(TD *tdp,int wn,int n)
{
	int		rs = SR_OK ;

	if (tdp == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

	if (n != 0) {
	    TD_WIN	*wp ;
	    if ((rs = vecitem_get(&tdp->wins,wn,&wp)) >= 0) {
	        int		index_line ;
	        int		i ;
	        int		na ;
	        int		f = FALSE ;
	        char		index_chars[8] ;

/* prepare a move for after the entire scroll operation */

#if	CF_DEBUGS
	        debugprintf("td_scroll: w=%u saved move r=%d c=%d\n",
	            wn,wp->move.row, wp->move.col) ;
#endif

/* do it */

	        tdp->cur.row = -1 ; /* indicates unknown cursor location */
	        tdp->cur.col = -1 ; /* indicates unknown cursor location */

	        na = abs(n) ;
	        if (na > wp->rows)
	            na = wp->rows ;

	        if ((rs = td_termstrbegin(tdp)) >= 0) {
	            TERMSTR	*tsp = &tdp->enter ;

	            if ((rs = termstr_ssr(tsp,wp->srow,wp->rows)) >= 0) {
/* move to the boundary of the scroll region for the proper direction */
	                if (n >= 0) {
	                    index_line = (wp->srow + wp->rows - 1) ;
	                    strcpy(index_chars,"\033D") ;	/* UP */
	                } else {
	                    index_line = wp->srow ;
	                    strcpy(index_chars,"\033M") ;	/* DOWN */
	                }
	            } /* end if */

	            if (rs >= 0) {
	                if ((rs = termstr_curh(tsp,index_line,-1)) >= 0) {
	                    for (i = 0 ; (rs >= 0) && (i < na) ; i += 1) {
	                        rs = termstr_write(tsp,index_chars,-1) ;
	                    }
	                    if (rs >= 0) {
	                        if ((rs = termstr_ssr(tsp,0,tdp->rows)) >= 0) {
	                            rs = td_termstrend(tdp) ;
	                        }
	                    }
	                }
	            }

	        } /* end if (transaction) */

	    } /* end if (vecitem_get) */
	} /* end if (not-zero) */

	return rs ;
}
/* end subroutine (td_scroll) */


/* position print-formatted */
int td_pprintf(TD *tdp,int wn,int r,int c,cchar *fmt,...)
{
	int		rs ;
	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = td_vpprintf(tdp,wn,r,c,fmt,ap) ;
	    va_end(ap) ;
	}
	return rs ;
}
/* end subroutine (td_pprintf) */


/* regular print-formatted */
int td_printf(TD *tdp,int wn,cchar *fmt,...)
{
	int		rs ;
	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = td_vpprintf(tdp,wn,-1,-1,fmt,ap) ;
	    va_end(ap) ;
	}
	return rs ;
}
/* end subroutine (td_printf) */


int td_vprintf(TD *tdp,int wn,cchar *fmt,va_list ap)
{
	return td_vpprintf(tdp,wn,-1,-1,fmt,ap) ;
}
/* end subroutine (td_vprintf) */


/* vprintf-like thing */
int td_vpprintf(TD *tdp,int wn,int r,int c,cchar *fmt,va_list ap)
{
	const int	llen = LINEBUFLEN ;
	int		rs ;
	char		lbuf[LINEBUFLEN + 1] ;

	if (tdp == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

	if ((rs = vbufprintf(lbuf,llen,fmt,ap)) > 0) {
	    rs = td_pwrite(tdp,wn,r,c,lbuf,rs) ;
	}

	return rs ;
}
/* end subroutine (td_vpprintf) */


int td_write(TD *tdp,int wn,cchar *wbuf,int wlen)
{
	return td_pwrite(tdp,wn,-1,-1,wbuf,wlen) ;
}
/* end subroutine (td_write) */


/* position-write */
int td_pwrite(TD *tdp,int wn,int r,int c,cchar *wbuf,int wlen)
{
	return td_pwritegr(tdp,wn,r,c,0,wbuf,wlen) ;
}
/* end subroutine (td_pwrite) */


/* position-write w/ graphic-rendition */
int td_pwritegr(TD *tdp,int wn,int r,int c,int gr,cchar *wbuf,int wlen)
{
	TD_WIN		*wp ;
	int		rs ;
	int		len = 0 ;

	if (tdp == NULL) return SR_FAULT ;
	if (wbuf == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

#ifdef	OPTIONAL
	if (wn < 0) return SR_INVALID ;
#endif

	if (wlen < 0)
	    wlen = strlen(wbuf) ;

#if	CF_DEBUGS
	{
	    char	hexbuf[HEXBUFLEN + 1] ;
	    debugprintf("td_pwritegr: w=%u r=%d c=%d\n",wn,r,c) ;
	    debugprintf("td_pwritegr: gr=\\x%04X\n",gr) ;
	    debugprintf("td_pwritegr: gr=\\b%4ß\n",gr) ;
	    mkhexstr(hexbuf,HEXBUFLEN,wbuf,MIN(wlen,16)) ;
	    debugprintf("td_pwritegr: >%s<\n",hexbuf) ;
	}
#endif /* CF_DEBUGS */

/* is it a valid window number? */

	if ((rs = vecitem_get(&tdp->wins,wn,&wp)) >= 0) {
	    const int	nrow = (r >= 0) ? r : wp->move.row ;
	    const int	ncol = (c >= 0) ? c : wp->move.col ;
	    if ((nrow < wp->rows) && (ncol < wp->cols)) {
	        if ((rs = td_termstrbegin(tdp)) >= 0) {

	            rs = td_flushmove(tdp,wp,nrow,ncol) ;

	            if (rs >= 0)
	                rs = td_procstr(tdp,wp,gr,wbuf,wlen) ;

	            if (rs >= 0) {
	                rs = td_termstrend(tdp) ;
	                len = rs ;
	            }
	        } /* end if (transaction) */
	    } /* end if (ok) */
	} /* end if (get-window-pointer) */

#if	CF_DEBUGS
	debugprintf("td_pwritegr: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (td_pwritegr) */


/* clear a window */
int td_clear(TD *tdp,int w)
{
	TD_WIN		*wp ;
	int		rs ;
	int		len = 0 ;

	if (tdp == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("td_clear: w=%d\n",w) ;
#endif

	if ((rs = vecitem_get(&tdp->wins,w,&wp)) >= 0) {
	    if ((rs = td_termstrbegin(tdp)) >= 0) {

	        if ((rs = td_flushmove(tdp,wp,-1,-1)) >= 0) {

	            if (w == 0) {
	                rs = termstr_ed(&tdp->enter,2) ;
	            } else {
	                int	i ;
	                for (i = 0 ; (rs >= 0) && (i < wp->rows) ; i += 1) {
	                    rs = termstr_el(&tdp->enter,2) ;
	                    if (rs >= 0)
	                        rs = termstr_char(&tdp->enter,'\n') ;
	                }
	            } /* end if */

	        } /* end if (flushmove) */

	        if (rs >= 0) {
	            rs = td_termstrend(tdp) ;
	            len = rs ;
	        }
	    } /* end if (transaction) */
	} /* end if (get-window-pointer) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (td_clear) */


/* erase-window */
/* type: 0=forward, 1=back, 2=whole */
int td_ew(TD *tdp,int w,int r,int type)
{
	TD_WIN		*wp ;
	int		rs ;
	int		len = 0 ;

	if (tdp == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("td_ew: w=%d t=%d\n",w,type) ;
#endif

	if ((rs = vecitem_get(&tdp->wins,w,&wp)) >= 0) {
	    if ((rs = td_termstrbegin(tdp)) >= 0) {

	        rs = td_flushmove(tdp,wp,r,-1) ;

	        if (rs >= 0)
	            rs = termstr_ed(&tdp->enter,type) ;

	        if (rs >= 0) {
	            rs = td_termstrend(tdp) ;
	            len = rs ;
	        }
	    } /* end if (transaction) */
	} /* end if (get-window-pointer) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (td_ew) */


/* erase-line */
int td_el(TD *tdp,int w,int type)
{
	TD_WIN		*wp ;
	int		rs ;
	int		len = 0 ;

	if (tdp == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("td_el: w=%d t=%d\n",w,type) ;
#endif

	if ((rs = vecitem_get(&tdp->wins,w,&wp)) >= 0) {
	    if ((rs = td_termstrbegin(tdp)) >= 0) {

	        rs = td_flushmove(tdp,wp,-1,-1) ;

	        if (rs >= 0)
	            rs = termstr_el(&tdp->enter,type) ;

	        if (rs >= 0) {
	            rs = td_termstrend(tdp) ;
	            len = rs ;
	        }
	    } /* end if (transaction) */
	} /* end if (get-window-pointer) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (td_el) */


/* erase-character */
int td_ec(TD *tdp,int w,int n)
{
	TD_WIN		*wp ;
	int		rs ;
	int		len = 0 ;

	if (tdp == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("td_ec: w=%d n=%d\n",w,n) ;
#endif

	if ((rs = vecitem_get(&tdp->wins,w,&wp)) >= 0) {
	    if ((rs = td_termstrbegin(tdp)) >= 0) {

	        rs = td_flushmove(tdp,wp,-1,-1) ;

	        if (rs >= 0)
	            rs = termstr_ec(&tdp->enter,n) ;

	        if (rs >= 0) {
	            rs = td_termstrend(tdp) ;
	            len = rs ;
	        }
	    } /* end if (transaction) */
	} /* end if (get-window-pointer) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (td_ec) */


/* flush buffer */
int td_flush(TD *tdp)
{

	if (tdp == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

	return td_iflush(tdp) ;
}
/* end subroutine (td_flush) */


/* terminal window size change */
int td_setsize(TD *tdp,int rows,int cols)
{
	TD_WIN		*wp ;
	int		rs ;

	if (tdp == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

	if ((rows < 0) || (cols < 0)) return SR_INVALID ;

/* is it a valid window number? */

	if ((rs = vecitem_get(&tdp->wins,0,&wp)) >= 0) {
	    wp->rows = rows ;
	    wp->cols = cols ;
	} /* end if (get-window-pointer) */

	return rs ;
}
/* end subroutine (td_setsize) */


/* check */
int td_check(TD *tdp)
{
	int		rs ;

	if (tdp == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

	if ((rs = td_flushmove(tdp,0,-1,-1)) >= 0) {
	    rs = td_iflush(tdp) ;
	}

	return rs ;
}
/* end subroutine (td_check) */


int td_position(TD *tdp,int wn,TD_POSITION *pp)
{
	TD_WIN		*wp ;
	int		rs ;

	if (tdp == NULL) return SR_FAULT ;
	if (pp == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

	memset(pp,0,sizeof(TD_POSITION)) ;

	if ((rs = vecitem_get(&tdp->wins,wn,&wp)) >= 0) {
	    pp->timecount = wp->cur.timecount ;
	    pp->row = wp->cur.row ;
	    pp->col = wp->cur.col ;
	} /* end if (get-window-pointer) */

	return rs ;
}
/* end subroutine (td_position) */


/* private subroutines */


/* create and initialize a new sub-window */
static int td_startwin(TD *tdp,int srow,int scol,int rows,int cols)
{
	TD_WIN		win ;
	int		rs = SR_OK ;
	int		size ;
	int		f = FALSE ;

	f = f || (srow >= tdp->rows) ;
	f = f || ((srow + rows) > tdp->rows) ;
	f = f || (scol >= tdp->cols) ;
	f = f || ((scol + cols) > tdp->cols) ;
	if (f) {
	    rs = SR_DOM ;
	}

	if (rs >= 0) {
	    memset(&win,0,sizeof(TD_WIN)) ;
	    win.srow = MIN(srow,(tdp->rows-1)) ;
	    win.scol = MIN(scol,(tdp->cols-1)) ;
	    win.rows = MIN(rows,tdp->rows) ;
	    win.cols = MIN(cols,tdp->cols) ;
	    win.move.row = -1 ;
	    win.move.col = -1 ;
	    size = sizeof(TD_WIN) ;
	    rs = vecitem_add(&tdp->wins,&win,size) ;
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (td_startwin) */


/* flush buffer */
static int td_iflush(TD *tdp)
{
	TD_WIN		*wp ;
	TD_WIN		*mwp = NULL ;
	uint		mtimecount = 0 ;
	int		rs = SR_OK ;
	int		i ;

	if (tdp == NULL) return SR_FAULT ;

	if (tdp->magic != TD_MAGIC) return SR_NOTOPEN ;

/* find the latest outstanding move */

	for (i = 0 ; vecitem_get(&tdp->wins,i,&wp) >= 0 ; i += 1) {
	    if (wp == NULL) continue ;
	    if ((wp->move.row >= 0) || (wp->move.col >= 0)) {
	        if (wp->move.timecount > mtimecount) {
	            mtimecount = wp->move.timecount ;
	            mwp = wp ;
	        }
	    }
	} /* end for */

	if (mwp != NULL)
	    rs = td_flushmove(tdp,mwp,-1,-1) ;

	if ((rs >= 0) && (tdp->curlen > 0)) {
	    rs = u_write(tdp->tfd,tdp->buf,tdp->curlen) ;
	    tdp->curlen = 0 ;
	}

	return rs ;
}
/* end subroutine (td_iflush) */


#if	CF_SAVERESTORE

/* set the save-cursor stuff in the buffer */
static int td_isave(TD *tdp)
{
	int		rs ;
	int		len = 0 ;

#ifdef	COMMENT
	rs = termstr_csr(&tdp->enter,TRUE) ;
	if (rs >= 0)
	    rs = td_istore(tdp) ;
	return (rs >= 0) ? len : rs ;
#else
	cp = strwcpy(tdp->buf,TERMSTR_RESTORE,-1) ;
	tdp->curlen = (cp - tdp->buf) ;
	return tdp->curlen ;
#endif
}
/* end subroutine (td_isave) */


/* set the restore-cursor stuff in the buffer */
static int td_irestore(TD *tdp)
{
	cchar		*cp = strwcpy(tdp->buf,TERMSTR_RESTORE,-1) ;
	tdp->curlen = (cp - tdp->buf) ;
	return tdp->curlen ;
}
/* end subroutine (td_irestore) */

#endif /* CF_SAVERESTORE */


/* flush out any accumulated moves */
static int td_flushmove(TD *tdp,TD_WIN *wp,int r,int c)
{
	int		rs = SR_OK ;
	int		nrow, ncol ;

#if	CF_DEBUGS
	debugprintf("td_flushmove: enter r=%d c=%d\n",r,c) ;
#endif

	nrow = (r >= 0) ? r : wp->move.row ;
	ncol = (c >= 0) ? c : wp->move.col ;

#if	CF_DEBUGS
	debugprintf("td_flushmove: nr=%d nc=%d\n",nrow,ncol) ;
#endif

	if ((nrow < wp->rows) && (ncol < wp->cols)) {
	    int		wrow, wcol ;

	    wrow = (nrow >= 0) ? nrow : wp->cur.row ;
	    wcol = (ncol >= 0) ? ncol : wp->cur.col ;

#if	CF_DEBUGS
	    debugprintf("td_flushmove: wr=%d wc=%d\n",wrow,wcol) ;
#endif

	    if ((wrow < wp->rows) && (wcol < wp->cols)) {
	        int		arow, acol ;
	        int		f_chosen = FALSE ;

/* relocate to absolute coordinates */

	        arow = wrow + wp->srow ;
	        acol = wcol + wp->scol ;

#if	CF_DEBUGS
	        debugprintf("td_flushmove: ar=%d ac=%d\n",arow,acol) ;
#endif

/* search for optimizations */

#if	CF_DEBUGS
	        debugprintf("td_flushmove: car=%d cac=%d\n",
	            tdp->cur.row,tdp->cur.col) ;
#endif

	        if ((tdp->cur.row >= 0) && (tdp->cur.col >= 0)) {

	            if ((arow == tdp->cur.row) && (acol == tdp->cur.col)) {
	                f_chosen = TRUE ;

#if	CF_DEBUGS
	                debugprintf("td_flushmove: choice-> no action needed\n") ;
#endif

	            }

	            if ((! f_chosen) && 
	                (arow == tdp->cur.row) && (acol == 0)) {
	                f_chosen = TRUE ;
	                rs = termstr_char(&tdp->enter,'\r') ;

#if	CF_DEBUGS
	                debugprintf("td_flushmove: termstr_char() rs=%d\n",rs) ;
	                debugprintf("td_flushmove: choice-> stored CR\n") ;
#endif

	            }

	            if ((! f_chosen) && 
	                (arow == tdp->cur.row) && 
	                (tdp->cur.col > 0) && (acol == (tdp->cur.col-1))) {
	                f_chosen = TRUE ;
	                rs = termstr_char(&tdp->enter,'\b') ;

#if	CF_DEBUGS
	                debugprintf("td_flushmove: termstr_char() rs=%d\n",rs) ;
	                debugprintf("td_flushmove: choice-> stored BS \n") ;
#endif

	            }

	            if ((! f_chosen) && 
	                (acol == tdp->cur.col) && (arow == (tdp->cur.row+1))) {
	                f_chosen = TRUE ;
#ifdef	COMMENT
	                rs = termstr_char(&tdp->enter,CH_IND) ;
#else
	                rs = termstr_write(&tdp->enter,"\033D",2) ;
#endif

#if	CF_DEBUGS
	                debugprintf("td_flushmove: termstr_write() rs=%d\n",rs) ;
	                debugprintf("td_flushmove: choice-> stored IND \n") ;
#endif

	            }

	        } /* end if (optimization) */

	        if (! f_chosen) {
	            f_chosen = TRUE ;

#if	CF_DEBUGS
	            debugprintf("td_flushmove: termstr_curh() r=%d c=%d\n",
	                arow,acol) ;
#endif

	            rs = termstr_curh(&tdp->enter,arow,acol) ;

#if	CF_DEBUGS
	            debugprintf("td_flushmove: termstr_curh() rs=%d\n",rs) ;
	            debugprintf("td_flushmove: choice-> cursor-position\n") ;
#endif

	        } /* end if */

	        if (rs >= 0) {
	            wp->cur.row = wrow ;
	            wp->cur.col = wcol ;
	            tdp->cur.row = arow ;
	            tdp->cur.col = acol ;
	            wp->move.row = -1 ;
	            wp->move.col = -1 ;
	        }

	    } /* end if (needed) */

	} /* end if (needed) */

#if	CF_DEBUGS
	debugprintf("td_flushmove: ret car=%d cac=%d\n",
	    tdp->cur.row, tdp->cur.col) ;
	debugprintf("td_flushmove: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (td_flushmove) */


/* process the special C-language string-escape characters */
static int td_procstr(TD *tdp,TD_WIN *wp,int gr,cchar *sbuf,int slen)
{
	const int	grmask = TD_GRMASK ;
	const int	clen = CBUFLEN ;
	int		rs = SR_OK ;
	int		sl ;
	int		cl ;
	int		ch ;
	int		irows, icols ;
	int		len = 0 ;
	int		f_cr = FALSE ;
	int		f_inrow = TRUE ;
	int		f_incol = TRUE ;
	int		f_gr = FALSE ;
	const char	*sp ;
	char		cbuf[CBUFLEN+1] ;

	sp = sbuf ;
	sl = slen ;
	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("td_procstr: cwr=%u cwc=%u\n",
	    wp->cur.row,wp->cur.col) ;
	debugprintf("td_procstr: car=%u cac=%u\n",
	    tdp->cur.row,tdp->cur.col) ;
#endif

	if ((sl > 0) && (gr & grmask)) {
	    int	a1 = (gr & TD_GRBOLD) ? ANSIGR_BOLD : -1 ;
	    int	a2 = (gr & TD_GRUNDER) ? ANSIGR_UNDER : -1 ;
	    int	a3 = (gr & TD_GRBLINK) ? ANSIGR_BLINK : -1 ;
	    int	a4 = (gr & TD_GRREV) ? ANSIGR_REV : -1 ;
	    f_gr = TRUE ;
	    rs = termconseq(cbuf,clen,'m',a1,a2,a3,a4) ;
	    cl = rs ;
	    if (rs >= 0) {
	        rs = termstr_write(&tdp->enter,cbuf,cl) ;
	        len += rs ;
	    }
	} /* end if */

	while ((rs >= 0) && (sl > 0) && *sp) {

	    f_cr = FALSE ;
	    irows = 0 ;
	    icols = 0 ;

	    f_inrow = (wp->cur.row < wp->rows) ;
	    f_incol = (wp->cur.col < wp->cols) ;

	    ch = (*sp & 0xff) ;
	    switch (ch) {

	    case '\v':
	        if (f_inrow && f_incol) {
	            rs = termstr_el(&tdp->enter,-1) ;
	            len += rs ;
	        }
	        break ;

	    case '\f':
	        if (f_inrow && f_incol) {
	            rs = termstr_ed(&tdp->enter,-1) ;
	            len += rs ;
	        }
	        break ;

	    case '\t':
	        if (f_inrow && f_incol) {
	            icols = tabcols(NTABCOLS,tdp->cur.col) ;
	            rs = termstr_char(&tdp->enter,'\t') ;
	            len += rs ;
	        }
	        break ;

	    case CH_CSI:
	    case CH_ESC:
	    case CH_BEL:
	    case CH_SO:
	    case CH_SI:
	    case CH_SS2:
	    case CH_SS3:
	        if (f_inrow && f_incol) {
	            rs = termstr_char(&tdp->enter,ch) ;
	            len += rs ;
	        }
	        break ;

	    case CH_DEL:
	        break ;

	    case '\r':
	        if (f_inrow) {
	            f_cr = TRUE ;
	            rs = termstr_char(&tdp->enter,ch) ;
	            len += rs ;
	        }
	        break ;

	    case '\n':
	        if (f_inrow) {
	            irows = 1 ;
	            f_cr = tdp->f.nlcr ;
	            if (! f_cr) {
	                rs = termstr_char(&tdp->enter,'\r') ;
	                len += rs ;
	            }
	            if ((rs >= 0) && (wp->cur.row < (wp->rows-1))) {
	                rs = termstr_char(&tdp->enter,ch) ;
	                len += rs ;
	            }
	        }
	        break ;

	    case CH_IND:
	        if (f_inrow) {
	            irows = 1 ;
	            if (wp->cur.row < (wp->rows-1)) {
	                rs = termstr_char(&tdp->enter,ch) ;
	                len += rs ;
	            }
	        }
	        break ;

	    case '\b':
	        if (wp->cur.col > 0) {
	            f_incol = (tdp->cur.col < tdp->cols) ;
	            if (f_incol) {
	                tdp->cur.col -= 1 ;
	                rs = termstr_char(&tdp->enter,ch) ;
	                len += rs ;
	            }
	            wp->cur.col -= 1 ;
	        }
	        break ;

	    default:
	        if (f_inrow && ((ch & 0x7f) >= 0x20)) {
	            icols = 1 ;
	            if (f_incol) {
	                rs = termstr_char(&tdp->enter,ch) ;
	                len += rs ;
	            }
	        }
	        break ;

	    } /* end switch */

	    if (rs >= 0) {
	        sp += 1 ;
	        sl -= 1 ;
	        if (irows > 0) {
	            int	n, brows = irows ;
	            if ((wp->cur.row + irows) > (wp->rows - 1)) {
	                n = (wp->cur.row + irows) - (wp->rows - 1) ;
	                brows -= n ;
	            }
	            wp->cur.row += irows ;
	            tdp->cur.row = MIN((tdp->cur.row + brows),(tdp->rows-1)) ;
	        }
	        if (f_cr) {
	            f_cr = FALSE ;
	            wp->cur.col = 0 ;
	            tdp->cur.col = 0 ;
	        } else if (icols > 0) {
	            int	n, bcols = icols ;
	            if ((wp->cur.col + icols) > (wp->cols - 1)) {
	                n = (wp->cur.col + icols) - (wp->cols - 1) ;
	                bcols -= n ;
	            }
	            wp->cur.col += icols ;
	            tdp->cur.col = MIN((tdp->cur.col + bcols),(tdp->cols-1)) ;
	        }
	    } /* end if */

	} /* end while */

	if ((rs >= 0) && f_gr) {
	    int	a1 = ANSIGR_OFFALL ;
	    rs = termconseq(cbuf,clen,'m',a1,-1,-1,-1) ;
	    cl = rs ;
	    if (rs >= 0) {
	        rs = termstr_write(&tdp->enter,cbuf,cl) ;
	        len += rs ;
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("td_procstr: ret cwr=%u cwc=%u\n",
	    wp->cur.row,wp->cur.col) ;
	debugprintf("td_procstr: ret car=%u cac=%u\n",
	    tdp->cur.row,tdp->cur.col) ;
	debugprintf("td_procstr: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (td_procstr) */


static int td_termstrbegin(TD *tdp)
{
	int		rs ;

	rs = termstr_clean(&tdp->enter) ; /* clear any buffered actions */
	return rs ;
}
/* end subroutine (td_termstrbegin) */


static int td_termstrend(TD *tdp)
{
	int		rs ;
	int		len = 0 ;
	const char	*bp ;

	if ((rs = termstr_get(&tdp->enter,&bp)) >= 0) {
	    int	bl = rs ;

#if	CF_DEBUGS
	    {
	        char	hexbuf[HEXBUFLEN + 1] ;
	        mkhexstr(hexbuf,HEXBUFLEN,bp,MIN(bl,16)) ;
	        debugprintf("td_termstrend: l=%u >%s<\n",bl,hexbuf) ;
	    }
#endif

	    rs = td_store(tdp,bp,bl) ;
	    len = rs ;
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (td_termstrend) */


static int td_store(TD *tdp,cchar *bp,int bl)
{
	int		rs = SR_OK ;
	int		rlen ;
	int		len = 0 ;

	if (bl < 0) bl = strlen(bp) ;

	rlen = (tdp->buflen - tdp->curlen) ;
	if (bl > rlen) {
	    rs = u_write(tdp->tfd,tdp->buf,tdp->curlen) ;
	    tdp->curlen = 0 ;
	}

	if ((rs >= 0) && (bl > 0)) {
	    if (bl > tdp->buflen) {
	        rs = u_write(tdp->tfd,bp,bl) ;
	        len += rs ;
	    } else {
	        rlen = (tdp->buflen - tdp->curlen) ;
	        rs = snwcpy((tdp->buf + tdp->curlen),rlen,bp,bl) ;
	        if (rs >= 0) tdp->curlen += rs ;
	        len += rs ;
	    }
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (td_store) */


#ifdef	COMMENT

/* match a terminal name with our list */
static int termmatch(const struct termtype *list,const char *term)
{
	int		i ;
	for (i = 0 ; list[i].term != NULL ; i += 1) {
	    if (strcmp(term,list[i].term) == 0) break ;
	} /* end for */
	return (list[i].term != NULL) ? i : -1 ;
}
/* end subroutine (termmatch) */

#endif /* COMMENT */


