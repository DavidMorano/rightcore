/* inter_trans */

/* translate (TRANS) compoeent of INTER */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 2009-01-20, David A­D­ Morano
	This is new code.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines for the TRANS component of INTER.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stddef.h>		/* for 'wchar_t' */

#include	<vsystem.h>
#include	<estrings.h>
#include	<hdrdecode.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"inter.h"


/* local defines */

#define	INTER_TRANS	struct inter_trans


/* external subroutines */

extern int	snsdd(char *,int,const char *,uint) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	wsnwcpynarrow(wchar_t *,int,cchar *,int) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	pathclean(char *,const char *,int) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfnext(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	nleadcasestr(const char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	msleep(int) ;
extern int	hasallalnum(const char *,int) ;
extern int	hasprintbad(const char *,int) ;
extern int	isprintlatin(int) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const void *,int) ;
extern int	strlinelen(const char *,int,int) ;
extern int	mkhexstr(char *,int,const void *,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnrchr(cchar *,int,int) ;
extern char	*strnsub(cchar *,int,cchar *) ;


/* external variables */


/* local structures */

struct inter_trans {
	HDRDECODE	hd ;
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int inter_transbegin(INTER *iap)
{
	if (iap == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (inter_transbegin) */


int inter_transend(INTER *iap)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (iap->trans != NULL) {
	    HDRDECODE	*hdp = iap->trans ;
	    rs1 = hdrdecode_finish(hdp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(iap->trans) ;
	    if (rs >= 0) rs = rs1 ;
	    iap->trans = NULL ;
	}
	return rs ;
}
/* end subroutine (inter_transend) */


int inter_transhd(INTER *iap)
{
	int		rs = SR_OK ;
	if (iap->trans == NULL) {
	    const int	osize = sizeof(INTER_TRANS) ;
	    void	*p ;
	    if ((rs = uc_malloc(osize,&p)) >= 0) {
		INTER_TRANS	*itp = p ;
		iap->trans = p ;
		memset(itp,0,osize) ;
		{	
		    PROGINFO	*pip = iap->pip ;
	            HDRDECODE	*hdp = &itp->hd ;
	            rs = hdrdecode_start(hdp,pip->pr) ;
		}
		if (rs < 0) {
		    uc_free(iap->trans) ;
	            iap->trans = NULL ;
		}
	    } /* end if (m-a) */
	}
	return rs ;
}
/* end subroutine (inter_transhd) */


int inter_transproc(INTER *iap,char *dbuf,int dlen,cchar *sp,int sl,int n)
{
	int		rs = SR_OK ;
	int		len = 0 ;
	if (strnsub(sp,sl,"=?") != NULL) {
	    if ((rs = inter_transhd(iap)) >= 0) {
	        INTER_TRANS	*itp = iap->trans ;
		const int	wsize = ((sl+1)*sizeof(wchar_t)) ;
		const int	wlen = sl ;
		wchar_t		*wbuf ;
		if ((rs = uc_malloc(wsize,&wbuf)) >= 0) {
		    HDRDECODE	*hdp = &itp->hd ;
		    if ((rs = hdrdecode_proc(hdp,wbuf,wlen,sp,sl)) >= 0) {
	    		rs = snwcpywidehdr(dbuf,dlen,wbuf,MIN(n,rs)) ;
			len = rs ;
		    }
		    uc_free(wbuf) ;
		} /* end if (m-a-f) */
	    } /* end if (inter_transhd) */
	} else {
	    rs = snwcpy(dbuf,dlen,sp,MIN(n,sl)) ;
	    len = rs ;
	}
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (inter_transproc) */


