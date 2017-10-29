/* tty */

/* last modified %G% version %I% */



/* revision history:

	= 1985-02-01, David A­D­ Morano


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*************************************************************************

	These routines provide a stand-alone TTY environment.
	This routines are designed to work with the MK68901 MFP
	device.


*************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"tty.h"
#include	"crp.h"
#include	"lrp.h"
#include	"cpb.h"
#include	"q.h"


/* local defines */

#define	TIMER		0	/* 0 = no timer, 1 = yes timer */

#define	WPRINT		0
#define	TPRINT		0
#define	RPRINT		0

#define	MONITOR		0

/* output priorities */

#define	OPM_WRITE	0x01
#define	OPM_READ	0x02
#define	OPM_ECHO	0x04
#define	OPM_ANY		0x07

#define	OPV_WRITE	0
#define	OPV_READ	1
#define	OPV_ECHO	2


#define	CC	0x03
#define	BAC	0x08
#define	BS	0x08
#define	TAB	0x09
#define	LF	0x0A
#define	CR	0x0D
#define	SO	0x0F
#define	DLE	0x10
#define	DC1	0x11
#define	DC3	0x13
#define	NAK	0x15
#define	CAN	0x18
#define	REF	0x12
#define	CY	0x19
#define	ESC	0x1B
#define	SP	0x20
#define	DEL	0x7F


/* external subroutines */

	extern struct qh	lrp_q, crp_q ;

	extern int	sys_poll(), sys_waiti(), sys_cpost() ;
	extern int	insq(), remq() ;

	extern int	kprintf() ;


#if	TIMER
	extern long	daytime() ;
#endif

	extern int	tty_poll() ;
	extern int	charq_ins(), charq_rem() ;

	extern void	newop(), nextop() ;




/* static data */


	static long	dterms[8] = {
			0xFEC0F8FFL, 0x00000000L, 
			0x00000000L, 0x00000000L,
			0x00000000L, 0x00000000L,
			0x00000000L, 0x00000000L
	} ;


	static char	enc[] = { -1, 0, 1, 1, 2, 2, 2, 2,
				3, 3, 3, 3, 3, 3, 3, 3,
				4, 4, 4, 4, 4, 4, 4, 4,
				4, 4, 4, 4, 4, 4, 4, 4 } ;



/* forward references */

static void	tty_echo() ;





/* TTY initialization routine */
int tty_init(ucbp)
struct ucb	*ucbp ;
{
	struct cpb	*cpbp ;


	ucbp->stat = 0 ;
	ucbp->co = 0 ;
	ucbp->cc = 0 ;

	ucbp->mode = 0 ;

	ucbp->iq = ucbp->iqt = (long) &ucbp->iq ;
	ucbp->rq = ucbp->rqt = (long) &ucbp->rq ;
	ucbp->wq = ucbp->wqt = (long) &ucbp->wq ;

/* initialize device before all else */

	(*(ucbp->device[DF_INIT]))(ucbp->base) ;

/* initialize the TA buffer */

	remq(&lrp_q,&ucbp->taq) ;

	charq_start(ucbp->taq,ucbp->taq + sizeof(CHARQ),
		sizeof(struct lrp) - sizeof(CHARQ)) ;

/* initialize the EC buffer */

	remq(&lrp_q,&ucbp->ecq) ;

	charq_start(ucbp->ecq,ucbp->ecq + sizeof(CHARQ),
		sizeof(struct lrp) - sizeof(CHARQ)) ;


/* initialize two CPBs for RC and TX */

	remq(&lrp_q,&ucbp->cpb0) ;

	cpbp = (struct cpb *) ucbp->cpb0 ;
	cpbp->stat = 0 ;

	remq(&lrp_q,&ucbp->cpb1) ;

	cpbp = (struct cpb *) ucbp->cpb1 ;
	cpbp->stat = 0 ;




	ucbp->stat |= USM_RI ;

}
/* end subroutine */


