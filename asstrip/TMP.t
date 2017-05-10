/* as_prefix */
/* last modified %G% version %I% */


#define		abcd	abcd.b
#define		abcdb	abcd.b
#define		addb	add.b
#define		addw	add.w
#define		addl	add.l
#define		addxb	addx.b
#define		addxw	addx.w
#define		addxl	addx.l
#define		andb	and.b
#define		andw	and.w
#define		andl	and.l
#define		aslb	asl.b
#define		aslw	asl.w
#define		asll	asl.l
#define		asrb	asr.b
#define		asrw	asr.w
#define		asrl	asr.l
#define		brb	bra.b
#define		bsrb	bsr.b
#define		bhib	bhi.b
#define		blsb	bls.b
#define		bccb	bcc.b
#define		bhsb	bhs.b
#define		bcsb	bcs.b
#define		blob	blo.b
#define		bneb	bne.b
#define		beqb	beq.b
#define		bvcb	bvc.b
#define		bvsb	bvs.b
#define		bplb	bpl.b
#define		bmib	bmi.b
#define		bgeb	bge.b
#define		bltb	blt.b
#define		bgtb	bgt.b
#define		bleb	ble.b
#define		tstb	tst.b
#define		tstw	tst.w
#define		tstl	tst.l
#define		casb	cas.b
#define		casw	cas.w
#define		casl	cas.l
#define		cmpb	cmp.b
#define		cmpw	cmp.w
#define		cmpl	cmp.l
#define		clrb	clr.b
#define		clrw	clr.w
#define		clrl	clr.l
#define		divw	divs.w
#define		divl	divs.l
#define		tdivl	tdivs.l
#define		divsw	divs.w
#define		divsl	divs.l
#define		tdivsl	tdivs.l
#define		divuw	divu.w
#define		divul	divu.l
#define		tdivul	tdivu.l
#define		eorb	eor.b
#define		eorw	eor.w
#define		eorl	eor.l
#define		xorb	eor.b
#define		xorw	eor.w
#define		xorl	eor.l
#define		extbw	ext.w
#define		extw	ext.w
#define		extwl	ext.l
#define		extl	ext.l
#define		extbl	extb.l
#define		illegal	short 0x4AFC
#define		lslb	lsl.b
#define		lslw	lsl.w
#define		lsll	lsl.l
#define		lsrb	lsr.b
#define		lsrw	lsr.w
#define		lsrl	lsr.l
#define		moval	lea.l
#define		movb	mov.b
#define		movw	mov.w
#define		movl	mov.l
#define		movmw	movm.w
#define		movrw	movm.w
#define		movml	movm.l
#define		movrl	movm.l
#define		movr	movm.l
#define		mulw	muls.w
#define		mull	muls.l
#define		tmull	tmuls.l
#define		mulsw	muls.w
#define		mulsl	muls.l
#define		tmulsl	tmuls.l
#define		muluw	mulu.w
#define		mulul	mulu.l
#define		tmulul	tmulu.l
#define		negb	neg.b
#define		negw	neg.w
#define		negl	neg.l
#define		notb	not.b
#define		notw	not.w
#define		notl	not.l
#define		orb	or.b
#define		orw	or.w
#define		orl	or.l
#define		pushal	pea.l
#define		rolb	rol.b
#define		rolw	rol.w
#define		roll	rol.l
#define		rorb	ror.b
#define		rorw	ror.w
#define		rorl	ror.l
#define		rolxb	roxl.b
#define		rolxw	roxl.w
#define		rolxl	roxl.l
#define		roxlb	roxl.b
#define		roxlw	roxl.w
#define		roxll	roxl.l
#define		rorxb	roxr.b
#define		rorxw	roxr.w
#define		rorxl	roxr.l
#define		roxrb	roxr.b
#define		roxrw	roxr.w
#define		roxrl	roxr.l
#define		sbcd	sbcd.b
#define		sbcdb	sbcd.b
#define		subb	sub.b
#define		subw	sub.w
#define		subl	sub.l
#define		subxb	subx.b
#define		subxw	subx.w
#define		subxl	subx.l
#define		swapw	swap.w
#define		tas	tas.b
#define		tasb	tas.b

#define		di	or.w &0x0700,%sr



; sys3

; more system service routines
; last modified %G% version %I%


; Dave Morano
; October 83



;*****************************************************************************

;	This file contains the code for several of the system service
;	routines used in the MC68000 based operating system. 
;	These routines are called
;	from the system service routine dispatcher.  These routines
;	execute in supervisor mode and therefor the calling process
;	can not be context switched during system service execution.


;*****************************************************************************



#include	"asm.h"
#include	"sysmac.h"		; additional instructions
#include	"crp.h"			; CRP definitions
#include	"pcb.h"			; include the PCB definitions
#include	"ddb.h"
#include	"ddt.h"
#include	"ucb.h"
#include	"syspar.h"
#include	"lev.h"
#include	"arg.h"
#include	"carg.h"
#include	"fc.h"
#include	"sysdef.h"




; process control system services


;-----------------------------------------------------------------------------
; go into hibernation type sleep

;	Arguments :
;	* NONE *

;	Returns :
;	- always OK

	global	ss_hiber

ss_hiber:
	movl	pcbp,%a0
	sipl_sync

	bclr	&pcv_hi,pcb_stat(%a0)	; is hibernate semaphore set ?
	bneb	hib_ret			; jump if yes

	movb	&ps_hib,pcb_ps(%a0)	; put into hibernate state
	mi_schedule

