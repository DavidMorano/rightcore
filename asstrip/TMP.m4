# 1 "dam21741.c" 
# 1 "./asm.h" 1





	set	vbo_trap0,((32+0)*4)
	set	vbo_trap1,((32+1)*4)
	set	vbo_trap6,((32+6)*4)
	set	vbo_trap7,((32+7)*4)
	set	vbo_trap8,((32+8)*4)
	set	vbo_trap9,((32+9)*4)
	set	vbo_trap10,((32+10)*4)
	set	vbo_trap11,((32+11)*4)
	set	vbo_trap12,((32+12)*4)
	set	vbo_trap13,((32+13)*4)
	set	vbo_trap14,((32+14)*4)
	set	vbo_trap15,((32+15)*4)
	set	srm_c,0x01
	set	srm_v,0x02
	set	srm_z,0x04
	set	srm_n,0x08
	set	srm_x,0x10
	set	srm_ipl,0x0700
	set	srm_m,0x1000
	set	srm_s,0x2000
	set	srm_t0,0x4000
	set	srm_t1,0x8000
	set	srm_t,0x8000
	set	srm_tt,0xC000
	set	srim_c,0xFFFE
	set	srim_v,0xFFFD
	set	srim_z,0xFFFB
	set	srim_n,0xFFF7
	set	srim_x,0xFFEF
	set	srim_ipl,0xF8FF
	set	srim_m,0xEFFF
	set	srim_s,0xDFFF
	set	srim_t0,0xBFFF
	set	srim_t1,0xEFFF
	set	srim_t,0xEFFF
	set	srim_tt,0x3FFF
	set	ccm_c,0x01
	set	ccm_v,0x02
	set	ccm_z,0x04
	set	ccm_n,0x08
	set	ccm_x,0x10
	set	ccm_vz,0x06
	set	ccim_c,0xFE
	set	ccim_v,0xFD
	set	ccim_z,0xFB
	set	ccim_n,0xF7
	set	ccim_x,0xEF
	set	ccim_vz,0xF9
	set	srv_c,0
	set	srv_v,1
	set	srv_z,2
	set	srv_n,3
	set	srv_x,4
	set	srv_m,12
	set	srv_s,13
	set	srv_t,14
	set	srv_t0,14
	set	srv_t1,15
	set	ccv_c,0
	set	ccv_v,1
	set	ccv_z,2
	set	ccv_n,3
	set	ccv_x,4
# 149 "dam21741.c" 2
# 1 "./sysmac.h" 1










# 150 "dam21741.c" 2
# 1 "./crp.h" 1




	set	crp_fl,0
	set	crp_bl,1*4
	set	crp_pcb,2*4		
	set	crp_fun,3*4		
	set	crp_fc,3*4		
	set	crp_sem,3*4+2		
	set	crp_ef,3*4+2		
	set	crp_sb,4*4		
	set	crp_isr,5*4		
	set	crp_param,6*4
	set	crp_ip,6*4		
	set	crp_p0,8*4
	set	crp_p1,crp_p0+4
	set	crp_p2,crp_p0+8
	set	crp_p3,crp_p0+3*4
	set	crp_p4,crp_p0+4*4
	set	crp_p5,crp_p0+5*4
	set	tqe_time,crp_p0
	set	tqe_tl,crp_p0		
	set	tqe_th,crp_p1
	set	crp_prey,crp_p2
	set	crp_d0,crp_p0		
	set	crp_d1,crp_p1
	set	crpl,16*4		
