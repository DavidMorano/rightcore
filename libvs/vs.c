/* vs */

/* virtual system stuff */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the famous (infamous) virtual system stuff.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/resource.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<pthread.h>
#include	<malloc.h>

#include	<vsystem.h>
#include	<hdb.h>
#include	<vecobj.h>
#include	<ptm.h>
#include	<localmisc.h>

#include	"vstab.h"
#include	"vs.h"


/* global variables */

struct vs_head	vshead ;		/* zeroed out by loader */


/* exported subroutines */


int vsinit()
{
	struct vs_head	*hp = &vshead ;
	int		rs = SR_OK ;
	int		size ;

#if	CF_DEBUGS
	debugprintf("vsinit: entered\n") ;
#endif

	if (hp->f_init)
		return SR_OK ;

	size = sizeof(struct vstab_head) ;
	rs = vstab_init(&hp->fds,size) ;

	if (rs < 0)
		goto bad0 ;

/* initialize the process-wide variables */

	rs = ptm_create(&hp->pm,NULL) ;

#if	CF_DEBUGS
	debugprintf("vsinit: ret rs=%d\n",rs) ;
#endif

bad0:
	return rs ;
}
/* end subroutine (vsinit) */


int vsfree()
{
	struct vs_head	*hp = &vshead ;
	int		i ;

	if (hp->f_init) {

	ptm_free(&hp->pm) ;

	vstab_free(&hp->fds) ;

	hp->f_init = 0 ;

	}

	return SR_OK ;
}
/* end subroutine (vsfree) */


struct thrarg {
	PSEM	ssem ;
	ULONG	iarg ;
	void	*parg ;
} ;

int thrspawn(ptap,start,parg,iarg)
PTA	*ptap ;
int	(*start)() ;
void	*parg ;
ULONG	iarg ;
{
	struct vs_head	*hp = &vshead ;
	struct thrarg	a ;
	pthread_t	ptid ;
	int		rs ;
	int		rs1 ;
	int		tid = 0 ;

/* prepare arguments */

	memset(&a,0,sizeof(struct thrarg)) ;
	a.parg = parg ;
	a.iarg = iarg ;

	if ((psem_start(&a.ssem,0,0)) >= 0) {
	    if ((rs = uptcreate(&ptid,ptap,start,&a)) >= 0) {
	        if ((rs1 = vstid_mk(&hp->ths,&ptid)) >= 0) {
		    tid = rs1 ;
	        }
	        psem_post(&a.ssem) ;
	    } /* end if (successful spawn) */
	    psem_finish(&a.ssem) ;
	} /* end if (psem) */

	return (rs >= 0) ? tid : rs ;
}
/* end subroutine (thrspawn) */


int thrid()
{
	struct vs_head	*hp = &vshead ;
	pthread_t	ptid ;
	int		rs ;

	ptid = pthread_self() ;

	rs = vstab_getkey(&hp->ths,ptid) ;

	return rs ;
}
/* end subroutine (thrid) */


#ifdef	COMMENT

/* wait on all semaphores */
int vswaitall(args)
int	args ;
{




}
/* end subroutine (vswaitall) */


int vsopener(fname,oflags,perm,semp,sbp,ifp,ifa)
char	fname[] ;
int	oflags ;
int	perm ;
VSSEM	*semp ;
VSTAT	*sbp ;
{



}
/* end subroutine (vsopener) */


int vsreader(fd,fc,semp,sbp,ifp,ifa,buf,buflen,p2,p3,p4,p5)
int	fd ;
int	fc ;
VSSEM	*semp ;
VSSTAT	*sbp ;
int	(*ifp)() ;
union sigval	ifa ;
{



}




#endif /* COMMENT */



int vsintdisable(op)
sigset_t	*op ;
{
	sigset_t	newsigmask ;


	(void) sigemptyset(&newsigmask) ;

	(void) sigaddset(&newsigmask,SIGALRM) ;

	(void) sigaddset(&newsigmask,SIGPOLL) ;

	(void) sigaddset(&newsigmask,SIGHUP) ;

	(void) sigaddset(&newsigmask,SIGTERM) ;

	(void) sigaddset(&newsigmask,SIGINT) ;

	(void) sigaddset(&newsigmask,SIGQUIT) ;

	(void) sigaddset(&newsigmask,SIGWINCH) ;

	(void) sigaddset(&newsigmask,SIGURG) ;

	pthread_sigmask(SIG_BLOCK,&newsigmask,op) ;

	return SR_OK ;
}
/* end subroutine (vsdisable) */


/* enable thread interrupts */
int vsintenable(op)
sigset_t	*op ;
{
	sigset_t	oldsigs ;

	if (op == NULL) {
	    op = &oldsigs ;
	    sigemptyset(&oldsigs) ;
	}

	pthread_sigmask(SIG_SETMASK,op,NULL) ;

	return SR_OK ;
}
/* end subroutine (vsenable) */


/* public (exported) subroutines */


/* system event semaphore initialization */
int vsesem_init(esp)
VSESEM	*esp ;
{

	if (esp == NULL) return SR_FAULT ;

	esp->esem = TRUE ;
	esp->magic = VSESEM_MAGIC ;
	return SR_OK ;
}
/* end subroutine (vsesem_init) */


/* free up a system event semaphore */
int vsesem_free(esp)
VSESEM	*esp ;
{

	if (esp == NULL) return SR_FAULT ;

	esp->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (vsesem_free) */


/* wait on a single event semaphore */
int vsesem_wait(esp)
VSESEM	*esp ;
{
	struct vs_head	*hp = &vshead ;
	int		rs = SR_OK ;
	int		f ;

	if (esp == NULL) return SR_FAULT ;

	if (esp->esem) return SR_OK ;

	if (! hp->f_init)
	    rs = vsinit() ;

loop:
	PTM_LOCK(&vshead.pm) ;

	while (esp->esem == FALSE) {


	    PTC_WAIT(&vshead.pcv,&vshead.pm) ;

	}

	PTM_UNLOCK(&vshead.pm) ;



	return rs ;
}
/* end subroutine (vsesem_wait) */


