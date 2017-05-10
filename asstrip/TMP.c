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













#include	"asm.h"
#include	"sysmac.h"		
#include	"crp.h"			
#include	"pcb.h"			
#include	"ddb.h"
#include	"ddt.h"
#include	"ucb.h"
#include	"syspar.h"
#include	"lev.h"
#include	"arg.h"
#include	"carg.h"
#include	"fc.h"
#include	"sysdef.h"







	global	ss_hiber
ss_hiber:
	movl	pcbp,%a0
	sipl_sync
	bclr	&pcv_hi,pcb_stat(%a0)	
	bneb	hib_ret			
	movb	&ps_hib,pcb_ps(%a0)	
	mi_schedule
hib_ret:
	movl	&r_ok,%d0
	rts








	global	ss_wake
ss_wake:
	movl	pcbp,%a0
	sipl_sch
	movl	arg1(%a1),%d1
	beqb	wak_me			
	cmpb	%d1,pcb_pid(%a0)
	beqb	wak_me			
	andw	&0x000F,%d1
	cmpb	%d1,&peti
	bhsb	wak_noexist		
	lslw	&2,%d1
	moval	pet,%a0
	movl	0x00(%a0,%d1.w),%d1	
	beqb	wak_noexist		
	movl	%d1,%a0

	cmpb	pcb_ps(%a0),&ps_hib
	beqb	wak_other		
	bset	&pcv_hi,pcb_stat(%a0)	
	brb	wak_ok

wak_other:
	mi_compute
wak_ok:
	movl	&r_ok,%d0
	rts
wak_me:
	bset	&pcv_hi,pcb_stat(%a0)	
	brb	wak_ok
wak_noexist:
	movl	&r_noexist,%d0
	rts






	set	sus_mask,0x0000		
	set	sus_off,-0*4
	global	ss_suspend
ss_suspend:
	link	%fp,&sus_off
	movr	&sus_mask,sus_off(%fp)
	movl	pcbp,%a0		
	sipl_sch
	movl	arg1(%a1),%d1		
	beqb	sus_me			
	cmpb	%d1,pcb_pid(%a0)	
	beqb	sus_me			
	andw	&0x000F,%d1		
	cmpb	%d1,&peti		
	bhsb	sus_noexist		
	lslw	&2,%d1
	moval	pet,%a1
	movl	0x00(%a1,%d1.w),%d1	
	beqb	sus_noexist		

	movl	%d1,%a0
	cmpb	pcb_ps(%a0),&ps_sus
	beqb	sus_as			
	movb	pcb_ps(%a0),%d0		
	movb	%d0,pcb_pps(%a0)	
	movb	&ps_sus,pcb_ps(%a0)	

	cmpb	%d0,&ps_cef
	beqb	sus_csw
	cmpb	%d0,&ps_mrw		
	beqb	sus_mrw			
	cmpb	%d0,&ps_com		
	bneb	sus_done		
sus_csw:
sus_mrw:
sus_com:

sus_remq:
	bclr	&pcv_iq,pcb_stat(%a0)	
	movl	4(%a0),%a1
	mi_remq
sus_hib:
sus_lef:

sus_done:
	clrl	%d0
sus_ret:
	movr	sus_off(%fp),&sus_mask
	unlk	%fp
	rts

sus_me:
	movb	pcb_ps(%a0),pcb_pps(%a0)
	movb	&ps_sus,pcb_ps(%a0)
	mi_schedule			
	brb	sus_done

sus_noexist:
	movl	&r_noexist,%d0
	brb	sus_ret

sus_as:
	movl	&2,%d0
	brb	sus_ret








	global	ss_resume
