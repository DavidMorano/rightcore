/* statvfsdir */

/* like 'statvfs(2)' but will not return zero blocks on automounts */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The subroutine was written from scratch.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine performs almost exactly like 'statvfs(2)'. The
        difference is that if zero total-blocks are returned by the OS we assume
        that an unmounted automount point was accessed. In this case we will try
        to access something inside the directory in order to get it mounted so
        that a second attempt will succeed.

	Synopsis:

	int statvfsdir(fname,sbp)
	const char	*fname ;
	struct statvfs	*sbp ;

	Arguments:

	fname		source string
	sbp		pointer to 'statvfs' structure

	Return:

	<0		error
	>=0		OK


******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/statvfs.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<fsdir.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	isNotAccess(int) ;

extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	trytouch(const char *) ;


/* local variables */


/* exported subroutines */


int statvfsdir(cchar *fname,struct statvfs *sbp)
{
	struct ustat	sb ;
	int		rs ;
	int		rs1 ;

	if ((rs = uc_statvfs(fname,sbp)) >= 0) {

	    if (sbp->f_blocks == 0) {
	        if (((rs = u_stat(fname,&sb)) >= 0) && S_ISDIR(sb.st_mode)) {
	            char	tmpfname[MAXPATHLEN + 1] ;
	            rs1 = SR_NOENT ;
	            if ((rs = mkpath2(tmpfname,fname,".")) >= 0) {
	                if ((rs1 = u_stat(tmpfname,&sb)) >= 0) {
	                    rs = uc_statvfs(fname,sbp) ;
	                }
	            }
	            if (rs >= 0) {
	                if ((rs1 == SR_NOENT) || (rs1 == SR_ACCESS)) {
	                    if ((rs1 = trytouch(fname)) >= 0) {
	                        rs = uc_statvfs(fname,sbp) ;
	                    }
	                }
	            }
	        }
	    } /* end if */

	} /* end if (uc_statvfs) */

	return rs ;
}
/* end subroutine (statvfsdir) */


/* local subroutines */


static int trytouch(cchar *fname)
{
	FSDIR		dir ;
	FSDIR_ENT	ds ;
	int		rs ;
	const char	*np ;

	if ((rs = fsdir_open(&dir,fname)) >= 0) {

	    while (fsdir_read(&dir,&ds) > 0) {
	        np = ds.name ;
	        if (np[0] != '.') break ;
	        if ((np[1] != '\0') && (np[1] != '.')) break ;
	    }

	    fsdir_close(&dir) ;
	} /* end if (fsdir) */

	return rs ;
}
/* end subroutine (trytouch) */


