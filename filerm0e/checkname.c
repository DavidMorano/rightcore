/* checkname */

/* check a directory entry for a match */


#define	CF_DEBUG 	0		/* run-time debugging */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/***********************************************************************

	This subroutine is called by the 'process()' subroutine in order
	to check if a filename is marked for some disposition.

	Synopsis:

	int checkname(name,sbp,pip)
	char		name[] ;
	struct ustat	*sbp ;
	struct proginfo	*pip ;

	Arguments:

	name		directory entry
	sbp		'stat' block pointer
	pip		user specified argument

	Returns:

	>0		skip this directory entry in directory walk
	0		continue with directory walk as usual
	<0		exit directory walk altogether


***********************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<bfile.h>
#include	<fsdir.h>
#include	<storebuf.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"removename.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 10),2048)
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	checklink(char *,struct ustat *,struct proginfo *) ;
extern int	bufprintf(char *,int,const char *,...) ;

extern int	burn(randomvar *,int,const char *) ;


/* external variables */


/* forward references */

static int	checkad(struct proginfo *,const char *,int,const char *) ;
static int	mkpathadd(char *,const char *,int,const char *,int) ;


/* exported subroutines */


int checkname(name,sbp,pip)
char		name[] ;
struct ustat	*sbp ;
struct proginfo	*pip ;
{
	struct ustat	sb2 ;

	randomvar	*rvp = pip->rvp ;

	int	rs = SR_OK ;
	int	rs1 = 0 ;
	int	rs2 ;
	int	i ;
	int	bnl, dnl ;
	int	bcount = pip->bcount ;
	int	rc = 0 ;
	int	f_link ;
	int	f_follow ;

	const char	*sp ;
	const char	*dnp ;
	const char	*bnp ;
	const char	*cp ;


	pip->c_processed += 1 ;
	bnl = sfbasename(name,-1,&bnp) ;
	if (bnl <= 0) goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("checkname: mode=%7o name=%s\n",
	        sbp->st_mode,bnp) ;
#endif

	f_link = S_ISLNK(sbp->st_mode) ? TRUE : FALSE ;
	f_follow = pip->f.follow ;

/* don't follow the directory (if there is one) under these conditions */

	rc = f_link && (! f_follow) ;

/* OK, do the appropriate thing */

	if (pip->f.all) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("checkname: remove all mode\n") ;
#endif

	    if (! pip->f.no) {

	    rs1 = 0 ;
	        if (S_ISREG(sbp->st_mode) || S_ISDIR(sbp->st_mode)) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("checkname: REG or DIR\n") ;
#endif

	            rs1 = removename(rvp,bcount,pip->rn_opts,name) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("checkname: removename() rs=%d\n",rs1) ;
#endif

	        } else if (! (f_link && f_follow)) {

	            rs1 = u_unlink(name) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("checkname: uc_remove() rs=%d\n",rs1) ;
#endif

	            if (rs1 >= 1) rs1 = 1 ;

	        } /* end if */

	    if (rs1 > 0)
	        pip->c_removed += rs1 ;

	    } /* end if */

	    if (pip->f.print)
	        bprintf(pip->ofp,"%s\n",name) ;

	} /* end if (remove all) */

/* check for a dangling symbolic link */

	if (pip->f.links && S_ISLNK(sbp->st_mode)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("checkname: symbolic link\n") ;
#endif

	    if (checklink(name,sbp,pip) > 0) {
	        rc = 0 ;
	 	goto ret0 ;
	    }

	} /* end if */

/* should we check for a dangling AppleDouble file? */

	if (pip->f.appledouble && (strcmp(bnp,APPLEDOUBLE) == 0)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("checkname: AD ad=%s\n",bnp) ;
#endif

	    if (pip->f.follow && S_ISLNK(sbp->st_mode)) {

	        sbp = &sb2 ;
	        rs2 = u_stat(name,&sb2) ;
	        if (rs2 < 0) {
	            rc = 0 ;
		    goto ret0 ;
		}

	    } /* end if */

	    if (S_ISDIR(sbp->st_mode)) {

/* get the parent directory name */

	        if ((dnl = sfdirname(name,-1,&dnp)) > 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("checkname: AD pdir=%W\n",dnp,dnl) ;
#endif

	            checkad(pip,dnp,dnl,name) ;
	            rc = 1 ;
		    goto ret0 ;
	        }

	    } /* end if */

	} /* end if (AppleDouble check) */