# 151 "dam21741.c" 2
# 1 "./pcb.h" 1








	set	pcb_fl,0
	set	pcb_bl,4
	set	pcb_name,2*4		
	set	pcb_nam,2*4
	set	pcb_ima,3*4		
	set	pcb_ps,4*4		
	set	pcb_pri,4*4+1		
	set	pcb_stat,4*4+2		
	set	pcb_pid,4*4+3		
	set	pcb_wm,5*4		
	set	pcb_sem,5*4+2		
	set	pcb_r,8*4		
	set	pcb_d0,pcb_r
	set	pcb_d1,pcb_r+4
	set	pcb_d4,pcb_r+4*4
	set	pcb_a0,pcb_r+8*4
	set	pcb_a1,pcb_r+9*4
	set	pcb_a4,pcb_r+12*4
	set	pcb_a5,pcb_r+13*4
	set	pcb_fp,pcb_r+14*4
	set	pcb_sp,pcb_r+15*4
	set	pcb_usp,pcb_sp		
	set	pcb_on,24*4		
	set	pcb_sr,pcb_on
	set	pcb_pc,pcb_on+2
	set	pcb_ft,pcb_on+6

	set	pcb_crpq,26*4		
	set	pcb_crpc,26*4+2		
	set	pcb_bpri,27*4+0		
	set	pcb_quantum,27*4+1	
	set	pcb_ctime,28*4		
	set	pcb_pps,30*4		
	set	pcb_mrw,30*4+1		
	set	pcb_quan,30*4+2		
	set	pcb_ipl,30*4+3		
	set	pcb_ds,31*4		
	set	pcb_ccb,32*4		
	set	pcb_vq,36*4		
	set	pcb_preyq,38*4		
	set	pcb_except,40*4		
	set	pcb_exparam,41*4	
	set	pcb_ppid,42*4		
	set	pcb_psem,42*4+2		
	set	pcb_exitq,44*4		
	set	pcb_msp,47*4		
	set	pcb_uisrq,52*4		
	set	pcb_kisrq,54*4		
	set	fdp,56*4		

	set	pcbl,64*4		

	set	pcv_wc,0		
	set	pcv_bc,1		
	set	pcv_iq,2		
	set	pcv_rd,3		
	set	pcv_ip,4		
	set	pcv_hi,5		
	set	pcv_km,6		

	set	ps_nul,0		
	set	ps_cur,1		
	set	ps_com,2		
	set	ps_lef,3		
	set	ps_cef,4		
	set	ps_hib,5		
	set	ps_sus,6		
	set	ps_mrw,7		
	set	ps_max,8

	set	pm_peti,0x000F
	set	pv_maxpri,7

# 152 "dam21741.c" 2
# 1 "./ddb.h" 1





	set	ddb_ddt,0*4		
	set	ddb_device,1*4		
	set	ddb_dev,1*4
	set	ddb_sync,2*4		
	set	ddb_sr,2*4
	set	ddb_vn,2*4+2		
# 153 "dam21741.c" 2
# 1 "./ddt.h" 1





	set	ddt_dci,0*4
	set	ddt_uci,1*4
	set	ddt_issue,2*4
# 154 "dam21741.c" 2
# 1 "./ucb.h" 1





	set	ucb_ddt,0*4		
	set	ucb_dev,1*4		
	set	ucb_device,1*4		
	set	ucb_cw,2*4		
	set	ucb_un,2*4+1
	set	ucb_sr,2*4+2
	set	ucb_cpb,4*4		
	set	ucb_cpb0,4*4
	set	ucb_cpb1,5*4
	set	ucb_cpb2,6*4
	set	ucb_cpb3,7*4
	set	ucb_q0,8*4		
	set	ucb_q1,10*4
	set	ucb_q2,12*4
	set	ucb_q3,14*4
	set	ucb_q4,16*4		
	set	ucb_q5,18*4
	set	ucb_crp,20*4
	set	ucb_crp0,20*4
	set	ucb_crp1,21*4
	set	ucb_crp2,22*4
	set	ucb_crp3,23*4
	set	ucb_data,24*4		
	set	ucbl,32*4


# 155 "dam21741.c" 2
# 1 "./syspar.h" 1





























	set	ss_size,0x200		
	set	sys_size,0x3000		
	set	vec_size,0x400		
	set	top_size,0x1000		

	set	image_start,0x00000000
	set	image_size, 0x00020000
	set	rw_start,0x01000000
	set	rw_size, 0x003A0000
	set	load_start,0x013A0000
	set	load_size, 0x00020000

	set	bss_start,0x013C0000
	set	bss_size, 0x00040000-top_size-vec_size


	set	peti,10			

	set	ceti,8			

	set	deti,0			

	set	ueti,0			

	set	nsrps,8			

	set	ncrps,32		

	set	nlrps,peti+16		

	set	npcbs,peti		

	set	nfiles,8		

	set	vbase,bss_start+bss_size
	set	issp,vbase		
	set	imsp,vbase-ss_size	
	set	sbase,vbase-ss_size-sys_size
	set	stack_size,0x000E00	
	set	ks_size,0x100		
	set	stack_area,sbase-npcbs*stack_size


	set	mfp_base,0x00306000
	set	iplm_mfp,0x0100		

	set	snapshot,0x00309000	


	set	quo_pcrp,6		

















	set 	ts_quantum,10		


	set	ct_quantum,6		
# 156 "dam21741.c" 2
# 1 "./lev.h" 1





	set	sr_sync,0x2600

	set	ft_short,0x0000
	set	ft_long,0x0001
	set	sc_size,4		
