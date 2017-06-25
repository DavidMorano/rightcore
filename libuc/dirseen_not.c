/* dirseen_not */

/* some extra utility subroutines (methods) for the DIRSEEN object */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

        This pair of subroutines makes finding previously not seen directories
        easier.

	Synopsis:

	int dirseen_notseen(op,sbp,dbuf,slen)
	int		*op ;
	struct ustat	*sbp ;
	int		dbuf[] ;
	int		dlen ;

	Arguments:

	op		pointer to object
	sbp		pointer to USTAT structure
	dbuf		directory buffer
	dlen		length of string in directory buffer

	Returns:

	<0		error
	==0		previously seen
	>0		not yet seen


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<dirseen.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	isprintlatin(int) ;
extern int	isOneOf(const int *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strcpylc(char *,const char *) ;
extern char	*strcpyuc(char *,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int dirseen_notseen(DIRSEEN *dsp,struct ustat *sbp,cchar *dbuf,int dlen)
{
	const int	nrs = SR_NOENT ;
	int		rs ;
	int		f = FALSE ;

	sbp->st_ino = 0 ;
	sbp->st_dev = 0 ;
	if ((rs = dirseen_havename(dsp,dbuf,dlen)) == nrs) {
	    if ((rs = u_stat(dbuf,sbp)) >= 0) {
	        if (S_ISDIR(sbp->st_mode)) {
	    	    if ((rs = dirseen_havedevino(dsp,sbp)) == nrs) {
			rs = SR_OK ;
			f = TRUE ;
		    }
		}
	    } else if (isNotPresent(rs)) {
		rs = SR_OK ;
	    }
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (dirseen_notseen) */


int dirseen_notadd(DIRSEEN *dsp,struct ustat *sbp,cchar *dbuf,int dlen)
{
	int		rs ;
	char		cbuf[MAXPATHLEN+1] ;

	if ((rs = pathclean(cbuf,dbuf,dlen)) > 0) {
	    rs = dirseen_add(dsp,cbuf,rs,sbp) ;
	}

	return rs ;
}
/* end subroutine (dirseen_notadd) */