hib_ret:
	movl	&r_ok,%d0
	rts

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; wake up a process from a hibernation type of sleep

;	Arguments :
;	- PID of process to wake

;	Returns :
;	- return status

	global	ss_wake

ss_wake:
	movl	pcbp,%a0
	sipl_sch
	movl	arg1(%a1),%d1
	beqb	wak_me			; jump if myself

	cmpb	%d1,pcb_pid(%a0)
	beqb	wak_me			; jump if myself

	andw	&0x000F,%d1
	cmpb	%d1,&peti
	bhsb	wak_noexist		; jump if no process

	lslw	&2,%d1
	moval	pet,%a0
	movl	0x00(%a0,%d1.w),%d1	; get PCB address
	beqb	wak_noexist		; jump if no process

	movl	%d1,%a0

; check to see if the other process is waiting

	cmpb	pcb_ps(%a0),&ps_hib
	beqb	wak_other		; jump if process is waiting

	bset	&pcv_hi,pcb_stat(%a0)	; set hibernate semaphore
	brb	wak_ok

; put into compute state (A0 set to PCB address)
wak_other:
	mi_compute

wak_ok:
	movl	&r_ok,%d0
	rts

wak_me:
	bset	&pcv_hi,pcb_stat(%a0)	; set hibernate semaphore
	brb	wak_ok

wak_noexist:
	movl	&r_noexist,%d0
	rts

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; suspend a process

;	Argumsnts :
;	- PID of process to suspend

	set	sus_mask,0x0000		; none
	set	sus_off,-0*4

	global	ss_suspend

ss_suspend:
	link	%fp,&sus_off
	movr	&sus_mask,sus_off(%fp)

	movl	pcbp,%a0		; assume default is myself
	sipl_sch
	movl	arg1(%a1),%d1		; get PID argument
	beqb	sus_me			; jump to suspend myself

	cmpb	%d1,pcb_pid(%a0)	; my process PID ? (biased compare)
	beqb	sus_me			; jump if suspend myself

	andw	&0x000F,%d1		; crop it (unbias it)
	cmpb	%d1,&peti		; is PID out of bounds ?
	bhsb	sus_noexist		; jump if no process

	lslw	&2,%d1
	moval	pet,%a1
	movl	0x00(%a1,%d1.w),%d1	; get process PCB
	beqb	sus_noexist		; jump if no process

; suspend this other guy

	movl	%d1,%a0
	cmpb	pcb_ps(%a0),&ps_sus
	beqb	sus_as			; jump if already suspended

	movb	pcb_ps(%a0),%d0		; get old process state
	movb	%d0,pcb_pps(%a0)	; save old state
	movb	&ps_sus,pcb_ps(%a0)	; set new process state to SUSpend

; take action depending on previous state

	cmpb	%d0,&ps_cef
	beqb	sus_csw

	cmpb	%d0,&ps_mrw		; in resource wait ?
	beqb	sus_mrw			; jump if in resource wait

	cmpb	%d0,&ps_com		; is other process computing ?
	bneb	sus_done		; jump if not COM

sus_csw:
sus_mrw:
sus_com:

; remove from queue
sus_remq:
	bclr	&pcv_iq,pcb_stat(%a0)	; clear in Q bit
	movl	4(%a0),%a1
	mi_remq

sus_hib:
sus_lef:

; suspend was successful
sus_done:
	clrl	%d0

sus_ret:
	movr	sus_off(%fp),&sus_mask
	unlk	%fp
	rts

; suspend myself
sus_me:
	movb	pcb_ps(%a0),pcb_pps(%a0)
	movb	&ps_sus,pcb_ps(%a0)
	mi_schedule			; request scheduler wakeup

	brb	sus_done

; no process found
sus_noexist:
	movl	&r_noexist,%d0
	brb	sus_ret

; already suspended
sus_as:
	movl	&2,%d0
	brb	sus_ret

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; resume a process from a suspended state

;	Arguments :
;	- PID of process to resume

;	Returns :
;	- return status

	global	ss_resume

ss_resume:
	link	%fp,&sus_off
	movr	&sus_mask,sus_off(%fp)

	movl	arg1(%a1),%d1
	beq	res_me			; jump if myself

	movl	pcbp,%a0
	sipl_sch
	cmpb	%d1,pcb_pid(%a0)
	beq	res_me			; jump if myself

	andw	&0x000F,%d1
	cmpb	%d1,&peti
	bhs	res_noexist		; jump if no process

	lslw	&2,%d1
	moval	pet,%a1
	movl	0x00(%a1,%d1.w),%d1
	beq	res_noexist		; jump if no process

; resume this other guy

	movl	%d1,%a0
	cmpb	pcb_ps(%a0),&ps_sus
	bneb	res_done		; jump if already resumed

	movb	pcb_pps(%a0),%d1	; get old state
	movb	%d1,pcb_ps(%a0)		; restore old state

	cmpb	%d1,&ps_max
	bgeb	res_bad			; jump out if bad

	andw	&0x0007,%d1		; important to clear high byte

#ifdef	ASM20
	movw	10(%pc,%d1.w*2),%d0
	jmp	6(%pc,%d0.w)
	swbeg	&8
#else
	addw	%d1,%d1			; multiply by 2 (faster than shift)
	movw	10(%pc,%d1.w),%d0
	jmp	6(%pc,%d0.w)
	illegal
	short	8
#endif