# 157 "dam21741.c" 2
# 1 "./arg.h" 1




	set	arg1,0
	set	arg2,1*4
	set	arg3,2*4
	set	arg4,3*4
	set	arg5,4*4
	set	arg6,5*4
	set	arg7,6*4
	set	arg8,7*4
	set	arg9,8*4
	set	arg10,9*4
	set	arg11,10*4
	set	arg12,11*4
# 158 "dam21741.c" 2
# 1 "./carg.h" 1




	set	carg1,2*4
	set	carg2,3*4
	set	carg3,4*4
	set	carg4,5*4
	set	carg5,6*4
# 159 "dam21741.c" 2
# 1 "./fc.h" 1





	set	fc_all,0
	set	fc_open,1
	set	fc_close,2
	set	fc_read,3
	set	fc_write,4
	set	fc_control,5
	set	fc_status,6
	set	fc_cancel,7
	set	fc_seek,8
	set	fc_over,9		

	set	fm_getmode,0x0100
	set	fm_setmode,0x1000
	set	fm_intatt, 0x0200
	set	fm_killatt,0x0400
	set	fm_outband,0x0800
	set	fv_getmode,8
	set	fv_setmode,12
	set	fv_intatt,9
	set	fv_killatt,10
	set	fv_outband,11

	set	fm_cco,0x0100
	set	fv_cco,8
	set	fm_rawout,0x0400
	set	fv_rawout,10

	set	fm_noecho,0x0800
	set	fv_noecho,11
	set	fm_notecho,0x2000
	set	fv_notecho,13
	set	fm_nofilter,0x1000
	set	fv_nofilter,12
	set	fm_rawin,0x0200
	set	fv_rawin,9

	set	r_out,0
	set	r_doneok,1
	set	r_timeout,2
	set	r_canceled,3
	set	r_ok,0
	set	r_resource,-1
	set	r_invalid,-2
	set	r_offline,-3
	set	r_notopen,-4
	set	r_noexist,-5
	set	r_illegal,-6

	set	srs_scrp,0		
	set	srs_pcrp,1		
# 160 "dam21741.c" 2
# 1 "./sysdef.h" 1














# 161 "dam21741.c" 2
	global	ss_hiber
ss_hiber:
	mov.l	pcbp,%a0
	or.w &0x0600,%sr
	bclr	&pcv_hi,pcb_stat(%a0)	
	bne.b	hib_ret			
	mov.b	&ps_hib,pcb_ps(%a0)	
	bset &0,sisum
hib_ret:
	mov.l	&r_ok,%d0
	rts
	global	ss_wake
ss_wake:
	mov.l	pcbp,%a0
	or.w &0x0600,%sr
	mov.l	arg1(%a1),%d1
	beq.b	wak_me			
	cmp.b	%d1,pcb_pid(%a0)
	beq.b	wak_me			
	and.w	&0x000F,%d1
	cmp.b	%d1,&peti
	bhs.b	wak_noexist		
	lsl.w	&2,%d1
	lea.l	pet,%a0
	mov.l	0x00(%a0,%d1.w),%d1	
	beq.b	wak_noexist		
	mov.l	%d1,%a0
	cmp.b	pcb_ps(%a0),&ps_hib
	beq.b	wak_other		
	bset	&pcv_hi,pcb_stat(%a0)	
	bra.b	wak_ok
wak_other:
	jsr sp_compute
wak_ok:
	mov.l	&r_ok,%d0
	rts
wak_me:
	bset	&pcv_hi,pcb_stat(%a0)	
	bra.b	wak_ok
wak_noexist:
	mov.l	&r_noexist,%d0
	rts
	set	sus_mask,0x0000		
	set	sus_off,-0*4
	global	ss_suspend
ss_suspend:
	link	%fp,&sus_off
	movm.l	&sus_mask,sus_off(%fp)
	mov.l	pcbp,%a0		
	or.w &0x0600,%sr
	mov.l	arg1(%a1),%d1		
	beq.b	sus_me			
	cmp.b	%d1,pcb_pid(%a0)	
	beq.b	sus_me			
	and.w	&0x000F,%d1		
	cmp.b	%d1,&peti		
	bhs.b	sus_noexist		
	lsl.w	&2,%d1
	lea.l	pet,%a1
	mov.l	0x00(%a1,%d1.w),%d1	
	beq.b	sus_noexist		
	mov.l	%d1,%a0
	cmp.b	pcb_ps(%a0),&ps_sus
	beq.b	sus_as			
	mov.b	pcb_ps(%a0),%d0		
	mov.b	%d0,pcb_pps(%a0)	
	mov.b	&ps_sus,pcb_ps(%a0)	
	cmp.b	%d0,&ps_cef
	beq.b	sus_csw
	cmp.b	%d0,&ps_mrw		
	beq.b	sus_mrw			
	cmp.b	%d0,&ps_com		
	bne.b	sus_done		
