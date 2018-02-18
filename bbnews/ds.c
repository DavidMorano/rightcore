/* ds */

/* low-level terminal-display manager */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_TD		1		/* use TD object */


/* revision history:

	= 2009-01-20, David A­D­ Morano
        This is a complete rewrite of the trash that performed this function
        previously.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module implements the display related functions for the program.
        This code is actually an API-independent front-end for one or more
        underlying terminal-display APIs. The primary terminal-display API of
        the past used to be 'curses(3)' but that had way too many bugs for
        continued use. The primary underlying terminal-display API is now
        'td(3dam)'.


*******************************************************************************/


#define	DS_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<termstr.h>
#include	<ascii.h>
#include	<uterm.h>
#include	<localmisc.h>

#include	"ds.h"


/* local defines */

/* mask for graphic renditions */
#define	DS_GRMASK	(DS_GRBOLD| DS_GRUNDER| DS_GRBLINK| DS_GRREV)


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strncpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strnrpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int ds_start(DS *dsp,int tfd,cchar *termtype,int rows,int cols)
{
	int		rs ;
	const char	*cp ;

	if (dsp == NULL) return SR_FAULT ;
	if (termtype == NULL) return SR_FAULT ;

	if (termtype[0] == '\0') return SR_INVALID ;
	if (tfd < 0) return SR_INVALID ;

#if	CF_DEBUGS
	    debugprintf("ds_start: termtype=%s\n",termtype) ;
#endif /* CF_DEBUGS */

	memset(dsp,0,sizeof(DS)) ;
	dsp->tfd = tfd ;

	if ((rs = uc_mallocstrw(termtype,-1,&cp)) >= 0) {
	    dsp->termtype = cp ;
#if	CF_TD
	    rs = td_start(&dsp->dm,tfd,termtype,rows,cols) ;
#else
	    rs = SR_NOSYS ;
#endif
	    if (rs >= 0) {
	        dsp->magic = DS_MAGIC ;
	    }
	    if (rs < 0) {
	        uc_free(dsp->termtype) ;
	        dsp->termtype = NULL ;
	    }
	} /* end if (m-a) */

#if	CF_DEBUGS
	    debugprintf("ds_start: ret rs=%d\n",rs) ;
#endif /* CF_DEBUGS */

	return rs ;
}
/* end subroutine (ds_start) */