ss_resume:
	link	%fp,&sus_off
	movr	&sus_mask,sus_off(%fp)
	movl	arg1(%a1),%d1
	beq	res_me			
	movl	pcbp,%a0
	sipl_sch
	cmpb	%d1,pcb_pid(%a0)
	beq	res_me			
	andw	&0x000F,%d1
	cmpb	%d1,&peti
	bhs	res_noexist		
	lslw	&2,%d1
	moval	pet,%a1
	movl	0x00(%a1,%d1.w),%d1
	beq	res_noexist		

	movl	%d1,%a0
	cmpb	pcb_ps(%a0),&ps_sus
	bneb	res_done		
	movb	pcb_pps(%a0),%d1	
	movb	%d1,pcb_ps(%a0)		
	cmpb	%d1,&ps_max
	bgeb	res_bad			
	andw	&0x0007,%d1		
#ifdef	ASM20
	movw	10(%pc,%d1.w*2),%d0
	jmp	6(%pc,%d0.w)
	swbeg	&8
#else
	addw	%d1,%d1			
	movw	10(%pc,%d1.w),%d0
	jmp	6(%pc,%d0.w)
	illegal
	short	8
#endif
res_base:
	short	res_bad		- res_base	
	short	res_bad		- res_base	
	short	res_com		- res_base
	short	res_lef		- res_base
	short	res_cef		- res_base
	short	res_hib		- res_base
	short	res_bad		- res_base	
	short	res_mrw		- res_base

res_hib:
	subl	&sc_size,pcb_pc(%a0)		


res_mrw:


res_com:
	mi_compute
	brb	res_done

res_lef:
	movw	pcb_sem(%a0),%d0	
	brb	res_cond		

res_cef:
	moval	cef_q,%a1
	mi_insq
	bset	&pcv_iq,pcb_stat(%a0)	
	movw	cef,%d0			

res_cond:
	mi_condition


res_done:
	movl	&r_ok,%d0		
res_ret:
	movr	sus_off(%fp),&sus_mask
	unlk	%fp
	rts
res_noexist:
	movl	&r_noexist,%d0
	brb	res_ret

res_bad:
	xdt(9)

res_me:
	movl	&r_invalid,%d0
	brb	res_ret




	set	day_mask,0x2800		
	set	day_off,-2*4
	global	ss_daytime
ss_daytime:
	link	%fp,&day_off
	movr	&day_mask,day_off(%fp)
	movl	timer_ucb,%a3		
	mi_tod				
	movl	carg1(%fp),%a5		
	movl	arg1(%a5),%a0		
	movr	&0x0003,(%a0)		
	movl	&r_ok,%d0		
	movr	day_off(%fp),&day_mask
	unlk	%fp
	rts




	global	ss_schedule
ss_schedule:
	di
	mi_schedule
	movl	&r_ok,%d0
	rts


















	set	tim_mask,0x2C0C		
	set	tim_off,-5*4
	global	ss_timer
ss_timer:
	link	%fp,&tim_off
	movr	&tim_mask,tim_off(%fp)
	movl	carg1(%fp),%a5		

	jsr	sp_crpget		
	bvsb	tim_ret			

	movl	%a0,%a2			
	movl	arg1(%a5),%d0		
	movw	%d0,crp_ef(%a2)		
	movl	pcbp,%a0		
	movl	%a0,crp_pcb(%a2)	
	mi_clear			
	movl	%a2,%a0			

	movl	arg2(%a5),%d0
	beqb	tim_sb			
	movl	%d0,%a1			
	clrl	(%a1)+			

tim_sb:
	movl	%d0,crp_sb(%a0)

tim30:
	movl	arg3(%a5),crp_isr(%a0)	
	movl	arg4(%a5),crp_ip(%a0)	

	movl	arg5(%a5),%a1		
	movl	(%a1)+,tqe_tl(%a0)	
	movl	(%a1),tqe_th(%a0)	

	movl	%a0,-(%sp)		
	movl	timer_ucb,-(%sp)	
	jsr	mft_set			


tim_ret:
	movr	tim_off(%fp),&tim_mask
	unlk	%fp
	rts












	global	ss_cantime
