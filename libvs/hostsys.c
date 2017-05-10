/* hostsys */


#define	CF_DEBUGS	1


/* revision history:

	- 97/01/11, David A­D­ Morano

	This program was originally written.


*/



/**************************************************************************

	I am just playing around with the Virtual System idea.



**************************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<netdb.h>
#include	<stropts.h>
#include	<poll.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>

#include	"localmisc.h"
#include	"syspar.h"
#include	"crp.h"
#include	"rp.h"
#include	"tty.h"
#include	"sys.h"
#include	"q.h"
#include	"pcb.h"
#include	"cpb.h"



/* local defines */

#define		KPRINT		0
#define		LONGJUMP	1



/* external subroutines */

extern char	*strshrink() ;
extern char	*strbasename() ;


#if	LONGJUMP
	extern void	longjmp() ;

	extern int	setjmp() ;
#endif

	extern int	sys_fsinit(), sys_fsinit() ;

	extern int	tty_poll() ;


/* external variables */

	int	sys_env[20] ;

	struct pcb	*pcbp, pcbs[NPCBS] ;

	struct ucb	con_ucb[NCON] ;

	struct qh	srp_q, mrp_q, lrp_q, crp_q, pcb_q, com_q[8] ;

	struct srp	srps[NSRPS] ;

	struct crp	crps[NCRPS] ;

	struct mrp	mrps[NMRPS] ;

	struct lrp	lrps[NLRPS] ;


/* external variables */


/* local variables */

static int	vs_init = FALSE ;







int sys_init()
{
	extern long	(*CONDRIVER[])() ;

	int	i, j ;

#if	KPRINT
	kprintf("start init\n") ;
#endif

	crp_q.head = crp_q.tail = (struct entry *) &crp_q ;

	srp_q.head = srp_q.tail = (struct entry *) &srp_q ;

	mrp_q.head = mrp_q.tail = (struct entry *) &mrp_q ;

	lrp_q.head = lrp_q.tail = (struct entry *) &lrp_q ;

	pcb_q.head = pcb_q.tail = (struct entry *) &pcb_q ;

	for (i = 0 ; i < 8 ; i++)
	com_q[i].head = com_q[i].tail = (struct entry *) &com_q[i] ;


	for (i = 0 ; i < NCRPS ; i++) insq(&crp_q,crps + i) ;

	for (i = 0 ; i < NMRPS ; i++) insq(&srp_q,srps + i) ;

	for (i = 0 ; i < NMRPS ; i++) insq(&mrp_q,mrps + i) ;

	for (i = 0 ; i < NLRPS ; i++) insq(&lrp_q,lrps + i) ;

/*
	if (sizeof(struct pcb) > sizeof()) {

		kprintf("PCBs don't fit into allocated slots\n") ;

		while (1) ;
	}
*/

/* allocate space for register state save areas and link into the PCBs */

	for (i = 0 ; i < NPCBS ; i++) {

		struct crp	*crpp ;

		remq(&crp_q,&crpp) ;

		pcbs[i].save = (int *) crpp ;
		insq(&pcb_q,pcbs + i) ;
	} ;


/* set up the four console terminals */

	for (i = 0 , j = 0 ; i < 2 ; i += 1 , j += 1) {

		con_ucb[i].base = (long) (CONSOLE0 + (j * 8)) ;
		con_ucb[i].device = CONDRIVER ;

		tty_init(con_ucb + i) ;		/* standard TTY driver */

	} ;

	for (i = 2 , j = 0 ; i < 4 ; i += 1 , j += 1) {

		con_ucb[i].base = (long) (CONSOLE2 + (j * 8)) ;
		con_ucb[i].device = CONDRIVER ;

		tty_init(con_ucb + i) ;		/* standard TTY driver */

	} ;




	pcbp = (struct pcb *) 0 ;

#if	KPRINT
	kprintf("initialized\n") ;
#endif

}
/* end subroutine (sys_init) */



int sc_wait(sem)
long	sem ;
{

	while (! btst(sem,&pcbp->sem)) {

		sys_poll() ; 

	} ;
}


int sc_waitsome(sem,mask)
long	sem, mask ;
{


	pcbp->wmask = mask ;
	while (! (pcbp->wmask & pcbp->sem)) sys_poll() ;

	return (pcbp->sem) ;
}


int sc_waitall(sem,mask)
long	sem, mask ;
{


	pcbp->wmask = mask ;
	while ((pcbp->sem & pcbp->wmask) != pcbp->wmask) sys_poll() ;

	return (pcbp->sem) ;
}


int sc_setef(sem)
unsigned long	sem ;
{


	sem &= 0x001F ;
	return (bset(sem,&pcbp->sem)) ;
}


int sc_clref(sem)
unsigned long	sem ;
{


	sem &= 0x001F ;
	return (bclr(sem,&pcbp->sem)) ;
}