int ds_finish(DS *dsp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (dsp == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_TD
	rs1 = td_finish(&dsp->dm) ;
	if (rs >= 0) rs = rs1 ;
#else
	rs = SR_NOSYS ;
#endif

	if (dsp->termtype != NULL) {
	    rs1 = uc_free(dsp->termtype) ;
	    if (rs >= 0) rs = rs1 ;
	    dsp->termtype = NULL ;
	}

	dsp->magic = 0 ;
	return rs ;
}
/* end subroutine (ds_finish) */


int ds_subnew(DS *dsp,int srow,int scol,int rows,int cols)
{
	int		rs ;

	if (dsp == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_TD
	rs = td_subnew(&dsp->dm,srow,scol,rows,cols) ;
#else
	rs = SR_NOSYS ;
#endif /* CF_TD */

	return rs ;
}
/* end subroutine (ds_subnew) */


int ds_subdel(DS *dsp,int w)
{
	int		rs ;

	if (dsp == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_TD
	rs = td_subdel(&dsp->dm,w) ;
#else
	rs = SR_NOSYS ;
#endif /* CF_TD */

	return rs ;
}
/* end subroutine (ds_subdel) */


int ds_getlines(DS *dsp,int w)
{
	int		rs ;

	if (dsp == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_TD
	rs = td_getlines(&dsp->dm,w) ;
#else
	rs = SR_NOSYS ;
#endif /* CF_TD */

	return rs ;
}
/* end subroutine (ds_getlines) */


int ds_setlines(DS *dsp,int w,int nl)
{
	int		rs ;

	if (dsp == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_TD
	rs = td_setlines(&dsp->dm,w,nl) ;
#else
	rs = SR_NOSYS ;
#endif /* CF_TD */

	return rs ;
}
/* end subroutine (ds_setlines) */


int ds_move(DS *dsp,int w,int r,int c)
{
	int		rs ;

	if (dsp == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("ds_move: w=%d r=%d c=%d\n",w,r,c) ;
#endif

#if	CF_TD
	rs = td_move(&dsp->dm,w,r,c) ;
#else
	rs = SR_NOSYS ;
#endif /* CF_TD */

	return rs ;
}
/* end subroutine (ds_move) */


/* print to a window */
int ds_printf(DS *dsp,int w,const char *fmt,...)
{
	int		rs ;

	if (dsp == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_TD
	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = td_vpprintf(&dsp->dm,w,-1,-1,fmt,ap) ;
	    va_end(ap) ;
	}
#else
	rs = SR_OK ;
#endif /* CF_TD */

	return rs ;
}
/* end wubroutine (ds_printf) */


/* print to a window */
int ds_pprintf(DS *dsp,int w,int r,int c,const char *fmt,...)
{
	int		rs ;

	if (dsp == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_TD
	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = td_vpprintf(&dsp->dm,w,r,c,fmt,ap) ;
	    va_end(ap) ;
	}
#else
	    rs = SR_OK ;
#endif /* CF_TD */

	return rs ;
}
/* end wubroutine (ds_pprintf) */


/* print to a window */
int ds_vprintf(DS *dsp,int w,const char *fmt,va_list ap)
{
	int		rs ;

	if (dsp == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_TD
	rs = td_vpprintf(&dsp->dm,w,-1,-1,fmt,ap) ;
#else
	rs = SR_OK ;
#endif /* CF_TD */

	return rs ;
}
/* end wubroutine (ds_vprintf) */


/* print to a window */
int ds_vpprintf(DS *dsp,int w,int r,int c,const char *fmt,va_list ap)
{
	int		rs ;

	if (dsp == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("ds_vpprintf: r=%d c=%d\n",r,c) ;
#endif

#if	CF_TD
	rs = td_vpprintf(&dsp->dm,w,r,c,fmt,ap) ;
#else
	rs = SR_OK ;
#endif /* CF_TD */

	return rs ;
}
/* end wubroutine (ds_vpprintf) */


/* write to a window */
int ds_write(DS *dsp,int w,cchar *bp,int bl)
{
	int		rs ;

	if (dsp == NULL) return SR_FAULT ;
	if (bp == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_TD
	rs = td_pwrite(&dsp->dm,w,-1,-1,bp,bl) ;
#else
	rs = SR_OK ;
#endif /* CF_TD */

	return rs ;
}
/* end wubroutine (ds_write) */


/* write to a window */
int ds_pwrite(DS *dsp,int w,int r,int c,cchar *bp,int bl)
{
	int		rs ;

	if (dsp == NULL) return SR_FAULT ;
	if (bp == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	{
	debugprintf("ds_pwrite: w=%d r=%d c=%d\n",w,r,c) ;
	debugprintf("ds_pwrite: bl=%d \n",bl) ;
	}
#endif

#if	CF_TD
	rs = td_pwrite(&dsp->dm,w,r,c,bp,bl) ;
#else
	rs = SR_OK ;
#endif /* CF_TD */

	return rs ;
}
/* end wubroutine (ds_pwrite) */


/* write to a window w/ graphic rendition */
int ds_pwritegr(DS *dsp,int w,int r,int c,int gr,cchar *bp,int bl)
{
	int		rs ;
	int		tdgr = 0 ;

	if (dsp == NULL) return SR_FAULT ;
	if (bp == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	{
	debugprintf("ds_pwritegr: w=%d r=%d c=%d\n",w,r,c) ;
	debugprintf("ds_pwritegr: bl=%d \n",bl) ;
	}
#endif

#if	CF_TD
	if (gr) {
	    if (gr & DS_GRBOLD) tdgr |= TD_GRBOLD ;
	    if (gr & DS_GRUNDER) tdgr |= TD_GRUNDER ;
	    if (gr & DS_GRBLINK) tdgr |= TD_GRBLINK ;
	    if (gr & DS_GRREV) tdgr |= TD_GRREV ;
	}
	rs = td_pwritegr(&dsp->dm,w,r,c,tdgr,bp,bl) ;
#else
	rs = SR_OK ;
#endif /* CF_TD */

#if	CF_DEBUGS
	debugprintf("ds_pwritegr: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end wubroutine (ds_pwritegr) */


/* erase window; type: 0=forward, 1=back, 2=whole */
int ds_ew(DS *dsp,int w,int r,int type)
{
	int		rs ;
	int		len = 0 ;

	if (dsp == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_TD
	rs = td_ew(&dsp->dm,w,r,type) ;
	len = rs ;
#else
	rs = SR_NOSYS ;
#endif /* CF_TD */

	return (rs >= 0) ? len : rs ;
}
/* end wubroutine (ds_ew) */


int ds_el(DS *dsp,int w,int type)
{
	int		rs ;
	int		len = 0 ;

	if (dsp == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_TD
	rs = td_el(&dsp->dm,w,type) ;
	len = rs ;
#else
	rs = SR_NOSYS ;
#endif /* CF_TD */

	return (rs >= 0) ? len : rs ;
}
/* end wubroutine (ds_el) */


int ds_ec(DS *dsp,int w,int n)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (dsp == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_TD
	rs = td_ec(&dsp->dm,w,n) ;
	len = rs ;
#else
	rs = SR_NOSYS ;
#endif /* CF_TD */

	return (rs >= 0) ? len : rs ;
}
/* end wubroutine (ds_ec) */


int ds_scroll(DS *dsp,int w,int n)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (dsp == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_TD
	rs = td_scroll(&dsp->dm,w,n) ;
	len = rs ;
#else
	rs = SR_NOSYS ;
#endif /* CF_TD */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (ds_scroll) */


/* clear (blank out) a window */
int ds_clear(DS *dsp,int w)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (dsp == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_TD
	rs = td_clear(&dsp->dm,w) ;
	len = rs ;
#else
	rs = SR_NOSYS ;
#endif /* CF_TD */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (ds_clear) */


int ds_flush(DS *dsp)
{
	int		rs ;

	if (dsp == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

#if	CF_TD
	rs = td_flush(&dsp->dm) ;
#else
	rs = SR_NOSYS ;
#endif /* CF_TD */

	return rs ;
}
/* end subroutine (ds_flush) */


/* suspend the display (optionally leaving the cursor someplace) */
int ds_suspend(DS *dsp,int r,int c)
{
	int		rs ;

	if (dsp == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

	rs = ds_flush(dsp) ;

#if	CF_TD
	if (rs >= 0)
	    rs = td_suspend(&dsp->dm,r,c) ;
#else
	rs = SR_NOSYS ;
#endif /* CF_TD */

	return rs ;
}
/* end subroutine (ds_suspend) */


int ds_done(DS *dsp)
{
	int		rs ;

	if (dsp == NULL) return SR_FAULT ;

	if (dsp->magic != DS_MAGIC) return SR_NOTOPEN ;

	rs = ds_pwrite(dsp,0,(dsp->rows-1),0,"\v",1) ;

	if (rs >= 0)
	    rs = ds_flush(dsp) ;

#if	CF_TD
	if (rs >= 0)
	    rs = td_suspend(&dsp->dm,-1,-1) ;
#else
	rs = SR_NOSYS ;
#endif /* CF_TD */

	return rs ;
}
/* end subroutine (ds_done) */


