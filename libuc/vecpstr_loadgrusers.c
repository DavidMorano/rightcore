/* vecpstr_loadgrusers */

/* find and load UNIX® users who have the given group as their default */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine finds all users who have the given specified group as
	their default group.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecpstr.h>
#include	<passwdent.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */

#define	SUBINFO		struct subinfo

#ifndef	SYSPASSWD
#define	SYSPASSWD	"/sys/passwd"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	strwcmp(const char *,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnwcpy(char *,int,const char *,int) ;


/* local structures */

struct subinfo {
	VECPSTR		*ulp ;
	void		*mapdata ;
	size_t		mapsize ;
	size_t		fsize ;
	gid_t		sgid ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,VECPSTR *,gid_t) ;
static int	subinfo_pwmapbegin(SUBINFO *) ;
static int	subinfo_pwmapload(SUBINFO *) ;
static int	subinfo_pwmapend(SUBINFO *) ;
static int	subinfo_finish(SUBINFO *) ;

static int	pwentparse(const char *,int,gid_t *) ;


/* local variables */


/* exported subroutines */


int vecpstr_loadgrusers(VECPSTR *ulp,gid_t sgid)
{
	SUBINFO		si, *sip = &si ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (ulp == NULL) return SR_FAULT ;

	if (sgid < 0) sgid = getgid() ;

	if ((rs = subinfo_start(sip,ulp,sgid)) >= 0) {
	    rs = subinfo_pwmapload(sip) ;
	    c = rs ;
	    rs1 = subinfo_finish(sip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecpstr_loadgrusers) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,VECPSTR *ulp,gid_t sgid)
{
	memset(sip,0,sizeof(SUBINFO)) ;
	sip->ulp = ulp ;
	sip->sgid = sgid ;
	return SR_OK ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	if (sip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_pwmapload(SUBINFO *sip)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = subinfo_pwmapbegin(sip)) >= 0) {
	    int		ml = sip->fsize ;
	    const char	*mp = (const char *) sip->mapdata ;
	    const char	*tp ;
	    while ((tp = strnchr(mp,ml,CH_NL)) != NULL) {
	        gid_t	gid ;
	        int	len = (tp-mp) ;
	        int	ul ;
	        if ((rs = pwentparse(mp,len,&gid)) > 0) {
		    ul = rs ;
#if	CF_DEBUGS
	            debugprintf("subinfo_pwmapload: un=%t gid=%d\n",
	                mp,ul,gid) ;
#endif
		    if (sip->sgid == gid) {
	                c += 1 ;
	                rs = vecpstr_adduniq(sip->ulp,mp,ul) ;
	            }
	        } /* end if (pwentparse) */
	        ml -= ((tp+1)-mp) ;
	        mp = (tp+1) ;
	        if (rs < 0) break ;
	    } /* end while (reading lines) */
	    rs1 = subinfo_pwmapend(sip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (grmems-pwmap) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_pwmapload) */


static int subinfo_pwmapbegin(SUBINFO *sip)
{
	int		rs = SR_OK ;
	if (sip->mapdata == NULL) {
	    const mode_t	om = 0666 ;
	    const int		of = O_RDONLY ;
	    const char		*fn = SYSPASSWD ;
	    if ((rs = uc_open(fn,of,om)) >= 0) {
	        const int	fd = rs ;
	        if ((rs = uc_fsize(fd)) >= 0) {
	            size_t	ms = rs ;
	            int		mp = PROT_READ ;
	            int		mf = MAP_SHARED ;
	            void	*md ;
	            sip->fsize = rs ;
	            if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	                const int	madv = MADV_SEQUENTIAL ;
			const caddr_t	ma = md ;
	                if ((rs = uc_madvise(ma,ms,madv)) >= 0) {
	                    sip->mapdata = md ;
	                    sip->mapsize = ms ;
	                } /* end if (advise) */
	                if (rs < 0)
	                    u_munmap(md,ms) ;
	            } /* end if (mmap) */
	        } /* end if (file-size) */
	        u_close(fd) ;
	    } /* end if (file-open) */
	} /* end if (need mapping) */
	return rs ;
}
/* end subroutine (subinfo_pwmapbegin) */


static int subinfo_pwmapend(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (sip->mapdata != NULL) {
	    size_t	ms = sip->mapsize ;
	    const void	*md = sip->mapdata ;
	    rs1 = u_munmap(md,ms) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->mapdata = NULL ;
	    sip->mapsize = 0 ;
	}
	return rs ;
}
/* end subroutine (subinfo_pwmapend) */


/* PASSWD entry parsing */
static int pwentparse(cchar lbuf[],int llen,gid_t *gp)
{
	int		rs = SR_OK ;
	int		fi ;
	int		ll = llen ;
	int		ul = 0 ;
	const char	*lp = lbuf ;
	const char	*tp ;
	for (fi = 0 ; fi < 4 ; fi += 1) {
	    if ((tp = strnchr(lp,ll,':')) != NULL) {
	        switch (fi) {
	        case 0:
	            ul = (tp-lp) ;
	            break ;
	        case 3:
	            {
	                int	v ;
	                if (cfdeci(lp,(tp-lp),&v) >= 0) {
	                    *gp = (gid_t) v ;
	                } else
	                    ul = 0 ;
	            } /* end block */
	            break ;
	        } /* end switch */
	        ll -= ((tp+1)-lp) ;
	        lp = (tp+1) ;
	    } /* end if (had separator) */
	} /* end for (looping through fields) */
	if (fi < 4) ul = 0 ;
	return (rs >= 0) ? ul : rs ;
}
/* end subroutine (pwentparse) */


