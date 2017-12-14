/* display */

/* manage the display for VMAIL */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */
#define	CF_SCANFILLOUT	1		/* blank out end of scan lines */
#define	CF_SFEND	0		/* compile 'sfend()' */
#define	CF_BLINKER	1		/* try to blink colons (for time) */


/* revision history:

	= 1998-01-10 David A.D. Morano
        This is a complete rewrite of the previous set of subroutines (might not
        be able to even call them an object) of the same name. This present
        module does fairly well qualify indeed as an object.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine displays the message to the screen.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<ascii.h>
#include	<char.h>
#include	<sbuf.h>
#include	<tmtime.h>
#include	<sntmtime.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"ds.h"
#include	"display.h"


/* local defines */

#define	DISPLAY_NDASHES		16
#define	DISPLAY_NBLANKS		20
#define	DISPLAY_STZSTR		(COL_DATESTR+4+5)
#define	DISPLAY_STZLEN		(DISPLAY_LTZNAME + 2)
#define	DISPLAY_MBSTRLEAD	"mb: "
#define	DISPLAY_VALS		struct display_vals

#define	SCANLINE		struct scanline

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY		100
#endif

#ifndef	NTABCOLS
#define	NTABCOLS	8
#endif

#define	SCANBUFLEN	((DISPLAY_LSCANLINE*2) + 20)
#define	DISPBUFLEN	((DISPLAY_LSCANLINE*2) + 20)

#define	NMSGBUFLEN	12

#define	LINESBUFLEN	(4+1)

#define	DCH_MARK	'*'
#define	DCH_POINT	'>'

#ifndef	COL_TZNAME
#define	COL_TZNAME	(DISPLAY_STZSTR+1)
#endif


/* external subroutines */