ss_cantime:
	link	%fp,&tim_off
	movr	&tim_mask,tim_off(%fp)
	movl	carg1(%fp),%a5		

	jsr	sp_crpget		
	bvsb	ct_ret			
	movl	%a0,%a2			

	movl	arg1(%a5),%d0		
	movw	%d0,crp_ef(%a2)		
	movl	pcbp,%a0		
	movl	%a0,crp_pcb(%a2)	
	mi_clear			

	movl	arg2(%a5),%d0
	beqb	ct_sb			
	movl	%d0,%a0			
	clrl	(%a0)+			

ct_sb:
	movl	%d0,crp_sb(%a2)

	movl	arg3(%a5),crp_isr(%a2)	
	movl	arg4(%a5),crp_ip(%a2)	

	movl	arg5(%a5),crp_p0(%a2)

	movl	%a2,-(%sp)		
	movl	timer_ucb,-(%sp)	
	jsr	mft_cancel


ct_ret:
	movr	tim_off(%fp),&tim_mask
	unlk	%fp
	rts








	set	ope_mask,0x000C		
	set	ope_off,-2*4
	global	ss_open
ss_open:
	link	%fp,&ope_off
	movr	&ope_mask,ope_off(%fp)
	movl	pcbp,%a0
	movl	con_ucb,%d1		
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























	set	comm_mask,0x2C0C	
	set	comm_off,-5*4
	global	ss_comm
ss_comm:
	link	%fp,&comm_off
	movr	&comm_mask,comm_off(%fp)
	movl	carg1(%fp),%a5		
	movl	arg1(%a5),%d1		
	cmpb	%d1,&8
	bhs	comm_offline		
	movl	pcbp,%a0
	andw	&0x000F,%d1
#if	ASM20 && (ARCH20 || ARCH32)
	movl	pcb_ccb(%a0,%d1.w*4),%d0
#else
	lslw	&2,%d1			
	addw	&pcb_ccb,%d1
	movl	0x00(%a0,%d1.w),%d0	
#endif
	beq	comm_offline		
	movl	%d0,%a3			

	jsr	sp_crpget		
	bvsb	comm_ret			

	movl	%a0,%a2			

	movl	arg2(%a5),%d3		
	cmpb	%d3,&fc_over
	bhsb	comm_badfc		
	movw	%d3,crp_fun(%a2)	

	movl	arg3(%a5),%d0		
	movw	%d0,crp_sem(%a2)	
	movl	pcbp,%a0		
	movl	%a0,crp_pcb(%a2)	
	mi_clear			

	movl	arg4(%a5),%d0		
	beqb	comm_sb			
	movl	%d0,%a0			
	clrl	(%a0)+			
	clrl	(%a0)
comm_sb:
	movl	%d0,crp_sb(%a2)		

icr30:
	movl	arg5(%a5),crp_isr(%a2)	
	movl	arg6(%a5),crp_ip(%a2)	

	addl	&arg7,%a5
	moval	crp_p0(%a2),%a0
	movl	&5,%d0
comm_ca:
	movl	(%a5)+,(%a0)+
	dbf	%d0,comm_ca

	movl	ucb_ddt(%a3),%a0	
	movl	ddt_issue(%a0),%a0	
	movw	%d3,%d1
	andw	&0x0F,%d1
	lslw	&2,%d1			
	movl	00(%a0,%d1.w),%d0	
	beqb	comm_notthere
	movl	%d0,%a0
	movl	%a2,-(%sp)		
	movl	%a3,-(%sp)		
	jsr	(%a0)			


comm_ret:
	movr	comm_off(%fp),&comm_mask
	unlk	%fp
	rts
comm_offline:
	movl	&r_offline,%d0		
	brb	comm_ret

comm_notthere:
	movl	%a2,%a0

comm_badfc:
	jsr	sp_crpret
comm_invalid:
	movl	&r_invalid,%d0
	brb	comm_ret
















	set	can_mask,0x2C0C		
	set	can_off,-5*4
	global	ss_cancel
