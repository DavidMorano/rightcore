/* uproguser */

/* get or set a cached username given a UID */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2004-11-22, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Set (as if for a cache) and get a username given a UID.

	Synopsis:

	int uproguser_nameget(rbuf,rlen,uid)
	char		rbuf[] ;
	int		rlen ;
	uid_t		uid ;

	Arguments:

	rbuf		buffer to receive the requested username
	rlen		length of supplied buffer
	uid		UID of user to get name for

	Returns:

	==0		could not get a name
	>0		string length of found username
	<0		some other error

	Design note:

	Q. Is this (mess) multi-thread safe?
	A. Duh!

	Q. Does this need to be so complicated?
	A. Yes.

	Q. Was the amount of complication warranted by the need?
	A. Looking at it now ... maybe not.

	Q. Were there any alternatives?
	A. Yes; the predicessor to this present implementation was an 
	   implementation that was quite simple, but it had a lot of static
	   memory storage.  It was the desire to eliminate the static memory
	   storage that led to this present implementation.

	Q. Are there ways to clean this up further?
	A. Probably, but it looks I have already done more to this simple
	   function than may have been ever warranted to begin with!

	Q. Did these subroutines have to be Async-Signal-Safe?
	A. Not really.

	Q. Then why did you do it?
	A. The system-call |uname(2)| is Async-Signal-Safe.  Since these
	   subroutines sort of look like |uname(2)| (of a sort), I thought 
	   it was a good idea.

	Q. Was it really a good idea?
	A. I guess not.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sigblock.h>
#include	<ptm.h>
#include	<localmisc.h>


/* local defines */

#define	UPROGUSER	struct uproguser

#define	TO_TTL		(2*3600) /* two hours */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	msleep(int) ;
extern int	isNotPresent(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* external variables */


/* local structures */

struct uproguser {
	PTM		m ;		/* data mutex */
	time_t		et ;
	uid_t		uid ;
	int		ttl ;		/* time-to-live */
	volatile int	f_init ; 	/* race-condition, blah, blah */
	volatile int	f_initdone ;
	char		*userhome ;	/* memory-allocated */
	char		username[USERNAMELEN+1] ;
} ;


/* forward references */

int		uproguser_init() ;
void		uproguser_fini() ;

static void	uproguser_atforkbefore() ;
static void	uproguser_atforkafter() ;

static int	uproguser_end(UPROGUSER *) ;


/* local variables */

static UPROGUSER	uproguser_data ; /* zero-initialized */


/* exported subroutines */


int uproguser_init()
{
	UPROGUSER	*uip = &uproguser_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        void	(*b)() = uproguser_atforkbefore ;
	        void	(*a)() = uproguser_atforkafter ;
	        if ((rs = uc_atfork(b,a,a)) >= 0) {
	            if ((rs = uc_atexit(uproguser_fini)) >= 0) {
	                uip->f_initdone = TRUE ;
	                f = TRUE ;
	            }
	            if (rs < 0)
	                uc_atforkrelease(b,a,a) ;
	        } /* end if (uc_atfork) */
	        if (rs < 0)
	            ptm_destroy(&uip->m) ;
	    } /* end if (ptm_create) */
	    if (rs < 0)
	        uip->f_init = FALSE ;
	} else {
	    while ((rs >= 0) && uip->f_init && (! uip->f_initdone)) {
		rs = msleep(1) ;
		if (rs == SR_INTR) rs = SR_OK ;
	    }
	    if ((rs >= 0) && (! uip->f_init)) rs = SR_LOCKLOST ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uproguser_init) */


void uproguser_fini()
{
	UPROGUSER	*uip = &uproguser_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    {
	        uproguser_end(uip) ;
	    }
	    {
	        void	(*b)() = uproguser_atforkbefore ;
	        void	(*a)() = uproguser_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(UPROGUSER)) ;
	} /* end if (was initialized) */
}
/* end subroutine (uproguser_fini) */


int uproguser_nameget(char *rbuf,int rlen,uid_t uid)
{
	UPROGUSER	*uip = &uproguser_data ;
	SIGBLOCK	b ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		len = 0 ;

	if (rbuf == NULL) return SR_FAULT ;

	if (uid < 0) uid = getuid() ;

	rbuf[0] = '\0' ;
	if (uip->username[0] != '\0') {
	    if ((rs = sigblock_start(&b,NULL)) >= 0) {
	        if ((rs = uproguser_init()) >= 0) {
	            if ((rs = uc_forklockbegin(-1)) >= 0) {
	                if ((rs = ptm_lock(&uip->m)) >= 0) {
	                    if (uip->username[0] != '\0') {
	                        if (uip->et > 0) {
		                    const time_t	dt = time(NULL) ;
				    if ((dt-uip->et) < uip->ttl) {
	                                if (uip->uid == uid) {
					    cchar	*un = uip->username ;
	                                    rs = sncpy1(rbuf,rlen,un) ;
	                                    len = rs ;
	                                }
	                            } /* end if (not timed-out) */
				} /* end if (possible) */
			    } /* end if (nul-terminated) */
	                    rs1 = ptm_unlock(&uip->m) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (mutex) */
	 		rs1 = uc_forklockend() ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (forklock) */
	        } /* end if (uproguser_init) */
	        sigblock_finish(&b) ;
	    } /* end if (sigblock) */
	} /* end if (possible match) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (uproguser_nameget) */


int uproguser_nameset(cchar *cbuf,int clen,uid_t uid,int ttl)
{
	SIGBLOCK	b ;
	int		rs ;
	int		rs1 ;

	if (cbuf == NULL) return SR_FAULT ;

	if (cbuf[0] == '\0') return SR_INVALID ;

	if (uid < 0) uid = getuid() ;
	if (ttl < 0) ttl = TO_TTL ;

	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = uproguser_init()) >= 0) {
	        UPROGUSER	*uip = &uproguser_data ;
	        if ((rs = uc_forklockbegin(-1)) >= 0) {
	            if ((rs = ptm_lock(&uip->m)) >= 0) {
			{
		            const time_t	dt = time(NULL) ;
		            const int		ulen = USERNAMELEN ;
		            uip->et = dt ;
		            uip->ttl = ttl ;
		            uip->uid = uid ;
		            strdcpy1w(uip->username,ulen,cbuf,clen) ;
			}
	                rs1 = ptm_unlock(&uip->m) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (mutex) */
	            rs1 = uc_forklockend() ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (forklock) */
	    } /* end if (uproguser_init) */
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */

	return rs ;
}
/* end subroutine (uproguser_set) */


/* local subroutines */


static int uproguser_end(UPROGUSER *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (uip->userhome != NULL) {
	    rs1 = uc_libfree(uip->userhome) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->userhome = NULL ;
	}
	return rs ;
}
/* end subroutine (uproguser_end) */


static void uproguser_atforkbefore()
{
	UPROGUSER	*uip = &uproguser_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (uproguser_atforkbefore) */


static void uproguser_atforkafter()
{
	UPROGUSER	*uip = &uproguser_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (uproguser_atforkafter) */


