/* uc_opendev */

/* interface component for UNIX® library-3c */
/* open special overlay mount under the '/dev' directory*/


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine attempts to open a special overlay mounted resource of
	some kind under the '/dev' directory.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<opensysfs.h>
#include	<localmisc.h>


/* local defines */

#define	OPENDEV_DEVDNAME	"/dev"

#define	INETARGS	struct inetargs


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	getaf(const char *) ;
extern int	dialtcp(cchar *,cchar *,int,int,int) ;
extern int	dialtcpnls(const char *,const char *,int,const char *,int,int) ;
extern int	dialtcpmux(cchar *,cchar *,int,cchar *,cchar **,int,int) ;
extern int	dialudp(cchar *,cchar *,int,int,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* local structures */

struct argitem {
	const char	*p ;
	int		l ;
} ;

enum das {
	da_proto,
	da_af,
	da_host,
	da_svc,
	da_overlast
} ;

struct inetargs {
	struct argitem	ia[da_overlast] ;
	const char	*a ;		/* memory allocation */
} ;


/* forward references */

static int opendev_default(cchar *,int,mode_t) ;
static int opendev_inet(int,cchar *,int,int,int) ;

static int inetargs_start(INETARGS *,cchar *,int) ;
static int inetargs_finish(INETARGS *) ;


/* local variables */

static const char	*devnames[] = {
	"users",
	"groups",
	"projects",
	"tcp",
	"udp",
	NULL
} ;

enum devnames {
	devname_users,
	devname_groups,
	devname_projects,
	devname_tcp,
	devname_udp,
	devname_overlast
} ;

static const int	whiches[] = {
	OPENSYSFS_WUSERNAMES,
	OPENSYSFS_WGROUPNAMES,
	OPENSYSFS_WPROJECTNAMES,
	-1
} ;


/* exported subroutines */


/* ARGSUSED */
int uc_opendev(cchar *fname,int of,mode_t om,cchar **envv,int to,int opts)
{
	int		rs = SR_OK ;
	int		fl = -1 ;
	int		fi ;
	int		f_more = FALSE ;
	cchar		*tp ;

#if	CF_DEBUGS
	debugprintf("uc_opendev: fname=%s\n",fname) ;
#endif

	if (fname == NULL) return SR_FAULT ;
	if (fname[0] == '\0') return SR_INVALID ;

	while (fname[0] == '/') fname += 1 ;

	if ((tp = strchr(fname,'/')) != NULL) {
	    fl = (tp-fname) ;
	    while (fl && (fname[fl-1] == '/')) fl -= 1 ;
	    while (tp[0] == '/') tp += 1 ;
	    f_more = (tp[0] != '\0') ;
	}

	if ((fi = matstr(devnames,fname,fl)) >= 0) {
	    switch (fi) {
	    case devname_users:
	    case devname_groups:
	    case devname_projects:
		{
	            int	w = whiches[fi] ;
	            rs = opensysfs(w,of,-1) ;
		}
		break ;
	    case devname_tcp:
	    case devname_udp:
		{
		    if (f_more) {
		        rs = opendev_inet(fi,fname,of,to,opts) ;
		    } else {
			rs = opendev_default(fname,of,om) ;
		    }
		}
		break ;
	    default:
		rs = opendev_default(fname,of,om) ;
		break ;
	    } /* end switch */
	} else {
	    rs = opendev_default(fname,of,om) ;
	}

#if	CF_DEBUGS
	debugprintf("uc_opendev: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_opendev) */


/* private subroutines */


static int opendev_default(cchar *fname,int of,mode_t om)
{
	int		rs ;
	int		rs1 ;
	int		size = 0 ;
	int		fd = -1 ;
	cchar		*devdname = OPENDEV_DEVDNAME ;
	char		*fnbuf ;

	size += (strlen(devdname) + 1) ;
	size += 1 ;
	size += (strlen(fname) + 1) ;
	size += 1 ;
	if ((rs = uc_libmalloc(size,&fnbuf)) >= 0) {
	    if ((rs = mkpath2(fnbuf,devdname,fname)) >= 0) {
	        rs = u_open(fnbuf,of,om) ;
		fd = rs ;
	    }
	    rs1 = uc_libfree(fnbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (memory-allocation) */
	if ((rs < 0) && (fd >= 0)) u_close(fd) ;

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opendev_default) */


static int opendev_inet(int fi,cchar *fname,int of,int to,int no)
{
	INETARGS	ia ;
	int		rs ;
	int		fd = -1 ;

	if ((rs = inetargs_start(&ia,fname,-1)) >= 0) {
	    const char	*a = ia.ia[da_af].p ;
	    const char	*h = ia.ia[da_host].p ;
	    const char	*s = ia.ia[da_svc].p ;
	    if ((h != NULL) && (h[0] != '\0')) {
	        int	af = AF_UNSPEC ;
		if ((a != NULL) && (a[0] != '\0')) af = getaf(a) ;
		switch (fi) {
	        case devname_tcp:
	            rs = dialtcp(h,s,af,to,no) ;
		    fd = rs ;
		    break ;
	        case devname_udp:
	            rs = dialudp(h,s,af,to,no) ;
		    fd = rs ;
		    break ;
		} /* end switch */
	    } else {
		rs = SR_INVALID ;
	    }
	    inetargs_finish(&ia) ;
	} /* end if (inetargs) */

	if ((rs >= 0) && (of & O_CLOEXEC)) {
	    rs = uc_closeonexec(fd,TRUE) ;
	    if (rs < 0)
		u_close(fd) ;
	}

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opendev_inet) */


static int inetargs_start(INETARGS *iap,const char *sp,int sl)
{
	int		rs = SR_OK ;
	int		cl ;
	const char	*tp, *cp ;
	if (sl < 0) sl = strlen(sp) ;
	memset(iap,0,sizeof(INETARGS)) ;
	while (sl && (sp[0] == '/')) {
	    sp += 1 ;
	    sl -= 1 ;
	}
	iap->ia[da_proto].l = sl ;
	iap->ia[da_proto].p = sp ;
	if ((tp = strnchr(sp,sl,'/')) != NULL) {
	    iap->ia[da_host].l = (tp-sp) ;
	    sl -= ((tp+1)-sp) ;
	    sp = (tp+1) ;
	    iap->ia[da_host].l = sl ;
	    iap->ia[da_host].p = sp ;
	    if ((tp = strnchr(sp,sl,'/')) != NULL) {
	        iap->ia[da_host].l = (tp-sp) ;
	        sl -= ((tp+1)-sp) ;
	        sp = (tp+1) ;
	  	cl = sl ;
		cp = sp ;
	        if ((tp = strnchr(sp,sl,'/')) != NULL) {
	            cl = (tp-sp) ;
		}
	        iap->ia[da_svc].l = cl ;
	        iap->ia[da_svc].p = cp ;
		if ((tp = strnchr(cp,cl,'.')) != NULL) {
	            iap->ia[da_af].l = (tp-cp) ;
	            iap->ia[da_af].p = cp ;
	            iap->ia[da_svc].l = ((cp+cl)-(tp+1)) ;
	            iap->ia[da_svc].p = (tp+1) ;
		}
	    }
	}

	{
	    int		size = 0 ;
	    int		i ;
	    char	*bp ;
	    for (i = 0 ; i < da_overlast ; i += 1) {
		if ((iap->ia[i].l < 0) && (iap->ia[i].p != NULL)) {
		    iap->ia[i].l = strlen(iap->ia[i].p) ;
		}
	    } /* end for */
	    for (i = 0 ; i < da_overlast ; i += 1) {
		size += (iap->ia[i].p != NULL) ? (iap->ia[i].l+1) : 1 ;
	    }
	    if ((rs = uc_libmalloc(size,&bp)) >= 0) {
		cchar	*asp ;
		iap->a = bp ;
	        for (i = 0 ; i < da_overlast ; i += 1) {
		    asp = bp ;
		    if (iap->ia[i].p != NULL) {
			bp = (strwcpy(bp,iap->ia[i].p,iap->ia[i].l) + 1) ;
		    } else {
			*bp++ = '\0' ;
		    }
		    iap->ia[i].p = asp ;
		} /* end for */
	    } /* end if (memory-allocation) */
	} /* end block */

	return rs ;
}
/* end subroutine (inetargs_start) */


static int inetargs_finish(INETARGS *iap)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (iap->a != NULL) {
	    rs1 = uc_libfree(iap->a) ;
	    if (rs >= 0) rs = rs1 ;
	    iap->a = NULL ;
	}
	return rs ;
}
/* end subroutine (inetargs_finish) */


