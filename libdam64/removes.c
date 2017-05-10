/* removes */

/* removes (recursively as necessary) a directory tree */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2002-07-13, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine deletes (removes) a directory tree. We recursively
        delete directories as they are encountered. Yhis results in a recursive
	depth-dirst deletion solution.

	Synopsis:

	int removes(const char *dname)

	Arguments:

	dname		directory name (or regular file) to unlink

	Returns:

	>=0		number of items removed
	<0		error


	Notes: 

	= Stack space:

        Yes, now-a-days (since everything has gone multi-threaded -- finally) we
        are more sensitive about our use of stack space. In our algorithm below,
        we only allocate (on the stack) a single path buffer that is used
        throughout the recursive deletion strategy. Other than this, stack space
        us (indeed) used for each level of the directory structure we encunter
        for fairly regular automatic stack-frame variables.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<fsdir.h>
#include	<vecpstr.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	pathadd(char *,int,const char *) ;
extern int	mkexpandpath(char *,cchar *,int) ;
extern int	hasNotDots(const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* local structures */


/* forward references */

static int	remover(char *,int) ;
static int	removedir(char *,int) ;
static int	loadnames(VECPSTR *,const char *) ;


/* local variables */


/* exported subroutines */


int removes(cchar *tardname)
{
	int		rs ;
	char		pbuf[MAXPATHLEN+1] ;

	if (tardname == NULL) return SR_FAULT ;

	if (tardname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("removes: ent\n") ;
	debugprintf("removes: tar=%s\n",tardname) ;
#endif

	if ((rs = mkexpandpath(pbuf,tardname,-1)) > 0) {
	    rs = remover(pbuf,rs) ;
	} else if (rs == 0) {
	    if ((rs = mkpath1(pbuf,tardname)) >= 0) {
	        rs = remover(pbuf,rs) ;
	    }
	} /* end if (mkexpandpath) */

#if	CF_DEBUGS
	debugprintf("removes: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (removes) */


/* local subroutines */


static int remover(char *pbuf,int plen)
{
	struct ustat	sb ;
	int		rs ;
	int		c = 0 ;
	if ((rs = u_lstat(pbuf,&sb)) >= 0) {
	    if (S_ISDIR(sb.st_mode)) {
		rs = removedir(pbuf,plen) ;
		c = rs ;
	    } else {
		rs = u_unlink(pbuf) ;
		c = 1 ;
	    }
	} /* end if (lstat) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (remover) */


static int removedir(char *pbuf,int plen)
{
	VECPSTR		names ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("removes/removedir: pbuf=%s\n",pbuf) ;
#endif

	if (plen < 0) plen = strlen(pbuf) ;

	if ((rs = vecpstr_start(&names,0,0,0)) >= 0) {

	    if ((rs = loadnames(&names,pbuf)) > 0) {
	        struct ustat	sb ;
	        int		i ;
	        const char	*np ;

	        for (i = 0 ; vecpstr_get(&names,i,&np) >= 0 ; i += 1) {
	            if (np != NULL) {
	                if ((rs = pathadd(pbuf,plen,np)) >= 0) {
	                    const int	pl = rs ;
	                    if ((rs = uc_lstat(pbuf,&sb)) >= 0) {
	                        if (S_ISDIR(sb.st_mode)) {
	                            rs = removedir(pbuf,pl) ;
	                            c += rs ;
	                        } else {
	                            rs = u_unlink(pbuf) ;
	                            c += 1 ;
	                        }
	                    } /* end if (lstat) */
	                } /* end if (pathadd) */
		    }
	            if (rs < 0) break ;
	        } /* end for */

		pbuf[plen] = '\0' ; /* restore */
	    } /* end if (loadnames) */

	    if (rs >= 0) {
	        rs = u_rmdir(pbuf) ;
	        c += 1 ;
	    }

	    rs1 = vecpstr_finish(&names) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (names) */

#if	CF_DEBUGS
	debugprintf("removes/removedir: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs > 0) ? c : rs ;
}
/* end subroutine (removedir) */


static int loadnames(VECPSTR *nlp,cchar *dname)
{
	fsdir		d ;
	fsdir_ent	ds ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	if ((rs = fsdir_open(&d,dname)) >= 0) {
	    int		nl ;
	    const char	*np ;
	    while ((rs = fsdir_read(&d,&ds)) > 0) {
	        nl = rs ;
	        np = ds.name ;
	        if ((np[0] != '.') || hasNotDots(np,nl)) {
	            c += 1 ;
	            rs = vecpstr_add(nlp,np,nl) ;
	        }
	        if (rs < 0) break ;
	    } /* end while */
	    rs1 = fsdir_close(&d) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (fsdir) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadnames) */