extern int	sfsubstance(cchar *,int,cchar **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	ctdeci(char *,int,int) ;
extern int	ctdecui(char *,int,uint) ;
extern int	ctdecpi(char *,int,int,int) ;
extern int	iceil(int,int) ;
extern int	strwcmp(const char *,const char *,int) ;
extern int	charcols(int,int,int) ;
extern int	mkdisphdr(char *,int,cchar *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	vbufprintf(char *,int,const char *,va_list) ;
extern int	isprintlatin(int) ;
extern int	isdigitlatin(int) ;

extern int	mkdisplayable(char *,int,cchar *,int) ;
extern int	digit3(char *,int) ;
extern int	compactstr(char *,int) ;
extern int	tabexpand(char *,int,const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strsub(cchar *,cchar *) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnrpbrk(const char *,int,const char *) ;
extern char	*strwcpyblanks(char *,int) ;	/* NUL-terminaed */
extern char	*strncpyblanks(char *,int) ;	/* not NUL-terminated */
extern char	*strwset(char *,int,int) ;
extern char	*strnset(char *,int,int) ;
extern char	*strdfill1(char *,int,cchar *) ;
extern char	*strdfill2(char *,int,cchar *,cchar *) ;
extern char	*strdfill3(char *,int,cchar *,cchar *,cchar *) ;


/* external variables */


/* local structures */

struct scanline {
	int		mark ;		/* the "mark" */
	char		*data ;		/* all data preformatted */
} ;

struct scantitle {
	char		*name ;
	int		col ;
} ;


/* forward references */

int		display_refresh(DISPLAY *) ;
int		display_rtop(DISPLAY *) ;
int		display_rmid(DISPLAY *) ;
int		display_rbot(DISPLAY *) ;
int		display_rscantitle(DISPLAY *) ;
int		display_rscan(DISPLAY *,int) ;
int		display_beep(DISPLAY *) ;
int		display_scanblanks(DISPLAY *,int) ;

static int	display_linesload(DISPLAY *,int,int) ;
static int	display_linescalc(DISPLAY *) ;
static int	display_subwinbegin(DISPLAY *) ;
static int	display_subwinend(DISPLAY *) ;
static int	display_subwinadj(DISPLAY *) ;
static int	display_scanfins(DISPLAY *) ;
static int	display_rscanscroll(DISPLAY *,int) ;
static int	display_scanprint(DISPLAY *,int) ;
static int	display_nnbscans(DISPLAY *,int,int) ;
static int	display_setstzone(DISPLAY *) ;
static int	display_scantitlemk(DISPLAY *) ;
static int	display_scanpointclear(DISPLAY *,int,int) ;
static int	display_scanpointset(DISPLAY *,int,int,int) ;
static int	display_scanmarkset(DISPLAY *,int,int) ;

static int	display_botloadnum(DISPLAY *,int) ;
static int	display_botloadfrom(DISPLAY *,const char *) ;
static int	display_botloadline(DISPLAY *,int) ;
static int	display_botloadlines(DISPLAY *,int) ;

static int	display_hdcheck(DISPLAY *,int) ;
static int	display_hd(DISPLAY *) ;
static int	display_hdfin(DISPLAY *) ;
static int	display_scancvtbegin(DISPLAY *,DISPLAY_SS *,DISPLAY_SDATA *) ;
static int	display_scancvtend(DISPLAY *,DISPLAY_SS *,DISPLAY_SDATA *) ;
static int	display_scancvtalt(DISPLAY *,DISPLAY_SDATA *) ;
static int	display_hdcvt(DISPLAY *,cchar *,int) ;

static int	scanline_start(SCANLINE *,DISPLAY_SDATA *) ;
static int	scanline_fill(SCANLINE *,DISPLAY_SDATA *) ;
static int	scanline_blank(SCANLINE *) ;
static int	scanline_mark(SCANLINE *,int) ;
static int	scanline_setlines(SCANLINE *,int) ;
static int	scanline_checkblank(SCANLINE *) ;
static int	scanline_finish(SCANLINE *) ;

static int	nstrcols(int,int,const char *,int) ;

#if	CF_SFEND
static int	sfend(const char *,int,int,const char **) ;
#endif

#if	CF_DEBUGS
static char	*stridig(char *,int,int) ;
#endif /* CF_DEBUGS */


/* local variables */

#if	CF_BLINKER
#else /* CF_BLINKER */

static const char	*months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	NULL
} ;

static const char	*days[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", NULL
} ;

#endif /* CF_BLINKER */

static const char	dashes[] = "----------------" ;

static const char	blanks[] = "                    " ;

static const struct scantitle	scantitles[] = {
	{ "FROM", COL_SCANFROM },
	{ "SUBJECT", COL_SCANSUBJECT },
	{ "DATE", COL_SCANDATE },
	{ "LINES", COL_SCANLINES },
	{ NULL, 0 }
} ;

enum scantitles {
	scantitle_from,
	scantitle_subject,
	scantitle_date,
	scantitle_lines,
	scantitle_overlast
} ;


/* exported subroutines */


int display_start(DISPLAY *op,PROGINFO *pip,DISPLAY_ARGS *dap)
{
	const int	cols = COLUMNS ;
	int		rs ;
	int		dlines ;
	int		slines ;
	const char	*termtype ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (pip == NULL) return SR_FAULT ;
	if (dap == NULL) return SR_FAULT ;
	if (dap->termtype == NULL) return SR_FAULT ;

	if (dap->termtype[0] == '\0') return SR_INVALID ;
	if (dap->displines <= 0) return SR_INVALID ;

	memset(op,0,sizeof(DISPLAY)) ;
	op->pip = pip ;
	op->tfd = dap->tfd ;
	op->termlines = dap->termlines ;

	termtype = dap->termtype ;
	dlines = dap->displines ;
	slines = dap->scanlines ;

	if ((rs = display_linesload(op,dlines,slines)) >= 0) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("display_start: displines=%u\n",op->displines) ;
	        debugprintf("display_start: scanlines=%u\n",op->scanlines) ;
	        debugprintf("display_start: viewlines=%u\n",op->viewlines) ;
	    }
#endif /* CF_DEBUG */
	    op->si_top = -1 ;
	    op->si_point = -1 ;
	    op->v.msgnum = -1 ;
	    op->v.msgline = -1 ;
	    op->v.msglines = -1 ;
	    if ((rs = uc_mallocstrw(termtype,-1,&cp)) >= 0) {
	        const int	n = 1000 ;
	        const int	esize = sizeof(SCANLINE) ;
	        op->termtype = cp ;
	        if ((rs = varray_start(&op->scans,esize,n)) >= 0) {
	            const char	*tt = termtype ;
	            if ((rs = ds_start(&op->ds,dap->tfd,tt,dlines,cols)) >= 0) {
	                op->rl_top = 0 ;
	                op->rl_scantitle = 1 ;
	                op->rl_scan = 2 ;
	                if ((rs = display_linescalc(op)) >= 0) {
	                    if ((rs = display_subwinbegin(op)) >= 0) {
	                        op->f.scanfull = TRUE ;
	                        op->magic = DISPLAY_MAGIC ;
	                    }
	                    if (rs < 0)
	                        display_subwinend(op) ;
	                } /* end if */
	                if (rs < 0)
	                    ds_finish(&op->ds) ;
	            } /* end if (ds) */
	            if (rs < 0)
	                varray_finish(&op->scans) ;
	        } /* end if (vecstr_start) */
	        if (rs < 0) {
	            uc_free(op->termtype) ;
	            op->termtype = NULL ;
	        }
	    } /* end if (m-a) */
	} /* end if (display_linesload) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (display_start) */


int display_finish(DISPLAY *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	rs1 = display_hdfin(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = display_subwinend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ds_finish(&op->ds) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = display_scanfins(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = varray_finish(&op->scans) ;
	if (rs >= 0) rs = rs1 ;

	if (op->termtype != NULL) {
	    rs1 = uc_free(op->termtype) ;
	    if (rs >= 0) rs = rs1 ;
	    op->termtype = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (display_finish) */


int display_flush(DISPLAY *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	rs = ds_flush(&op->ds) ;

	return rs ;
}
/* end subroutine (display_flush) */


/* display some information */
int display_info(DISPLAY *op,const char *fmt,...)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		x, y ;
	int		w = 0 ;
	int		len = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;
	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;
	if (fmt[0] == '\0') return SR_INVALID ;

	pip = op->pip ;
	op->ti_info = pip->daytime ;

	x = 0 ;
	y = op->rl_info ;

	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = ds_vpprintf(&op->ds,w,y,x,fmt,ap) ;
	    len += rs ;
	    va_end(ap) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("ds_info: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (display_info) */


int display_winfo(DISPLAY *op,const char *sp,int sl)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		x, y ;
	int		w = 0 ;
	int		len = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;
	op->ti_info = pip->daytime ;

	x = 0 ;
	y = op->rl_info ;

	{
	    rs = ds_pwrite(&op->ds,w,y,x,sp,sl) ;
	    len += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("ds_winfo: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (display_winfo) */


int display_vinfo(DISPLAY *op,const char *fmt,va_list ap)
{
	PROGINFO	*pip ;
	int		rs ;
	int		x, y ;
	int		w = 0 ;
	int		len = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	if (fmt[0] == '\0') return SR_INVALID ;

	pip = op->pip ;
	op->ti_info = pip->daytime ;

	x = 0 ;
	y = op->rl_info ;

	rs = ds_vpprintf(&op->ds,w,y,x,fmt,ap) ;
	len += rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("display_vinfo: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (display_vinfo) */


/* display in the input area */
int display_input(DISPLAY *op,const char *fmt,...)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		x, y ;
	int		w ;
	int		len = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	if (fmt[0] == '\0') return SR_INVALID ;

	pip = op->pip ;
	w = 0 ;
	x = 0 ;
	y = op->rl_input ;

	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = ds_vpprintf(&op->ds,w,y,x,fmt,ap) ;
	    len += rs ;
	    va_end(ap) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("display_input: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (display_input) */


/* display in the input area */
int display_vinput(DISPLAY *op,const char *fmt,va_list ap)
{
	PROGINFO	*pip ;
	int		rs ;
	int		x, y ;
	int		w = 0 ;
	int		len = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	if (fmt[0] == '\0') return SR_INVALID ;

	pip = op->pip ;
	w = 0 ;
	x = 0 ;
	y = op->rl_input ;

	rs = ds_vpprintf(&op->ds,w,y,x,fmt,ap) ;
	len = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("display_vinput: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (display_vinput) */


int display_done(DISPLAY *op)
{
	int		rs ;
	int		w, x, y ;

	w = 0 ;
	x = 0 ;
	y = op->rl_input ;
	rs = ds_pwrite(&op->ds,w,y,x,"\r\v",2) ;

	return rs ;
}
/* end subroutine (display_done) */


/* update the time string on the display */
int display_setdate(DISPLAY *op,int f_end)
{
	PROGINFO	*pip ;
	TMTIME		d ;
	int		rs ;
	int		len = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;
	if (pip->daytime == 0)
	    pip->daytime = time(NULL) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_setdate: ent\n") ;
#endif

/* get current time */

	if ((rs = tmtime_localtime(&d,pip->daytime)) >= 0) {
	    const int	tlen = TIMEBUFLEN ;
	    const int	w = 0 ;
	    int		timelen = 0 ; /* ¥ GCC false complaint */
	    int		x, y ;
	    int		tl = 0 ;
	    int		dl ;
	    int		ml ;
	    int		m = 0 ;
	    const char	*fmt ;
	    const char	*tp ;
	    char	*dp ;
	    char	tbuf[TIMEBUFLEN + 1] = { 0 } ;

/* datestr=> ddd MMM DD hh:mm:ss <= (Mon Dec 25 09:08:07) */

#if	CF_BLINKER
	if (rs >= 0) {
	    int	f = FALSE ;

	    if ((op->datestr[0] == '\0') || (d.hour != op->date.d.hour)) {
	        f = TRUE ;
	        fmt = "%a %e %b %T" ;
	        timelen = 13 ;
	        rs = sntmtime(tbuf,tlen,&d,fmt) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("display_setdate: mid2 rs=%d\n",rs) ;
#endif

	    if ((rs >= 0) && (f || (d.hour != op->date.d.hour))) {
	        tp = tbuf ;
	        tl = MIN(timelen,DISPLAY_LDATESTR) ;
	        m = nleadstr(op->datestr,tp,tl) ;
	        dp = (op->datestr + m) ;	/* update stored state */
	        dl = (tl-m) ;
	        if (dl > 0) {
	            f = TRUE ;
	            strwcpy(dp,(tp+m),dl) ;
	            x = (COL_DATESTR + m) ;
	            y = op->rl_top ;
	            rs = ds_pwrite(&op->ds,w,y,x,dp,dl) ;
	            len += rs ;
	        }
	    } /* end if (date string) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("display_setdate: mid3 rs=%d\n",rs) ;
#endif

	    if ((rs >= 0) && (f || f_end)) {
	        const int	gr = (f_end) ? 0 : DS_GRBLINK ;
	        const char	*s = ":" ;
	        x = (COL_DATESTR + 13) ;
	        y = op->rl_top ;
	        rs = ds_pwritegr(&op->ds,w,y,x,gr,s,1) ;
	        len += rs ;
	        if (rs >= 0) {
	            x = (COL_DATESTR + 16) ;
	            rs = ds_pwritegr(&op->ds,w,y,x,gr,s,1) ;
	            len += rs ;
	        }
	    } /* end if (colons) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("display_setdate: mid4 rs=%d\n",rs) ;
#endif

	    if ((rs >= 0) && (f || (op->date.d.min != d.min))) {
	        if (rs >= 0) {
	            if ((rs = ctdecpi(tbuf,tlen,2,d.min)) >= 0) {
	                x = (COL_DATESTR + 14) ;
	                y = op->rl_top ;
	                rs = ds_pwrite(&op->ds,w,y,x,tbuf,2) ;
	                len += rs ;
	            }
	        }
	    } /* end if (minutes) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("display_setdate: mid5 rs=%d\n",rs) ;
#endif

	    if ((rs >= 0) && (f || (op->date.d.sec != d.sec))) {
	        if ((rs = ctdecpi(tbuf,tlen,2,d.sec)) >= 0) {
	            x = (COL_DATESTR + 17) ;
	            y = op->rl_top ;
	            rs = ds_pwrite(&op->ds,w,y,x,tbuf,2) ;
	            len += rs ;
	        }
	    } /* end if (seconds) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("display_setdate: mid6 rs=%d\n",rs) ;
#endif

	    op->date.d = d ;	/* update stored state */
	} /* end if (date-time) */
#else /* CF_BLINKER */
	fmt = "%s %s %2u %02u:%02u:%02u",
	    rs = bufprintf(tbuf,tlen,fmt,
	    days[d.wday],
	    months[tmt.mon],
	    d.mday,
	    d.hour,
	    d.min,
	    d.sec) ;
	timelen = rs ;
	if (rs >= 0) {
	    tp = tbuf ;
	    tl = MIN(tlen,DISPLAY_LDATESTR) ;
	    m = nleadstr(op->datestr,tp,tl) ;
	    dp = (op->datestr + m) ;
	    dl = (tl-m) ;
	    if (dl > 0) {
	        strwcpy(dp,(tp+m),dl) ;
	        x = (COL_DATESTR + m) ;
	        y = op->rl_top ;
	        rs = ds_pwrite(&op->ds,w,y,x,dp,dl) ;
	        len += rs ;
	    }
	} /* end if (timestr) */
#endif /* CF_BLINKER */

/* update 'scantitle' storage buffer and display */

	if (rs >= 0) {

	    ml = MIN(tlen,DISPLAY_LTZNAME) ;
	    tp = tbuf ;
	    tl = (strwcpylc(tbuf,d.zname,ml) - tbuf) ;

	    m = nleadstr(op->tzname,tp,tl) ;

	    dp = (op->tzname + m) ;
	    dl = (tl-m) ;
	    if (dl > 0) {
	        strwcpy(dp,(tp+m),dl) ;
	        rs = display_setstzone(op) ;
	        if (rs >= 0) {
	            dp = (op->scantitle + DISPLAY_STZSTR + m) ;
	            dl = DISPLAY_STZLEN ;
	            x = DISPLAY_STZSTR ;
	            y = op->rl_scantitle ;
	            rs = ds_pwrite(&op->ds,w,y,x,dp,dl) ;
	            len += rs ;
	        }
	    }

	} /* end if (tzname) */

	} /* end if (tmtime_localtime) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_setdate: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (display_setdate) */


/* load up the new mailbox name */
int display_setmbname(DISPLAY *op,cchar *mbname,int mblen)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		cl ;
	int		fl ;
	int		mbleadlen ;
	int		w, x, y ;
	int		f_change = FALSE ;
	const char	*mbleadstr = DISPLAY_MBSTRLEAD ;
	const char	*ccp ;
	const char	*cp ;
	char		*fp ;

	if (op == NULL) return SR_FAULT ;
	if (mbname == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;

	fp = op->mbstr ;
	fl = DISPLAY_LMBSTR ;

/* has the field changed? */

	mbleadlen = strlen(mbleadstr) ;
	ccp = (fp + mbleadlen) ;
	f_change = (strwcmp(ccp,mbname,mblen) != 0) ;

/* put the mailbox-name in buffer */

	if (f_change) {
	    const int	mbnamelen = DISPLAY_LMBSTR ;
	    char	mbnamebuf[DISPLAY_LMBSTR+1] ;

	    strdcpy1w(mbnamebuf,mbnamelen,mbname,mblen) ;

	    ccp = (mbnamebuf[0] != '\0') ? mbleadstr : "" ;

	    cp = fp ;
	    cl = strdfill3(fp,fl,ccp,mbnamebuf,dashes) - fp ;

	    w = 0 ;
	    x = COL_MAILBOX ;
	    y = op->rl_top ;
	    rs = ds_pwrite(&op->ds,w,y,x,cp,cl) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("display_setmbname: ds_pwrite() rs=%d\n",rs) ;
#endif

	} /* end if (field changed) */

	return rs ;
}
/* end subroutine (display_setmbname) */


/* update the new-mail indicator on the display */
int display_setnewmail(DISPLAY *op,int nmsgs)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		w, x, y ;
	int		f_newmail ;
	int		f_change = FALSE ;
	const char	*sp ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;
	f_newmail = (nmsgs > 0) ;
	f_change = (! LEQUIV(op->f.newmail,f_newmail)) ;
	if (f_newmail) {
	    sp = "new mail arrived" ;
	    if (! op->f.newmail) rs = display_beep(op) ;
	    op->f.newmail = TRUE ;
	} else {
	    sp = "----------------" ;
	    op->f.newmail = FALSE ;
	} /* end if */

	if ((rs >= 0) && f_change) {
	    strwcpy(op->newmail,sp,DISPLAY_LNEWMAIL) ;
	    w = 0 ;
	    x = COL_NEWMAIL ;
	    y = op->rl_top ;
	    rs = ds_pwrite(&op->ds,w,y,x,sp,-1) ;
	}

	return (rs >= 0) ? f_change : rs ;
}
/* end subroutine (display_setnewmail) */


int display_setscanlines(DISPLAY *op,int slines)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	if (slines < 1) return SR_INVALID ;

	op->scanlines = slines ;
	rs = display_linescalc(op) ;

	return rs ;
}
/* end subroutine (display_setscanlines) */


int display_beep(DISPLAY *op)
{
	PROGINFO	*pip ;
	int		rs ;
	int		w ;
	char		buf[10] ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;
	buf[0] = CH_BEL ;
	buf[1] = '\0' ;

	w = 0 ;
	rs = ds_write(&op->ds,w,buf,1) ;

	return rs ;
}
/* end subroutine (display_beep) */


int display_scanclear(DISPLAY *op)
{
	PROGINFO	*pip ;
	int		rs ;
	int		w ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;
	w = op->wscan ;
	if ((rs = ds_move(&op->ds,w,0,0)) >= 0) {
	    int		i ;
	    for (i = 0 ; (rs >= 0) && (i < op->scanlines) ; i += 1) {
	        rs = ds_write(&op->ds,w,"\v\n",2) ;
	    }
	}

	return rs ;
}
/* end subroutine (display_scanclear) */


int display_viewclear(DISPLAY *op)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		w ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;
	w = op->wview ;
	if ((rs = ds_move(&op->ds,w,0,0)) >= 0) {
	    int		i ;
	    for (i = 0 ; (rs >= 0) && (i < op->viewlines) ; i += 1) {
	        rs = ds_write(&op->ds,w,"\v\n",2) ;
	    }
	}

	return rs ;
}
/* end subroutine (display_viewclear) */


int display_viewload(DISPLAY *op,int ln,cchar *sp,int sl)
{
	PROGINFO	*pip ;
	const int	dislen = DISPLAY_LSCANLINE ;
	int		rs = SR_OK ;
	int		w ;
	int		dl ;
	const char	*dp ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	if ((ln < 0) || (ln >= op->viewlines))
	    return SR_INVALID ;

	if ((sp == NULL) && (sl > 0))
	    return SR_FAULT ;

	pip = op->pip ;
	if (sl < 0)
	    sl = strlen(sp) ;

	while ((sl > 0) && isspace(sp[sl-1] & 0xff))
	    sl -= 1 ;

	w = op->wview ;
	rs = ds_move(&op->ds,w,ln,0) ;

	if (rs >= 0) {
	    char	disbuf[DISPLAY_LSCANLINE + 1] ;
	    if (sl > dislen) sl = dislen ;
	    if (strnchr(sp,sl,'\t') != NULL) {
	        dp = disbuf ;
	        dl = tabexpand(disbuf,dislen,sp,sl) ;
	    } else {
	        dp = sp ;
	        dl = sl ;
	    }
	    rs = ds_printf(&op->ds,w,"%t\v",dp,dl) ;
	} /* end if */

	return rs ;
}
/* end subroutine (display_viewload) */


int display_viewscroll(DISPLAY *op,int inc)
{
	const int	ainc = abs(inc) ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	if (ainc >= op->viewlines) {
	    rs = display_viewclear(op) ;
	} else {
	    const int	w = op->wview ;
	    rs = ds_scroll(&op->ds,w,inc) ;
	} /* end if */

	return rs ;
}
/* end subroutine (display_viewscroll) */


int display_cmddig(DISPLAY *op,int pos,int dig)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		w = 0 ;
	int		x, y ;
	char		buf[10] ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    int	ch = (isprintlatin(dig)) ? dig: ' ' ;
	    debugprintf("display_cmddig: pos=%d dig=>%c< (%02X)\n",
	        pos,ch,dig) ;
	}
#endif

	x = pos ;
	y = op->rl_input ;
	if (dig == CH_DEL) {
	    if ((rs = ds_move(&op->ds,w,y,x)) >= 0) {
	        rs = ds_ec(&op->ds,w,1) ;
	    }
	} else if (isdigitlatin(dig)) {
	    buf[0] = dig ;
	    buf[1] = '\0' ;
	    rs = ds_write(&op->ds,w,buf,1) ;
	}

	return rs ;
}
/* end subroutine (display_cmddig) */


int display_scanload(DISPLAY *op,int si,DISPLAY_SDATA *ddp)
{
	PROGINFO	*pip ;
	DISPLAY_SS	ss ;
	int		rs ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	if (si < 0) return SR_INVALID ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scanload: ent si=%d\n",si) ;
#endif

	if ((rs = display_scancvtbegin(op,&ss,ddp)) >= 0) {
	    SCANLINE	*slp ;
	    if ((rs = varray_acc(&op->scans,si,&slp)) >= 0) {
	        if (slp != NULL) {
	            rs = scanline_fill(slp,ddp) ;
	        } else {
	            if ((rs = varray_mk(&op->scans,si,&slp)) >= 0) {
	                rs = scanline_start(slp,ddp) ;
	                if (rs < 0)
	                    varray_del(&op->scans,si) ;
	            } /* end if (varray_mk) */
	        } /* end if (scanline) */
	    } /* end if (varray_acc) */
	    rs1 = display_scancvtend(op,&ss,ddp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (display-scancvt) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scanload: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (display_scanload) */


static int display_scancvtbegin(DISPLAY *op,DISPLAY_SS *ssp,DISPLAY_SDATA *ddp)
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_OK ;
	cchar		*ss = "=?" ;
	if (ddp == NULL) return SR_FAULT ;
	if (pip == NULL) return SR_FAULT ;
	ssp->fbuf = ddp->fbuf ;
	ssp->flen = ddp->flen ;
	ssp->sbuf = ddp->sbuf ;
	ssp->slen = ddp->slen ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scancvtbegin: ent\n") ;
#endif

	if (rs >= 0) {
	    const int	fcols = DISPLAY_LFROM ;
	    int		flen = ssp->flen ;
	    char	*fbuf = (char *) ssp->fbuf ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scancvtbegin: fcols=%u flen=%d\n",
		fcols,flen) ;
#endif
	    if ((fbuf != NULL) && (strsub(fbuf,ss) != NULL)) {
	        if (flen < 0) flen = strlen(fbuf) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scancvtbegin: flen=%d\n",flen) ;
#endif
		if ((rs = display_hdcvt(op,fbuf,flen)) >= 0) {
		    int		wl = rs ;
		    if (wl > fcols) wl = fcols ;
		    ddp->fcol = wl ;
		    if ((rs = display_scancvtalt(op,ddp)) >= 0) {
		        DISPLAY_SALT	*sap = &ddp->alt ;
		        const int	tlen = DISPLAY_FROMLEN ;
			wchar_t		*wbuf = op->wbuf ;
			char		tbuf[DISPLAY_FROMLEN+1] ;
		        if ((rs = snwcpywidehdr(tbuf,tlen,wbuf,wl)) >= 0) {
		            const int	flen = sap->flen ;
			    char	*fbuf = (char *) sap->fbuf ;
			    if ((rs = mkdisphdr(fbuf,flen,tbuf,rs)) >= 0) {
			        ddp->fbuf = sap->fbuf ;
			        ddp->flen = rs ;
			    }
			}
		    }
		}
	    } else {
		if (ddp->flen > fcols) ddp->flen = fcols ;
		ddp->fcol = ddp->flen ;
	    }
	}

	if (rs >= 0) {
	    const int	fcols = DISPLAY_LSUBJ ;
	    int		slen = ssp->slen ;
	    char	*sbuf = (char *) ssp->sbuf ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scancvtbegin: fcols=%u slen=%d\n",
		fcols,slen) ;
#endif
	    if ((sbuf != NULL) && (strsub(sbuf,ss) != NULL)) {
	        if (slen < 0) slen = strlen(sbuf) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scancvtbegin: slen=%d\n",slen) ;
#endif
		if ((rs = display_hdcvt(op,sbuf,slen)) >= 0) {
		    int		wl = rs ;
		    if (wl > fcols) wl = fcols ;
		    ddp->scol = wl ;
		    if ((rs = display_scancvtalt(op,ddp)) >= 0) {
		        DISPLAY_SALT	*sap = &ddp->alt ;
		        const int	slen = sap->slen ;
			wchar_t		*wbuf = op->wbuf ;
			char		*sbuf = (char *) sap->sbuf ;
		        if ((rs = snwcpywidehdr(sbuf,slen,wbuf,wl)) >= 0) {
			    ddp->sbuf = sap->sbuf ;
			    ddp->slen = rs ;
			}
		    }
		}
	    } else {
		if (ddp->slen > fcols) ddp->slen = fcols ;
		ddp->scol = ddp->slen ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scancvtbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (display_scancvtbegin) */


static int display_scancvtalt(DISPLAY *op,DISPLAY_SDATA *ddp)
{
	DISPLAY_SALT	*sap = &ddp->alt ;
	const int	flen = (2*DISPLAY_LFROM) ;
	const int	slen = (2*DISPLAY_LSUBJ) ;
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (sap->a == NULL) {
	    int		size = 0 ;
	    char	*bp ;
	    size += (flen+1) ;
	    size += (slen+1) ;
	    if ((rs = uc_malloc(size,&bp)) >= 0) {
		sap->a = bp ;
		sap->fbuf = bp ;
		sap->flen = flen ;
		sap->sbuf = (bp + (flen+1)) ;
		sap->slen = slen ;
	    }
	}
	return rs ;
}
/* end subroutine (display_scancvtalt) */


static int display_scancvtend(DISPLAY *op,DISPLAY_SS *ssp,DISPLAY_SDATA *ddp)
{
	DISPLAY_SALT	*sap = &ddp->alt ;
	int		rs = SR_OK ;
	int		rs1 ;
	if (op == NULL) return SR_FAULT ;
	if (sap->a != NULL) {
	    rs1 = uc_free(sap->a) ;
	    if (rs >= 0) rs = rs1 ;
	    sap->a = NULL ;
	}
	ddp->sbuf = ssp->sbuf ;
	ddp->slen = ssp->slen ;
	ddp->fbuf = ssp->fbuf ;
	ddp->flen = ssp->flen ;
	return rs ;
}
/* end subroutine (display_scancvtend) */


static int display_hdcvt(DISPLAY *op,cchar *sp,int sl)
{
	PROGINFO	*pip = op->pip ;
	int		rs ;
	if (pip == NULL) return SR_FAULT ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("display_hdcvt: ent sl=%d\n",sl) ;
	    debugprintf("display_hdcvt: s=>%t<\n",
		sp,strlinelen(sp,sl,40)) ;
	}
#endif
	if ((rs = display_hdcheck(op,sl)) >= 0) {
	    HDRDECODE	*hdp = &op->hd ;
	    rs = hdrdecode_proc(hdp,op->wbuf,op->wlen,sp,sl) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("display_hdcvt: ret w=>%wt<\n",op->wbuf,
		MIN(rs,40)) ;
	    debugprintf("display_hdcvt: ret rs=%d\n",rs) ;
	}
#endif
	return rs ;
}
/* end subroutine (display_hdcvt) */


int display_scanloadlines(DISPLAY *op,int si,int lines,int f_upd)
{
	PROGINFO	*pip ;
	SCANLINE	*slp ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	if (si < 0) return SR_INVALID ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_scanloadlines: ent si=%u lines=%u\n",
	        si,lines) ;
#endif

	if ((rs = varray_acc(&op->scans,si,&slp)) >= 0) {
	    if (slp != NULL) {
	        if ((rs = scanline_setlines(slp,lines)) >= 0) {
	            const int	ns = op->scanlines ;
	            const int	top = op->si_top ;
	            if (f_upd && (si >= top) && (si < (top + ns))) {
	                int	w, x, y ;
	                int	fl, fi ;
	                const char	*fp ;

	                fi = COL_SCANLINES ;
	                fp = (slp->data + fi) ;
	                fl = DISPLAY_LLINES ;

	                w = op->wscan ;
	                x = COL_SCANLINES ;
	                y = (si - op->si_top) ;
	                rs = ds_pwrite(&op->ds,w,y,x,fp,fl) ;

	            } /* end if (item in current view) */
	        } /* end if (scanline_setlines) */
	    } /* end if (non-null) */
	} /* end if (varray_acc) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scanloadlines: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (display_scanloadlines) */


int display_scanblank(DISPLAY *op,int si)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		rc = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_scanblank: ent si=%d\n",si) ;
#endif

	if (si < 0) {
	    op->f.scanfull = TRUE ;
	    rs = display_scanblanks(op,-1) ;
	} else {
	    SCANLINE	*slp ;
	    if ((rs = varray_acc(&op->scans,si,&slp)) >= 0) {
	        if (slp != NULL) {
	            rs = scanline_blank(slp) ;
	            rc = rs ;
	        }
	    } /* end if (varray_acc) */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scanblank: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (display_scanblank) */


int display_scanblanks(DISPLAY *op,int n)
{
	PROGINFO	*pip ;
	SCANLINE	*slp ;
	int		rs = SR_OK ;
	int		i = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;
	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scanblanks: ent n=%d\n",n) ;
#endif

	if (n < 0) {
	    op->f.scanfull = TRUE ;
	    n = INT_MAX ;
	}

	while ((i < n) && (varray_enum(&op->scans,i,&slp) >= 0)) {
	    if (slp != NULL) {
	        rs = scanline_blank(slp) ;
	    }
	    i += 1 ;
	    if (rs < 0) break ;
	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scanblanks: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (display_scanblanks) */


int display_scanfull(DISPLAY *op)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;
	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scanfull: ent\n") ;
#endif

	op->f.scanfull = TRUE ;
	return rs ;
}
/* end subroutine (display_scanfull) */


/* delete? one or more scan lines (whatever this means?) */
int display_scandel(DISPLAY *op,int si)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		rc = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scandel: ent si=%d\n",si) ;
#endif

	if (si < 0) {
	    rs = display_scanfins(op) ;
	} else {
	    SCANLINE	*slp ;
	    if ((rs = varray_acc(&op->scans,si,&slp)) >= 0) {
	        if (slp != NULL) {
	            rs1 = scanline_finish(slp) ;
	            if (rs >= 0) rs = rs1 ;
	            rc = rs1 ;
	            rs1 = varray_del(&op->scans,si) ;
	            if (rs >= 0) rs = rs1 ;
	        }
	    } /* end if (varray_acc) */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scandel: ret rs=%d rc=%d\n",rs,rc) ;
#endif

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (display_scandel) */


int display_scanmark(DISPLAY *op,int si,int mark)
{
	PROGINFO	*pip ;
	SCANLINE	*slp ;
	int		rs ;
	int		rc = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	if (si < 0) return SR_INVALID ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scanmark: ent si=%d\n",si) ;
#endif

	if ((rs = varray_acc(&op->scans,si,&slp)) >= 0) {
	    if (slp != NULL) {
	        rs = scanline_mark(slp,mark) ;
	        rc = rs ;
	        if (rs >= 0)
	            rs = display_scanmarkset(op,si,mark) ;
	    }
	} /* end if (varray_acc) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scanmark: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (display_scanmark) */


int display_scanpoint(DISPLAY *op,int nsi)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		rc = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("display_scanpoint: ent sipointnext=%d\n",nsi) ;
	    debugprintf("display_scanpoint: si_point=%d\n",op->si_point) ;
	    debugprintf("display_scanpoint: si_ppoint=%d\n",op->si_ppoint) ;
	}
#endif

#ifdef	COMMENT
	op->si_ppoint = op->si_point ;
#endif

	op->si_point = nsi ;
	if (nsi >= 0) {
	    SCANLINE	*slp ;
	    if ((rs = varray_acc(&op->scans,nsi,&slp)) >= 0) {
	        if (slp != NULL) {
	            rc = (slp->data != NULL) ? 1 : 0 ;
	        }
	    } /* end if (varray_acc) */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("display_scanpoint: ret si_ppoint=%d\n",op->si_ppoint) ;
	    debugprintf("display_scanpoint: ret si_point=%d\n",op->si_point) ;
	    debugprintf("display_scanpoint: ret rs=%d rc=%u\n",rs,rc) ;
	}
#endif

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (display_scanpoint) */


/* check if a scan-line has any data with it */
int display_scancheck(DISPLAY *op,int si)
{
	PROGINFO	*pip ;
	SCANLINE	*slp ;
	int		rs ;
	int		rc = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	if (si < 0) return SR_INVALID ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scancheck: ent si=%d\n",si) ;
#endif

	if ((rs = varray_acc(&op->scans,si,&slp)) >= 0) {
	    if (slp != NULL) {
	        rc = (slp->data != NULL) ? 1 : 0 ;
	    }
	} /* end if (varray_acc) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scancheck: ret rs=%d rc=%d\n",rs,rc) ;
#endif

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (display_scancheck) */


/* set a new scan-top and display any resulting scan-lines */
int display_scandisplay(DISPLAY *op,int sitopnext)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		ndiff ;
	int		ni ;
	int		n = 0 ;
	int		f ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("display_scandisplay: ent si_top=%d\n",op->si_top) ;
	    debugprintf("display_scandisplay: sitopnext=%d\n",sitopnext) ;
	}
#endif

	if (sitopnext < 0)
	    return SR_INVALID ;

/* try to display what we are supposed to */

	n = op->nnblank ;

/* OK, now handle the confusing cases! */

	f = (op->si_top < 0) || op->f.scanfull ;
	if (! f) {
	    ndiff = (sitopnext - op->si_top) ;
	    ni = abs(ndiff) ;
	    f = (ni > op->scanlines) ;
	}

	if (f) { /* no overlap */

	    op->f.scanfull = FALSE ;
	    rs = display_rscan(op,sitopnext) ;
	    n = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("display_scandisplay: no_overlap rs=%d\n",rs) ;
#endif

	} else { /* overlap */

	    rs = display_rscanscroll(op,sitopnext) ;
	    n = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("display_scandisplay: overlap rs=%d\n",rs) ;
#endif

	} /* end if */

/* update where the top of the scan window is */

	if (rs >= 0) {
	    op->si_top = sitopnext ;
	    op->nnblank = n ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_scandisplay: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (display_scandisplay) */


/* retrieve the "info" field time-stamp */
int display_infots(DISPLAY *op,time_t *tp)
{
	if (op == NULL) return SR_FAULT ;
	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;
	if (tp != NULL) *tp = op->ti_info ;
	return SR_OK ;
}
/* end subroutine (display_infots) */


int display_midmsgs(DISPLAY *op,int total,int current)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		fl ;
	int		nl ;
	int		w, x, y ;
	const char	*ccp ;
	char		nmsgbuf[NMSGBUFLEN + 1] ;
	char		*fp ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_nmsgs: nmsgs=%d mi=%d\n",total,current) ;
#endif

	fp = op->nmsgs ;
	fl = DISPLAY_LNMSGS ;
	if (total >= 0) {
	    if (current >= 0) {
	        rs = bufprintf(nmsgbuf,NMSGBUFLEN,"%5u:%-5u",
	            (current+1),total) ;
	        if (rs >= 0) {
	            nl = 6 ;
	            if (strncmp((nmsgbuf+nl),(fp+nl),(fl-nl)) == 0) {
	                fl = (nl-1) ;
	            }
	        }
	    } else {
	        rs = bufprintf(nmsgbuf,NMSGBUFLEN,"   *:%-5u", 
	            total) ;
	    }
	    ccp = nmsgbuf ;
	} else
	    ccp = dashes ;

	if (strncmp(ccp,fp,fl) != 0) {
	    strncpy(fp,ccp,fl) ;
	    w = 0 ;
	    x = COL_NMSGS ;
	    y = op->rl_middle ;
	    rs = ds_pwrite(&op->ds,w,y,x,fp,fl) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("display_nmsgs: ds_pwrite() rs=%d >%t<\n",
	            rs,fp,fl) ;
#endif

	} /* end if */

	op->v.total = total ;
	op->v.current = current ;

	return rs ;
}
/* end subroutine (display_midmsgs) */


int display_botclear(DISPLAY *op)
{
	PROGINFO	*pip ;
	DISPLAY_VALS	*dvp ;
	int		rs = SR_OK ;
	int		f_change = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;
	dvp = &op->v ;
	if (dvp->fbuf[0] != '\0') {
	    f_change = TRUE ;
	    dvp->fbuf[0] = '\0' ;
	}

	if (dvp->msgnum >= 0) {
	    f_change = TRUE ;
	    dvp->msgnum = -1 ;
	}

	if (dvp->msgline >= 0) {
	    f_change = TRUE ;
	    dvp->msgline = -1 ;
	}

	if (dvp->msglines >= 0) {
	    f_change = TRUE ;
	    dvp->msglines = -1 ;
	}

	if (f_change) {
	    rs = display_rbot(op) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_botclear: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (display_botclear) */


int display_botinfo(DISPLAY *op,DISPLAY_BOTINFO *mip)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		f_change = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_botinfo: ent\n") ;
#endif

	if (rs >= 0) {
	    rs = display_botloadnum(op,mip->msgnum) ;
	    f_change = f_change || (rs > 0) ;
	}

	if (rs >= 0) {
	    rs = display_botloadfrom(op,mip->msgfrom) ;
	    f_change = f_change || (rs > 0) ;
	}

	if (rs >= 0) {
	    rs = display_botloadline(op,mip->msgline) ;
	    f_change = f_change || (rs > 0) ;
	}

	if (rs >= 0) {
	    rs = display_botloadlines(op,mip->msglines) ;
	    f_change = f_change || (rs > 0) ;
	}

	if ((rs >= 0) && f_change) {
	    rs = display_rbot(op) ;
	}

	return rs ;
}
/* end subroutine (display_botinfo) */


/* refresh the line number at the bottom of the display */
int display_botline(DISPLAY *op,int msgline)
{
	PROGINFO	*pip ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;
	if ((rs = display_botloadline(op,msgline)) > 0) {
	    int		fi ;
	    int		fl ;
	    int		w, x, y ;
	    char	*fp ;
    
	    fi = COL_MSGLINES ;
	    fp = (op->linebot + fi) ;
	    fl = MIN(4,DISPLAY_LNMSGS) ;

	    w = 0 ;
	    x = COL_MSGLINES ;
	    y = op->rl_bottom ;
	    rs = ds_pwrite(&op->ds,w,y,x,fp,fl) ;

	} /* end if (display_botloadline) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_botline: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (display_botline) */


int display_allclear(DISPLAY *op)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		w ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;

	w = 0 ;
	rs = ds_clear(&op->ds,w) ;

	return rs ;
}
/* end subroutine (display_allclear) */


/* change the number of display lines and-or scanlines */
int display_winadj(DISPLAY *op,int dlines,int slines)
{
	PROGINFO	*pip ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_winadj: dlines=%d slines=%d\n",dlines,slines) ;
#endif

	if ((rs = display_linesload(op,dlines,slines)) >= 0) {
	    const int	w = 0 ;
	    if ((rs = ds_setlines(&op->ds,w,dlines)) >= 0) {

	        if ((rs = display_linescalc(op)) >= 0) {
	            if ((rs = display_subwinadj(op)) >= 0) {
	                const int	r = op->rl_input ;
#ifdef	COMMENT
	                if ((rs = ds_ew(&op->ds,w,r,2)) >= 0) {
	                    rs = display_refresh(op) ;
	                }
#else
	                rs = ds_ew(&op->ds,w,r,2) ;
#endif /* COMMENT */
	            } /* end if (display-subwinadj) */
	        } /* end if (display-linescalc) */
	    } /* end if (ds-setlines) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("display_winadj: ds_sublines-out rs=%d\n",rs) ;
#endif
	} /* end if (display-linesload) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_winadj: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (display_winadj) */


/* refresh the whole display */
int display_refresh(DISPLAY *op)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("display_refresh: ent\n") ;
#endif

	rs1 = display_rframe(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = display_rscan(op,-1) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("display_refresh: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (display_refresh) */


/* refresh the frame only */
int display_rframe(DISPLAY *op)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("display_rframe: ent\n") ;
#endif

/* setup: identify, scan, msg pointer */

	rs1 = display_rtop(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = display_rscantitle(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = display_rmid(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = display_rbot(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("display_rframe: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (display_rframe) */


int display_rtop(DISPLAY *op)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		w, x, y ;
	int		i ;
	int		cl = -1 ;
	int		ll = 0 ;
	const char	*lp ;
	const char	*cp ;
	char		lbuf[LINEBUFLEN + 1] ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("display_rtop: ent\n") ;
#endif

	lp = lbuf ;
	ll = MIN(DISPLAY_LSCANLINE,LINEBUFLEN) ;
	memset(lbuf,'-',ll) ;

	if (op->newmail[0] != '\0') {
	    i = COL_NEWMAIL ;
	    cp = op->newmail ;
	    cl = strnlen(cp,DISPLAY_LNEWMAIL) ;
	    strncpy((lbuf + i),cp,cl) ;
	}

	if (op->mbstr[0] != '\0') {
	    i = COL_MAILBOX ;
	    cp = op->mbstr ;
	    cl = strnlen(cp,DISPLAY_LMBSTR) ;
	    strncpy((lbuf + i),cp,cl) ;
	}

	if (op->datestr[0] != '\0') {
	    i = COL_DATESTR ;
	    cp = op->datestr ;
	    cl = strnlen(cp,DISPLAY_LDATESTR) ;
	    strncpy((lbuf + i),cp,cl) ;
	}

	w = 0 ;
	x = 0 ;
	y = op->rl_top ;
	if (rs >= 0)
	    rs = ds_pwrite(&op->ds,w,y,x,lp,ll) ;

/* indicate a refresh is needed */

	if (rs >= 0) op->datestr[0] = '\0' ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("display_rtop: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (display_rtop) */


int display_rmid(DISPLAY *op)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		w, x, y ;
	int		i ;
	int		cl ;
	int		ll = 0 ;
	const char	*lp ;
	const char	*cp ;
	char		lbuf[LINEBUFLEN + 1] ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("display_rmid: ent\n") ;
#endif

	lp = lbuf ;
	ll = MIN(DISPLAY_LSCANLINE,LINEBUFLEN) ;
	memset(lbuf,'-',ll) ;

	if (op->nmsgs[0] != '\0') {
	    i = COL_NMSGS ;
	    cp = op->nmsgs ;
	    cl = strnlen(cp,DISPLAY_LNMSGS) ;
	    strncpy((lbuf + i),cp,cl) ; /* insert */
	}

	if (op->f.moremsgs) {
	    i = COL_MOREMSGS ;
	    cp = "more messages" ;
	    cl = DISPLAY_LMOREMSGS ;
	    strncpy((lbuf + i),cp,cl) ; /* insert */
	}

	w = 0 ;
	x = 0 ;
	y = op->rl_middle ;
	if (rs >= 0)
	    rs = ds_pwrite(&op->ds,w,y,x,lp,ll) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("display_rmid: ret rs=%d ll=%d\n",rs,ll) ;
#endif

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (display_rmid) */


int display_rbot(DISPLAY *op)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		w, x, y ;
	int		ll = 0 ;
	char		*lp ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("display_rbot: ent\n") ;
#endif

	lp = op->linebot ;
	ll = DISPLAY_LSCANLINE ;
	if (lp[0] == '\0')
	    memset(lp,'-',ll) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("display_rbot: ll=%d\n",ll) ;
	    debugprintf("display_rbot: la=>%t<\n",lp,40) ;
	    debugprintf("display_rbot: lb=>%t<\n",(lp+40),40) ;
	}
#endif

	w = 0 ;
	x = 0 ;
	y = op->rl_bottom ;
	if (rs >= 0) {
	    rs = ds_pwrite(&op->ds,w,y,x,lp,ll) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("display_rbot: ret rs=%d ll=%d\n",rs,ll) ;
#endif

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (display_rbot) */


int display_rscantitle(DISPLAY *op)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		sl ;
	int		w, x, y ;
	int		len = 0 ;
	const char	*sp ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DISPLAY_MAGIC) return SR_NOTOPEN ;

	pip = op->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("display_rscantitle: ent\n") ;
#endif

	sp = op->scantitle ;
	sl = DISPLAY_LSCANLINE ;

/* setup on first time */

	if (op->scantitle[0] == '\0') {
	    rs = display_scantitlemk(op) ;
	}

/* time-zone name */

	if (rs >= 0) {
#ifdef	COMMENT
	if (op->tzname[0] != '\0')
	    rs = display_setstzone(op) ;
#else
	rs = display_setstzone(op) ;
#endif /* COMMENT */
	} /* end if (ok) */

/* write it out */

	if (rs >= 0) {
	    w = 0 ;
	    x = 0 ;
	    y = op->rl_scantitle ;
	        rs = ds_pwrite(&op->ds,w,y,x,sp,sl) ;
	        len += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_rscantitle: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (display_rscantitle) */


int display_rscan(DISPLAY *op,int si)
{
	PROGINFO	*pip = op->pip ;
	DS		*dsp = &op->ds ;
	int		rs = SR_OK ;
	int		i ;
	int		w, x, y ;
	int		c = 0 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_rscan: ent si=%d\n",si) ;
#endif

	if (si < 0)
	    si = op->si_top ;

/* proceed with display work */

	w = op->wscan ;
	x = 0 ;
	y = 0 ;
	if ((rs = ds_move(dsp,w,y,x)) >= 0) {

	    if (si >= 0) { /* scan-lines are present */
	        for (i = 0 ; (rs >= 0) && (i < op->scanlines) ; i += 1) {
	            rs = display_scanprint(op,(si+i)) ;
	            c += ((rs > 0) ? 1 : 0) ;
	        } /* end for */
	    } else { /* no scan-lines are present - just clear */
	        for (i = 0 ; (rs >= 0) && (i < op->scanlines) ; i += 1) {
	            rs = ds_write(&op->ds,w,"\v\n",2) ;
	        } /* end for */
	    } /* end if */

	} /* end if (ds_move) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_rscan: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (display_rscan) */


int display_suspend(DISPLAY *op)
{
	const int	r = op->rl_input ;
	const int	c = 0 ;
	int		rs ;

	if ((rs = display_flush(op)) >= 0) {
	    if ((rs = ds_pwrite(&op->ds,0,r,c,"\v",1)) >= 0) {
	        rs = ds_suspend(&op->ds,r,c) ;
	    }
	}

	return rs ;
}
/* end subroutine (display_suspend) */


int display_resume(DISPLAY *op)
{
	if (op == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (display_resume) */


int scandata_init(DISPLAY_SDATA *sdp)
{
	memset(sdp,0,sizeof(DISPLAY_SDATA)) ;
	sdp->fcol = -1 ;
	sdp->scol = -1 ;
	return SR_OK ;
}
/* end subroutine (scandata_init) */


/* private subroutines */


static int display_linesload(DISPLAY *op,int dlines,int slines)
{
	int		rs = SR_OK ;
	op->displines = dlines ;
	op->scanlines = (slines > 0) ? slines : (dlines/4) ;
	op->viewlines = (dlines - slines - FRAMELINES) ;
	if ((op->displines == 0) || (op->viewlines == 0)) {
	    rs = SR_INVALID ;
	}
	return rs ;
}
/* end subroutine (display_linesload) */


static int display_linescalc(DISPLAY *op)
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_OK ;
	int		dlines = op->displines ;
	int		slines = op->scanlines ;

	if (pip == NULL) return SR_FAULT ;

	op->rl_middle = (op->rl_scan + slines) ;
	op->rl_viewer = (op->rl_middle + 1) ;
	op->rl_bottom = (dlines-3) ;
	op->rl_info = (dlines-2) ;
	op->rl_input = (dlines-1) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("display_linescalc: rl_middle=%d\n",op->rl_middle) ;
	    debugprintf("display_linescalc: rl_viewer=%d\n",op->rl_viewer) ;
	    debugprintf("display_linescalc: rl_bottom=%d\n",op->rl_bottom) ;
	    debugprintf("display_linescalc: rl_info=%d\n",op->rl_info) ;
	    debugprintf("display_linescalc: rl_input=%d\n",op->rl_input) ;
	}
#endif /* CF_DEBUG */

	return rs ;
}
/* end subroutine (display_linescalc) */


/* create the "scan" and "viewer" sub-windows */
static int display_subwinbegin(DISPLAY *op)
{
	int		rs = SR_OK ;
	int		sr, sc ;
	int		rows, cols ;

	if (rs >= 0) {
	    sr = op->rl_scan ;
	    sc = 0 ;
	    rows = op->scanlines ;
	    cols = DISPLAY_LSCANLINE ;
	    rs = ds_subnew(&op->ds,sr,sc,rows,cols) ;
	    op->wscan = rs ;
	}

	if (rs >= 0) {
	    sr = op->rl_viewer ;
	    sc = 0 ;
	    rows = op->viewlines ;
	    cols = DISPLAY_LSCANLINE ;
	    rs = ds_subnew(&op->ds,sr,sc,rows,cols) ;
	    op->wview = rs ;
	}

	return rs ;
}
/* end subroutine (display_subwinbegin) */


static int display_subwinend(DISPLAY *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->wview > 0) {
	    rs1 = ds_subdel(&op->ds,op->wview) ;
	    if (rs >= 0) rs = rs1 ;
	    op->wview = 0 ;
	}

	if (op->wscan > 0) {
	    rs1 = ds_subdel(&op->ds,op->wscan) ;
	    if (rs >= 0) rs = rs1 ;
	    op->wscan = 0 ;
	}

	return rs ;
}
/* end subroutine (display_subwinend) */


static int display_subwinadj(DISPLAY *op)
{
	int		rs ;

	if ((rs = display_subwinend(op)) >= 0) {
	    rs = display_subwinbegin(op) ;
	}

	return rs ;
}
/* end subroutine (display_subwinadj) */


static int display_scanfins(DISPLAY *op)
{
	PROGINFO	*pip = op->pip ;
	SCANLINE	*slp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scanfins: ent\n") ;
#endif

	for (i = 0 ; varray_enum(&op->scans,i,&slp) >= 0 ; i += 1) {
	    if (slp != NULL) {
	        rs1 = scanline_finish(slp) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scanfins: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (display_scanfins) */


static int display_rscanscroll(DISPLAY *op,int sitopnext)
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		si_lose, si_print ;
	int		si_keep ;
	int		ndiff ;
	int		ni, i ;
	int		nkeep ;
	int		r ;
	int		w ;
	int		n = 0 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("display_rscanscroll: sitop=%d\n",op->si_top) ;
	    debugprintf("display_rscanscroll: sitopnext=%d\n",sitopnext) ;
	}
#endif

	ndiff = (sitopnext - op->si_top) ;
	if (ndiff >= 0) {
	    r = (op->scanlines - ndiff) ;
	    si_lose = op->si_top ;
	    si_keep = (op->si_top + ndiff) ;
	    si_print = (op->si_top + op->scanlines) ;
	} else {
	    r = 0 ;
	    si_lose = (op->si_top + op->scanlines + ndiff) ;
	    si_keep = op->si_top ;
	    si_print = sitopnext ; /* (op->si_top + ndiff) */
	} /* end if */

	w = op->wscan ;
	ni = abs(ndiff) ;

	nkeep = (op->scanlines - ni) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_rscanscroll: ni=%d\n",ni) ;
#endif

/* adjust the count of non-blank scan lines from the lines we're losing */

	n = op->nnblank ;
	rs1 = display_nnbscans(op,si_lose,ni) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_rscanscroll: display_nnbscans() rs=%d\n",rs1) ;
#endif

	if ((rs1 >= 0) && (n >= rs1))
	    n -= rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_rscanscroll: n=%d\n",n) ;
#endif

/* clear any pointer within the scanlines that are being kept */

	if (nkeep > 0)
	    rs = display_scanpointclear(op,si_keep,nkeep) ;

/* scroll the window */

	if ((rs >= 0) && (ni > 0)) {

	    rs = ds_scroll(&op->ds,w,ndiff) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("display_rscanscroll: ds_scroll() rs=%d\n",rs) ;
#endif

	} /* end if (scroll) */

/* now fill in any empty lines as we may have fillings */

	if ((rs >= 0) && (ni > 0)) {
	    rs = ds_move(&op->ds,w,r,0) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("display_rscanscroll: ds_move() rs=%d\n",rs) ;
#endif

	    for (i = 0 ; (rs >= 0) && (i < ni) ; i += 1) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("display_rscanscroll: i=%u\n",i) ;
#endif

	        rs = display_scanprint(op,(si_print+i)) ;
	        n += ((rs > 0) ? 1 : 0) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("display_rscanscroll: "
	                "display_scanprint() rs=%d\n",rs) ;
#endif

	    } /* end for */

	} /* end if */

	if (rs >= 0)
	    rs = display_scanpointset(op,si_keep,nkeep,sitopnext) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_rscanscroll: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (display_rscanscroll) */


static int display_scanprint(DISPLAY *op,int si)
{
	PROGINFO	*pip = op->pip ;
	SCANLINE	*slp = NULL ;
	DS		*dsp = &op->ds ;
	int		rs ;
	int		w = op->wscan ;
	int		ch_point ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_scanprint: ent si=%d\n",si) ;
#endif

	if (pip == NULL) return SR_FAULT ;

	ch_point = ' ' ;
	if (si == op->si_point) {
	    op->si_ppoint = op->si_point ;
	    ch_point = DCH_POINT ;
	}

	if ((rs = varray_acc(&op->scans,si,&slp)) >= 0) {
	    const char	*fmt ;
	    if (slp != NULL) {
	        int	ch_mark = (isprintlatin(slp->mark)) ? slp->mark : ' ' ;
	        c += 1 ;
	        fmt = (slp->data != NULL) ? "%c%c%s\n" : "%c%c\v\n" ;
	        rs = ds_printf(dsp,w,fmt,ch_mark,ch_point,(slp->data + 2)) ;
	    } else {
	        rs = ds_printf(dsp,w," %c\v\n",ch_point) ;
	    } /* end if */
	} /* end if (varray_acc) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_scanprint: ret rs=%d c=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (display_scanprint) */


static int display_nnbscans(DISPLAY *op,int si,int ni)
{
	PROGINFO	*pip = op->pip ;
	SCANLINE	*slp ;
	int		rs = SR_OK ;
	int		i ;
	int		n = 0 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_nnbscans: ent si=%d\n",si) ;
#endif

	for (i = 0 ; i < ni ; i += 1) {
	    if ((rs = varray_acc(&op->scans,(si+i),&slp)) >= 0) {
	        if (slp != NULL) {
	            if ((rs = scanline_checkblank(slp)) > 0) {
	                n += 1 ;
	            }
	        }
	    } /* end if (varray_acc) */
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("display_nnbscans: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (display_nnbscans) */


static int display_setstzone(DISPLAY *op)
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_OK ;
	int		fl ;
	int		len = 0 ;
	char		stbuf[DISPLAY_STZLEN + 1] ;
	char		*fp ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_setstzone: tzname=%s\n",op->tzname) ;
#endif

/* double check length of 'tzname' */

	{
	    int	zl = DISPLAY_LTZNAME ;
	    if (op->tzname[zl] != '\0')
	        op->tzname[zl] = '\0' ;
	}

/* do it */

	fp = op->scantitle + DISPLAY_STZSTR ;
	fl = DISPLAY_STZLEN ;

/* formulate string (that will be NUL-terminated) */

	if (op->tzname[0] != '\0') {
	    len = strdcpy4(stbuf,fl,"(",op->tzname,")",blanks) - stbuf ;
	} else {
	    len = strdcpy1(stbuf,fl,blanks) - stbuf ;
	}

/* insert into storage buffer */

	strncpy(fp,stbuf,fl) ;		/* insertions must use 'strncpy()' */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_setstzone: st=>%t<\n",fp,fl) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (display_setstzone) */


static int display_scanpointclear(DISPLAY *op,int si_keep,int nkeep)
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_OK ;
	int		rc = 0 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("display_scanpointclear: si_top=%d\n",op->si_top) ;
	    debugprintf("display_scanpointclear: si_ppoint=%d\n",
	        op->si_ppoint) ;
	    debugprintf("display_scanpointclear: si_point=%d\n",op->si_point) ;
	    debugprintf("display_scanpointclear: si_keep=%d\n",si_keep) ;
	    debugprintf("display_scanpointclear: nkeep=%u\n",nkeep) ;
	}
#endif

	if (op->si_ppoint >= 0) {
	    int	si_ppoint = op->si_ppoint ;
	    if ((si_ppoint >= si_keep) && (si_ppoint < (si_keep+nkeep))) {
	        int	w, x, y ;
	        char	buf[2] ;
	        buf[0] = ' ' ;
	        buf[1] = '\0' ;
	        w = op->wscan ;
	        x = 1 ;
	        y = (op->si_ppoint - op->si_top) ;
	        rs = ds_pwrite(&op->ds,w,y,x,buf,1) ;
	        rc = 1 ;
	        op->si_ppoint = -1 ;
	    } /* end if */
	} /* end if */

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (display_scanpointclear) */


static int display_scanpointset(DISPLAY *op,int si_keep,int nkeep,int sitopnext)
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_OK ;
	int		rc = 0 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("display_scanpointset: si_top=%d\n",op->si_top) ;
	    debugprintf("display_scanpointset: si_ppoint=%d\n",op->si_ppoint) ;
	    debugprintf("display_scanpointset: si_point=%d\n",op->si_point) ;
	    debugprintf("display_scanpointset: si_keep=%d\n",si_keep) ;
	    debugprintf("display_scanpointset: nkeep=%u\n",nkeep) ;
	}
#endif

	if (op->si_point >= 0) {
	    if ((op->si_point >= si_keep) && (op->si_point < (si_keep+nkeep))) {
	        int	w, x, y ;
	        char	buf[2] ;
	        buf[0] = DCH_POINT ;
	        buf[1] = '\0' ;
	        w = op->wscan ;
	        x = 1 ;
	        y = (op->si_point - sitopnext) ;
	        rs = ds_pwrite(&op->ds,w,y,x,buf,1) ;
	        rc = 1 ;
	        op->si_ppoint = op->si_point ;
	    } /* end if */
	} /* end if */

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (display_scanpointset) */


static int display_scanmarkset(DISPLAY *op,int si,int mark)
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_OK ;
	int		rc = 0 ;

	if (pip == NULL) return SR_FAULT ;

	if (op->si_point >= 0) {
	    if ((si >= op->si_point) && (si < (op->si_point+op->scanlines))) {
	        int	w, x, y ;
	        char	buf[2] ;
	        buf[0] = mark ;
	        buf[1] = '\0' ;
	        w = op->wscan ;
	        x = 0 ;
	        y = (si - op->si_top) ;
	        rs = ds_pwrite(&op->ds,w,y,x,buf,1) ;
	        rc = 1 ;
	    } /* end if */
	} /* end if */

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (display_scanmarkset) */


static int display_scantitlemk(DISPLAY *op)
{
	int		rs = SR_OK ;
	int		cl ;
	int		tcol ;
	int		i ;
	const char	*cp ;
	char		*bp = op->scantitle ;

	strwcpyblanks(bp,DISPLAY_LSCANLINE) ;

	for (i = 0 ; scantitles[i].name != NULL ; i += 1) {
	    cp = scantitles[i].name ;
	    cl = strlen(cp) ;
	    tcol = scantitles[i].col ;
	    strncpy((bp + tcol),cp,cl) ;
	} /* end for */

	return rs ;
}
/* end subroutine (display_scantitlemk) */


static int display_botloadfrom(DISPLAY *op,cchar *msgfrom)
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_OK ;
	int		fi ;
	int		fl, bl ;
	int		f_change = FALSE ;
	char		*fp, *bp ;

	if (pip == NULL) return SR_FAULT ;

	fi = COL_MSGFROM ;
	fl = DISPLAY_LMSGFROM ;
	fp = (op->linebot + fi) ;
	if (strncmp(msgfrom,fp,fl) != 0) {
	    f_change = TRUE ;
	    if (msgfrom[0] != '\0') {
	        bp = strdfill1(fp,fl,msgfrom) ;
	        bl = ((fp + fl) - bp) ;
	        memset(bp,'-',bl) ;
	    } else
	        memset(fp,'-',fl) ;
	} /* end if (not-equal) */

	return (rs >= 0) ? f_change : rs ;
}
/* end subroutine (display_botloadfrom) */


static int display_botloadnum(DISPLAY *op,int msgnum)
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_OK ;
	int		f_change = FALSE ;

	if (pip == NULL) return SR_FAULT ;

	if (msgnum != op->v.msgnum) {
	    int		fi = COL_MSGNUM ;
	    f_change = TRUE ;
	    op->v.msgnum = msgnum ;
	    {
	        int	bl = MIN(5,DISPLAY_LMSGNUM) ;
	        char	*bp = (op->linebot + fi) ;
	        if (msgnum >= 0) {
	            const int	v = MIN(9999,msgnum) ;
	            char	dbuf[DIGBUFLEN + 1] ;
	            if ((rs = ctdeci(dbuf,DIGBUFLEN,v)) >= 0) {
	                strdfill3(bp,bl,"#",dbuf,blanks) ;
		    }
	        } else
	            memset(bp,'-',bl) ;
	    } /* end block */
	} /* end if (needed replacement) */

	return (rs >= 0) ? f_change : rs ;
}
/* end subroutine (display_botloadnum) */


static int display_botloadline(DISPLAY *op,int msgline)
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_OK ;
	int		f_change = FALSE ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_botloadline: msgline=%d\n",msgline) ;
#endif

	if (msgline != op->v.msgline) {
	    int		i ;
	    int		fi ;
	    int		fl ;
	    char	*fp ;
	    f_change = TRUE ;
	    op->v.msgline = msgline ;
	    fi = COL_MSGLINES ;
	    fp = (op->linebot + fi) ;
	    fl = MIN(4,DISPLAY_LMSGLINES) ;
	    if (op->v.msgline >= 0) {
	        const int	v = MIN(9999,op->v.msgline) ;
	    	int		ml ;
	        char		digbuf[DIGBUFLEN + 1] ;
	        if ((rs = ctdeci(digbuf,DIGBUFLEN,v)) >= 0) {
	            ml = rs ;
	            i = (fl-ml) ;
	            memset(fp,' ',i) ;
	            strncpy((fp+i),digbuf,ml) ;
	        }
	    } else
	        memset(fp,'-',fl) ;
	} /* end if (needed replacement) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_botloadline: ret rs=%d f_ch=%u\n",
	        rs,f_change) ;
#endif

	return (rs >= 0) ? f_change : rs ;
}
/* end subroutine (display_botloadline) */


static int display_botloadlines(DISPLAY *op,int msglines)
{
	PROGINFO	*pip = op->pip ;
	const int	n = 4 ;
	int		rs = SR_OK ;
	int		f_change = FALSE ;

	if (pip == NULL) return SR_FAULT ;

	if (msglines != op->v.msglines) {
	    int		fi = (COL_MSGLINES + n) ;
	    f_change = TRUE ;
	    op->v.msglines = msglines ;
	    {
	        int	bl = (DISPLAY_LMSGLINES - n) ;
	        char	*bp = (op->linebot + fi) ;
	        if (msglines >= 0) {
	            const int	v = MIN(9999,msglines) ;
	            char	dbuf[DIGBUFLEN + 1] ;
	            if ((rs = ctdeci(dbuf,DIGBUFLEN,v)) >= 0) {
	                strdfill3(bp,bl,":",dbuf,blanks) ;
		    }
	        } else
	            memset(bp,'-',bl) ;
	    } /* end block */
	} /* end if (need replacement) */

	return (rs >= 0) ? f_change : rs ;
}
/* end subroutine (display_botloadlines) */


static int display_hdcheck(DISPLAY *op,int wlen)
{
	PROGINFO	*pip = op->pip ;
	int		rs ;
	if (pip == NULL) return SR_FAULT ;
	if ((rs = display_hd(op)) >= 0) {
	    if (wlen > op->wlen) {
	        if (op->wbuf != NULL) {
		    rs = uc_free(op->wbuf) ;
		    op->wbuf = NULL ;
		    op->wlen = 0 ;
	        }
	        if (rs >= 0) {
		    wchar_t	*wbuf ;
		    int		wsize ;
		    wlen = iceil(wlen,MAXNAMELEN) ;
		    wsize = ((wlen+1)*sizeof(wchar_t)) ;
		    if ((rs = uc_malloc(wsize,&wbuf)) >= 0) {
		        op->wbuf = wbuf ;
		        op->wlen = wlen ;
		    }
	        }
	        if (rs < 0) {
		    display_hdfin(op) ;
	        }
	    } /* end if (needed enlargement) */
	} /* end if (display-hd) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_hdcheck: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (display_hdcheck) */


static int display_hd(DISPLAY *op)
{
	PROGINFO	*pip = op->pip ;
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (! op->open.hd) {
	    PROGINFO	*pip = op->pip ;
	    HDRDECODE	*hdp = &op->hd ;
	    if ((rs = hdrdecode_start(hdp,pip->pr)) >= 0) {
		op->open.hd = TRUE ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_hd: hdrdecode_start() rs=%d\n",rs) ;
#endif
	    }
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("display_hd: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (display_hd) */


static int display_hdfin(DISPLAY *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->wbuf != NULL) {
	    rs1 = uc_free(op->wbuf) ;
	    if (rs >= 0) rs = rs1 ;	
	    op->wbuf = NULL ;
	}
	if (op->open.hd) {
	    HDRDECODE	*hdp = &op->hd ;
	    op->open.hd = FALSE ;
	    rs1 = hdrdecode_finish(hdp) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (display_hdfin) */


static int scanline_start(SCANLINE *slp,DISPLAY_SDATA *ddp)
{
	int		rs = SR_OK ;

	if (slp == NULL) return SR_FAULT ;

	memset(slp,0,sizeof(SCANLINE)) ;

	if (ddp != NULL) {
	    rs = scanline_fill(slp,ddp) ;
	}

	return rs ;
}
/* end subroutine (scanline_start) */


static int scanline_finish(SCANLINE *slp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		rc = 0 ;

#if	CF_DEBUGS
	debugprintf("display/scanline_finish: ent slp{%p}\n",slp) ;
#endif

	if (slp == NULL) return SR_FAULT ;

	if (slp->data != NULL) {
	    rs1 = uc_free(slp->data) ;
	    if (rs >= 0) rs = rs1 ;
	    slp->data = NULL ;
	    rc = 1 ;
	}

#if	CF_DEBUGS
	debugprintf("display/scanline_finish: ret rs=%d rc=%d\n",rs,rc) ;
#endif

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (scanline_finish) */


static int scanline_fill(SCANLINE *slp,DISPLAY_SDATA *ddp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		ll = 0 ;

	if (slp == NULL) return SR_FAULT ;

	if (slp->data != NULL) {
	    uc_free(slp->data) ;
	    slp->data = NULL ;
	}

	if (ddp != NULL) {
	    SBUF	b ;
	    int		cl, fl ;
	    int		icols ;
	    int		nblank ;
	    int		nncol ;
	    int		dl = 0 ;
	    int		f_check = FALSE ;
	    const char	*dp ;
	    const char	*cp ;
	    const char	*ccp ;
	    char	scanbuf[SCANBUFLEN + 1] ;
	    char	dispbuf[DISPBUFLEN + 1] ;
	    char	linesbuf[LINESBUFLEN + 1] ;
	    char	*lp ;
	    if ((rs = sbuf_start(&b,scanbuf,SCANBUFLEN)) >= 0) {
	        int	fi ;
		int	fcol ;
	        int	ncol = 0 ;

	        for (fi = 0 ; scantitles[fi].name != NULL ; fi += 1) {
		    char	*abuf = NULL ;
	            f_check = TRUE ;
	            dp = NULL ;
	            dl = -1 ;
		    fcol = -1 ;

	            nncol = scantitles[fi].col ;
	            nblank = (nncol - ncol) ;
#if	CF_DEBUGS
	            debugprintf("scanline_fill: fi=%u nncol=%u nblank=%u\n",
			fi,nncol,nblank) ;
#endif
	            if (nblank > 0) {
	                sbuf_blanks(&b,nblank) ;
	                ncol += nblank ;
	            }

	            switch (fi) {

	            case scantitle_from:
	                dp = ddp->fbuf ;
	                dl = ddp->flen ;
			fcol = ddp->fcol ;
			fl = DISPLAY_LFROM ;
	                break ;

	            case scantitle_subject:
	                dp = ddp->sbuf ;
	                dl = ddp->slen ;
			fcol = ddp->scol ;
			fl = DISPLAY_LSUBJ ;
			if (dp != NULL) {
			    if (dl < 0) dl = strlen(dp) ;
			    if ((rs = uc_malloc((dl+1),&abuf)) >= 0) {
			        if ((rs = snwcpycompact(abuf,dl,dp,dl)) >= 0) {
				    dl = rs ;
				    dp = abuf ;
			        }
			        if (rs < 0) {
				    uc_free(abuf) ;
				    abuf = NULL ;
			        }
			    } /* end if (m-a) */
			} /* end if (non-null) */
	                break ;

	            case scantitle_date:
	                f_check = FALSE ;
	                dp = ddp->date ;
	                fl = DISPLAY_LDATE ;
	                break ;

	            case scantitle_lines:
	                f_check = FALSE ;
	                if (ddp->lines >= 0) {
	                    fl = DISPLAY_LLINES ;
	                    rs1 = digit3(linesbuf,ddp->lines) ;
	                    if (rs1 >= 0)
	                        dp = linesbuf ;
	                } else {
	                    dp = blanks ;
	                    dl = DISPLAY_LLINES ;
	                }
	                fl = DISPLAY_LLINES ;
	                break ;

	            } /* end switch */

#if	CF_DEBUGS
	            debugprintf("scanline_fill: fi=%u fl=%d dl=%d\n",
			fi,fl,dl) ;
#endif

	            if ((rs >= 0) && (dp != NULL)) {
	                if (dl < 0) dl = strlen(dp) ;

#if	CF_DEBUGS
	                debugprintf("scanline_fill: d=>%t<\n",dp,dl) ;
#endif

	                if (f_check) {
	                    cp = dispbuf ;
	                    rs = mkdisplayable(dispbuf,DISPBUFLEN,dp,dl) ;
	                    cl = rs ;
	                } else {
	                    cp = dp ;
	                    cl = dl ;
	                }

	                if ((rs >= 0) && (cp != NULL) && (cl > 0)) {
	                    rs = sbuf_strw(&b,cp,cl) ;
	                        icols = nstrcols(NTABCOLS,ncol,cp,cl) ;
#if	CF_DEBUGS
	                debugprintf("scanline_fill: icols=%u\n",icols) ;
#endif
	                        ncol += icols ;
	                }
	            } /* end if (non-empty field) */

		    if (abuf != NULL) uc_free(abuf) ;
	            if (rs < 0) break ;
	        } /* end for */

	        if (rs >= 0) {

#if	CF_SCANFILLOUT
	            nncol = DISPLAY_LSCANLINE ;
	            nblank = (nncol - ncol) ;
	            if (nblank > 0) {
	                sbuf_blanks(&b,nblank) ;
	                ncol += nblank ;
	            }
#endif /* CF_SCANFILLOUT */

	            lp = scanbuf ;
	            rs = sbuf_getlen(&b) ;
	            ll = rs ;

	            if (rs >= 0) {
	                rs = uc_mallocstrw(lp,ll,&ccp) ;
	                slp->data = (char *) ccp ;

#if	CF_DEBUGS
	                debugprintf("scanline_fill: ll=%u lp=>%t<\n",
	                    ll,lp,strlinelen(lp,ll,40)) ;
	                debugprintf("scanline_fill: uc_mallocstrw() rs=%d\n",
				rs) ;
#endif /* CF_DEBUGS */

	            }

/* process the "mark" */

	            if (ddp->mark != 0) {
	                slp->mark = ddp->mark ;
		    }

	        } /* end if (ok) */

	        rs1 = sbuf_finish(&b) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sbuf) */
	} /* end if (non-null) */

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (scanline_fill) */


static int scanline_blank(SCANLINE *slp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		rc = 0 ;

	if (slp == NULL) return SR_FAULT ;

	slp->mark = 0 ;
	if (slp->data != NULL) {
	    rs1 = uc_free(slp->data) ;
	    if (rs >= 0) rs = rs1 ;
	    slp->data = NULL ;
	    rc = 1 ;
	}

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (scanline_blank) */


static int scanline_mark(SCANLINE *slp,int mark)
{
	int		rs = SR_OK ;
	int		rc = 0 ;

	if (slp == NULL) return SR_FAULT ;

	slp->mark = mark ;
	if (slp->data != NULL)
	    rc = 1 ;

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (scanline_mark) */


static int scanline_setlines(SCANLINE *slp,int lines)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		di ;
	int		n = 3 ;
	int		rc = 0 ;
	char		linesbuf[LINESBUFLEN + 1] ;
	char		*dp ;

	if (slp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("display/scanline_setlines: lines=%u\n",lines) ;
#endif

	dp = slp->data ;
	di = COL_SCANLINES ;
	if (dp != NULL) {
	    rs1 = digit3(linesbuf,lines) ;
	    if ((rs1 >= 0) && (strncmp((dp+di),linesbuf,n) != 0)) {
	        rc = 1 ;
	        strncpy((dp+di),linesbuf,n) ;
	    }
	}

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (scanline_setlines) */


static int scanline_checkblank(SCANLINE *slp)
{
	int		rs = SR_OK ;
	int		rc = 0 ;

	if (slp == NULL) return SR_FAULT ;

	if (slp->data != NULL)
	    rc = 1 ;

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (scanline_checkblank) */


#if	CF_SFEND

/* string-find-end */
static int sfend(cchar *sp,int sl,int n,cchar **rpp)
{
	int		rs = SR_OK ;
	int		ml ;

	if (sl < 0) sl = strlen(sp) ;

	ml = MIN(sl,n) ;
	if (rpp != NULL)
	    *rpp = (rs >= 0) ? (sp + sl - ml) : NULL ;

	return (rs >= 0) ? ml : rs ;
}
/* end subroutine (sfend) */

#endif /* CF_SFEND */


/* number-string-colums (numer of columns a string takes up) */
static int nstrcols(int ntabcols,int nc,const char *sp,int sl)
{
	int		n = 0 ;

	if (sp != NULL) {
	    int	icols ;
	    int	scols ;
	    int	i ;
	    if (sl < 0) strlen(sp) ;
	    scols = nc ;
	    for (i = 0 ; i < sl ; i += 1) {
	        icols = charcols(ntabcols,nc,sp[i]) ;
	        nc += icols ;
	    }
	    n = (nc - scols) ;
	} /* end if (non-null) */

	return n ;
}
/* end subroutine (nstrcols) */


#if	CF_DEBUGS
static char *stridig(char *ip,int il,int n)
{
	int		nd ;
	char		digbuf[DIGBUFLEN + 1] ;
	nd = ctdecui(digbuf,DIGBUFLEN,n) ;
	if ((nd > 0) && (nd < il)) {
	    ip = strnset(ip,' ',1) ;
	    il -= 1 ;
	    ip = strnset(ip,' ',(il - nd)) ;
	    ip = strncpy(ip,digbuf,nd) ;
	} else
	    ip = strnset(ip,' ',il) ;
	return ip ;
}
/* end subroutine (stridig) */
#endif /* CF_DEBUGS */