ss_cancel:
	link	%fp,&can_off
	movr	&can_mask,can_off(%fp)
	movl	carg1(%fp),%a5		
	movl	arg1(%a5),%d1		
	cmpb	%d1,&4
	bge	can_offline		
	movl	pcbp,%a0
	andw	&0x0F,%d1
	lslw	&2,%d1			
	addw	&pcb_ccb,%d1
	movl	0x00(%a0,%d1.w),%d0	
	beqb	can_off			
	movl	%d0,%a3			

	jsr	sp_crpget		
	bvsb	can_ret			

	movl	%a0,%a2			
	movl	arg2(%a5),%d3		
	movw	%d3,crp_fun(%a2)	

	movl	arg3(%a5),%d0		
	movw	%d0,crp_sem(%a2)	
	movl	pcbp,%a0		
	movl	%a0,crp_pcb(%a2)	
	mi_clear			

	movl	arg4(%a5),%d0
	beqb	can_sb			
	movl	%d0,%a0			
	clrl	(%a0)+			
	clrl	(%a0)
can_sb:
	movl	%d0,crp_sb(%a2)

can30:
	movl	arg5(%a5),crp_isr(%a2)	
	movl	arg6(%a5),crp_ip(%a2)	

	movl	arg7(%a5),crp_p0(%a0)

	movl	ucb_ddt(%a3),%a0	
	movl	ddt_issue(%a0),%a0	
	movw	&fc_cancel*4,%d0	
	movl	00(%a0,%d1.w),%a0	
	movl	%a2,-(%sp)		
	movl	%a3,-(%sp)		
	jsr	(%a0)			


can_ret:
	movr	can_off(%fp),&can_mask
	unlk	%fp
	rts
can_offline:
	movl	&r_offline,%d0		
	brb	can_ret
can_invalid:
	movl	&r_invalid,%d0
	brb	can_ret







	set	spi_mask,0x2000		
	set	spi_off,-1*4
	global	ss_setpri
ss_setpri:
	link	%fp,&spi_off
	movr	&spi_mask,spi_off(%fp)
	movl	carg1(%fp),%a5		
	movl	arg2(%a5),%d1		
	cmpb	%d1,&peti
	bhsb	spi_bad			
	movl	pcbp,%a0		
	movl	arg1(%a5),%d0		
	beqb	spi_my			

	andw	&0x000F,%d0
	cmpb	%d0,&peti
	bhsb	spi_noexist		
	lslw	&2,%d0
	moval	pet,%a1
	movl	00(%a1,%d0.w),%d0
	beqb	spi_noexist		
	cmpl	%a0,%d0
	beqb	spi_my			
	movl	%d0,%a0			

	movb	%d1,pcb_pri(%a0)	
	cmpb	pcb_ps(%a0),&ps_com
	bneb	spi_ok			
	movl	pcb_bl(%a0),%a1
	mi_remq
	mi_compute			
spi_ok:
	clrl	%d0			

spi_ret:
	movr	spi_off(%fp),&spi_mask
	unlk	%fp
	rts





























spi_my:
	movb	%d1,pcb_pri(%a0)	
#if	PRI16
	movw	comsum,%d0
	bset	%d1,%d0
	movw	%d0,comsum		
#else
	bset	%d1,comsum+1
#endif
	brb	spi_ok
spi_bad:
	movl	&r_invalid,%d0		
	brb	spi_ret
spi_noexist:
	movl	&r_noexist,%d0		
	brb	spi_ret










pit:
	short	0x0000+pcb_pid		
	short	0x0600+pcb_d0		
	short	0x0200+pcb_pc		
	short	0x0100+pcb_sr		
	short	0x0100+pcb_ft		
	short	0x0200+pcb_nam		
	short	0x0200+pcb_ima		
	short	0x0200+pcb_sp		
	short	0x0000+pcb_ps		
	short	0x0000+pcb_pri		
	short	0x0100+pcb_wm		
	short	0x0100+pcb_sem		
	short	0x0100+pcb_crpq		
	short	0x0100+pcb_crpc		
	short	0x0300+pcb_ctime	
	short	0x0200+pcb_ds		
	short	0x0000+pcb_stat		
	short	0x0000+pcb_ppid		
	set	piti,18			
	set	gpi_mask,0x3C0C		
	set	gpi_off,-6*4
	global	ss_getproc
