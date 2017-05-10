
	.section	".text",#alloc,#execinstr
	.align	8
	.skip	16

	! block 0

	.global	sub
	.type	sub,2
sub:
	save	%sp,-112,%sp
	st	%i1,[%fp+72]
	st	%i0,[%fp+68]

	! block 1
.L151:

! File sub.c:
!    1	/* main */
!    2	
!    4	#include	<sys/types.h>
!    5	
!    6	#include	<bio.h>
!    7	
!   10	/* private module data structures */
!   11	
!   12	struct thing {
!   13		int	a, b ;
!   14	} ;
!   15	
!   19	struct thing	sub(a,b)
!   20	int	a, b ;
!   21	{
!   22		struct thing	this ;
!   23	
!   24		this.a = a ;

	ld	[%fp+68],%l0
	st	%l0,[%fp-12]

!   25		this.b = b ;

	ld	[%fp+72],%l0
	st	%l0,[%fp-8]

!   26		return this ;

	add	%fp,-12,%l0
	st	%l0,[%fp-4]
	ba	.L150
	nop

	! block 2
.L150:
	ld	[%fp-4],%l0
	mov	%l0,%o0
	or	%g0,8,%o1
	call	.stret4
	nop
	.size	sub,(.-sub)
	.align	8

	.file	"sub.c"
	.xstabs	".stab.index","Xa ; V=3.1 ; R=SC4.0 06/11/96 C 4.0 patch 102955-08",60,0,0,0
	.xstabs	".stab.index","/home/dam/src/teststruct; /opt/SUNWspro/bin/../SC4.0/bin/cc -I/home/dam/include -S  sub.c -W0,-xp",52,0,0,0
	.ident	"@(#)types.h	1.38	95/11/14 SMI"
	.ident	"@(#)feature_tests.h	1.7	94/12/06 SMI"
	.ident	"@(#)isa_defs.h	1.7	94/10/26 SMI"
	.ident	"@(#)machtypes.h	1.9	94/11/05 SMI"
	.ident	"@(#)select.h	1.10	92/07/14 SMI"
	.ident	"@(#)time.h	2.47	95/08/24 SMI"
	.ident	"@(#)time.h	1.23	95/08/28 SMI"
	.ident	"@(#)siginfo.h	1.36	95/08/24 SMI"
	.ident	"@(#)machsig.h	1.10	94/11/05 SMI"
	.ident	"@(#)unistd.h	1.33	95/08/28 SMI"
	.ident	"@(#)unistd.h	1.24	95/08/24 SMI"
	.ident	"acomp: SC4.0 06/11/96 C 4.0 patch 102955-08"