sus_csw:
sus_mrw:
sus_com:
sus_remq:
	bclr	&pcv_iq,pcb_stat(%a0)	
	mov.l	4(%a0),%a1
	trap &3
sus_hib:
sus_lef:
sus_done:
	clr.l	%d0
sus_ret:
	movm.l	sus_off(%fp),&sus_mask
	unlk	%fp
	rts
sus_me:
	mov.b	pcb_ps(%a0),pcb_pps(%a0)
	mov.b	&ps_sus,pcb_ps(%a0)
	bset &0,sisum			
	bra.b	sus_done
sus_noexist:
	mov.l	&r_noexist,%d0
	bra.b	sus_ret
sus_as:
	mov.l	&2,%d0
	bra.b	sus_ret
	global	ss_resume
ss_resume:
	link	%fp,&sus_off
	movm.l	&sus_mask,sus_off(%fp)
	mov.l	arg1(%a1),%d1
	beq	res_me			
	mov.l	pcbp,%a0
	or.w &0x0600,%sr
	cmp.b	%d1,pcb_pid(%a0)
	beq	res_me			
	and.w	&0x000F,%d1
	cmp.b	%d1,&peti
	bhs	res_noexist		
	lsl.w	&2,%d1
	lea.l	pet,%a1
	mov.l	0x00(%a1,%d1.w),%d1
	beq	res_noexist		
	mov.l	%d1,%a0
	cmp.b	pcb_ps(%a0),&ps_sus
	bne.b	res_done		
	mov.b	pcb_pps(%a0),%d1	
	mov.b	%d1,pcb_ps(%a0)		
	cmp.b	%d1,&ps_max
	bge.b	res_bad			
	and.w	&0x0007,%d1		
	mov.w	10(%pc,%d1.w*2),%d0
	jmp	6(%pc,%d0.w)
	swbeg	&8
# 334 "dam21741.c" 
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
	sub.l	&sc_size,pcb_pc(%a0)		
res_mrw:
res_com:
	jsr sp_compute
	bra.b	res_done
res_lef:
	mov.w	pcb_sem(%a0),%d0	
	bra.b	res_cond		
res_cef:
	lea.l	cef_q,%a1
	trap &2
	bset	&pcv_iq,pcb_stat(%a0)	
	mov.w	cef,%d0			
res_cond:
	jsr sp_condition
res_done:
	mov.l	&r_ok,%d0		
res_ret:
	movm.l	sus_off(%fp),&sus_mask
	unlk	%fp
	rts
res_noexist:
	mov.l	&r_noexist,%d0
	bra.b	res_ret
res_bad:
	long 0x4E4E0000+(9)
res_me:
	mov.l	&r_invalid,%d0
	bra.b	res_ret
	set	day_mask,0x2800		
	set	day_off,-2*4
	global	ss_daytime
ss_daytime:
	link	%fp,&day_off
	movm.l	&day_mask,day_off(%fp)
	mov.l	timer_ucb,%a3		
	trap &8				
	mov.l	carg1(%fp),%a5		
	mov.l	arg1(%a5),%a0		
	movm.l	&0x0003,(%a0)		
	mov.l	&r_ok,%d0		
	movm.l	day_off(%fp),&day_mask
	unlk	%fp
	rts
	global	ss_schedule
ss_schedule:
	or.w &0x0700,%sr
	bset &0,sisum
	mov.l	&r_ok,%d0
	rts
	set	tim_mask,0x2C0C		
	set	tim_off,-5*4
	global	ss_timer
ss_timer:
	link	%fp,&tim_off
	movm.l	&tim_mask,tim_off(%fp)
	mov.l	carg1(%fp),%a5		
	jsr	sp_crpget		
	bvs.b	tim_ret			
	mov.l	%a0,%a2			
	mov.l	arg1(%a5),%d0		
	mov.w	%d0,crp_ef(%a2)		
	mov.l	pcbp,%a0		
	mov.l	%a0,crp_pcb(%a2)	
	jsr sp_clear			
	mov.l	%a2,%a0			
	mov.l	arg2(%a5),%d0
	beq.b	tim_sb			
	mov.l	%d0,%a1			
	clr.l	(%a1)+			
tim_sb:
	mov.l	%d0,crp_sb(%a0)
