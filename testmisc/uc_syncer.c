/* uc_syncer */

/* UNIX® file-system synchronization */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine proforms a file-system synchronization (possibly
	asynchronously if specified).

	Synopsis:

	int uc_syncer(int w)

	Arguments:

	w		which type:
				0=synchronous
				1=parallel

	Returns:

	<0		error
	>=0		OK


	Notes:

	We need to carefully watch out for what happens after a |fork(2)|.
	Although data structures survice a |fork(2)|, running threads
	besides the one that that forked do not!  Therein lie the potential
	problems.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sigblock.h>
#include	<ptm.h>
#include	<ptc.h>
#include	<pta.h>
#include	<upt.h>
#include	<localmisc.h>


/* local defines */

#define	UCSYNCER	struct ucsyncer
#define	UCSYNCER_SCOPE	PTHREAD_SCOPE_SYSTEM

#define	NDF		"ucsyncer.deb"


/* external subroutines */

extern int	msleep(int) ;


/* local structures */

#ifndef	TYPEDEF_TWORKER
#define	TYPEDEF_TWORKER	1
typedef	int (*tworker)(void *) ;
#endif

struct ucsyncer {
	PTM		m ;		/* data mutex */
	PTC		c ;		/* condition variable */
	pid_t		pid ;
	pthread_t	tid ;
	volatile int	f_init ;	/* race-condition, blah, blah */
	volatile int	f_initdone ;
	volatile int	f_running ;
	volatile int	f_cmd ;
	volatile int	f_syncing ;
	volatile int	f_exiting ;
	volatile int	waiters ;
	int		cmd ;
	int		count ;
} ;

enum cmds {
	cmd_exit,
	cmd_sync,
	cmd_overlast
} ;


/* forward references */

int		ucsyncer_init() ;
void		ucsyncer_fini() ;

static int	ucsyncer_sendsync(UCSYNCER *) ;
static int	ucsyncer_run(UCSYNCER *) ;
static int	ucsyncer_runcheck(UCSYNCER *) ;
static int	ucsyncer_runner(UCSYNCER *) ;
static int	ucsyncer_worker(UCSYNCER *) ;
static int	ucsyncer_worksync(UCSYNCER *) ;
static int	ucsyncer_cmdsend(UCSYNCER *,int) ;
static int	ucsyncer_cmdrecv(UCSYNCER *) ;
static int	ucsyncer_waitdone(UCSYNCER *) ;
static void	ucsyncer_atforkbefore() ;
static void	ucsyncer_atforkparent() ;
static void	ucsyncer_atforkchild() ;


/* local variables */

static UCSYNCER		ucsyncer_data ; /* zero-initialized */


/* exported subroutines */


int ucsyncer_init()
{
	UCSYNCER	*uip = &ucsyncer_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        if ((rs = ptc_create(&uip->c,NULL)) >= 0) {
	    	    void	(*b)() = ucsyncer_atforkbefore ;
	    	    void	(*ap)() = ucsyncer_atforkparent ;
	    	    void	(*ac)() = ucsyncer_atforkchild ;
	            if ((rs = uc_atfork(b,ap,ac)) >= 0) {
	                if ((rs = uc_atexit(ucsyncer_fini)) >= 0) {
	    	            uip->f_initdone = TRUE ;
			    uip->pid = getpid() ;
			    f = TRUE ;
		        }
		        if (rs < 0)
		            uc_atforkrelease(b,ap,ac) ;
	            } /* end if (uc_atfork) */
	            if (rs < 0)
	                ptc_destroy(&uip->c) ;
	        } /* end if (ptc_create) */
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
/* end subroutine (ucsyncer_init) */


void ucsyncer_fini()
{
	UCSYNCER	*uip = &ucsyncer_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    ucsyncer_waitdone(uip) ;
	    {
	        void	(*b)() = ucsyncer_atforkbefore ;
	        void	(*ap)() = ucsyncer_atforkparent ;
	        void	(*ac)() = ucsyncer_atforkchild ;
	        uc_atforkrelease(b,ap,ac) ;
	    }
	    ptc_destroy(&uip->c) ;
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(UCSYNCER)) ;
	} /* end if (atexit registered) */
}
/* end subroutine (ucsyncer_fini) */


