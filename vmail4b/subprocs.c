/* subprocs */

/* manage a list of sub-process PIDs */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object manages a list of sub-processes that are desired to be
        tracked.

	Synopsis:

	int subprocs_start(SUBPROCS *op)

	Arguments:

	op		pointer to object

	Returns:

	>=0		OK
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>
#include	<exitcodes.h>

#include	"subprocs.h"


/* local defines */

#ifndef	NOFILE
#define	NOFILE		20
#endif

#define	DEFENTRIES	4
#define	DEFPATH		"/usr/preroot/bin:/usr/xpg4/bin:/usr/bin:/usr/extra/bin"

#ifndef	ENVBUFLEN
#define	ENVBUFLEN	(MAXPATHLEN + 40)
#endif

#ifndef	PATHBUFLEN
#define	PATHBUFLEN	(8 * MAXPATHLEN)
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snshellunder(char *,int,pid_t,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	strkeycmp(const char *,const char *) ;
extern int	vstrkeycmp(const void **,const void **) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	getpwd(char *,int) ;
extern int	dupup(int,int) ;
extern int	isIOError(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward reference */


/* local variables */


/* exported subroutines */


int subprocs_start(SUBPROCS *op)
{
	const int	n = DEFENTRIES ;
	const int	vo = (VECINT_OCOMPACT|VECINT_OSWAP|VECINT_OREUSE) ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(SUBPROCS)) ;

	if ((rs = vecint_start(&op->pids,n,vo)) >= 0) {
	    op->magic = SUBPROCS_MAGIC ;
	}

	return rs ;
}
/* end subroutine (subprocs_start) */


int subprocs_finish(SUBPROCS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != SUBPROCS_MAGIC) return SR_NOTOPEN ;

	rs1 = vecint_finish(&op->pids) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (subprocs_finish) */


int subprocs_add(SUBPROCS *op,pid_t pid)
{
	int		rs ;
	int		v = pid ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != SUBPROCS_MAGIC) return SR_NOTOPEN ;
	rs = vecint_add(&op->pids,v) ;
	return rs ;
}
/* end subroutine (subprocs_add) */


int subprocs_poll(SUBPROCS *op)
{
	VECINT		*vlp ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != SUBPROCS_MAGIC) return SR_NOTOPEN ;
	vlp = &op->pids ;
	if ((rs = vecint_count(vlp)) > 0) {
	    int		pi = op->pi ;
	    int		v ;
	    while ((rs = vecint_getval(vlp,pi,&v)) >= 0) {
		const pid_t	pid = v ;
		int		cs = 0 ;
		if (v != INT_MIN) {
		    if ((rs1 = u_waitpid(pid,&cs,WNOHANG)) > 0) {
		        f = TRUE ;
		        rs = vecint_del(vlp,pi--) ;
		    } else if (rs1 == SR_CHILD) {
		        rs = vecint_del(vlp,pi--) ;
		    } else {
		        rs = rs1 ;
		    }
		} /* end if (valid value) */
		pi = (pi+1) ;
#if	CF_DEBUGS
	debugprintf("subprocs_poll: waitpid-out pid=%d rs=%d\n",v,rs) ;
#endif
	    } /* end if (vecint-get) */
	    if (rs == SR_NOTFOUND) {
		rs = SR_OK ;
		op->pi = 0 ;
	    } else {
		op->pi = pi ;
	    }
	} /* end if (vecint-count) */
#if	CF_DEBUGS
	debugprintf("subprocs_poll: waitpid-out rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subprocs_poll) */


int subprocs_count(SUBPROCS *op)
{
	int		rs ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != SUBPROCS_MAGIC) return SR_NOTOPEN ;
	rs = vecint_count(&op->pids) ;
	return rs ;
}
/* end subroutine (subprocs_count) */