tim30:
	mov.l	arg3(%a5),crp_isr(%a0)	
	mov.l	arg4(%a5),crp_ip(%a0)	
	mov.l	arg5(%a5),%a1		
	mov.l	(%a1)+,tqe_tl(%a0)	
	mov.l	(%a1),tqe_th(%a0)	
	mov.l	%a0,-(%sp)		
	mov.l	timer_ucb,-(%sp)	
	jsr	mft_set			
tim_ret:
	movm.l	tim_off(%fp),&tim_mask
	unlk	%fp
	rts
	global	ss_cantime
ss_cantime:
	link	%fp,&tim_off
	movm.l	&tim_mask,tim_off(%fp)
	mov.l	carg1(%fp),%a5		
	jsr	sp_crpget		
	bvs.b	ct_ret			
	mov.l	%a0,%a2			
	mov.l	arg1(%a5),%d0		
	mov.w	%d0,crp_ef(%a2)		
	mov.l	pcbp,%a0		
	mov.l	%a0,crp_pcb(%a2)	
	jsr sp_clear			
	mov.l	arg2(%a5),%d0
	beq.b	ct_sb			
	mov.l	%d0,%a0			
	clr.l	(%a0)+			
ct_sb:
	mov.l	%d0,crp_sb(%a2)
	mov.l	arg3(%a5),crp_isr(%a2)	
	mov.l	arg4(%a5),crp_ip(%a2)	
	mov.l	arg5(%a5),crp_p0(%a2)
	mov.l	%a2,-(%sp)		
	mov.l	timer_ucb,-(%sp)	
	jsr	mft_cancel
ct_ret:
	movm.l	tim_off(%fp),&tim_mask
	unlk	%fp
	rts
	set	ope_mask,0x000C		
	set	ope_off,-2*4
	global	ss_open
ss_open:
	link	%fp,&ope_off
	movm.l	&ope_mask,ope_off(%fp)
	mov.l	pcbp,%a0
	mov.l	con_ucb,%d1		
	clr.l	%d3
	mov.l	&2,%d2
ope_loop:
	mov.w	%d3,%d0
	lsl.w	&2,%d0
	add.w	&pcb_ccb,%d0
	mov.l	%d1,0x00(%a0,%d0.w)
	add.l	&1,%d3
	dbf	%d2,ope_loop
	clr.l	%d0
	movm.l	ope_off(%fp),&ope_mask
	unlk	%fp
	rts
	global	ss_close
ss_close:
	link	%fp,&ope_off
	movm.l	&ope_mask,ope_off(%fp)
	mov.l	pcbp,%a0
	clr.l	%d3
	mov.l	&2,%d2
clo_loop:
	mov.w	%d3,%d0
	lsl.w	&2,%d0
	add.w	&pcb_ccb,%d0
	clr.l	0x00(%a0,%d0.w)
	add.l	&1,%d3
	dbf	%d2,clo_loop
	clr.l	%d0
	movm.l	ope_off(%fp),&ope_mask
	unlk	%fp
	rts
	set	comm_mask,0x2C0C	
	set	comm_off,-5*4
	global	ss_comm
ss_comm:
	link	%fp,&comm_off
	movm.l	&comm_mask,comm_off(%fp)
	mov.l	carg1(%fp),%a5		
	mov.l	arg1(%a5),%d1		
	cmp.b	%d1,&8
	bhs	comm_offline		
	mov.l	pcbp,%a0
	and.w	&0x000F,%d1
	mov.l	pcb_ccb(%a0,%d1.w*4),%d0
# 619 "dam21741.c" 
	beq	comm_offline		
	mov.l	%d0,%a3			
	jsr	sp_crpget		
	bvs.b	comm_ret			
	mov.l	%a0,%a2			
	mov.l	arg2(%a5),%d3		
	cmp.b	%d3,&fc_over
	bhs.b	comm_badfc		
	mov.w	%d3,crp_fun(%a2)	
	mov.l	arg3(%a5),%d0		
	mov.w	%d0,crp_sem(%a2)	
	mov.l	pcbp,%a0		
	mov.l	%a0,crp_pcb(%a2)	
	jsr sp_clear			
	mov.l	arg4(%a5),%d0		
	beq.b	comm_sb			
	mov.l	%d0,%a0			
	clr.l	(%a0)+			
	clr.l	(%a0)
comm_sb:
	mov.l	%d0,crp_sb(%a2)		