/* control function */
int tty_control(ucbp,crpp)
struct ucb	*ucbp ;
struct crp	*crpp ;
{
	int	fc, fm ;


	fc = crpp->p[0] & 0xFFC0L ;

	if (fc & fm_setmode) {

		ucbp->mode = crpp->p[1] ;

		sys_cpost(crpp,0L,(long) R_DONEOK) ;

	} else if (fc & fm_getmode) {

		*((long *) crpp->p[1]) = ucbp->mode ;

		sys_cpost(crpp,0L,(long) R_DONEOK) ;

	} else if (fc & fm_int) {

		insq(&ucbp->iq,crpp) ;

	} else if (fc & fm_kill) {

		insq(&ucbp->iq,crpp) ;

	} else
		sys_cpost(crpp,0L,(long) R_DONEOK) ;

	return (OK) ;
}
/* end subroutine */


/* status function */
int tty_status(ucbp,crpp)
struct ucb	*ucbp ;
struct crp	*crpp ;
{
	int	fc, fm ;

	fc = crpp->p[0] & 0xFFC0L ;

	sys_cpost(crpp,0L,R_DONEOK) ;

	return (OK) ;
}
/* end subroutine */



/* subroutine to read a line from terminal */
int tty_read(ucbp,crpp)
struct ucb	*ucbp ;
struct crp	*crpp ;
{
	struct cpb	*cpbp ;

	long	rs, *terms ;

	int	len, plen, count, timeout ;

	uchar	c, *buf, *pbuf ;


	cpbp = (struct cpb *) ucbp->cpb0 ;

	buf = (uchar *) crpp->p[0] ;
	len = (int) crpp->p[1] ;
	timeout = (int) crpp->p[2] ;
	terms = (long *) crpp->p[3] ;
	pbuf = (uchar *) crpp->p[4] ;
	plen = (int) crpp->p[5] ;


	crpp->fc |= ucbp->mode ;

	if (terms == ((long *) 0)) terms = dterms ;

	ucbp->cc = 0 ;
	ucbp->co = 0 ;			/* cancel ^O effect */

	ucbp->stat |= USM_READ ;

/* top of further access */
top:
	tty_wps(ucbp,pbuf,plen) ;	/* write out prompt if present */

#ifdef	COMMENT
	cqp = (CHARQ *) ucbp->taq ;
	if ((crpp->fc & fm_noblock) && (cqp->count == 0))
#endif


	count = 0 ;

/* check TA buffer first */
next:
	if (charq_rem(ucbp->taq,&c) != CHARQR_UNDER) goto read_got ;

	if (ucbp->cc) goto read_cc ;

/* wait for character while timing */

	if (sys_waiti(cpbp,timeout)) goto read_to ;

	if (ucbp->cc) goto read_cc ;

	c = ucbp->rc ;


/* got a character */
read_got:
	buf[count] = c ;

/* check for terminator */

	if (BATST((long) c,terms)) goto read_term ;

/* check for a filter character */

	if (crpp->fc & fm_nofilter) goto norm ;

	if (c == NAK) goto read_nak ;

	if (c == DEL) goto read_del ;

	if (c == BAC) goto read_del ;

	if (c == REF) goto read_ref ;

	if (c == CAN) goto read_can ;

/* normal character */
norm:
	if ((! (crpp->fc & fm_noecho)) && 
	((c & 0x60) && ((c & 0x7F) != 0x7F)) || (c == TAB))
		tty_echo(ucbp,&c,1) ;

	count += 1 ;
	if (count < len) goto next ;

	rs = R_DONEOK ;

/* this read operation is done */
read_done:
	ucbp->stat &= (~ USM_READ) ;

	sys_cpost(crpp,(long) count,rs) ;

	return (OK) ;

/* handle a terminator character */
read_term:
	if (c == CR) goto read_cr ;

	if (c == ESC) goto read_esc ;

/* we have a normal terminator */
read_tstore:
	buf[count++] = c ;

