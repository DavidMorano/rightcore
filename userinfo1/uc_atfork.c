/* uc_atfork */

/* an enhanced UNIX®-like 'pthread_atfork(3pthread)' subroutine */


#define	CF_DEBUGN	0		/* special compile-time debugging */
#define	CF_DEBUGINIT	0		/* debug initialization section */


/* revision history:

	= 2014-05-09, David A­D­ Morano
	This is being written to add an "unregister" feature to the 'atfork'
	capability that came with POSIX threads.  In the past we always had
	pure reentrant subroutines without hidden locks associated with them.
	But more recently we have started to dabble with hidden locks.  The
	problem is that there is no way to put these hidden locks into a
	library that also get unloaded (unmapped) after it is done being used.
	The standard 'pthread_atfork(3pthread)' does not have an "unregister"
	feature associated with it.  This is what we are creating here.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We are attempting to add an "unregister" feature to the
        'pthread_atfork(3pthread)' facility. We need to create a whole new
        interface for this. This new interface will consist of:

	+ uc_atfork(3uc)
	+ uc_atforkrelease(3uc)

        We suffered a lot when first learning that 'pthread_atfork(3pthread)'
        did not get its registered subroutines removed at load-module removal
        time (as though using something like 'dlclose(3dl)'). So we attempt here
        to provide something that does the un-registering at module un-load
        time.

	Enjoy.

	¥ Note: The type 'int' is assumed to be atomic for multithreaded
	synchronization purposes.  The atomic type |sig_atomic_t| is (just) an
	'int', so we do not feel too guilty ourselves about using an 'int' as
	an interlock.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sigblock.h>
#include	<ptm.h>
#include	<localmisc.h>


/* local defines */

#define	UCATFORK	struct ucatfork
#define	UCATFORK_ENT	struct ucatfork_ent
#define	UCATFORK_LIST	struct ucatfork_list

#define	NDF		"ucatfork.deb"


/* pragmas (these just do not work in shared object libraries) */

#ifdef	COMMENT
#pragma		init(ucatfork_init)
#pragma		fini(ucatfork_fini)
#endif /* COMMENT */


/* typedefs */

typedef void	(*atfork_t)(void) ;


/* external subroutines */