int sc_readef(sem,rp)
unsigned long	sem ;
int	*rp ;
{


	sem &= 0x001F ;
	sys_poll() ;

	*rp = pcbp->sem ;

	return (OK) ;
}


long ss_setsem(ap) struct {
unsigned long	sem ;
} *ap ; {


	ap->sem &= 0x001F ;
	return (bset(ap->sem,&pcbp->sem)) ;
}


long ss_clrsem(ap) struct {
unsigned long	sem ;
} *ap ; {


	ap->sem &= 0x001F ;
	return (bclr(ap->sem,&pcbp->sem)) ;
}


long ss_readsem(ap) struct {
unsigned long	sem ;
int	*rp ;
} *ap ; {


	ap->sem &= 0x001F ;
	sys_poll() ;

	*(ap->rp) = pcbp->sem ;
	return (OK) ;
}


/* the following are Systems Calls (SC) subroutines */


int sc_spawn(pc,al,ap,pri,pid,sem)
int (*pc)() ;
char	*ap ;
int	al, pri, *pid ;
unsigned long	sem ;
{
	struct pcb	*spcb ;

	struct csb	lsb ;

	int		v, i ;


#if	KPRINT
	kprintf("spawn start\n") ;
#endif

	sys_poll() ;

	if (pcbp == ((struct pcb *) 0)) {

#if	KPRINT
	kprintf("about to initialize file system\n") ;
#endif

		sys_fsinit() ;

		remq(&pcb_q,&pcbp) ;

#if	KPRINT
	kprintf("about to initialize process files\n") ;
#endif

		sys_pfinit() ;

#if	KPRINT
	kprintf("process files\n") ;
#endif

		pcbp->sem = 0xFFFFFFFFL ;
		for (i = 0 ; i < NOFILE ; i += 1) 
			pcbp->ccb[i] = (long) (con_ucb + 0) ;

		(*pc)(al,ap) ;

		return ;
	} ;

	sem &= 0x001F ;
	if (sem != 0) bclr(sem,&pcbp->sem) ;

	pcbp->psem = sem ;

	spcb = pcbp ;


	remq(&pcb_q,&pcbp) ;

	sys_pfinit() ;

	pcbp->sem = 0xFFFFFFFFL ;
	for (i = 0 ; i < NOFILE ; i += 1) 
			pcbp->ccb[i] = (long) &con_ucb[0] ;

#if	KPRINT
	kprintf("spawning\n") ;
#endif

#if	LONGJUMP
	if ((v = setjmp(pcbp->save)) == 0) 
#endif

		(*pc)(al,ap) ;

#if	LONGJUMP && 0
	else
		kprintf("exit return value %d\n",v) ;
#endif

/* we are still executing in the context of the dying process */


/* close all outstanding requests from this process */

	for (i = 0 ; i < NOFILE ; i += 1) {

		sc_close((long) i,0L,lsb,0L,0L) ;

		while (lsb.cs == R_OUT) sc_wait(0L) ;

	} ;



/* blow off this PCB */

	insq(&pcb_q,pcbp) ;


/* restore the old PCB */

	pcbp = spcb ;

	if (pid) *pid = 0x01 ;

	if (sem) bset((long) sem,&pcbp->sem) ;

	return (OK) ;
}
/* end subroutine (sc_spawn) */


int sc_timer(sem,sb,isr,param,wake)
long	sem, sb[], (*isr)(), param, wake[2] ;
{


	bset(sem,&pcbp->sem) ;

	if (sb != ((long *) 0))
		sb[0] = R_DONEOK ;

	return OK ;
}


int sc_except() { }

int sc_kill() { }

int sc_suspend() { }

int sc_resume() { }

int sc_getproc() { }

int sc_setpri() { }

int sc_exit(val)
int	val ;
{
	sys_poll() ;

#if	LONGJUMP
	longjmp(pcbp->save,2) ;
#endif

	while (1) ;
}