icr30:
	mov.l	arg5(%a5),crp_isr(%a2)	
	mov.l	arg6(%a5),crp_ip(%a2)	
	add.l	&arg7,%a5
	lea.l	crp_p0(%a2),%a0
	mov.l	&5,%d0
comm_ca:
	mov.l	(%a5)+,(%a0)+
	dbf	%d0,comm_ca
	mov.l	ucb_ddt(%a3),%a0	
	mov.l	ddt_issue(%a0),%a0	
	mov.w	%d3,%d1
	and.w	&0x0F,%d1
	lsl.w	&2,%d1			
	mov.l	00(%a0,%d1.w),%d0	
	beq.b	comm_notthere
	mov.l	%d0,%a0
	mov.l	%a2,-(%sp)		
	mov.l	%a3,-(%sp)		
	jsr	(%a0)			
comm_ret:
	movm.l	comm_off(%fp),&comm_mask
	unlk	%fp
	rts
comm_offline:
	mov.l	&r_offline,%d0		
	bra.b	comm_ret
comm_notthere:
	mov.l	%a2,%a0
comm_badfc:
	jsr	sp_crpret
comm_invalid:
	mov.l	&r_invalid,%d0
	bra.b	comm_ret
	set	can_mask,0x2C0C		
	set	can_off,-5*4
	global	ss_cancel
ss_cancel:
	link	%fp,&can_off
	movm.l	&can_mask,can_off(%fp)
	mov.l	carg1(%fp),%a5		
	mov.l	arg1(%a5),%d1		
	cmp.b	%d1,&4
	bge	can_offline		
	mov.l	pcbp,%a0
	and.w	&0x0F,%d1
	lsl.w	&2,%d1			
	add.w	&pcb_ccb,%d1
	mov.l	0x00(%a0,%d1.w),%d0	
	beq.b	can_off			
	mov.l	%d0,%a3			
	jsr	sp_crpget		
	bvs.b	can_ret			
	mov.l	%a0,%a2			
	mov.l	arg2(%a5),%d3		
	mov.w	%d3,crp_fun(%a2)	
	mov.l	arg3(%a5),%d0		
	mov.w	%d0,crp_sem(%a2)	
	mov.l	pcbp,%a0		
	mov.l	%a0,crp_pcb(%a2)	
	jsr sp_clear			
	mov.l	arg4(%a5),%d0
	beq.b	can_sb			
	mov.l	%d0,%a0			
	clr.l	(%a0)+			
	clr.l	(%a0)
can_sb:
	mov.l	%d0,crp_sb(%a2)
can30:
	mov.l	arg5(%a5),crp_isr(%a2)	
	mov.l	arg6(%a5),crp_ip(%a2)	
	mov.l	arg7(%a5),crp_p0(%a0)
	mov.l	ucb_ddt(%a3),%a0	
	mov.l	ddt_issue(%a0),%a0	
	mov.w	&fc_cancel*4,%d0	
	mov.l	00(%a0,%d1.w),%a0	
	mov.l	%a2,-(%sp)		
	mov.l	%a3,-(%sp)		
	jsr	(%a0)			
can_ret:
	movm.l	can_off(%fp),&can_mask
	unlk	%fp
	rts
can_offline:
	mov.l	&r_offline,%d0		
	bra.b	can_ret
can_invalid:
	mov.l	&r_invalid,%d0
	bra.b	can_ret
	set	spi_mask,0x2000		
	set	spi_off,-1*4
	global	ss_setpri
ss_setpri:
	link	%fp,&spi_off
	movm.l	&spi_mask,spi_off(%fp)
	mov.l	carg1(%fp),%a5		
	mov.l	arg2(%a5),%d1		
	cmp.b	%d1,&peti
	bhs.b	spi_bad			
	mov.l	pcbp,%a0		
	mov.l	arg1(%a5),%d0		
	beq.b	spi_my			
	and.w	&0x000F,%d0
	cmp.b	%d0,&peti
	bhs.b	spi_noexist		
	lsl.w	&2,%d0
	lea.l	pet,%a1
	mov.l	00(%a1,%d0.w),%d0
	beq.b	spi_noexist		
	cmp.l	%a0,%d0
	beq.b	spi_my			
	mov.l	%d0,%a0			
	mov.b	%d1,pcb_pri(%a0)	
	cmp.b	pcb_ps(%a0),&ps_com
	bne.b	spi_ok			
	mov.l	pcb_bl(%a0),%a1
	trap &3
	jsr sp_compute			
spi_ok:
	clr.l	%d0			
spi_ret:
	movm.l	spi_off(%fp),&spi_mask
	unlk	%fp
	rts
