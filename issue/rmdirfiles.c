/* rmdirfiles */

/* remove directory files (as specified with a time-out) */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The subroutine was written from scratch but based on previous versions.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Remove files from a specified directory that are older than a specified
	interval (in seconds).

	Synopsis:

	int rmdirfiles(cchar *dname,cchar *prefix,int to)

	Arguments:

	dname		directory name (as a string)
	prefix		optional prefix string for selection for removal
	to		time-out interval in seconds

	Returns:

	<0		error
	>=0		number of files removed (deleted)


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<fsdir.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	pathadd(char *,int,const char *) ;
extern int	nleadstr(cchar *,cchar *,int) ;
extern int	hasNotDots(const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* global variables */


/* local structures */


/* forward references */

static int	vecstr_dirfilesload(vecstr *,cchar *,cchar *,int) ;
static int	vecstr_dirfilesdelete(vecstr *) ;
static int	premat(cchar *,int,cchar *,int) ;


/* local variables */


/* exported subroutines */


int rmdirfiles(cchar *dname,cchar *prefix,int to)
{
	vecstr		files ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (dname == NULL) return SR_FAULT ;
	if (dname[0] == '\0') return SR_INVALID ;

	if ((rs = vecstr_start(&files,0,0)) >= 0) {
	    if ((rs = vecstr_dirfilesload(&files,dname,prefix,to)) > 0) {
	        rs = vecstr_dirfilesdelete(&files) ;
	        c += rs ;
	    }
	    rs1 = vecstr_finish(&files) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (files) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (rmdirfiles) */


/* local subroutines */


static int vecstr_dirfilesload(vecstr *flp,cchar *dname,cchar *prefix,int to)
{
	FSDIR		dir ;
	FSDIR_ENT	de ;
	const time_t	dt = time(NULL) ;
	int		rs ;
	int		rs1 ;
	int		prelen = 0 ;
	int		c = 0 ;
	if (prefix != NULL) prelen = strlen(prefix) ;
	if ((rs = fsdir_open(&dir,dname)) >= 0) {
	    char	rbuf[MAXPATHLEN + 1] ;
	    if ((rs = mkpath1(rbuf,dname)) >= 0) {
	        struct ustat	usb ;
	        const int	rlen = rs ;
	        int		nl ;
	        cchar		*np ;
	        while ((rs = fsdir_read(&dir,&de)) > 0) {
	            nl = rs ;
	            np = de.name ;
	            if (hasNotDots(np,nl)) {
	                if (premat(prefix,prelen,np,nl)) {
	                    if ((rs = pathadd(rbuf,rlen,np)) >= 0) {
	                        const int	rl = rs ;
	                        if (u_stat(rbuf,&usb) >= 0) {
	                            int	ft = usb.st_mtime ;
	                            if ((dt-ft) >= to) {
	                                c += 1 ;
	                                rs = vecstr_add(flp,rbuf,rl) ;
	                            }
	                        } /* end if (stat) */
	                    } /* end if (pathadd) */
	                } /* end if (premat) */
	            } /* end if (hasNotDots) */
	            if (rs < 0) break ;
	        } /* end while (fsdir_read) */
	    } /* end if (mkpath) */
	    rs1 = fsdir_close(&dir) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (fsdir) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_loaddirfiles) */


static int vecstr_dirfilesdelete(vecstr *flp)
{
	int		i ;
	int		c = 0 ;
	cchar		*fp ;
	for (i = 0 ; vecstr_get(flp,i,&fp) >= 0 ; i += 1) {
	    if ((fp != NULL) && (fp[0] != '\0')) {
	        c += 1 ;
	        u_unlink(fp) ;
	    }
	} /* end for */
	return c ;
}
/* end subroutine (vecstr_dirfilesdelete) */


static int premat(cchar *prefix,int prelen,cchar *np,int nl)
{
	int		f = TRUE ;
	if ((prefix != NULL) && (prefix[0] != '\0')) {
	    const int	m = nleadstr(prefix,np,nl) ;
	    f = (m == prelen) ;
	}
	return f ;
}
/* end subroutine (premat) */