extern int	msleep(int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct ucatfork_ent {
	UCATFORK_ENT	*next ;
	UCATFORK_ENT	*prev ;
	void		(*sub_before)() ;
	void		(*sub_parent)() ;
	void		(*sub_child)() ;
} ;

struct ucatfork_list {
	UCATFORK_ENT	*head ;
	UCATFORK_ENT	*tail ;
} ;

struct ucatfork {
	PTM		m ;		/* data mutex */
	UCATFORK_LIST	list ;		/* memory allocations */
	volatile int	f_init ;	/* race-condition, blah, blah */
	volatile int	f_initdone ;
	volatile int	f_track ;	/* race-condition, blah, blah */
} ;


/* forward references */

int		ucatfork_init() ;
void		ucatfork_fini() ;

static int	ucatfork_trackbegin(UCATFORK *) ;
static void	ucatfork_trackend(UCATFORK *) ;

static void	ucatfork_atforkbefore() ;
static void	ucatfork_atforkparent() ;
static void	ucatfork_atforkchild() ;

static int	entry_match(UCATFORK_ENT *,void (*)(),void (*)(),void (*)()) ;


/* local variables */

static UCATFORK		ucatfork_data ; /* zero-initialized */


/* exported subroutines */


int ucatfork_init()
{
	UCATFORK	*udp = &ucatfork_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
#if	CF_DEBUGINIT
	nprintf(NDF,"ucatfork_init: ent\n") ;
#endif
	if (! udp->f_init) {
	    udp->f_init = TRUE ;
	    if ((rs = ptm_create(&udp->m,NULL)) >= 0) {
	        void	(*sb)() = ucatfork_atforkbefore ;
	        void	(*sp)() = ucatfork_atforkparent ;
	        void	(*sc)() = ucatfork_atforkchild ;
	        if ((rs = pt_atfork(sb,sp,sc)) >= 0) {
	            if ((rs = uc_atexit(ucatfork_fini)) >= 0) {
	        	udp->f_initdone = TRUE ;
	        	f = TRUE ;
	            } /* end if (uc_atexit) */
	        } /* end if (pt_atfork) */
#if	CF_DEBUGINIT
		nprintf(NDF,"ucatfork_init: done\n") ;
#endif
	    } /* end if (ptm_create) */
	    if (rs < 0)
	        udp->f_init = FALSE ;
	} else {
	    while ((rs >= 0) && udp->f_init && (! udp->f_initdone)) {
		rs = msleep(1) ;
		if (rs == SR_INTR) break ;
	    }
	    if ((rs >= 0) && (! udp->f_init)) rs = SR_LOCKLOST ;
	}
#if	CF_DEBUGINIT
	nprintf(NDF,"ucatfork_init: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (ucatfork_init) */


void ucatfork_fini()
{
	UCATFORK	*udp = &ucatfork_data ;
	if (udp->f_initdone) {
	    udp->f_initdone = FALSE ;
	    if (udp->f_track) {
	        ucatfork_trackend(udp) ;
	    }
	    ptm_destroy(&udp->m) ;
	    memset(udp,0,sizeof(ucatfork_data)) ;
	} /* end if (was initialized) */
}
/* end subroutine (ucatfork_fini) */


int uc_atfork(atfork_t sb,atfork_t sp,atfork_t sc)
{
	UCATFORK	*udp = &ucatfork_data ;
	SIGBLOCK	b ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGINIT
	nprintf(NDF,"uc_atfork: ent\n") ;
#endif

	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = ucatfork_init()) >= 0) {

	        if ((rs = uc_forklockbegin(-1)) >= 0) { /* multi */
	            if ((rs = ptm_lock(&udp->m)) >= 0) { /* single */

	                if ((rs = ucatfork_trackbegin(udp)) >= 0) {
	                    UCATFORK_ENT	*ep, *lep ;
	                    const int		esize = sizeof(UCATFORK_ENT) ;
	                    if ((rs = uc_libmalloc(esize,&ep)) >= 0) {
	                        ep->sub_before = sb ;
	                        ep->sub_parent = sp ;
	                        ep->sub_child = sc ;
	                        ep->next = NULL ;
	                        lep = udp->list.tail ;
	                        if (lep != NULL) {
	                            lep->next = ep ;
	                            ep->prev = lep ;
	                        } else {
	                            ep->prev = NULL ;
				}
	                        udp->list.tail = ep ;
	                        if (udp->list.head == NULL) {
	                            udp->list.head = ep ;
				}
	                    } /* end if (memory-allocation) */
	                } /* end if (track-begin) */

	                rs1 = ptm_unlock(&udp->m) ;
			if (rs >= 0) rs = rs1 ;
	            } /* end if (mutex) */
	            rs1 = uc_forklockend() ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (forklock) */

	    } /* end if (init) */
	    rs1 = sigblock_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */

#if	CF_DEBUGINIT
	nprintf(NDF,"uc_atfork: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_atfork) */


int uc_atforkrelease(atfork_t sb,atfork_t sp,atfork_t sc)
{
	UCATFORK	*udp = &ucatfork_data ;
	SIGBLOCK	b ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGN
	nprintf(NDF,"uc_atforkrelease: ent\n") ;
	nprintf(NDF,"uc_atforkrelease: sb{%p}\n",sb) ;
	nprintf(NDF,"uc_atforkrelease: sp{%p}\n",sp) ;
	nprintf(NDF,"uc_atforkrelease: sc{%p}\n",sc) ;
#endif

	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = ucatfork_init()) >= 0) {

	        if ((rs = uc_forklockbegin(-1)) >= 0) { /* multi */
	            if ((rs = ptm_lock(&udp->m)) >= 0) { /* single */

	                if ((rs = ucatfork_trackbegin(udp)) >= 0) {
	                    UCATFORK_ENT	*ep = udp->list.head ;
	                    UCATFORK_ENT	*nep ;
	                    while (ep != NULL) {
	                        nep = ep->next ;
				if (entry_match(ep,sb,sp,sc)) {
	                            UCATFORK_ENT	*pep = ep->prev ;
#if	CF_DEBUGN
	nprintf(NDF,"uc_atforkrelease: ent{%p}\n",ep) ;
#endif
	                            c += 1 ;
	                            if (nep != NULL) {
	                                nep->prev = ep->prev ;
	                            } else {
	                                udp->list.tail = pep ;
	                            }
	                            if (pep != NULL) {
	                                pep->next = ep->next ;
	                            } else {
	                                udp->list.head = nep ;
	                            }
	                        } /* end if (match) */
	                        ep = nep ;
	                    } /* end while (deleting matches) */
	                } /* end if (track-begin) */

	                rs1 = ptm_unlock(&udp->m) ;
			if (rs >= 0) rs = rs1 ;
	            } /* end if (mutex) */
	            rs1 = uc_forklockend() ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (forklock) */

	    } /* end if (init) */
	    rs1 = sigblock_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (uc_atforkrelease) */


/* local subroutines */


int ucatfork_trackbegin(UCATFORK *udp)
{
	int		rs = SR_OK ;
	if (! udp->f_track) {
	    udp->f_track = TRUE ;
	} /* end if (tracking-needed) */
	return rs ;
}
/* end subroutine (ucatfork_trackbegin) */


void ucatfork_trackend(UCATFORK *udp) 
{
	if (udp->f_track) {
	    UCATFORK_ENT	*ep = udp->list.head ;
	    UCATFORK_ENT	*nep ;
	    udp->f_track = FALSE ;
	    while (ep != NULL) {
	        nep = ep->next ;
	        uc_libfree(ep) ;
	        ep = nep ;
	    } /* end while */
	    udp->list.head = NULL ;
	    udp->list.tail = NULL ;
	} /* end if (tracking was started) */
}
/* end subroutine (ucatfork_trackend) */


static void ucatfork_atforkbefore()
{
	UCATFORK	*udp = &ucatfork_data ;
	if (ptm_lock(&udp->m) >= 0) {
	    UCATFORK_ENT	*ep = udp->list.tail ;
	    UCATFORK_ENT	*nep ;
	    while (ep != NULL) {
	        nep = ep->prev ;
	        if (ep->sub_before != NULL) (*ep->sub_before)() ;
	        ep = nep ;
	    } /* end if while */
	} /* end if (locked) */
}
/* end subroutine (ucatfork_atforkbefore) */


static void ucatfork_atforkparent()
{
	UCATFORK	*udp = &ucatfork_data ;
	{
	    UCATFORK_ENT	*ep = udp->list.head ;
	    UCATFORK_ENT	*nep ;
	    while (ep != NULL) {
	        nep = ep->next ;
	        if (ep->sub_parent != NULL) (*ep->sub_parent)() ;
	        ep = nep ;
	    } /* end while */
	    ptm_unlock(&udp->m) ;
	}
}
/* end subroutine (ucatfork_atforkparent) */


static void ucatfork_atforkchild()
{
	UCATFORK	*udp = &ucatfork_data ;
	{
	    UCATFORK_ENT	*ep = udp->list.head ;
	    UCATFORK_ENT	*nep ;
	    while (ep != NULL) {
	        nep = ep->next ;
	        if (ep->sub_child != NULL) (*ep->sub_child)() ;
	        ep = nep ;
	    } /* end while */
	    ptm_unlock(&udp->m) ;
	}
}
/* end subroutine (ucatfork_atforkchild) */


static int entry_match(UCATFORK_ENT *ep,atfork_t sb,atfork_t sp,atfork_t sc)
{
	int		f = TRUE ;
	f = f && (ep->sub_before == sb) ;
	f = f && (ep->sub_parent == sp) ;
	f = f && (ep->sub_child == sc) ;
	return f ;
}
/* end subroutine (entry_match) */