ss_getproc:
	link	%fp,&gpi_off
	movr	&gpi_mask,gpi_off(%fp)
	movl	carg1(%fp),%a5		
	movl	pcbp,%a0		
	movl	arg1(%a5),%d0		
	beqb	gpi_me			

	andw	&0x000F,%d0
	cmpb	%d0,&peti
	bge	gpi_noexist		
#if	ASM20 && (ARCH20 || ARCH32)
	movl	(pet,%d0.w*4),%d0
#else
	lslw	&2,%d0
	moval	pet,%a1
	movl	00(%a1,%d0.w),%d0
#endif
	beq	gpi_noexist		
	movl	%d0,%a0			

gpi_me:
	movl	arg2(%a5),%a3		
	movl	&piti-1,%d3
	moval	pit,%a2

gpi14:
	movl	(%a3)+,%d0		
	cmpw	%d0,&0xFFFF		
	beqb	gpi60			
	cmpw	%d0,&piti		
	bge	gpi72			

	movl	(%a3)+,%a1		
#if	ASM20 && (ARCH20 || ARCH32)
	movw	(%a2,%d0.w*2),%d2
#else
	addw	%d0,%d0			
	movw	0(%a2,%d0.w),%d2	
#endif
	movw	%d2,%d1			
	lsrw	&7,%d1			
	andw	&0x00FF,%d2		
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

gpi30:
	clrl	%d0
	movb	0x00(%a0,%d2.w),%d0
	movl	%d0,(%a1)
	brb	gpi50

gpi31:
	clrl	%d0
	movw	0x00(%a0,%d2.w),%d0
	movl	%d0,(%a1)
	brb	gpi50

gpi32:
	movl	0x00(%a0,%d2.w),(%a1)
	brb	gpi50

gpi33:
	movl	0x00(%a0,%d2.w),(%a1)+
	movl	0x04(%a0,%d2.w),(%a1)
	brb	gpi50

gpi34:

gpi35:

gpi36:
	movl	&15,%d0
	movl	%a0,%a4
	addl	%d2,%a4
gpi66:
	movl	(%a4)+,(%a1)+
	dbf	%d0,gpi66
	brb	gpi50

gpi37:
	movl	&31,%d0
	movl	%a0,%a4
	addl	%d2,%a4
gpi69:
	movl	(%a4)+,(%a1)+
	dbf	%d0,gpi69
	brb	gpi50

gpi50:
	dbf	%d3,gpi14		

	tstw	%d3
	bltb	gpi73			
gpi60:
	clrl	%d0
gpi_ret:
	movr	gpi_off(%fp),&gpi_mask
	unlk	%fp
	rts

gpi_noexist:
	movl	&r_noexist,%d0
	brb	gpi_ret

gpi72:
	movl	&r_invalid,%d0
	brb	gpi_ret

gpi73:
	movl	&r_invalid,%d0
	brb	gpi_ret










	set	chm_mask,0x2000		
	set	chm_off,-1*4
	global	ss_chmod
ss_chmod:
	link	%fp,&chm_off
	movr	&chm_mask,chm_off(%fp)
	movl	carg1(%fp),%a5		
	movl	arg1(%a5),%a0		
	movl	arg2(%a5),-(%sp)	
	movl	arg2(%a5),%a5		
	jsr	(%a0)			
	addl	&4,%sp			
chm_ret:
	movr	chm_off(%fp),&chm_mask
	unlk	%fp
	rts








	set	exc_mask,0x2000		
	set	exc_off,-1*4
	global	ss_except
ss_except:
	link	%fp,&exc_off
	movr	&exc_mask,exc_off(%fp)
	movl	carg1(%fp),%a5		
	movl	pcbp,%a0
	movl	arg3(%a5),%d0
	beqb	exc_skip		
	movl	%d0,%a1
	movl	pcb_except(%a0),(%a1)+	
	movl	pcb_exparam(%a0),(%a1)	