int uc_syncer(int w)
{
	UCSYNCER	*uip = &ucsyncer_data ;
	int		rs = SR_NOSYS ;

	if (w < 0) return SR_INVALID ;

	switch (w) {
	case 0:
	    rs = u_sync() ;
	    break ;
	case 1:
	    rs = ucsyncer_sendsync(uip) ;
	    break ;
	} /* end switch */

	return rs ;
}
/* end subroutine (uc_syncer) */


/* local subroutines */


static int ucsyncer_sendsync(UCSYNCER *uip)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (! uip->f_syncing) {
	    SIGBLOCK	b ;
	    if ((rs = sigblock_start(&b,NULL)) >= 0) {
	        if ((rs = ucsyncer_init()) >= 0) {
		    if ((rs = ucsyncer_run(uip)) >= 0) {
		        const int	cmd = cmd_sync ;
		        rs = ucsyncer_cmdsend(uip,cmd) ;
		        c = uip->count ;
		    }
	        } /* end if (init) */
	        sigblock_finish(&b) ;
	    } /* end if (sigblock) */
	} /* end if (syncing not in progress) */

#if	CF_DEBUGN
	nprintf(NDF,"ucsyncer_sendsync: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (ucsyncer_sendsync) */


static int ucsyncer_run(UCSYNCER *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

	if (! uip->f_running) {
	    if ((rs = uc_forklockbegin(-1)) >= 0) { /* multi */
	        if ((rs = ptm_lock(&uip->m)) >= 0) { /* single */
		    if (! uip->f_running) {
		        rs = ucsyncer_runner(uip) ;
		        f = rs ;
		    } else {
			const pid_t	pid = getpid() ;
			if (pid != uip->pid) {
				uip->f_running = FALSE ;
				uip->f_exiting = FALSE ;
				uip->pid = pid ;
				rs = ucsyncer_runner(uip) ;
				f = rs ;
			}
		    } /* end if (not running) */
	            rs1 = ptm_unlock(&uip->m) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (mutex) */
	        rs1 = uc_forklockend() ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (forklock) */
	} else {
	    rs = ucsyncer_runcheck(uip) ;
	    f = rs ;
	} /* end if (not-running) */

#if	CF_DEBUGN
	nprintf(NDF,"ucsyncer_run: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (ucsyncer_run) */


static int ucsyncer_runcheck(UCSYNCER *uip)
{
	const pid_t	pid = getpid() ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (pid != uip->pid) {
	    uip->f_running = FALSE ;
	    uip->f_exiting = FALSE ;
	    uip->pid = pid ;
	    rs = ucsyncer_run(uip) ;
	    f = rs ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (ucsyncer_runcheck) */


static int ucsyncer_runner(UCSYNCER *uip)
{
	PTA		ta ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

	if ((rs = pta_create(&ta)) >= 0) {
	    const int	scope = UCSYNCER_SCOPE ;
	    if ((rs = pta_setscope(&ta,scope)) >= 0) {
		pthread_t	tid ;
		tworker		wt = (tworker) ucsyncer_worker ;
		if ((rs = uptcreate(&tid,&ta,wt,uip)) >= 0) {
		    uip->f_running = TRUE ;
		    uip->tid = tid ;
		    f = TRUE ;
		} /* end if (pthread-create) */
	    } /* end if (pta-setscope) */
	    rs1 = pta_destroy(&ta) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (pta) */

#if	CF_DEBUGN
	nprintf(NDF,"ucsyncer_runner: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (ucsyncer_runner) */


/* it always takes a good bit of code to make this part look easy! */
static int ucsyncer_worker(UCSYNCER *uip)
{
	int		rs ;

	while ((rs = ucsyncer_cmdrecv(uip)) > 0) {
	    switch (rs) {
	    case cmd_sync:
		rs = ucsyncer_worksync(uip) ;
		break ;
	    } /* end switch */
	    if (rs < 0) break ;
	} /* end while (looping on commands) */

#if	CF_DEBUGN
	nprintf(NDF,"ucsyncer_worker: ret rs=%d\n",rs) ;
#endif

	uip->f_exiting = TRUE ;
	return rs ;
}
/* end subroutine (ucsyncer_worker) */


static int ucsyncer_worksync(UCSYNCER *uip)
{
	int		rs ;
	uip->f_syncing = TRUE ;
	if ((rs = u_sync()) >= 0) {
	    uip->count += 1 ;
	}
	uip->f_syncing = FALSE ;
	return rs ;
}
/* end subroutine (ucsyncer_worksync) */


static int ucsyncer_cmdsend(UCSYNCER *uip,int cmd)
{
	int		rs ;
	int		rs1 ;
	int		to = 5 ;

	if ((rs = ptm_lockto(&uip->m,to)) >= 0) {
	    if (! uip->f_exiting) {
	        uip->waiters += 1 ;

	        while ((rs >= 0) && uip->f_cmd) {
		    rs = ptc_waiter(&uip->c,&uip->m,to) ;
	        } /* end while */

	        if (rs >= 0) {
	            uip->cmd = cmd ;
	            uip->f_cmd = TRUE ;
		    if (uip->waiters > 1) {
	                rs = ptc_signal(&uip->c) ;
		    }
	        }

	        uip->waiters -= 1 ;
	    } else {
		rs = SR_HANGUP ;
	    }
	    rs1 = ptm_unlock(&uip->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mutex-section) */

	return rs ;
}
/* end subroutine (ucsyncer_cmdsend) */


static int ucsyncer_cmdrecv(UCSYNCER *uip)
{
	int		rs ;
	int		rs1 ;
	int		to = 1 ;
	int		cmd = 0 ;

	if ((rs = ptm_lockto(&uip->m,to)) >= 0) {
	    uip->waiters += 1 ;
	    to = -1 ;

	    while ((rs >= 0) && (! uip->f_cmd)) {
		rs = ptc_waiter(&uip->c,&uip->m,to) ;
	    } /* end while */

	    if (rs >= 0) {
	        cmd = uip->cmd ;
	        uip->f_cmd = FALSE ;
		if (uip->waiters > 1) {
	            rs = ptc_signal(&uip->c) ;
		}
	    }

	    uip->waiters -= 1 ;
	    rs1 = ptm_unlock(&uip->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mutex-section) */

	return (rs >= 0) ? cmd : rs ;
}
/* end subroutine (ucsyncer_cmdrecv) */


static int ucsyncer_waitdone(UCSYNCER *uip)
{
	int		rs = SR_OK ;

	if (uip->f_running) {
	    const pid_t	pid = getpid() ;
	    if (pid == uip->pid) {
	        const int	cmd = cmd_exit ;
	        if ((rs = ucsyncer_cmdsend(uip,cmd)) >= 0) {
	 	    pthread_t	tid = uip->tid ;
		    int		trs ;
		    if ((rs = uptjoin(tid,&trs)) >= 0) {
		        uip->f_running = FALSE ;
		        rs = trs ;
		    } else if (rs == SR_SRCH) {
		        uip->f_running = FALSE ;
		        rs = SR_OK ;
		    }
	        } /* end if (ucsyncer_sendsync) */
	    } else {
		uip->f_running = FALSE ;
	    }
	} /* end if (running) */

	return rs ;
}
/* end subroutine (ucsyncer_waitdone) */


static void ucsyncer_atforkbefore()
{
	UCSYNCER	*uip = &ucsyncer_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (ucsyncer_atforkbefore) */


static void ucsyncer_atforkparent()
{
	UCSYNCER	*uip = &ucsyncer_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (ucsyncer_atforkparent) */


static void ucsyncer_atforkchild()
{
	UCSYNCER	*uip = &ucsyncer_data ;
	uip->f_running = FALSE ;
	uip->f_exiting = FALSE ;
	uip->pid = getpid() ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (ucsyncer_atforkchild) */


