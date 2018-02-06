/* pcsgetprog */

/* get the path to a program that is used within the PCS system */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used to find a PCS related program and to verify that
        it is executable.

	Important:

        This subroutine differs from 'pcsgetprogpath(3pcs)' in that a full path
        is returned *only* when the program path is not absolute-rooted and it
        is found in the PCS distribution area. The 'pcsgetprogpath(3pcs)'
        subroutine, in contrast, returns a full path of the found program
        whenever it is different than that supplied.

	Synopsis:

	int pcsgetprog(pcsroot,programpath,program)
	const char	pcsroot[] ;
	char		programpath[] ;
	const char	program[] ;

	Arguments:

	pcsroot		PCS program-root
	programpath	resulting path to program if it is not absolute
			and it is found in the PCS distribution
	program		program to find

	Returns:

	>0		found the program in the PCS distribution and
			this is the length of the returned path string
	0		found the program in user's PATH
	<0		did not find the program

	programpath	resulting path to program if it is not absolute
			and it is found in the PCS distribution


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath3w(char *,const char *,const char *,const char *,int) ;
extern int	getfiledirs(const char *,const char *,const char *,vecstr *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int pcsgetprog(cchar *pcsroot,char *output,cchar *name)
{
	struct ustat	sb ;
	int		rs = SR_NOTFOUND ;
	int		namelen ;
	int		outlen = 0 ;
	int		f_output = FALSE ;
	char		outbuf[MAXPATHLEN + 2] ;

	if (name == NULL) return SR_FAULT ;

	if (name[0] == '\0') return SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("pcsgetprog: pcsroot=%s name=%s\n",pcsroot,name) ;
#endif

/* was output specified? */

	if (output != NULL) {
	    f_output = TRUE ;
	    output[0] = '\0' ;
	} else {
	    output = outbuf ;
	}

/* check input */

	namelen = strlen(name) ;

	while ((namelen > 0) && (name[namelen - 1] == '/')) {
	    namelen -= 1 ;
	}

/* start the checks */

	if (strnchr(name,namelen,'/') != NULL) {

#if	CF_DEBUGS
	debugprintf("pcsgetprog: slashed\n") ;
#endif

	    if ((rs = mkpath1w(output,name,namelen)) >= 0)
	        rs = u_stat(output,&sb) ;

	    if (rs >= 0) {
	        rs = SR_NOTFOUND ;
	        if (S_ISREG(sb.st_mode))
	            rs = perm(output,-1,-1,NULL,X_OK) ;
	    }

	    if (rs >= 0)
		rs = 0 ;

	} else {

/* check if the PCS root directory exists */

	if ((rs == SR_NOENT) || (rs == SR_ACCESS) &&
	    (pcsroot != NULL)) {

#if	CF_DEBUGS
	debugprintf("pcsgetprog: rooted\n") ;
#endif

	    if ((rs = mkpath3w(output,pcsroot,"bin",name,namelen)) >= 0) {
	    outlen = rs ;
	    if ((rs = u_stat(output,&sb)) >= 0) {
	        rs = SR_NOTFOUND ;
	        if (S_ISREG(sb.st_mode))
	            rs = perm(output,-1,-1,NULL,X_OK) ;
	    } /* end if */
	    } /* end if */

	    if ((rs == SR_NOENT) || (rs == SR_ACCESS)) {

	        rs = mkpath3w(output,pcsroot,"sbin",name,namelen) ;
	        outlen = rs ;
	        if ((rs = u_stat(output,&sb)) >= 0) {
	            rs = SR_NOTFOUND ;
	            if (S_ISREG(sb.st_mode)) {
	                rs = perm(output,-1,-1,NULL,X_OK) ;
		    }
	        }

	    } /* end if */

	    if (rs >= 0)
	        rs = outlen ;

	} /* end if (non-null PCS root) */

/* search the execution path */

	if ((rs == SR_NOENT) || (rs == SR_ACCESS)) {

#if	CF_DEBUGS
	debugprintf("pcsgetprog: pathed\n") ;
#endif

	    if (f_output)
	        output[0] = '\0' ;

	    if (getfiledirs(NULL,name,"x",NULL) > 0)
	        rs = 0 ;

	} /* end if */

	} /* end if */

#if	CF_DEBUGS
	debugprintf("pcsgetprog: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pcsgetprog) */