res_base:
	short	res_bad		- res_base	; state not defined
	short	res_bad		- res_base	; state not defined
	short	res_com		- res_base
	short	res_lef		- res_base
	short	res_cef		- res_base
	short	res_hib		- res_base
	short	res_bad		- res_base	; no suspend from SUS state
	short	res_mrw		- res_base


; hibernate
res_hib:
	subl	&sc_size,pcb_pc(%a0)		; back up PC
;	brb	res_com

; miscellaneous resource wait
res_mrw:
;	brb	res_com

; put into compute (A0 set to PCB address)
res_com:
	mi_compute

	brb	res_done

; LEF
res_lef:
	movw	pcb_sem(%a0),%d0	; set D0 as argument for call
	brb	res_cond		; jump to common condition test code

; CEF
res_cef:
	moval	cef_q,%a1
	mi_insq

	bset	&pcv_iq,pcb_stat(%a0)	; set in Q bit
	movw	cef,%d0			; set D0 as argument for call

; common condition test here
res_cond:
	mi_condition

;	brb	res_done

; finish up
res_done:
	movl	&r_ok,%d0		; set good return code

res_ret:
	movr	sus_off(%fp),&sus_mask
	unlk	%fp
	rts

res_noexist:
	movl	&r_noexist,%d0
	brb	res_ret

; bad situation has happened
res_bad:
	xdt(9)

; trying to resume myself, invalid operation
res_me:
	movl	&r_invalid,%d0
	brb	res_ret

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; return the time of day to the user

	set	day_mask,0x2800		; A3, A5
	set	day_off,-2*4

	global	ss_daytime

ss_daytime:
	link	%fp,&day_off
	movr	&day_mask,day_off(%fp)

	movl	timer_ucb,%a3		; get UCB address for timer
	mi_tod				; get the time of day in D0-D1

	movl	carg1(%fp),%a5		; get pointer to user arguments

	movl	arg1(%a5),%a0		; get user buffer address
	movr	&0x0003,(%a0)		; put time in user buffer

	movl	&r_ok,%d0		; always return good status !

	movr	day_off(%fp),&day_mask
	unlk	%fp
	rts

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; relinquish the CPU

	global	ss_schedule

ss_schedule:
	di
	mi_schedule
	movl	&r_ok,%d0
	rts

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; set a timer

;	This is a system service that issues a timer request to the
;	timer manager.  If a feature of the service is not wanted or
;	needed, that parameter should be set to zero.  An event flag
;	number must always be specified, even if specified as zero.

;	Arguments :
;		event flag
;		status block
;		process interrupt service routine
;		interrupt service routine parameter
;		address of time parameter

;	Outputs to Driver Function Dispatcher :
;		a TQE

;	Outputs to Requesting Process :
;		request return status


	set	tim_mask,0x2C0C		; D2-D3, A2-A3, A5
	set	tim_off,-5*4

	global	ss_timer

ss_timer:
	link	%fp,&tim_off
	movr	&tim_mask,tim_off(%fp)

	movl	carg1(%fp),%a5		; get pointer to user arguments

; get a CRP from the free list

	jsr	sp_crpget		; try to get a CRP
	bvsb	tim_ret			; can't get a packet, exit now

; clear the user specified event flag

	movl	%a0,%a2			; save address of CRP in A2

	movl	arg1(%a5),%d0		; get SEM
	movw	%d0,crp_ef(%a2)		; set SEM field of CRP (word field)

	movl	pcbp,%a0		; get PCB
	movl	%a0,crp_pcb(%a2)	; set PCB in CRP
	mi_clear			; clear SEM in user's cluster

	movl	%a2,%a0			; restore CRP address in A0

; see if caller specified a completion status block address

	movl	arg2(%a5),%d0
	beqb	tim_sb			; jump if no status block

	movl	%d0,%a1			; get status block address
	clrl	(%a1)+			; zero out the status block
;	clrl	(%a1)

tim_sb:
	movl	%d0,crp_sb(%a0)

; copy over the process ISR arguments
tim30:
	movl	arg3(%a5),crp_isr(%a0)	; set ISR address
	movl	arg4(%a5),crp_ip(%a0)	; set ISR parameter

; fill in the user specified wake up time

	movl	arg5(%a5),%a1		; get address of time parameter
	movl	(%a1)+,tqe_tl(%a0)	; insert low half of time
	movl	(%a1),tqe_th(%a0)	; insert high half

; call the driver function dispatch routine

	movl	%a0,-(%sp)		; set CRP address
	movl	timer_ucb,-(%sp)	; set timer UCB address
	jsr	mft_set			; call driver function

;	addl	&2*4,%sp		; adjust stack after call

; return to caller ; return code should already be in D0
tim_ret:
	movr	tim_off(%fp),&tim_mask
	unlk	%fp
	rts

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; cancel a timer request

;	Arguments :
;	- semaphore
;	- status block address
;	- ISR address
;	- ISR parameter
;	- search parameter

;	Returns :
;	- success or type of failure


	global	ss_cantime

ss_cantime:
	link	%fp,&tim_off
	movr	&tim_mask,tim_off(%fp)

	movl	carg1(%fp),%a5		; get pointer to user arguments

; get a CRP from the free list

	jsr	sp_crpget		; get a CRP
	bvsb	ct_ret			; jump if can't get a CRP

	movl	%a0,%a2			; save address of CRP in A2

; clear the user specified event flag

	movl	arg1(%a5),%d0		; get SEM in D0
	movw	%d0,crp_ef(%a2)		; set SEM field of CRP (word field)

	movl	pcbp,%a0		; get PCB in A0
	movl	%a0,crp_pcb(%a2)	; set PCB in CRP
	mi_clear			; clear SEM in D0