int sc_icr(chan,fc,sem,sb,isr,param,p0,p1,p2,p3,p4,p5)
unsigned long	chan, fc, sem ;
long		sb[2], (*isr)(), param ;
int	p0, p1, p2, p3, p4, p5 ;
{
	struct ucb	*ucbp ;

	struct crp	*p ;

	int	rs ;


#if	KPRINT
	kprintf("icr\n") ;
#endif

	sys_poll() ;

	if ((chan >= NOFILE) || (pcbp->ccb[chan] == 0)) 
		return (R_NOTOPEN) ;

	ucbp = (struct ucb *) pcbp->ccb[chan] ;


	if (remq(&crp_q,&p) == Q_UNDER) {

		kprintf("sc_icr : no more CRPs\n") ;

		return (R_NOTOPEN) ;
	} ;

	sem &= 0x001F ;


	p->pcbp = (long) pcbp ;
	p->fc = fc ;
	p->sem = sem ;
	p->sb = sb ;

	p->isr = isr ;
	p->param = param ;

	p->p[0] = p0 ;
	p->p[1] = p1 ;
	p->p[2] = p2 ;
	p->p[3] = p3 ;
	p->p[4] = p4 ;
	p->p[5] = p5 ;

	bclr(sem,&pcbp->sem) ;

	if (p->sb != ((long *) 0)) p->sb[0] = p->sb[1] = 0 ;

	switch (fc & 0x000FL) {

case 0:
	rs = R_INVALID ;
	break ;

case 1:
	rs = tty_control(ucbp,p) ;

	break ;

case 2:
	rs = tty_status(ucbp,p) ;

	break ;

case 3:
	rs = tty_read(ucbp,p) ;

	break ;

case 4:
	rs = tty_write(ucbp,p) ;

	break ;

default:
	insq(&crp_q,p) ;

	return (R_INVALID) ;
	} ;

	sys_poll() ;

	return (rs) ;
}
/* end subroutine (icr) */


#ifdef	COMMENT

/* cancel outstanding communication requests */
int sc_cancel(chan,fc,sem,sb,isr,param,cparam)
int	chan, fc, sem, *sb, (*isr)(), param, cparam ;
{
	struct crp	*p ;

	int	rs ;

#if	KPRINT
	kprintf("os : cancel\n") ;
#endif

	sys_poll() ;

	if ((chan < 0) || (chan > NOFILE)) 
		return (R_NOTOPEN) ;

	if (remq(&crp_q,&p) == Q_UNDER) {

		kprintf("sc_cancel : no more CRPs\n") ;

		return (R_NOTOPEN) ;
	} ;

	sem &= 0x001F ;


	p->pcbp = (long) pcbp ;
	p->fc = fc ;
	p->sem = sem ;
	p->sb = sb ;

	p->isr = isr ;
	p->param = param ;

	p->p[0] = cparam ;

	bclr(sem,&pcbp->sem) ;

	if (p->sb != ((long *) 0)) p->sb[0] = p->sb[1] = 0 ;


/* call the device handler for the actual cancel operation */

	rs = tty_cancel(&con_ucb[0],fc,cparam) ;


/* complete this cancel request */

	sys_cpost(p,(long) R_DONEOK,0L) ;



	sys_poll() ;

	return (rs) ;
}
/* end subroutine (sc_cancel) */

#endif /* COMMENT */


int sys_cpost(crpp,r0,r1)
struct crp	*crpp ;
long		r0, r1 ;
{
	struct pcb	*tpcb ;


	tpcb = (struct pcb *) crpp->pcbp ;

	bset(crpp->sem,&tpcb->sem) ;

	if (crpp->sb != ((long *) 0)) {

		crpp->sb[0] = r0 ; crpp->sb[1] = r1 ;
	} ;

	if (((long) crpp->isr) != 0L) (*crpp->isr)(crpp->param) ;

	insq(&crp_q,crpp) ;
}


/* routine to wait for an interrupt */
int sys_waiti(cpbp,timeout)
struct cpb	*cpbp ;
int		timeout ;
{
	cpbp->stat |= CSM_EXP ;

	while (cpbp->stat & CSM_EXP) sys_poll() ;

	return (OK) ;
} 
/* end subroutine (sys_waiti) */


int sys_poll()
{
	int	i ;

	for (i = 0 ; i < NCON ; i += 1) tty_poll(con_ucb + i) ;

}
/* end subroutine (sys_poll) */


/* insert into queue */
int insq(q,e)
struct qh	*q ;
struct entry	*e ;
{
	e->fl = (struct entry *) q ;
	e->bl = (struct entry *) q->tail ;
	if (q->head == ((struct entry *) q)) {

		q->head = q->tail = e ;
		return (Q_EMPTY) ;

	} else {

		(q->tail)->fl = e ;
		q->tail = e ;
		return (Q_OK) ;

	} ;
}
/* end subroutine (insq) */


/* remove from queue */
int remq(q,ep)
struct qh	*q ;
struct entry	*(*ep) ;
{
	if (q->head == ((struct entry *) q)) return (Q_UNDER) ;

	(*ep) = q->head ;
	((*ep)->fl)->bl = (struct entry *) q ;
	q->head = (*ep)->fl ;
	return (Q_OK) ;
}
/* end subroutine (remq) */














int vs_waitany(cluster,mask)
int	cluster ;
long	mask ;
{


	if (! vs_init) vs_initialize() ;



}


int vs_controller(fd,sem,sbp,ihp,iha,arg1,arg2)
int	fd ;
int	sem ;
struct csb	*sbp ;
void	(*ihp)() ;
void	*iha ;
int	arg1 ;
void	*arg2 ;
{

	if (! vs_init) vs_initialize() ;






}



