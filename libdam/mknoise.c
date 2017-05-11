/* mknoise */

/* retrieve process ID related noise from the system */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SAFE		0
#define	CF_STAT		1		/* 'stat(2)' the file also */


/* revision history:

	= 2002-07-13, David A­D­ Morano
        This is just a quick hack to get some additional noise from a
        UNIX® system that is fairly portable (a big problem). I really
        failed here because the most noise we get is from the presence
        of the current processes on the machine and this is very
        non-portable on systems not running System V.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine acquires some of the process noise in the system.  It
	basically just looks at the process IDs that exist at the moment of the
	call and returns a random sequence of data based on that.

	Synopsis:

	int mknoise(uint *a,int n)

	Arguments:

	a	array of integers to receive the noise
	n	number of integers supplied by caller

	Returns:

	>=0	number of intergers containing random data being returned
	<0	error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<fsdir.h>
#include	<sha1.h>
#include	<localmisc.h>


/* local defines */

#define	PROCDNAME	"/proc"
#define	NENTS	1000

#define	NOISEDATA	struct noisedata


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

NOISEDATA {
	int		maxi ;
	int		i, c ;
	int		pass ;
	ushort		buf[NENTS + 1] ;
} ;


/* forward references */

static int	noisedata_start(NOISEDATA *) ;
static int	noisedata_add(NOISEDATA *,uint) ;
static int	noisedata_finish(NOISEDATA *) ;

static int	noisedata_addtime(NOISEDATA *) ;
static int	noisedata_addids(NOISEDATA *,const char *) ;


/* local variables */


/* exported subroutines */


int mknoise(uint *a,int n)
{
	NOISEDATA	nd ;
	int		rs ;

	if (a == NULL) return SR_FAULT ;

	if ((rs = noisedata_start(&nd)) >= 0) {
	    if ((rs = noisedata_addtime(&nd)) >= 0) {
	        if ((rs = noisedata_addids(&nd,PROCDNAME)) >= 0) {

	if (nd.c > 0) {
	    SHA1	d ;
	    int		size ;

	    if ((rs = sha1_start(&d)) >= 0) {

	        size = nd.maxi * sizeof(ushort) ;
	        sha1_update(&d,(char *) nd.buf,size) ;

	        if (n >= 5) {
	            sha1_digest(&d,(uchar *) a) ;
	            n = 5 ;
	        } else {
	            uint	aa[5] ;
	            sha1_digest(&d,(uchar *) aa) ;
	            size = n * sizeof(uint) ;
	            memcpy(a,aa,size) ;
	        }

	        sha1_finish(&d) ;
	    } /* end if (sha1) */

	            } /* end if (positive) */
	        } /* end if (noisedata_addids) */
	    } /* end if (noisedata_addtime) */
	    noisedata_finish(&nd) ;
	} /* end if (noisedata) */

	return (nd.c > 0) ? n : rs ;
}
/* end subroutine (mknoise) */


/* local subroutines */


static int noisedata_addtime(NOISEDATA *ndp)
{
	struct timeval	tv ;
	int		rs ;

	if ((rs = uc_gettimeofday(&tv,NULL)) >= 0) {
	noisedata_add(ndp,(uint) (tv.tv_sec >> 16)) ;
	noisedata_add(ndp,(uint) (tv.tv_sec >> 0)) ;
	noisedata_add(ndp,(uint) (tv.tv_usec >> 16)) ;
	noisedata_add(ndp,(uint) (tv.tv_usec >> 0)) ;
	}

	return rs ;
}
/* end subroutine (noisedata_addtime) */


static int noisedata_addids(NOISEDATA *ndp,cchar *procdname)
{
	struct ustat	sb ;
	fsdir		pdir ;
	fsdir_ent	ds ;
	int		rs, rs1 ;
	int		dlen ;
	int		v ;
	char		tmpfname[MAXPATHLEN + 3], *bp ;
	char		*dnp ;

	bp = strwcpy(tmpfname,procdname,MAXPATHLEN) ;
	*bp++ = '/' ;

	if ((rs = fsdir_open(&pdir,procdname)) >= 0) {

	while ((dlen = fsdir_read(&pdir,&ds)) > 0) {
	    dnp = ds.name ;
	    if (*dnp == '.') {
	        if (dnp[1] == '\0') continue ;
	        if ((dnp[1] == '.') && (dnp[2] == '\0')) continue ;
	    }

#if	CF_STAT

	    strwcpy(bp,dnp,(MAXPATHLEN - (bp - tmpfname))) ;

	    rs1 = u_stat(tmpfname,&sb) ;

	    if (rs1 >= 0)
	        noisedata_add(ndp,(uint) sb.st_mtime) ;

#endif /* CF_STAT */

	    rs1 = cfdeci(dnp,dlen,&v) ;

	    if (rs1 >= 0) {

	        if (ndp->pass & 1)
	            v = (v << 8) | ((v >> 8) & 255) ;

	        noisedata_add(ndp,v) ;

	    } /* end if (got a PID) */

	} /* end while (reading entries) */

	fsdir_close(&pdir) ;
	} /* end if (fsdir) */

	return rs ;
}
/* end subroutine (noisedata_addids) */


static int noisedata_start(NOISEDATA *ndp)
{

	ndp->i = 0 ;
	ndp->c = 0 ;
	ndp->maxi = 0 ;
	ndp->pass = 0 ;
	return 0 ;
}
/* end subroutine (noisedata_start) */


static int noisedata_finish(NOISEDATA *ndp)
{
	int		i ;
	for (i = 0 ; i < ndp->maxi ; i += 1) {
	    ndp->buf[i] = 0 ;
	}
	return 0 ;
}
/* end subroutine (noisedata_finish) */


static int noisedata_add(NOISEDATA *ndp,uint v)
{

	ndp->buf[ndp->i] ^= v ;
	ndp->i = (ndp->i + 1) % NENTS ;
	if (ndp->i > ndp->maxi)
	    ndp->maxi = ndp->i ;

	if (ndp->i == 0)
	    ndp->pass += 1 ;

	ndp->c += 1 ;
	return 0 ;
}
/* end subroutine (noisedata_add) */