	rs = R_DONEOK ;
	goto	read_done ;


/* we read a CR */
read_cr:
	if (! ((crpp->fc & fm_notecho) || (crpp->fc & fm_noecho)))
		tty_echo(ucbp,"\r\n",2) ;

	if (crpp->fc & fm_nofilter) goto read_tstore ;

	c = LF ;
	goto read_tstore ;

/* we got an ESC character */
read_esc:
	if (! ((crpp->fc & fm_notecho) || (crpp->fc & fm_noecho)))
		tty_echo(ucbp,"$",1) ;

	goto read_tstore ;


/* handle a restart line */
read_nak:
	if (! (crpp->fc & fm_noecho)) tty_echo(ucbp," ^U\r\n",5) ;

	goto top ;


/* handle a delete character */
read_del:
	if (count == 0) goto next ;

	count -= 1 ;

	if (! (crpp->fc & fm_noecho)) tty_echo(ucbp,"\b \b",3) ;

	goto next ;

/* we got a ^C character */
read_cc:
	ucbp->cc = 0 ;			/* shut off control C flag */

	rs = TT_CC ;
	goto read_done ;

/* we got a ^R */
read_ref:
	if (crpp->fc & fm_noecho) goto next ;

	tty_echo(ucbp," ^R\r\n",5) ;

	tty_wps(ucbp,pbuf,(int) plen) ;	/* re-print out the prompt string */

	tty_wps(ucbp,buf,(int) count) ;	/* print out the input line */

	goto next ;

read_can:
	if (! (crpp->fc & fm_noecho)) tty_echo(ucbp," ^X\r\n",5) ;

	goto top ;


/* we got a read timeout */
read_to:
	rs = TT_TO ;
	goto read_done ;
}
/* end subroutine */


/* write out a prompt string */
static int tty_wps(ucbp,buf,len)
struct ucb	*ucbp ;
char	*buf ;
int	len ;
{
	struct cpb	*cpbp ;

	register int	i ;


	cpbp = (struct cpb *) ucbp->cpb0 ;

	newop(ucbp,OPM_READ) ;

	ucbp->stat &= (~ USM_READ) ;

	if (! (ucbp->stat & USM_SUS)) ucbp->stat |= USM_TI ;

	for (i = 0 ; i < len ; i++) {

		if ((buf[i] == '\n') && (! (ucbp->mode & fm_rawout))) {

			ucbp->rc = '\r' ;
			sys_waiti(cpbp,0) ;
		}

		ucbp->rc = buf[i] ;
		sys_waiti(cpbp,0) ;

	} ;

	ucbp->stat &= (~ OPM_READ) ;
	nextop(ucbp) ;

	ucbp->stat |= USM_READ ;

	return ;
}
/* end subroutine */


/* write routine */
int tty_write(ucbp,crpp)
struct ucb	*ucbp ;
struct crp	*crpp ;
{
	struct cpb	*cpbp ;

	int		i, len ;

	char		*buf ;


	crpp->fc |= ucbp->mode ;

	buf = (char *) crpp->p[0] ;
	len = (int) crpp->p[1] ;

	i = 0 ;
	if (ucbp->co) goto done ;


	cpbp = (struct cpb *) ucbp->cpb1 ;

	newop(ucbp,OPM_WRITE) ;

	if (! (ucbp->stat & USM_SUS)) ucbp->stat |= USM_TI ;

	for ( ; i < len ; i += 1) {

		if ((buf[i] == '\n') && (! (crpp->fc & fm_rawout))) {

			ucbp->wc = '\r' ;
			sys_waiti(cpbp,0) ;
		}

		ucbp->wc = buf[i] ;
		sys_waiti(cpbp,0) ;

		if (ucbp->co) break ;

	} ;

#ifdef	COMMENT
	while (i < len) {

		if (charq_ins(ucbp->wq,buf[i]) == CHARQR_OVER) {

			sys_waiti(cpbp,0) ;

			continue ;
		} ;

		i += 1 ;
	} ;
#endif
	ucbp->stat &= (~ OPM_WRITE) ;
	nextop(ucbp) ;

done:
	sys_cpost(crpp,(long) i,(i < len) ? TT_CO : R_DONEOK) ;

