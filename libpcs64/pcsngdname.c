/* pcsngdname */

/* create a directory name when given a newsgroup name */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will take its argument to be a newsgroup name.
	The subroutine will return the corresponding directory name in the
	spool area.

	Synopsis:

	int pcsngdname(pcs,rbuf,newsdname,ngname)
	const char	pcs[] ;
	char		rbuf[] ;
	const char	newsdname[] ;
	const char	ngname[] ;

	Arguments:

	pcs		PCS program root
	rbuf		the directory path to the newsgroup relative
			to the BBNEWS spool directory
	newsdname	the top of the BBNEWS spool directory
	ngname		a name of a newsgroup

	Returns:

	>0		length of returned directory name
	SR_FAULT	NULL argument(s) was given
	SR_INVALID	an invalid argument was given


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bwops.h>
#include	<localmisc.h>


/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(c)	((c) & 0xff) ;
#endif


/* external subroutines */

extern int	pathadd(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	ipow(int,int) ;
extern int	isOneOf(const int *,int) ;
extern int	isNotPresent(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* external variables */


/* global variables */


/* local structures */


/* forward references */

static int	mknewsdname(const char *,char *,const char *) ;
static int	mkdname(char *,int,const char *,int) ;
static int	ndots(const char *) ;
static int	isNotOurs(int) ;


/* local variables */


/* exported subroutines */


int pcsngdname(pcs,rbuf,newsdname,ngname)
const char	pcs[] ;
char		rbuf[] ;
const char	newsdname[] ;
const char	ngname[] ;
{
	struct ustat	statbuf ;

	int	rs = SR_OK ;
	int	n, c ;
	int	len = 0 ;
	int	f_first ;

	char	bbnewsdir2[MAXPATHLEN + 1] ;
	char	*bp ;
	char	*cp2 ;
	char	*ndp ;


#if	CF_DEBUGS
	debugprintf("pcsngdname: pcs=%s\n",pcs) ;
	debugprintf("pcsngdname: newsdname=%s\n",newsdname) ;
	debugprintf("pcsngdname: ngname=%s\n",ngname) ;
#endif

	if (pcs == NULL) return SR_FAULT ;
	if (newsdname == NULL) return SR_FAULT ;
	if (ngname== NULL) return SR_FAULT ;

	if (ngname[0] == '\0') return SR_INVALID ;

	n = ndots(ngname) ;

	if ((rs = mknewsdname(pcs,rbuf,newsdname)) >= 0) {
	    struct ustat	sb ;
	    const int	npow = ipow(2,n) ;
	    int		rlen = rs ;
	    if (n > 0) {
	        for (c = 0 ; c < npow ; c += 1) {
	            if ((rs = mkdname(rbuf,rlen,ngname,c)) >= 0) {
		        len = rs ;
#if	CF_DEBUGS
	debugprintf("pcsngdname: rbuf=%s\n",rbuf) ;
#endif
		        if ((rs = u_stat(rbuf,&sb)) >= 0) {
			    if (S_ISDIR(sb.st_mode)) break ;
			    rs = SR_NOTDIR ;
		        }
#if	CF_DEBUGS
	debugprintf("pcsngdname: u_stat() rs=%d\n",rs) ;
#endif
		    } /* end if (mkdname) */
		    if ((rs >= 0) || isNotOurs(rs)) break ;
	        } /* end for */
	    } else {
	        if ((rs = pathadd(rbuf,rlen,ngname)) >= 0) {
		    len = rs ;
		    if ((rs = u_stat(rbuf,&sb)) >= 0) {
			if (! S_ISDIR(sb.st_mode)) rs = SR_NOTDIR ;
		    }
		} /* end if (pathadd) */
	    } /* end if */
	} /* end if (mkpath) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (pcsngdname) */


/* local subroutines */


static int mknewsdname(const char *pr,char *rbuf,const char *newsdname)
{
	int	rs ;
	if (newsdname[0] != '/') {
	    rs = mkpath2(rbuf,pr,newsdname) ;
	} else {
	    rs = mkpath1(rbuf,newsdname) ;
	}
	return rs ;
}
/* end subroutine (mknewsdname) */


static int mkdname(char *rbuf,int rlen,const char *ngname,int mask)
{
	int	rs ;
	int	len = 0 ;

	if ((rs = pathadd(rbuf,rlen,ngname)) >= 0) {
	    len = rs ;
	    int	nlen = (rlen+1) ;
	    int	c = 0 ;
	    int	i ;
	    const char	*tp ;
	    if ((tp = strnchr(rbuf,rlen,'.')) != NULL) {
		nlen = (tp-rbuf) ;
	    }
	    for (i = (len-1) ; i >= nlen ; i -= 1) {
		int	ch = MKCHAR(rbuf[i]) ;
		if (ch == '.') {
		    if (! bwtsti(mask,c)) {
			rbuf[i] = '/' ;
			c += 1 ;
		    }
		}
	    } /* end if */
	} /* end if (pathadd) */

	return rs ;
}
/* end subroutine (mkdname) */


static int ndots(const char *ngname)
{
	int	n = 0 ;
	int	i ;

	for (i = 0 ; ngname[i] ; i += 1) {
	    if (ngname[i] == '.') n += 1 ;
	}

	return n ;
}
/* end subroutine (ndots) */


#if	CF_DEBUGS
static int isNotOurs(int rs) {
	int	f = TRUE ;
	debugprintf("isNotOurs: isNotPresent()=%u\n",isNotPresent(rs)) ;
	if (isNotPresent(rs) || (rs == SR_NOTDIR)) f = FALSE ;
	debugprintf("isNotOurs: ret f=%u\n",f) ;
	return f ;
}
#else /* CF_DEBUGS */
static int isNotOurs(int rs) {
	int	f = TRUE ;
	if (isNotPresent(rs) || (rs == SR_NOTDIR)) f = FALSE ;
	return f ;
}
/* end subroutine (isNotOurs) */
#endif /* CF_DEBUGS */



