/* pcsngdir */

/* return a directory name when given a newsgroup name */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will take its argument to be a newsgroup name. The
        subroutine will return the corresponding directory name in the spool
        area.

	The 'PCS' variable must be set.

	Synopsis:

	int pcsngdir(pcs,rbuf,bbnewsdir,newsgroup)
	const char	pcs[] ;
	char		rbuf[] ;
	const char	bbnewsdir[] ;
	const char	newsgroup[] ;

	Arguments:

	pcs		PCS program root
	rbuf		the directory path to the newsgroup relative
			to the BBNEWS spool directory
	bbnewsdir	the top of the BBNEWS spool directory
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
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	pathadd(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* global variables */


/* local structures */


/* exported subroutines */


int pcsngdir(pcs,ngdir,bbnewsdir,newsgroup)
const char	pcs[] ;
char		ngdir[] ;
const char	bbnewsdir[] ;
const char	newsgroup[] ;
{
	struct ustat	statbuf ;
	int		rs = SR_OK ;
	int		len = 0 ;
	int		f_first ;
	char		bbnewsdir2[MAXPATHLEN + 1] ;
	char		*bp ;
	char		*cp2 ;
	char		*ndp ;

#if	CF_DEBUGS
	debugprintf("pcsngdir: pcs=%s\n",pcs) ;
	debugprintf("pcsngdir: bbnewsdir=%s\n",bbnewsdir) ;
	debugprintf("pcsngdir: entered newsgroup=%s\n",newsgroup) ;
#endif

	if (pcs == NULL) return SR_FAULT ;
	if (newsgroup == NULL) return SR_FAULT ;
	if (bbnewsdir == NULL) return SR_FAULT ;

	if (newsgroup[0] == '\0') return SR_INVALID ;

	ndp = bbnewsdir2 ;
	if (bbnewsdir[0] != '/') {
	    ndp = strwcpy(ndp,pcs,-1) ;
	    ndp = strwcpy(ndp,"/",1) ;
	} /* end if */

	ndp = strwcpy(ndp,bbnewsdir,-1) ;

	ndp = strwcpy(ndp,"/",1) ;

#if	CF_DEBUGS
	debugprintf("pcsngdir: modified bbnewsdir=%s\n",bbnewsdir2) ;
#endif

	rs = u_stat(bbnewsdir2,&statbuf) ;

	if (rs < 0)
	    goto badnewsdir ;

#if	CF_DEBUGS
	debugprintf("pcsngdir: checking if not a DIR\n") ;
#endif

	rs = SR_NOTDIR ;
	if (! S_ISDIR(statbuf.st_mode))
	    goto badnewsdir ;

#if	CF_DEBUGS
	debugprintf("pcsngdir: about to do it\n") ;
#endif

/* assume the newsgroup is "add directory path" for starters */

	rs = mkpath1(ngdir,newsgroup) ;
	if (rs < 0) goto done ;

	cp2 = ngdir ;
	while ((bp = strchr(cp2,'.')) != NULL) {
	    *bp = '/' ;
	    cp2 = (bp + 1) ;
	} /* end while */

/* OK, start looking for the closest directory that matches */

#if	CF_DEBUGS
	debugprintf("pcsngdir: about to loop\n") ;
#endif

	f_first = TRUE ;
	rs = SR_ACCESS ;
	while (f_first || ((bp = strrchr(ngdir,'/')) != NULL)) {

	    if (! f_first)
	        *bp = '.' ;

	    f_first = FALSE ;
	    if ((rs = mkpath1(ndp,ngdir)) >= 0) {

	        if ((u_stat(bbnewsdir2,&statbuf) >= 0) && 
	            S_ISDIR(statbuf.st_mode) &&
	            (u_access(bbnewsdir2,W_OK) >= 0)) {

	            rs = SR_OK ;
	            break ;
	        }

	    } /* end if */

	    if (rs < 0) break ;
	} /* end while */

/* if we do not have a directory yet, try the last name we are left with */

	if (rs < 0) {

	    strcpy(ndp,ngdir) ;

	    if ((u_stat(bbnewsdir2,&statbuf) >= 0) && 
	        S_ISDIR(statbuf.st_mode) &&
	        (u_access(bbnewsdir2,W_OK) >= 0))
	        rs = SR_OK ;

	} /* end if (tried to make it) */

/* done */
done:
badnewsdir:
ret0:

#if	CF_DEBUGS
	debugprintf("pcsngdir: ret rs=%d len=%u\n",rs,len) ;
	debugprintf("pcsngdir: ngdir=%s\n",ngdir) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (pcsngdir) */