	return (OK) ;
}
/* end subroutine */


/* cancel a terminal request */
int tty_cancel(ucbp,fc,cparam)
struct ucb	*ucbp ;
long int	fc, cparam ;
{
	extern struct pcb	*pcbp ;

	struct crp	*q, *crpp, *next ;

	long int	v ;
	int		all ;


	all = FALSE ;
	switch (fc & 0x1F) {

case fc_all:
	all = TRUE ;

case fc_set:
	q = (struct crp *) &(ucbp->iq) ;
	next = (struct crp *) ucbp->iq ;

	while (next != ((struct crp *) q)) {

		crpp = next ;
		next = (struct crp *) crpp->fl ;

		if (
		(crpp->pcbp == (long) pcbp) &&
		((fc & fm_all) || (crpp->param == cparam))
		) {
			remq(crpp->bl,&v) ;

			sys_cpost(crpp,(long) R_CANCELED,0L) ;

		} ;
	} ;

	if (! all) break ;

case fc_read:
	q = (struct crp *) &(ucbp->rq) ;
	crpp = (struct crp *) ucbp->rq ;

	while (crpp != (struct crp *) q) {

		next = (struct crp *) crpp->fl ;

		if (
		(crpp->pcbp == (long) pcbp) &&
		((fc & fm_all) || (crpp->param == cparam))
		) {
			remq(crpp->bl,&v) ;

			sys_cpost(crpp,(long) R_CANCELED,0L) ;

		} ;
		crpp = next ;
	} ;

	if (! all) break ;

case fc_write:
	q = (struct crp *) &(ucbp->wq) ;
	crpp = (struct crp *) ucbp->wq ;
	while (crpp != (struct crp *) q) {

		next = (struct crp *) crpp->fl ;
		if (
		(crpp->pcbp == (long) pcbp) &&
		((fc & fm_all) || (crpp->param == cparam))
		) {
			remq(crpp->bl,&v) ;

			sys_cpost(crpp,(long) R_CANCELED,0L) ;

		} ;
		crpp = next ;
	} ;

	if (! all) break ;

default:
	break ;
	} ; /* end switch */


	return (OK) ;
}
/* end subroutine */


/* check for any terminal activity */
int tty_poll(ucbp)
struct ucb	*ucbp ;
{

	while (((*ucbp->device[DF_STAT])(ucbp->base) & TM_RR) &&
		(ucbp->stat & USM_RI)) tty_risr(ucbp) ;


	if (((*ucbp->device[DF_STAT])(ucbp->base) & TM_TR) &&
		(ucbp->stat & USM_TI)) tty_tisr(ucbp) ;

	return ;
}
/* end subroutine */


/* check for receiver got some thing */
static int tty_risr(ucbp)
struct ucb	*ucbp ;
{
	struct cpb	*cpbp ;
	struct crp	*crpp ;

	uchar		c ;


check:
	if (! ((*ucbp->device[DF_STAT])(ucbp->base) & TM_RR)) return ;

	(*ucbp->device[DF_READ])(ucbp->base,&c) ;


	cpbp = (struct cpb *) ucbp->cpb0 ;


	if (c == DC3) goto che_dc3 ;

	if (c == DC1) goto che_dc1 ;

	if (c == SO) goto che_so ;

	if (c == CC) goto che_cc ;

	if (c == CY) goto che_cy ;

	if (c == DLE) goto che_dle ;

	if ((ucbp->stat & USM_READ) && (cpbp->stat & CSM_EXP)) {

		cpbp->stat &= (~ CSM_EXP) ;	/* clear expected semaphore */
		ucbp->rc = c ;

	} else { /* store the character in the type-ahead queue */

		if (c == CAN) goto che_can ;

		charq_ins(ucbp->taq,c) ;

	} ;

