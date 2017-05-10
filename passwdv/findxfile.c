/* findxfile */

/* find an executable file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2008-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used to find an executable file using the existing
        PATH environment variable.

	Synopsis:

	int findxfile(idp,buf,pn)
	IDS		*idp ;
	char		buf[] ;
	const char	pn[] ;

	Arguments:

	idp		pointer to IDS object
	buf		buffer to receive resulting path
	pn		program-name string to search for

	Returns:

	>0		length of found path
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#define	NENTS	40		/* initial number path components */


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	strkeycmp(const char *,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	getpwd(char *,int) ;
extern int	perm(const char *,uid_t,gid_t,void *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	xfile(IDS *,const char *) ;
extern int	getprogpath(IDS *,VECSTR *,char *,const char *,int) ;
extern int	vecstr_adduniq(VECSTR *,cchar *,int) ;
extern int	vecstr_addpath(VECSTR *,cchar *,int) ;
extern int	vecstr_addcspath(VECSTR *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int findxfile(IDS *idp,char *rbuf,cchar *pn)
{
	int		rs = SR_NOENT ;
	int		rs1 ;
	int		cl ;
	int		len = 0 ;
	const char	*path = getenv(VARPATH) ;

	rbuf[0] = '\0' ;
	if (path != NULL) {
	    VECSTR	plist ;
	    const int	ne = NENTS ;
	    int		f_pwd = FALSE ;

	    if ((rs = vecstr_start(&plist,ne,0)) >= 0) {
		cchar	*tp ;
		cchar	*sp = path ;
	        char	cbuf[MAXPATHLEN + 1] ;

	        while ((tp = strpbrk(sp,":;")) != NULL) {

	            if ((tp-sp) == 0) {
	                f_pwd = TRUE ;
		    }

	            if ((rs = pathclean(cbuf,sp,(tp-sp))) >= 0) {
	                cl = rs ;
	                rs = vecstr_adduniq(&plist,cbuf,cl) ;
	            }

	            sp = (tp + 1) ;
	            if (rs < 0) break ;
	        } /* end while */

	        if ((rs >= 0) && (sp[0] != '\0')) {
	            rs = vecstr_adduniq(&plist,sp,-1) ;
		}

	        if (rs >= 0) {
	            rs = getprogpath(idp,&plist,rbuf,pn,-1) ;
		    len = rs ;
		}

	        if ((! f_pwd) && (rs == SR_NOENT)) {
	            if ((rs = xfile(idp,pn)) >= 0) {
	                rs = mkpath1(rbuf,pn) ;
		        len = rs ;
		    }
	        } /* end if */

	        rs1 = vecstr_finish(&plist) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (path-list) */
	} else {
	    if ((rs = xfile(idp,pn)) >= 0) {
		rs = mkpath1(rbuf,pn) ;
		len = rs ;
	    }
	} /* end if (non-null) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (findxfile) */