exc_skip:
	movl	arg1(%a5),pcb_except(%a0)
	movl	arg2(%a5),pcb_exparam(%a0)
	clrl	%d0
	movr	exc_off(%fp),&exc_mask
	unlk	%fp
	rts











	global	ss_idle
ss_idle:
	stop	&0x2000			
	clrl	%d0			
	rts



















	set	vul_mask,0x2C04		
	set	vul_off,-4*4
	global	ss_vulture
ss_vulture:
	link	%fp,&vul_off
	movr	&vul_mask,vul_off(%fp)
	movl	carg1(%fp),%a5		
	movl	arg5(%a5),%d1
	beq	vul_me			
	movl	pcbp,%d2		
	movl	%d2,%a0
	cmpb	%d1,pcb_pid(%a0)
	beq	vul_me			
	andw	&pm_peti,%d1
	cmpb	%d1,&peti
	bhs	vul_noexist		
	sipl_sch
	lslw	&2,%d1
	moval	pet,%a1
	movl	0x00(%a1,%d1.w),%d0
	beq	vul_noexist		
	movl	%d0,%a3			

	jsr	sp_crpget		
	bvsb	vul_ret			

	movl	%a0,%a2			

	movl	arg1(%a5),%d0		
	movw	%d0,crp_sem(%a2)	
	movl	%d2,%a0			
	movl	%a0,crp_pcb(%a2)	
	mi_clear			

	movl	arg2(%a5),%d0		
	beqb	vul_sb			
	movl	%d0,%a0			
	clrl	(%a0)+			
	clrl	(%a0)
vul_sb:
	movl	%d0,crp_sb(%a2)		

	movl	arg3(%a5),crp_isr(%a2)	
	movl	arg4(%a5),crp_ip(%a2)	

	movl	%d2,%a0
	moval	pcb_preyq(%a0),%a1
	moval	crp_prey(%a2),%a0
	mi_insq

	moval	pcb_vq(%a3),%a1		
	movl	%a2,%a0
	mi_insq

	clrl	%d0

vul_ret:
	movr	vul_off(%fp),&vul_mask
	unlk	%fp
	rts
vul_noexist:
	movl	&r_noexist,%d0		
	brb	vul_ret
vul_me:
	movl	&r_invalid,%d0
	brb	vul_ret








	set	cvul_mask,0x001C	
	set	cvul_off,-3*4
	global	ss_canvulture
ss_canvulture:
	link	%fp,&cvul_off
	movr	&cvul_mask,cvul_off(%fp)
	movl	pcbp,%a0
	sipl_sch
	movl	arg1(%a1),%d1
	beq	cvul_all		
	cmpb	%d1,pcb_pid(%a0)
	beq	cvul_me			
	andw	&pm_peti,%d1
	cmpb	%d1,&peti
	bhs	cvul_noexist		
	lslw	&2,%d1
	moval	pet,%a1
	movl	0x00(%a1,%d1.w),%d0
	beq	cvul_noexist		

cvul_one:
	movl	%a0,%d3			
	movl	%d0,%a0			
	moval	pcb_vq(%a0),%a1
	movl	%a1,%d4			
	movl	%a1,%a0
cvul_ol:
	movl	(%a0),%a0
	cmpl	%a0,%d4			
	beqb	cvul_ok
	cmpl	%d3,crp_pcb(%a0)	
	bneb	cvul_ol
	movl	%a0,%d2			
	movl	4(%a0),%a1
	mi_remq				
	addl	&crp_prey,%a0
	movl	4(%a0),%a1
	mi_remq				
	subl	&crp_prey,%a0
	jsr	sp_crpret		
	movl	%d2,%a1			
	brb	cvul_ol

cvul_all:
	moval	pcb_preyq(%a0),%a1
	movl	%a1,%d2
cvul_al:
	mi_remq
	bvsb	cvul_ok
	subl	&crp_prey,%a0
	movl	4(%a0),%a1
	mi_remq				
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