; see if caller specified a completion status block address

	movl	arg2(%a5),%d0
	beqb	ct_sb			; jump if no status block

	movl	%d0,%a0			; get status block address
	clrl	(%a0)+			; zero out the status block
;	clrl	(%a0)

ct_sb:
	movl	%d0,crp_sb(%a2)

; copy over the process ISR arguments

	movl	arg3(%a5),crp_isr(%a2)	; set ISR address
	movl	arg4(%a5),crp_ip(%a2)	; set ISR parameter

; copy argument specifying parameter of operation to cancel

	movl	arg5(%a5),crp_p0(%a2)

; call the timer cancel routine

	movl	%a2,-(%sp)		; set CRP address
	movl	timer_ucb,-(%sp)	; set timer UCB address
	jsr	mft_cancel

;	addl	&2*4,%sp		; adjust stack after call

; return to caller ; return code should already be in D0
ct_ret:
	movr	tim_off(%fp),&tim_mask
	unlk	%fp
	rts

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; open a channel

;	Arguments :
;	* NONE NOW (experimental) *

;	Returns :
;	* NONE *


	set	ope_mask,0x000C		; D2-D3
	set	ope_off,-2*4

	global	ss_open

ss_open:
	link	%fp,&ope_off
	movr	&ope_mask,ope_off(%fp)

	movl	pcbp,%a0
	movl	con_ucb,%d1		; get UCB address

	clrl	%d3
	movl	&2,%d2

ope_loop:
	movw	%d3,%d0
	lslw	&2,%d0
	addw	&pcb_ccb,%d0
	movl	%d1,0x00(%a0,%d0.w)

	addl	&1,%d3
	dbf	%d2,ope_loop

	clrl	%d0

	movr	ope_off(%fp),&ope_mask
	unlk	%fp
	rts

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; close a channel

	global	ss_close

ss_close:
	link	%fp,&ope_off
	movr	&ope_mask,ope_off(%fp)

	movl	pcbp,%a0

	clrl	%d3
	movl	&2,%d2

clo_loop:
	movw	%d3,%d0
	lslw	&2,%d0
	addw	&pcb_ccb,%d0
	clrl	0x00(%a0,%d0.w)

	addl	&1,%d3
	dbf	%d2,clo_loop

	clrl	%d0

	movr	ope_off(%fp),&ope_mask
	unlk	%fp
	rts

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; issue a communication request

;	This is a system service that issues a communication request
;	to an I/O driver.  If a feature of the service is not wanted or
;	needed, that parameter should be set to zero.  An event flag
;	number must always be specified, even if specified as zero.

;	Arguments :
;		channel number
;		function
;		event flag
;		status block
;		process interrupt service routine
;		interrupt service routine parameter
;		p1
;		p2
;		p3
;		p4

;	Outputs to Driver Function Dispatcher :
;		a CRP

;	Outputs to Requesting Process :
;		request return status



	set	comm_mask,0x2C0C	; D2-D3, A2-A3, A5
	set	comm_off,-5*4

	global	ss_comm

ss_comm:
	link	%fp,&comm_off
	movr	&comm_mask,comm_off(%fp)

	movl	carg1(%fp),%a5		; get pointer to user arguments

	movl	arg1(%a5),%d1		; get device channel number
	cmpb	%d1,&8
	bhs	comm_offline		; jump if out of bounds

	movl	pcbp,%a0
	andw	&0x000F,%d1

#if	ASM20 && (ARCH20 || ARCH32)
	movl	pcb_ccb(%a0,%d1.w*4),%d0
#else
	lslw	&2,%d1			; multiply by 4
	addw	&pcb_ccb,%d1
	movl	0x00(%a0,%d1.w),%d0	; get UCB address
#endif
	beq	comm_offline		; jump if unit off-line

	movl	%d0,%a3			; set UCB address in A3

; get a CRP from the free list

	jsr	sp_crpget		; get a CRP, result CRP address in A0
	bvsb	comm_ret			; jump if can't get a CRP

; fill in the CRP

	movl	%a0,%a2			; save address of CRP in A2

; get & check function code

	movl	arg2(%a5),%d3		; get function code in D3
	cmpb	%d3,&fc_over
	bhsb	comm_badfc		; jump if bad value given

	movw	%d3,crp_fun(%a2)	; insert function code

; clear the user specified event flag

	movl	arg3(%a5),%d0		; get SEM in D0
	movw	%d0,crp_sem(%a2)	; set SEM field of CRP (SHORT)

	movl	pcbp,%a0		; get PCB in A0
	movl	%a0,crp_pcb(%a2)	; set PCB in CRP
	mi_clear			; clear SEM

; see if caller specified a completion status block address

	movl	arg4(%a5),%d0		; get possible status block address
	beqb	comm_sb			; jump if no status block

	movl	%d0,%a0			; get status block address
	clrl	(%a0)+			; zero out the status block
	clrl	(%a0)

comm_sb:
	movl	%d0,crp_sb(%a2)		; write status block address to CRP

; copy over the process ISR arguments
icr30:
	movl	arg5(%a5),crp_isr(%a2)	; set ISR address
	movl	arg6(%a5),crp_ip(%a2)	; set ISR parameter

; copy request dependent parameters (6 arguments starting @ argument 7)

	addl	&arg7,%a5
	moval	crp_p0(%a2),%a0
	movl	&5,%d0