/* check for a name match */

	if (! pip->f.all) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("checkname: not-all mode name=%s bn=%s\n",
	            name,bnp) ;
#endif

	    for (i = 0 ; (rs = vecstr_get(&pip->names,i,&sp)) >= 0 ; i += 1) {
	        if (sp == NULL) continue ;

	        if (sfsub(bnp,-1,sp,&cp) >= 0)
	            break ;

	    } /* end for */

	    if (rs >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("checkname: name substring match \n") ;
#endif

	        if (! pip->f.no) {

		rs1 = 0 ;
	            if (S_ISREG(sbp->st_mode) || S_ISDIR(sbp->st_mode)) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("checkname: REG or DIR\n") ;
#endif

	                rs1 = removename(rvp,bcount,pip->rn_opts,name) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("checkname: removename() rs=%d\n",rs1) ;
#endif

	            } else if (! (f_link && f_follow)) {

	                rs1 = u_unlink(name) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("checkname: uc_remove() rs=%d\n",rs1) ;
#endif

	                if (rs1 >= 0) rs1 = 1 ;

	            } /* end if */

	        if (rs1 > 0)
	            pip->c_removed += rs1 ;

	        } /* end if */

	        if (pip->f.print)
	            bprintf(pip->ofp,"%s\n", name) ;

	    } /* end if */

	} /* end if (name match) */

ret0:
	return rc ;
}
/* end subroutine (checkname) */


/* local subroutines */


/* check for the proper APPLEDOUBLE file */
static int checkad(pip,pdirname,pdirlen,dirname)
struct proginfo	*pip ;
const char	pdirname[] ;
const char	dirname[] ;
int		pdirlen ;
{
	struct ustat	sb ;

	FSDIR		dir ;
	FSDIR_ENT	slot ;

	int	rs ;
	int	rs1, rs2 ;
	int	bnl ;

	const char	*bnp ;

	char	fname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("checkad: entered name=%s\n",
	        dirname) ;
#endif

	if ((rs = fsdir_open(&dir,dirname)) >= 0) {

	    while ((bnl = fsdir_read(&dir,&slot)) > 0) {

	        bnp = slot.name ;
	        if (strcmp(bnp,APPLEPARENT) == 0)
	            continue ;

		if ((rs = mkpathadd(fname,pdirname,pdirlen,bnp,bnl)) >= 0) {

	        rs2 = u_stat(fname,&sb) ;
	        if (rs2 == SR_NOENT) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("checkad: not there datafork=%s\n",
			fname) ;
#endif

	            rs = mkpath2(fname,dirname,bnp) ;

		    if (rs >= 0) {

	            if ((pip->verboselevel > 0) || pip->f.print)
	                bprintf(pip->ofp,"%s\n",fname) ;

	            rs1 = SR_OK ;
	            if (! pip->f.no) {

	                if (pip->f.burn)
	                    burn(pip->rvp,pip->bcount,fname) ;

	                rs1 = u_unlink(fname) ;
	                if (rs1 >= 0)
	                pip->c_removed += 1 ;

	            } /* end if */

		    } /* end if (have good path) */

	        } /* end if (stat was bad) */
		}

		if (rs < 0) break ;
	    } /* end while */

	    fsdir_close(&dir) ;
	} /* end if */

	return rs ;
}
/* end subroutine (checkad) */


static int mkpathadd(fbuf,pbuf,plen,np,nl)
char		*fbuf ;
const char	*pbuf ;
int		plen ;
const char	*np ;
int		nl ;
{
	const int	flen = MAXPATHLEN ;

	int	rs = SR_OK ;
	int	i = 0 ;


	if (plen < 0) plen = strlen(pbuf) ;

	if (rs >= 0) {
	    rs = storebuf_strw(fbuf,flen,i,pbuf,plen) ;
	    i += rs ;
	}

	if ((rs >= 0) && (plen > 0) && (pbuf[plen-1] != '/')) {
	    rs = storebuf_char(fbuf,flen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(fbuf,flen,i,np,nl) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkpathadd) */


