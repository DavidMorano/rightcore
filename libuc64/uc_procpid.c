/* uc_procpid */

/* interface component for UNIX® library-3c */
/* get a process ID by searching for its command string */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We search for a process with a given command string and UID.

	Synopsis:

	int uc_procpid(cchar *name,uid_t uid)

	Arguments:

	name		string with name of process (command-string) to search
	uid		process must have this UID

	Returns:

	<0		error
	==0		no such process
	>=0		proces s foudnd with this PID


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif


/* external subroutines */

extern int	nextfield(cchar *,int,cchar **) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	ctdeci(char *,int,int) ;


/* exported subroutines */


int uc_procpid(cchar *name,uid_t uid)
{
	const int	dlen = DIGBUFLEN ;
	int		v = uid ;
	int		rs ;
	int		pid = 0 ;
	char		dbuf[DIGBUFLEN+1] ;
	if (name == NULL) return SR_FAULT ;
	if (name[0] == '\0') return SR_INVALID ;
	if ((rs = ctdeci(dbuf,dlen,v)) >= 0) {
	    const int	of = O_RDONLY ;
	    int		i = 0 ;
	    cchar	*pfname = "sys:pgrep" ;
	    cchar	*argv[6] ;
	    argv[i++] = "PGREP" ;
	    if (v >= 0) {
	        argv[i++] = "-U" ;
	        argv[i++] = dbuf ;
	    }
	    argv[i++] = name ;
	    argv[i] = NULL ;
	    if ((rs = uc_openprog(pfname,of,argv,NULL)) >= 0) {
	        const int	llen = (MAXPATHLEN+DIGBUFLEN+2) ;
	        const int	fd = rs ;
	        char		lbuf[MAXPATHLEN+DIGBUFLEN+2] ;
	        if ((rs = u_read(fd,lbuf,llen)) > 0) {
	            int		sl = rs ;
	            int		cl ;
	            cchar	*sp = lbuf ;
	            cchar	*cp ;
	            if ((cl = nextfield(sp,sl,&cp)) > 0) {
	                rs = cfdeci(cp,cl,&v) ;
	                pid = v ;
	            }
	        } /* end if (u_read) */
	        uc_close(fd) ;
	    } /* end if (open) */
	} /* end if (ctdeci) */
	return (rs >= 0) ? pid : rs ;
}
/* end subroutine (uc_procpid) */