comm_ca:
	movl	(%a5)+,(%a0)+
	dbf	%d0,comm_ca

; call the driver function dispatch routine

	movl	ucb_ddt(%a3),%a0	; get DDT address for unit
	movl	ddt_issue(%a0),%a0	; get issue jump table addrress

	movw	%d3,%d1
	andw	&0x0F,%d1

	lslw	&2,%d1			; multiply by 4
	movl	00(%a0,%d1.w),%d0	; get function routine address
	beqb	comm_notthere

	movl	%d0,%a0

	movl	%a2,-(%sp)		; set CRP address as argument
	movl	%a3,-(%sp)		; set UCB address as argument
	jsr	(%a0)			; call driver function

;	addl	&2*4,%sp		; adjust stack after call

; return to caller ; return code should already be in D0
comm_ret:
	movr	comm_off(%fp),&comm_mask
	unlk	%fp
	rts

comm_offline:
	movl	&r_offline,%d0		; set return code for off-line
	brb	comm_ret

; we have no routine in this driver for the FC, CRP address in A2
comm_notthere:
	movl	%a2,%a0

; we have a bad FC, CRP address is in A0
comm_badfc:
	jsr	sp_crpret

comm_invalid:
	movl	&r_invalid,%d0
	brb	comm_ret

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; cancel a communication request

;	Arguments :
;	- channel number
;	- function to cancel
;	- event flag
;	- status block
;	- process interrupt service routine
;	- parameter
;	- parameter of operation to cancel

;	Outputs to Driver Function Dispatcher :
;	- a CRP

;	Outputs to Requesting Process :
;	- request return status


	set	can_mask,0x2C0C		; D2-D3, A2-A3, A5
	set	can_off,-5*4

	global	ss_cancel

ss_cancel:
	link	%fp,&can_off
	movr	&can_mask,can_off(%fp)

	movl	carg1(%fp),%a5		; get pointer to user arguments

	movl	arg1(%a5),%d1		; get device channel number
	cmpb	%d1,&4
	bge	can_offline		; jump if out of bounds

	movl	pcbp,%a0
	andw	&0x0F,%d1
	lslw	&2,%d1			; multiply by 4
	addw	&pcb_ccb,%d1
	movl	0x00(%a0,%d1.w),%d0	; get UCB address
	beqb	can_off			; jump if channel off-line

	movl	%d0,%a3			; set UCB address in A3

; get a CRP from the free list

	jsr	sp_crpget		; get a CRP
	bvsb	can_ret			; jump if can't get a CRP

; fill in the CRP

	movl	%a0,%a2			; save address of CRP in A2
	movl	arg2(%a5),%d3		; get function code in D3
	movw	%d3,crp_fun(%a2)	; insert function code

; clear the user specified event flag

	movl	arg3(%a5),%d0		; get SEM in D0
	movw	%d0,crp_sem(%a2)	; set SEM field of CRP (SHORT)

	movl	pcbp,%a0		; get PCB in A0
	movl	%a0,crp_pcb(%a2)	; set PCB in CRP
	mi_clear			; clear SEM

; see if caller specified a completion status block address

	movl	arg4(%a5),%d0
	beqb	can_sb			; jump if no status block

	movl	%d0,%a0			; get status block address
	clrl	(%a0)+			; zero out the status block
	clrl	(%a0)

can_sb:
	movl	%d0,crp_sb(%a2)

; copy over the process ISR arguments
can30:
	movl	arg5(%a5),crp_isr(%a2)	; set ISR address
	movl	arg6(%a5),crp_ip(%a2)	; set ISR parameter

; copy argument specifying parameter of operation to cancel

	movl	arg7(%a5),crp_p0(%a0)

; call the driver cancel dispatch routine (A2 has CRP address)

	movl	ucb_ddt(%a3),%a0	; get DDT address for unit
	movl	ddt_issue(%a0),%a0	; get issue jump table addrress
	movw	&fc_cancel*4,%d0	; get offset of cancel table entry
	movl	00(%a0,%d1.w),%a0	; get function routine address

	movl	%a2,-(%sp)		; set CRP address as argument
	movl	%a3,-(%sp)		; set UCB address as argument
	jsr	(%a0)			; call driver cancel subroutine

;	addl	&2*4,%sp		; adjust stack after call

; return to caller ; return code should already be in D0
can_ret:
	movr	can_off(%fp),&can_mask
	unlk	%fp
	rts

can_offline:
	movl	&r_offline,%d0		; set return code for off-line
	brb	can_ret

can_invalid:
	movl	&r_invalid,%d0
	brb	can_ret

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; set process priority

;	Arguments :
;	- PID of process which is to be altered
;	- priority to set


	set	spi_mask,0x2000		; A5
	set	spi_off,-1*4

	global	ss_setpri

ss_setpri:
	link	%fp,&spi_off
	movr	&spi_mask,spi_off(%fp)

	movl	carg1(%fp),%a5		; get pointer to user arguments

	movl	arg2(%a5),%d1		; get priority value
	cmpb	%d1,&peti
	bhsb	spi_bad			; jump if bad priority value

	movl	pcbp,%a0		; assume default
	movl	arg1(%a5),%d0		; get PID of process
	beqb	spi_my			; jump to use default

; see if we have a valid PID

	andw	&0x000F,%d0
	cmpb	%d0,&peti
	bhsb	spi_noexist		; jump if invalid PID

	lslw	&2,%d0
	moval	pet,%a1
	movl	00(%a1,%d0.w),%d0
	beqb	spi_noexist		; jump if invalid PID

	cmpl	%a0,%d0
	beqb	spi_my			; jump if it is myself

	movl	%d0,%a0			; set PCB

