/* process */

/* process a data file */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_CFDECULL	0		/* use 'cfdecull()' */


/* revision history:

	= 2004-03-01, David A­D­ Morano
        The subroutine was adapted from others programs that did similar types
        of functions.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine processes a file that contains numeric pairs.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	DEBUGLEVLE
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	strnnlen(cchar *,int,int) ;
extern int	cfdecf(cchar *,int,double *) ;
extern int	cfdecul(cchar *,int,ulong *) ;
extern int	cfdecull(cchar *,int,ULONG *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strnchr(cchar *,int,int) ;


/* external variables */


/* global variables */


/* local structures */


/* forward references */

static int	putpair(PROGINFO *,int,double,double) ;
static int	getval(PROGINFO	*,cchar *,int,double *) ;


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int progfile(PROGINFO *pip,cchar *fname)
{
	bfile		ifile ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process: entered fname=%s\n",fname) ;
#endif

	if (fname[0] == '-') fname = BFILE_STDIN ;

	if ((rs = bopen(&ifile,fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		line = 1 ;
	    int		fl ;
	    int		n ;
	    cchar	*fp ;
	    char	lbuf[LINEBUFLEN + 1] ;

/* read 'em */

	    while ((rs = breadline(&ifile,lbuf,llen)) > 0) {
	        FIELD	lf ;
	        double	x, y ;
	        int	len = rs ;

	        if (lbuf[len - 1] == '\n') len -= 1 ;
	        lbuf[len] = '\0' ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: line=>%t<\n", lbuf,len) ;
#endif /* CF_DEBUG */

	        if ((rs = field_start(&lf,lbuf,len)) >= 0) {

	            x = 0.0 ;
	            y = 0.0 ;
	            for (n = 1 ; n < pip->ncol ; n += 1) {

	                fl = field_get(&lf,NULL,&fp) ;
	                if (fl > 0)
	                    rs = getval(pip,fp,fl,&x) ;

	                if ((rs < 0) && pip->f.ignore)
	                    rs = SR_OK ;

	                if (rs < 0) break ;
	            } /* end for */

	            if (rs >= 0)
	                fl = field_get(&lf,NULL,&fp) ;

	            if ((rs >= 0) && (fl > 0))
	                rs = getval(pip,fp,fl,&y) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                debugprintf("process: getval() rs=%d\n",rs) ;
	                debugprintf("process: x=%12.4f y=%12.4f\n",x,y) ;
		    }
#endif /* CF_DEBUG */

/* we're out of this one */

	            field_finish(&lf) ;
	        } /* end if (field) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: field rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {
	            rs = putpair(pip,c,x,y) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("process: putpair() rs=%d\n",rs) ;
#endif /* CF_DEBUG */

	        } /* end if */

	        c += 1 ;
	        line += 1 ;

	        if (rs < 0) break ;
	    } /* end while (reading lines) */

	    rs1 = bclose(&ifile) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bfile) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process: ret rs=%d c=%u\n",rs,c) ;
#endif /* CF_DEBUG */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (process) */


/* local subroutines */


static int putpair(pip,i,x,y)
PROGINFO	*pip ;
int		i ;
double		x, y ;
{
	int	rs = SR_OK ;
	int	e ;
	int	size ;

	if ((i + 1) > pip->e) {
	    int	j ;

	    e = pip->e + 100 ;
	    size = e * sizeof(struct process_pair) ;
	    if (pip->n == 0) {
	        rs = uc_malloc(size,&pip->pairs) ;
	    } else {
	        rs = uc_realloc(pip->pairs,size,&pip->pairs) ;
	    }

	    if (rs >= 0) {

	        for (j = pip->e ; j < e ; j += 1) {
	            pip->pairs[j].x = 0.0 ;
	            pip->pairs[j].sum = 0.0 ;
	        }

	        pip->e = e ;

	    } /* end if */

	} /* end if (extending) */

	if (rs >= 0) {
	    pip->pairs[i].x = x ;
	    pip->pairs[i].sum += y ;
	    if (i >= pip->n)
	        pip->n = (i + 1) ;
	}

	return rs ;
}
/* end subroutine (putpair) */


static int getval(PROGINFO *pip,cchar *sp,int sl,double *vp)
{
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("getval: sl=%d sp=>%t<\n",
	        sl,sp,strnlen(sp,sl)) ;
#endif

	if (strnchr(sp,sl,'.') != NULL) {

	    pip->f.fdec = TRUE ;
	    rs = cfdecf(sp,sl,vp) ;

	} else {
	    ulong	v ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("getval: cfdecu() >%t<\n",
	            sp,strnnlen(sp,sl,40)) ;
#endif

#if	CF_CFDECULL
	    rs = cfdecull(sp,sl,&lw) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("getval: cfdecull() rs=%d v=%llu\n",rs,lw) ;
#endif

	    *vp = (rs >= 0) ? ((double) lw) : 0.0 ;
#else /* CF_DECULL */
	    rs = cfdecul(sp,sl,&v) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("getval: cfdecul() rs=%d v=%lu\n",rs,v) ;
#endif

	    *vp = (rs >= 0) ? ((double) v) : 0.0 ;
#endif /* CF_DECULL */

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("getval: ret rs=%d v=%12.4f\n", rs,*vp) ;
#endif

	return rs ;
}
/* end subroutine (getval) */


