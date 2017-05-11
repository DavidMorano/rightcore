/* uc_openuser */

/* interface component for UNIX® library-3c */
/* open a user file */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine opens a user file. This is a file that is relative
        (under) a user-home directory.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<opensysfs.h>
#include	<localmisc.h>


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;


/* local structures */


/* forward references */

int	uc_openuserbase(struct ucopeninfo *oip) ;
int	uc_openuserpath(struct ucopeninfo *oip) ;


/* local variables */


/* exported subroutines */


int uc_openuser(username,upath,oflags,operms,to)
const char	username[] ;
const char	upath[] ;
int		oflags ;
mode_t		operms ;
int		to ;
{
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	char		*pwbuf ;
	char		fname[MAXPATHLEN+1] ;

	if (username == NULL) return SR_FAULT ;
	if (upath == NULL) return SR_FAULT ;
	if (username[0] == '\0') return SR_INVALID ;

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    if (username[0] == '-') {
	        rs = getpwusername(&pw,pwbuf,pwlen,-1) ;
	    } else {
	        rs = GETPW_NAME(&pw,pwbuf,pwlen,username) ;
	    }
	    if (rs >= 0) {
	        if ((rs = mkpath2(fname,pw.pw_dir,upath)) >= 0) {
	            struct ucopeninfo	oi ;
	            memset(&oi,0,sizeof(struct ucopeninfo)) ;
	            oi.fname = fname ;
	            oi.oflags = oflags ;
	            oi.operms = operms ;
	            oi.to = to ;
	            rs = uc_openinfo(&oi) ;
	        } /* end if (mkpath) */
	    } /* end if (ok) */
	    uc_free(pwbuf) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (uc_openuser) */


int uc_openuserinfo(struct ucopeninfo *oip)
{
	int		rs ;
	const char	*fp = oip->fname ;

	while (fp[0] == '/') fp += 1 ;

	if (fp[0] == '\0') {
	    rs = uc_openuserbase(oip) ;
	} else {
	    rs = uc_openuserpath(oip) ;
	}

	return rs ;
}
/* end subroutine (uc_openuserinfo) */


int uc_openuserbase(struct ucopeninfo *oip)
{
	const int	w = OPENSYSFS_WUSERHOMES ;
	const int	of = oip->oflags ;
	const int	to = oip->to ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("uc_openuserbase: w=%u\n",w) ;
#endif

	rs = opensysfs(w,of,to) ;

#if	CF_DEBUGS
	debugprintf("uc_openuserbase: opensysfs() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_openuserbase) */


int uc_openuserpath(struct ucopeninfo *oip)
{
	const int	ulen = USERNAMELEN ;
	int		rs = SR_NOENT ;
	int		ul = -1 ;
	const char	*fp = oip->fname ;
	const char	*tp ;
	const char	*un = NULL ;
	char		ubuf[USERNAMELEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("uc_openuserpath: fname=%s\n",oip->fname) ;
	debugprintf("uc_openuserpath: oflags=%04X operms=0%3o\n",
	    oip->oflags,oip->operms) ;
#endif

	while (fp[0] == '/') fp += 1 ;

	un = fp ;
	if ((tp = strchr(fp,'/')) != NULL) {
	    ul = (tp-fp) ;
	    fp = (tp+1) ;
	} else {
	    fp += strlen(fp) ;
	}

#if	CF_DEBUGS
	debugprintf("uc_openuserpath: un=%t\n",un,ul) ;
	debugprintf("uc_openuserpath: fp=%s\n",fp) ;
#endif

	if (un[0] != '\0') {
	    if ((rs = snwcpy(ubuf,ulen,un,ul)) >= 0) {
		struct passwd	pw ;
		const int	pwlen = getbufsize(getbufsize_pw) ;
		char		*pwbuf ;
		if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	            if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,ubuf)) >= 0) {
	                cchar	*ud = pw.pw_dir ;
#if	CF_DEBUGS
	                debugprintf("uc_openuserpath: ud=%s\n",ud) ;
	                debugprintf("uc_openuserpath: fp=%s\n",fp) ;
#endif
	                if (ud[0] != '\0') {
	                    while (fp[0] == '/') fp += 1 ;
	                    if (fp[0] != '\0') {
	                        if ((rs = mkpath2(tmpfname,ud,fp)) >= 0) {
	                            oip->fname = tmpfname ;
	                            rs = uc_openinfo(oip) ;
	                        }
	                    } else {
	                        const int	of = oip->oflags ;
	                        rs = u_open(ud,of,0666) ;
	                    }
	                } else {
	                    rs = SR_NOENT ;
			}
	            } /* end if (getpw-name) */
		    uc_free(pwbuf) ;
		} /* end if (memory-allocation) */
	    } /* end if (cpy) */
	} else
	    rs = SR_NOENT ;

	return rs ;
}
/* end subroutine (uc_openuserpath) */