spi_my:
	mov.b	%d1,pcb_pri(%a0)	
# 847 "dam21741.c" 
	bset	%d1,comsum+1
	bra.b	spi_ok
spi_bad:
	mov.l	&r_invalid,%d0		
	bra.b	spi_ret
spi_noexist:
	mov.l	&r_noexist,%d0		
	bra.b	spi_ret
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
	movm.l	&gpi_mask,gpi_off(%fp)
	mov.l	carg1(%fp),%a5		
	mov.l	pcbp,%a0		
	mov.l	arg1(%a5),%d0		
	beq.b	gpi_me			
	and.w	&0x000F,%d0
	cmp.b	%d0,&peti
	bge	gpi_noexist		
	mov.l	(pet,%d0.w*4),%d0
# 907 "dam21741.c" 
	beq	gpi_noexist		
	mov.l	%d0,%a0			
gpi_me:
	mov.l	arg2(%a5),%a3		
	mov.l	&piti-1,%d3
	lea.l	pit,%a2
gpi14:
	mov.l	(%a3)+,%d0		
	cmp.w	%d0,&0xFFFF		
	beq.b	gpi60			
	cmp.w	%d0,&piti		
	bge	gpi72			
	mov.l	(%a3)+,%a1		
	mov.w	(%a2,%d0.w*2),%d2
# 929 "dam21741.c" 
	mov.w	%d2,%d1			
	lsr.w	&7,%d1			
	and.w	&0x00FF,%d2		
	mov.w	10(%pc,%d1.w),%d1
	jmp	6(%pc,%d1.w)
	swbeg	&8
# 942 "dam21741.c" 
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
	clr.l	%d0
	mov.b	0x00(%a0,%d2.w),%d0
	mov.l	%d0,(%a1)
	bra.b	gpi50
gpi31:
	clr.l	%d0
	mov.w	0x00(%a0,%d2.w),%d0
	mov.l	%d0,(%a1)
	bra.b	gpi50
gpi32:
	mov.l	0x00(%a0,%d2.w),(%a1)
	bra.b	gpi50
gpi33:
	mov.l	0x00(%a0,%d2.w),(%a1)+
	mov.l	0x04(%a0,%d2.w),(%a1)
	bra.b	gpi50
gpi34:
gpi35:
gpi36:
	mov.l	&15,%d0
	mov.l	%a0,%a4
	add.l	%d2,%a4
gpi66:
	mov.l	(%a4)+,(%a1)+
	dbf	%d0,gpi66
	bra.b	gpi50
gpi37:
	mov.l	&31,%d0
	mov.l	%a0,%a4
	add.l	%d2,%a4
gpi69:
	mov.l	(%a4)+,(%a1)+
	dbf	%d0,gpi69
	bra.b	gpi50
gpi50:
	dbf	%d3,gpi14		
	tst.w	%d3
	blt.b	gpi73			
gpi60:
	clr.l	%d0
gpi_ret:
	movm.l	gpi_off(%fp),&gpi_mask
	unlk	%fp
	rts
gpi_noexist:
	mov.l	&r_noexist,%d0
	bra.b	gpi_ret
gpi72:
	mov.l	&r_invalid,%d0
	bra.b	gpi_ret
gpi73:
	mov.l	&r_invalid,%d0
	bra.b	gpi_ret
	set	chm_mask,0x2000		
	set	chm_off,-1*4
	global	ss_chmod
ss_chmod:
	link	%fp,&chm_off
	movm.l	&chm_mask,chm_off(%fp)
	mov.l	carg1(%fp),%a5		
	mov.l	arg1(%a5),%a0		
	mov.l	arg2(%a5),-(%sp)	
	mov.l	arg2(%a5),%a5		
	jsr	(%a0)			
	add.l	&4,%sp			
chm_ret:
	movm.l	chm_off(%fp),&chm_mask
	unlk	%fp
	rts
	set	exc_mask,0x2000		
	set	exc_off,-1*4
	global	ss_except
ss_except:
	link	%fp,&exc_off
	movm.l	&exc_mask,exc_off(%fp)
	mov.l	carg1(%fp),%a5		
	mov.l	pcbp,%a0
	mov.l	arg3(%a5),%d0
	beq.b	exc_skip		
	mov.l	%d0,%a1
	mov.l	pcb_except(%a0),(%a1)+	
	mov.l	pcb_exparam(%a0),(%a1)	
exc_skip:
	mov.l	arg1(%a5),pcb_except(%a0)
	mov.l	arg2(%a5),pcb_exparam(%a0)
	clr.l	%d0
	movm.l	exc_off(%fp),&exc_mask
	unlk	%fp
	rts
	global	ss_idle