; prepare to give the stuff

	movb	%d1,pcb_pri(%a0)	; set new priority

	cmpb	pcb_ps(%a0),&ps_com
	bneb	spi_ok			; jump if process not computing now

	movl	pcb_bl(%a0),%a1
	mi_remq

	mi_compute			; put back into operation

spi_ok:
	clrl	%d0			; set good return status

; return to user
spi_ret:
	movr	spi_off(%fp),&spi_mask
	unlk	%fp
	rts

; set my own priority

;	Now this may seem very simple here.  You may think that we
;	only have to change the priority field in my PCB and
;	we will automatically get that priority value.  But alas,
;	I, being a bone head at times, did not design the scheduling
;	routine to run this way.  The scheduler does not automatically
;	set the 'comsum' compute summary bit for the priority level
;	of a process that it has just requeued, due to either a
;	process timeout or a preemption.  The scheduler just requeues
;	a process assumming that the compute summary bit is already set.
;	Of course, if we now change the process priority value,
;	the scheduler will go ahead and requeue the process, but
;	the compute summary bit may never get set for that new priority
;	and this will set up a situation which can lead to the process
;	never getting seen by the scheduler again since its priority
;	level compute summary bit is not set.

;	This whole thing could have
;	easily been avoided by a small additional action taken
;	by the scheduler routine (setting the "compute summary" bit
;	on each process requeue), but I will choose to obey the
;	strategy as is and set the compute summary bit here instead.
;	The strategy in place was not arrived at without extensive
;	thought on the matter.  It was felt that since setting
;	your own process priority is a fairly rare event as
;	compared to a process getting a timeout or a preemption in
;	the course of it being executed, we will save the small
;	time needed in the scheduler to set the compute summary bit
;	on every process requeue and instead set the summary bit here.
;

spi_my:
	movb	%d1,pcb_pri(%a0)	; set new priority
#if	PRI16
	movw	comsum,%d0
	bset	%d1,%d0
	movw	%d0,comsum		; set summary bit
#else
	bset	%d1,comsum+1
#endif
	brb	spi_ok

spi_bad:
	movl	&r_invalid,%d0		; set bad for bad PRI
	brb	spi_ret

spi_noexist:
	movl	&r_noexist,%d0		; set bad for invalid PID
	brb	spi_ret

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; get process parameters

;	Arguments :
;	- PID of process to get information about
;	- item list address

;	Returns :
;	- success or type of failure


; process information table
pit:
	short	0x0000+pcb_pid		; PID (byte to long)
	short	0x0600+pcb_d0		; D0-D7, A0-A7 (16*4 bytes)
	short	0x0200+pcb_pc		; PC (long)
	short	0x0100+pcb_sr		; SR (short to long)
	short	0x0100+pcb_ft		; FT (short to long)
	short	0x0200+pcb_nam		; name string address
	short	0x0200+pcb_ima		; image string address
	short	0x0200+pcb_sp		; SP (4 bytes)

	short	0x0000+pcb_ps		; PS (byte to long)
	short	0x0000+pcb_pri		; PRI (byte to long)
	short	0x0100+pcb_wm		; WM (2 bytes extended to 4 bytes)
	short	0x0100+pcb_sem		; EF (2 bytes extended to 4 bytes)
	short	0x0100+pcb_crpq		; RPL (short to long)
	short	0x0100+pcb_crpc		; RPC (short to long)
	short	0x0300+pcb_ctime	; CPU time (8 bytes)
	short	0x0200+pcb_ds		; DS (default stack pointer, long)
	short	0x0000+pcb_stat		; CW (byte to long)
	short	0x0000+pcb_ppid		; parent PID (byte to long)

	set	piti,18			; over last index length


	set	gpi_mask,0x3C0C		; D2-D3, A2-A5
	set	gpi_off,-6*4

	global	ss_getproc

ss_getproc:
	link	%fp,&gpi_off
	movr	&gpi_mask,gpi_off(%fp)

	movl	carg1(%fp),%a5		; get pointer to user arguments

	movl	pcbp,%a0		; assume default
	movl	arg1(%a5),%d0		; get PID of process
	beqb	gpi_me			; jump to use default (myself)

; see if we have a valid PID

	andw	&0x000F,%d0
	cmpb	%d0,&peti
	bge	gpi_noexist		; jump if invalid PID

#if	ASM20 && (ARCH20 || ARCH32)
	movl	(pet,%d0.w*4),%d0
#else
	lslw	&2,%d0
	moval	pet,%a1
	movl	00(%a1,%d0.w),%d0
#endif
	beq	gpi_noexist		; jump if invalid PID

	movl	%d0,%a0			; set PCB

; prepare to give the stuff
gpi_me:
	movl	arg2(%a5),%a3		; get item list address
	movl	&piti-1,%d3
	moval	pit,%a2

; top of major get item loop (keeping track of item count with D3)
gpi14:
	movl	(%a3)+,%d0		; get item code from user
	cmpw	%d0,&0xFFFF		; end of item list ?
	beqb	gpi60			; go exit normally

	cmpw	%d0,&piti		; is code out of range ?
	bge	gpi72			; jump if illegal item code

; perform the transfer

	movl	(%a3)+,%a1		; get item buffer address

#if	ASM20 && (ARCH20 || ARCH32)
	movw	(%a2,%d0.w*2),%d2
