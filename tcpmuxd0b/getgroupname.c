/* getgroupname */

/* get a groupname by GID */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        This subroutine is being written for use by PCS programs, but it
        obviously has wider applications. It is simple, but effective!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Get a groupname given a GID.

	Synopsis:

	int getgroupname(rbuf,rlen,gid)
	char		rbuf[] ;
	int		rlen ;
	gid_t		gid ;

	Arguements:

	buf		supplied buffer to receive groupname
	buflen		length of supplied buffer
	gid		GID of group to get

	Returns:

	>=0		length of return groupname
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<getax.h>
#include	<localmisc.h>


/* local defines */

#ifndef	USERNAMELEN
#ifndef	LOGNAME_MAX
#define	USERNAMELEN	LOGNAME_MAX
#else
#define	USERNAMELEN	32
#endif
#endif

#ifndef	GROUPNAMELEN
#define	GROUPNAMELEN	USERNAMELEN
#endif

#ifndef	VARGROUPNAME
#define	VARGROUPNAME	"GROUPNAME"
#endif


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	isOneOf(const int *,int) ;


/* local structures */


/* forward references */

static int	isNotOurs(int) ;


/* local variables */

const int	rsnotours[] = {
	SR_SEARCH,
	SR_NOTFOUND,
	0
} ;


/* exported subroutines */


int getgroupname(char gbuf[],int glen,gid_t gid)
{
	const gid_t	gid_our = getgid() ;
	const int	grlen = getbufsize(getbufsize_gr) ;
	int		rs ;
	char		*grbuf ;

	if (gbuf == NULL) return SR_FAULT ;

	if (glen < 0) glen = GROUPNAMELEN ;

	if (gid < 0) gid = gid_our ;

	if ((rs = uc_malloc((grlen+1),&grbuf)) >= 0) {
	    struct group	gr ;
	    const char		*vn = VARGROUPNAME ;
	    const char		*gn = NULL ;
	    if ((gid == gid_our) && ((gn = getenv(vn)) != NULL)) {
	        if ((rs = getgr_name(&gr,grbuf,grlen,gn)) >= 0) {
	            if (gr.gr_gid != gid) {
			rs = SR_SEARCH ;
		    }
		}
	    } else {
	        rs = SR_NOTFOUND ;
	    }
	    if (isNotOurs(rs)) {
	        rs = getgr_gid(&gr,grbuf,grlen,gid) ;
		gn = gr.gr_name ;
	    }
	    if (rs >= 0) {
	        rs = sncpy1(gbuf,glen,gn) ;
	    } else if (isNotOurs(rs)) {
	        rs = snsd(gbuf,glen,"G",(uint) gid) ;
	    }
	    uc_free(grbuf) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (getgroupname) */


/* local subroutines */


static int isNotOurs(int rs)
{
	return isOneOf(rsnotours,rs) ;
}
/* end subroutine (isNotOurs) */


