/* process */

/* process a data file */


#define	CF_DEBUG	1		/* compile-time debug print-outs */
#define	CF_CFDECULL	0		/* use 'cfdecull()' */


/* revision history:

	= 2004-03-01, David A­D­ Morano

	The subroutine was adapted from others programs that
	did similar types of functions.


*/


/******************************************************************************

	This subroutine processes a file that contains numeric pairs.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<field.h>
#include	<mallocstuff.h>
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

extern int	strnnlen(const char *,int,int) ;
extern int	cfdecf(const char *,int,double *) ;
extern int	cfdecul(const char *,int,ulong *) ;
extern int	cfdecull(const char *,int,ULONG *) ;


/* external variables */


/* global variables */


/* local structures */


/* forward references */

static int	putpair(struct proginfo *,int,double,double) ;
static int	getval(struct proginfo	*,char *,int,double *) ;


/* local variables */


/* exported subroutines */


int process(pip,kop,fname)
struct proginfo	*pip ;
KEYOPT		*kop ;
const char	fname[] ;
{
	FIELD	lf ;

	bfile	ifile ;

	int	rs = SR_OK ;
	int	len ;
	int	line ;
	int	fl, cl ;
	int	n ;
	int	c = 0 ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	namebuf[MAXNAMELEN + 1] ;
	char	*fp, *cp ;

	if (fname == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process: entered fname=%s\n",fname) ;
#endif

	if (fname[0] == '-')
	    rs = bopen(&ifile,BFILE_STDIN,"dr",0666) ;

	else
	    rs = bopen(&ifile,fname,"r",0666) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process: bopen() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

/* read 'em */

	c = 0 ;
	line = 1 ;
	len = 0 ;
	while ((rs = breadline(&ifile,linebuf,LINEBUFLEN)) > 0) {

	    double	x, y ;


	    len = rs ;
	    if (linebuf[len - 1] == '\n')
	        len -= 1 ;

	    linebuf[len] = '\0' ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("process: line=>%t<\n",
	            linebuf,len) ;
#endif /* CF_DEBUG */

	    if ((rs = field_start(&lf,linebuf,len)) >= 0) {

		x = 0.0 ;
		y = 0.0 ;
		for (n = 1 ; n < pip->ncol ; n += 1) {

	            fl = field_get(&lf,NULL,&fp) ;
	            if (fl > 0)
	                rs = getval(pip,fp,fl,&x) ;

		    if ((rs < 0) && pip->f.ignore)
			rs = SR_OK ;

		    if (rs < 0)
			break ;

		} /* end for */

		if (rs >= 0)
	            fl = field_get(&lf,NULL,&fp) ;

	        if ((rs >= 0) && (fl > 0))
	            rs = getval(pip,fp,fl,&y) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: getval() rs=%d x=%12.4f y=%12.4f\n",
			rs,x,y) ;
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

	    if (rs < 0)
	        break ;

	    c += 1 ;
	    line += 1 ;

	} /* end while (reading lines) */

	bclose(&ifile) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process: ret rs=%d c=%u\n",rs,c) ;
#endif /* CF_DEBUG */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (process) */


/* local subroutines */


static int putpair(pip,i,x,y)
struct proginfo	*pip ;
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
	    if (pip->n == 0)
	        rs = uc_malloc(size,&pip->pairs) ;

	    else
	        rs = uc_realloc(pip->pairs,size,&pip->pairs) ;

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


static int getval(pip,sp,sl,vp)
struct proginfo	*pip ;
char		*sp ;
int		sl ;
double		*vp ;
{
	int	rs ;

	char	*cp ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("getval: sl=%d sp=>%t<\n",
	        sl,sp,strnlen(sp,sl)) ;
#endif

	if (strnchr(sp,sl,'.') != NULL) {

	    pip->f.fdec = TRUE ;
	    rs = cfdecf(sp,sl,vp) ;

	} else {

	    ULONG	lw ;

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
	    debugprintf("getval: ret rs=%d v=%12.4f\n",
	        rs,*vp) ;
#endif

	return rs ;
}
/* end subroutine (getval) */