#else
	addw	%d0,%d0			; multiply by 2 (faster than shift)
	movw	0(%a2,%d0.w),%d2	; get item descriptor
#endif

	movw	%d2,%d1			; important that high byte is clear
	lsrw	&7,%d1			; switch index X 2
	andw	&0x00FF,%d2		; mask off all but offset

#ifdef	ASM20
	movw	10(%pc,%d1.w),%d1
	jmp	6(%pc,%d1.w)
	swbeg	&8
#else
	movw	10(%pc,%d1.w),%d1
	jmp	6(%pc,%d1.w)
	illegal
	short	8
#endif

gpi20:
	short	gpi30-gpi20
	short	gpi31-gpi20
	short	gpi32-gpi20
	short	gpi33-gpi20
	short	gpi34-gpi20
	short	gpi35-gpi20
	short	gpi36-gpi20
	short	gpi37-gpi20

; move a byte only
gpi30:
	clrl	%d0
	movb	0x00(%a0,%d2.w),%d0
	movl	%d0,(%a1)
	brb	gpi50

; move a word
gpi31:
	clrl	%d0
	movw	0x00(%a0,%d2.w),%d0
	movl	%d0,(%a1)
	brb	gpi50

; move a longword
gpi32:
	movl	0x00(%a0,%d2.w),(%a1)
	brb	gpi50

; move 8 bytes
gpi33:
	movl	0x00(%a0,%d2.w),(%a1)+
	movl	0x04(%a0,%d2.w),(%a1)
	brb	gpi50

; move 16 bytes
gpi34:

; move 32 bytes
gpi35:

; move 64 bytes
gpi36:
	movl	&15,%d0
	movl	%a0,%a4
	addl	%d2,%a4

gpi66:
	movl	(%a4)+,(%a1)+
	dbf	%d0,gpi66

	brb	gpi50

; copy entire PCB
gpi37:
	movl	&31,%d0
	movl	%a0,%a4
	addl	%d2,%a4

gpi69:
	movl	(%a4)+,(%a1)+
	dbf	%d0,gpi69

	brb	gpi50

; bottom of major loop
gpi50:
	dbf	%d3,gpi14		; jump back to the top of the loop

; end loop


	tstw	%d3
	bltb	gpi73			; jump if more than possible number

gpi60:
	clrl	%d0

gpi_ret:
	movr	gpi_off(%fp),&gpi_mask
	unlk	%fp
	rts

; illegal PID
gpi_noexist:
	movl	&r_noexist,%d0
	brb	gpi_ret

; illegal item code
gpi72:
	movl	&r_invalid,%d0
	brb	gpi_ret

; too many items
gpi73:
	movl	&r_invalid,%d0
	brb	gpi_ret

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; change to supervisor mode

;	This service provides a means by which a user mode program
;	can execute a subroutine in supervisor mode thus enjoying
;	the enhanced priviledges obtained there in.

;	Arguments :
;	- address of user supplied subroutine to be executed in supervisor mode
;	- address of the argument list to be passed to the subroutine


	set	chm_mask,0x2000		; A5
	set	chm_off,-1*4

	global	ss_chmod

ss_chmod:
	link	%fp,&chm_off
	movr	&chm_mask,chm_off(%fp)

	movl	carg1(%fp),%a5		; get argument list pointer

	movl	arg1(%a5),%a0		; get PC of subroutine to call

	movl	arg2(%a5),-(%sp)	; set user argument list pointer
	movl	arg2(%a5),%a5		; set new AP in A5 also
	jsr	(%a0)			; call user supplied subroutine

	addl	&4,%sp			; adjust stack for call

chm_ret:
	movr	chm_off(%fp),&chm_mask
	unlk	%fp
	rts

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; set process exception vector

;	Arguments :
;	- address of new handler
;	- parameter to be passed to handler
;	- address of two long word array to receive old handler and parameter


	set	exc_mask,0x2000		; A5
	set	exc_off,-1*4

	global	ss_except

ss_except:
	link	%fp,&exc_off
	movr	&exc_mask,exc_off(%fp)

	movl	carg1(%fp),%a5		; get user argument list pointer

	movl	pcbp,%a0
	movl	arg3(%a5),%d0
	beqb	exc_skip		; jump if doesn't want old values

	movl	%d0,%a1
	movl	pcb_except(%a0),(%a1)+	; return old handler
	movl	pcb_exparam(%a0),(%a1)	; return old parameter

exc_skip:
	movl	arg1(%a5),pcb_except(%a0)
	movl	arg2(%a5),pcb_exparam(%a0)

	clrl	%d0

	movr	exc_off(%fp),&exc_mask
	unlk	%fp
	rts

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; put the current process in a busy wait low power "idle" condition

;	This service will keep the process computing but it will
;	not be executing anything.  Instead the process will in effect
;	be idle for an indeterminent period of time until 
;	its time slice runs out, it gets preempted, or the OS gets an
;	interrupt (which is truly random from the process point of view).
;	This service has the side benefit of keeping the computer
;	in a low power consumming mode while the suspend is in effect.


	global	ss_idle

ss_idle:
	stop	&0x2000			; power down

	clrl	%d0			; always succeeds !!
	rts

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; issue a vulture request

;	This is a system service that issues a request to wait for the
;	death of the specified process.

;	Arguments :
;	- semaphore number
;	- status block address
;	- process interrupt service routine
;	- interrupt service routine parameter
;	- pid

;	Outputs to Requesting Process :
;	- request return status


