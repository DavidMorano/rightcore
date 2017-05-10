/* procdname */

/* process a directory */


#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The subroutine was adapted from others programs that
	did similar types of functions.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This module traverses the given directory and creates
	the Directory Cache (DIRCACHE) file from the traversal.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<field.h>
#include	<bfile.h>
#include	<fsdirtree.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int procdname(pip,ofp,newsdname)
struct proginfo	*pip ;
bfile		*ofp ;
const char	newsdname[] ;
{
	struct ustat	sb ;

	FSDIRTREE	dir ;

	bfile	dcfile ;

	int	rs, rs1, c ;
	int	opts = 0 ;
	int	f_local = FALSE ;

	char	dcfname[MAXPATHLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	template[MAXPATHLEN + 1], *fname = template ;


	if (newsdname == NULL)
	    return SR_FAULT ;

	if (newsdname[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procdname: dname=%s\n",newsdname) ;
#endif

	mkpath2(dcfname,newsdname,DIRCACHEFNAME) ;

/* create the replacement file for the directory cache */

	mkpath2(template,newsdname,"dircacheXXXXXX") ;

	rs = mktmpfile(tmpfname,0664,template) ;

	if ((rs < 0) && (pip->uid != pip->euid)) {

	    rs1 = u_seteuid(pip->uid) ;

	    if (rs1 >= 0) {

	        f_local = TRUE ;
	        rs = mktmpfile(template,0664,tmpfname) ;

	        u_seteuid(pip->euid) ;

	    }
	}

	if (rs < 0)
	    goto ret0 ;

	if ((pip->gid_pcs >= 0) && (! f_local))
	    u_chown(tmpfname,-1,pip->gid_pcs) ;

	c = 0 ;
	rs = bopen(&dcfile,tmpfname,"wct",0664) ;

	if (rs >= 0) {

	    bprintf(&dcfile,"DIRCACHE\n") ;

/* only gets directories, but follow links to find them */

	    opts = FSDIRTREE_MFOLLOW | FSDIRTREE_MDIR ;
	    rs = fsdirtree_open(&dir,newsdname,opts) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procdname: fsdirtree_open() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {

	        while ((rs1 = fsdirtree_read(&dir,&sb,fname)) > 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procdname: name=%t\n",fname,rs1) ;
#endif

		    if (pip->verboselevel >= 2)
			bprintf(ofp,"%s\n",fname,rs1) ;

	            c += 1 ;
	            bwrite(&dcfile,fname,rs1) ;

	            bputc(&dcfile,'\n') ;

	        } /* end while */

	        fsdirtree_close(&dir) ;

	    } /* end if */

	    bclose(&dcfile) ;

	} /* end if (opened replacement file) */

	if (rs >= 0) {

	    rs = u_rename(tmpfname,dcfname) ;

	    if ((rs < 0) && f_local) {

	        rs1 = u_seteuid(pip->uid) ;

	        if (rs1 >= 0) {

	            rs = u_rename(tmpfname,dcfname) ;

	            u_seteuid(pip->euid) ;

	        }
	    }

	} /* end if (renaming attempt) */

	if ((rs < 0) && (tmpfname[0] != '\0')) {

	    rs1 = u_unlink(tmpfname) ;

	    if ((rs1 < 0) && f_local) {

	        rs1 = u_seteuid(pip->uid) ;

	        if (rs1 >= 0) {

	            u_unlink(tmpfname) ;

	            u_seteuid(pip->euid) ;

	        }
	    }

	} /* end if (unlink attempt) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procdname: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procdname) */