ss_idle:
	stop	&0x2000			
	clr.l	%d0			
	rts
	set	vul_mask,0x2C04		
	set	vul_off,-4*4
	global	ss_vulture
ss_vulture:
	link	%fp,&vul_off
	movm.l	&vul_mask,vul_off(%fp)
	mov.l	carg1(%fp),%a5		
	mov.l	arg5(%a5),%d1
	beq	vul_me			
	mov.l	pcbp,%d2		
	mov.l	%d2,%a0
	cmp.b	%d1,pcb_pid(%a0)
	beq	vul_me			
	and.w	&pm_peti,%d1
	cmp.b	%d1,&peti
	bhs	vul_noexist		
	or.w &0x0600,%sr
	lsl.w	&2,%d1
	lea.l	pet,%a1
	mov.l	0x00(%a1,%d1.w),%d0
	beq	vul_noexist		
	mov.l	%d0,%a3			
	jsr	sp_crpget		
	bvs.b	vul_ret			
	mov.l	%a0,%a2			
	mov.l	arg1(%a5),%d0		
	mov.w	%d0,crp_sem(%a2)	
	mov.l	%d2,%a0			
	mov.l	%a0,crp_pcb(%a2)	
	jsr sp_clear			
	mov.l	arg2(%a5),%d0		
	beq.b	vul_sb			
	mov.l	%d0,%a0			
	clr.l	(%a0)+			
	clr.l	(%a0)
vul_sb:
	mov.l	%d0,crp_sb(%a2)		
	mov.l	arg3(%a5),crp_isr(%a2)	
	mov.l	arg4(%a5),crp_ip(%a2)	
	mov.l	%d2,%a0
	lea.l	pcb_preyq(%a0),%a1
	lea.l	crp_prey(%a2),%a0
	trap &2
	lea.l	pcb_vq(%a3),%a1		
	mov.l	%a2,%a0
	trap &2
	clr.l	%d0
vul_ret:
	movm.l	vul_off(%fp),&vul_mask
	unlk	%fp
	rts
vul_noexist:
	mov.l	&r_noexist,%d0		
	bra.b	vul_ret
vul_me:
	mov.l	&r_invalid,%d0
	bra.b	vul_ret
	set	cvul_mask,0x001C	
	set	cvul_off,-3*4
	global	ss_canvulture
ss_canvulture:
	link	%fp,&cvul_off
	movm.l	&cvul_mask,cvul_off(%fp)
	mov.l	pcbp,%a0
	or.w &0x0600,%sr
	mov.l	arg1(%a1),%d1
	beq	cvul_all		
	cmp.b	%d1,pcb_pid(%a0)
	beq	cvul_me			
	and.w	&pm_peti,%d1
	cmp.b	%d1,&peti
	bhs	cvul_noexist		
	lsl.w	&2,%d1
	lea.l	pet,%a1
	mov.l	0x00(%a1,%d1.w),%d0
	beq	cvul_noexist		
cvul_one:
	mov.l	%a0,%d3			
	mov.l	%d0,%a0			
	lea.l	pcb_vq(%a0),%a1
	mov.l	%a1,%d4			
	mov.l	%a1,%a0
cvul_ol:
	mov.l	(%a0),%a0
	cmp.l	%a0,%d4			
	beq.b	cvul_ok
	cmp.l	%d3,crp_pcb(%a0)	
	bne.b	cvul_ol
	mov.l	%a0,%d2			
	mov.l	4(%a0),%a1
	trap &3				
	add.l	&crp_prey,%a0
	mov.l	4(%a0),%a1
	trap &3				
	sub.l	&crp_prey,%a0
	jsr	sp_crpret		
	mov.l	%d2,%a1			
	bra.b	cvul_ol
cvul_all:
	lea.l	pcb_preyq(%a0),%a1
	mov.l	%a1,%d2
cvul_al:
	trap &3
	bvs.b	cvul_ok
	sub.l	&crp_prey,%a0
	mov.l	4(%a0),%a1
	trap &3				
	jsr	sp_crpret
	mov.l	%d2,%a1
	bra.b	cvul_al
cvul_ok:
	clr.l	%d0
cvul_ret:
	movm.l	cvul_off(%fp),&cvul_mask
	unlk	%fp
	rts
cvul_me:
	mov.l	&r_invalid,%d0
	bra.b	cvul_ret
cvul_noexist:
	mov.l	&r_noexist,%d0
	bra.b	cvul_ret