;	Register usage :
;	- D2 PCB address
;	- A2 CRP address
;	- A3 target process PCB address
;	- A5 argument list address


	set	vul_mask,0x2C04		; D2, A2-A3, A5
	set	vul_off,-4*4

	global	ss_vulture

ss_vulture:
	link	%fp,&vul_off
	movr	&vul_mask,vul_off(%fp)

	movl	carg1(%fp),%a5		; get pointer to user arguments

	movl	arg5(%a5),%d1
	beq	vul_me			; jump if myself

	movl	pcbp,%d2		; get & save in D2 for later
	movl	%d2,%a0
	cmpb	%d1,pcb_pid(%a0)
	beq	vul_me			; jump if myself

	andw	&pm_peti,%d1
	cmpb	%d1,&peti
	bhs	vul_noexist		; jump if no process

	sipl_sch
	lslw	&2,%d1
	moval	pet,%a1
	movl	0x00(%a1,%d1.w),%d0
	beq	vul_noexist		; jump if no process

	movl	%d0,%a3			; save target PCB address in A3

; get a CRP from the free list

	jsr	sp_crpget		; get a CRP
	bvsb	vul_ret			; jump if can't get a CRP

; fill in the CRP

	movl	%a0,%a2			; save address of CRP in A2

; clear the user specified semaphore

	movl	arg1(%a5),%d0		; get SEM in D0
	movw	%d0,crp_sem(%a2)	; set SEM field of CRP (SHORT)

	movl	%d2,%a0			; get my PCB in A0
	movl	%a0,crp_pcb(%a2)	; set PCB in CRP
	mi_clear			; clear SEM specified in D0 above

; see if caller specified a completion status block address

	movl	arg2(%a5),%d0		; get possible status block address
	beqb	vul_sb			; jump if no status block

	movl	%d0,%a0			; get status block address
	clrl	(%a0)+			; zero out the status block
	clrl	(%a0)

vul_sb:
	movl	%d0,crp_sb(%a2)		; write status block address to CRP

; copy over the process ISR arguments

	movl	arg3(%a5),crp_isr(%a2)	; set ISR address
	movl	arg4(%a5),crp_ip(%a2)	; set ISR parameter

; Q the middle of the CRP up to myself

	movl	%d2,%a0
	moval	pcb_preyq(%a0),%a1
	moval	crp_prey(%a2),%a0
	mi_insq

; Q this CRP up to the target

	moval	pcb_vq(%a3),%a1		; get target vulture Q address
	movl	%a2,%a0
	mi_insq

;	movl	&r_ok,%d0
	clrl	%d0

; return to caller ; return code should already be in D0
vul_ret:
	movr	vul_off(%fp),&vul_mask
	unlk	%fp
	rts

vul_noexist:
	movl	&r_noexist,%d0		; process does not exist
	brb	vul_ret

vul_me:
	movl	&r_invalid,%d0
	brb	vul_ret

; end subroutine
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; cancel a vulture request

;	Arguments :
;	- PID of request to cancel

;	Outputs to Requesting Process :
;	- request return status


	set	cvul_mask,0x001C	; D2-D4
	set	cvul_off,-3*4

	global	ss_canvulture

ss_canvulture:
	link	%fp,&cvul_off
	movr	&cvul_mask,cvul_off(%fp)

	movl	pcbp,%a0
	sipl_sch
	movl	arg1(%a1),%d1
	beq	cvul_all		; jump if cancel all

	cmpb	%d1,pcb_pid(%a0)
	beq	cvul_me			; jump if myself

	andw	&pm_peti,%d1
	cmpb	%d1,&peti
	bhs	cvul_noexist		; jump if no process

	lslw	&2,%d1
	moval	pet,%a1
	movl	0x00(%a1,%d1.w),%d0
	beq	cvul_noexist		; jump if no process

; cancel the ONE designated outstanding vulture request
cvul_one:
	movl	%a0,%d3			; save my PCB address for comparison
	movl	%d0,%a0			; get target PCB address
	moval	pcb_vq(%a0),%a1
	movl	%a1,%d4			; save Q header address in D4
	movl	%a1,%a0

cvul_ol:
	movl	(%a0),%a0
	cmpl	%a0,%d4			; we at end of Q ?
	beqb	cvul_ok

	cmpl	%d3,crp_pcb(%a0)	; this my request ?
	bneb	cvul_ol

	movl	%a0,%d2			; save A0 in D2 for storage
	movl	4(%a0),%a1
	mi_remq				; unlink from his vulture Q

	addl	&crp_prey,%a0
	movl	4(%a0),%a1
	mi_remq				; unlink from my prey Q

	subl	&crp_prey,%a0
	jsr	sp_crpret		; return CRP to pool

	movl	%d2,%a1			; restore A0
	brb	cvul_ol

; cancel ALL outstanding vulture requests
cvul_all:
	moval	pcb_preyq(%a0),%a1
	movl	%a1,%d2

cvul_al:
	mi_remq
	bvsb	cvul_ok

	subl	&crp_prey,%a0
	movl	4(%a0),%a1
	mi_remq				; unlink from target's vulture Q

	jsr	sp_crpret

	movl	%d2,%a1
	brb	cvul_al

cvul_ok:
	clrl	%d0

cvul_ret:
	movr	cvul_off(%fp),&cvul_mask
	unlk	%fp
	rts

cvul_me:
	movl	&r_invalid,%d0
	brb	cvul_ret

cvul_noexist:
	movl	&r_noexist,%d0
	brb	cvul_ret

; end subroutine
;-----------------------------------------------------------------------------



;	END	system services