	goto	check ;

che_dc3:
	ucbp->stat |= USM_SUS ;
	ucbp->stat &= (~ USM_TI) ;
	goto	check ;

che_dc1:
	ucbp->stat &= (~ USM_SUS) ;
	ucbp->stat |= USM_TI ;
	goto	check ;

che_so:
	if (ucbp->co) {

		ucbp->co = 0 ;

	} else {

		tty_echo(ucbp," ^O\r\n",5) ;

		ucbp->co = 1 ;

	} ;

	goto	check ;

/* handle control C */
che_cc:
	if (cpbp->stat & CSM_EXP) {

		cpbp->stat &= (~ CSM_EXP) ;
		ucbp->cc = 1 ;		/* signal control C interrupt */
	}

	tty_echo(ucbp," ^C\r\n",5) ;

	while (remq(&ucbp->iq,&crpp) != CHARQR_UNDER) {

		sys_cpost(crpp,0L,(long) R_DONEOK) ;

	} ;

	goto	check ;


/* handle control Y */
che_cy:
	if (cpbp->stat & CSM_EXP) {

		cpbp->stat &= (~ CSM_EXP) ;
		ucbp->cc = 1 ;		/* signal control C interrupt */
	}

	tty_echo(ucbp," ^Y\r\n",5) ;

	while (remq(&ucbp->iq,&crpp) != CHARQR_UNDER) {

		sys_cpost(crpp,0L,(long) R_DONEOK) ;

	} ;

	goto	check ;


/* data link excape */
che_dle:

#if	MONITOR
	XDT(0) ;
#endif
	return ;

/* handle a line cancel */
che_can:
	while (charq_rem(ucbp->taq,&c) != CHARQR_UNDER) ;

	goto	check ;
}
/* end subroutine */


/* transmit interrupt service routine */
static int tty_tisr(ucbp)
struct ucb	*ucbp ;
{
	struct cpb	*cpbp ;

	char	c ;

check:
	if (! ((*ucbp->device[DF_STAT])(ucbp->base) & TM_TR)) return ;


	if (! (ucbp->stat & OPM_ANY)) {

		ucbp->stat &= (~ USM_TI) ;
		return ;
	}

	switch ((int) ucbp->op) {

case OPV_WRITE:
	(*ucbp->device[DF_WRITE])(ucbp->base,ucbp->wc) ;

	cpbp = (struct cpb *) ucbp->cpb1 ;
	cpbp->stat &= (~ CSM_EXP) ;
	break ;

case OPV_READ:
	(*ucbp->device[DF_WRITE])(ucbp->base,ucbp->rc) ;

	cpbp = (struct cpb *) ucbp->cpb0 ;
	cpbp->stat &= (~ CSM_EXP) ;
	break ;

case OPV_ECHO:
	if (charq_rem(ucbp->ecq,&c) != CHARQR_UNDER) {

		(*ucbp->device[DF_WRITE])(ucbp->base,c) ;

	} else {

		ucbp->stat &= (~ OPM_ECHO) ;
		nextop(ucbp) ;

	}

	} ; /* end switch */

}
/* end subroutine */


/* echo a string out for internal use */
static void tty_echo(ucbp,buf,len)
struct ucb	*ucbp ;
char	*buf ;
int	len ;
{
	int	i ;


	for (i = 0 ; i < len ; i += 1)
		charq_ins(ucbp->ecq,buf[i]) ;

	newop(ucbp,OPM_ECHO) ;

	if (! (ucbp->stat & USM_SUS)) 
		ucbp->stat |= USM_TI ;

}
/* end subroutine */



void newop(ucbp,cm) 
struct ucb	*ucbp ; 
int	cm ;
{


	if (! (ucbp->stat & OPM_ANY)) {

		ucbp->stat |= cm ;
		ucbp->op = enc[ucbp->stat & OPM_ANY] ;

	} else
		ucbp->stat |= cm ;

}

void nextop(ucbp) 
struct ucb	*ucbp ;
{


	ucbp->op = enc[ucbp->stat & OPM_ANY] ;
}




