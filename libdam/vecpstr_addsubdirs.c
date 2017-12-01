/* vecpstr_addsubdirs */

/* find and load UNIX® directories under a given root */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine load all directories and sub-directories of a given
	root in the file-system into a VECPSTR (container) object.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecpstr.h>
#include	<fsdirtree.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	strwcmp(const char *,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnwcpy(char *,int,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int vecpstr_addsubdirs(VECPSTR *op,cchar *newsdname)
{
	FSDIRTREE	dir ;
	int		rs ;
	int		rs1 ;
	int		opts ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (newsdname == NULL) return SR_FAULT ;

	opts = (FSDIRTREE_MFOLLOW | FSDIRTREE_MDIR) ;
	if ((rs = fsdirtree_open(&dir,newsdname,opts)) >= 0) {
	    USTAT	sb ;
	    const int	flen = MAXPATHLEN ;
	    int		fl ;
	    char	fbuf[MAXPATHLEN+1] ;

	    while ((rs = fsdirtree_read(&dir,&sb,fbuf,flen)) > 0) {
	        fl = rs ;

	        if (fbuf[0] != '.') {
	            c += 1 ;
	            rs = vecpstr_add(op,fbuf,fl) ;
	        }

	        if (rs < 0) break ;
	    } /* end while */

	    rs1 = fsdirtree_close(&dir) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (fsdirtree) */

	if (rs >= 0) {
	    vecpstr_sort(op,NULL) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecpstr_addsubdirs) */


int vecpstr_loaddirs(VECPSTR *op,cchar *newsdname)
{
	return vecpstr_addsubdirs(op,newsdname) ;
}
/* end subroutine (vecpstr_loaddirs) */


