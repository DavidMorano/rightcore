/* mkuserpath */

/* make a user-path */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates a resolved filename path from the coded form.

	Synopsis:

	int mkuserpath(rbuf,pp,pl) ;
	char		rbuf[] ;
	cchar	*pp ;
	int		pl ;

	Arguments:

	rbuf		result buffer (should be MAXPATHLEN+1 long)
	pp		source path pointer
	pl		source path length

	Returns:

	<0		error
	==0		no expansion
	>0		expansion


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<localmisc.h>


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath2w(char *,cchar *,cchar *,int) ;
extern int	strwcmp(cchar *,cchar *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;


/* local structures */


/* forward references */

static int	mkpathsquiggle(char *,cchar *,cchar *,int) ;
static int	mkpathuserfs(char *,cchar *,int) ;
static int	mkpathusername(char *,cchar *,int,cchar *,int) ;


/* local variables */


/* exported subroutines */


int mkuserpath(char *rbuf,cchar *un,cchar *pp,int pl)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("mkuserpath: ent p=%t\n",pp,pl) ;
#endif

	if (rbuf == NULL) return SR_FAULT ;

	rbuf[0] = '\0' ;
	if (pp == NULL) return SR_FAULT ;

	if (pl < 0) pl = strlen(pp) ;

	while ((pl > 0) && (pp[0] == '/')) {
	    pp += 1 ;
	    pl -= 1 ;
	}

	if (pl > 0) {
	    if (pp[0] == '~') {
	        pp += 1 ;
	        pl -= 1 ;
	        rs = mkpathsquiggle(rbuf,un,pp,pl) ;
	    } else if (pp[0] == 'u') {
	        rs = mkpathuserfs(rbuf,pp,pl) ;
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("mkuserpath: ret rs=%d\n",rs) ;
	debugprintf("mkuserpath: ret rbuf=%s\n",rbuf) ;
#endif

	return rs ;
}
/* end subroutine (mkuserpath) */


/* local subroutines */


static int mkpathsquiggle(char *rbuf,cchar *un,cchar *pp,int pl)
{
	int		rs ;
	int		ul = pl ;
	cchar		*tp ;
	cchar		*up = pp ;

	if (pl < 0) pl = strlen(pp) ;

	if ((tp = strnchr(pp,pl,'/')) != NULL) {
	    ul = (tp-pp) ;
	    pl -= ((tp+1)-pp) ;
	    pp = (tp+1) ;
	} else {
	    pp += pl ;
	    pl = 0 ;
	}

	if ((ul == 0) && (un != NULL)) {
	    up = un ;
	    ul = -1 ;
	}

	rs = mkpathusername(rbuf,up,ul,pp,pl) ;

	return rs ;
}
/* end subroutine (mkpathsqiggle) */


static int mkpathuserfs(char *rbuf,cchar *pp,int pl)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("mkpathuserfs: ent p=>%t<\n",pp,strlinelen(pp,pl,60)) ;
#endif

	if ((pl >= 2) && (strncmp("u/",pp,2) == 0)) {
	    pp += 2 ;
	    pl -= 2 ;
	    if (pl > 0) {

/* get the username */

	        while (pl && (pp[0] == '/')) {
	            pp += 1 ;
	            pl -= 1 ;
	        }

#if	CF_DEBUGS
	        debugprintf("mkpathuserfs: p=>%t<\n",
	            pp,strlinelen(pp,pl,60)) ;
#endif

	        if (pl > 0) {
	            cchar	*tp ;
	            cchar	*up = pp ;
	            int		ul = pl ;
	            if ((tp = strnchr(pp,pl,'/')) != NULL) {
	                ul = (tp - pp) ;
	                pl -= ((tp+1)-pp) ;
	                pp = (tp+1) ;
	            } else {
	                pp += pl ;
	                pl = 0 ;
	            }
#if	CF_DEBUGS
	            debugprintf("mkpathuserfs: pop rs=%d u=%t s=%s\n",
			rs,up,ul,pp) ;
#endif
	            rs = mkpathusername(rbuf,up,ul,pp,pl) ;
	        } /* end if (positive) */

	    } /* end if (positive) */
	} /* end if (user-fs called for) */

#if	CF_DEBUGS
	debugprintf("mkpathuserfs: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mkpathuserfs) */


static int mkpathusername(char *rbuf,cchar *up,int ul,cchar *sp,int sl)
{
	const int	ulen = USERNAMELEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	cchar		*un = up ;
	char		ubuf[USERNAMELEN+1] ;

#if	CF_DEBUGS
	debugprintf("mkpathusername: ul=%d u=%t s=%t\n",ul,up,ul,sp,sl) ;
#endif

	if (ul >= 0) {
	    rs = strwcpy(ubuf,up,MIN(ul,ulen)) - ubuf ;
	    un = ubuf ;
	}

#if	CF_DEBUGS
	debugprintf("mkpathusername: mid rs=%d u=%s s=%t\n",rs,un,sp,sl) ;
#endif

	if (rs >= 0) {
	    if ((rs = getbufsize(getbufsize_pw)) >= 0) {
	        struct passwd	pw ;
	        const int	pwlen = rs ;
	        char		*pwbuf ;
	        if ((rs = uc_libmalloc((pwlen+1),&pwbuf)) >= 0) {
	            if ((un[0] == '\0') || (un[0] == '-')) {
	                rs = getpwusername(&pw,pwbuf,pwlen,-1) ;
	            } else {
	                rs = GETPW_NAME(&pw,pwbuf,pwlen,un) ;
	            }
	            if (rs >= 0) {
		        cchar	*dir = pw.pw_dir ;
	                if (sl > 0) {
	                    rs = mkpath2w(rbuf,dir,sp,sl) ;
	                } else {
	                    rs = mkpath1(rbuf,dir) ;
	                }
	            }
	            rs1 = uc_libfree(pwbuf) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (memory-allocation) */
	    } /* end if (getbufsize) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("mkpathusername: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mkpathusername) */


