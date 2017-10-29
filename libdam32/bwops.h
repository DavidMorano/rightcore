/* bwops */

/* bit-word defines */
/* last modifided %G% version %I% */


/* revision history:

	= 1998-08-27, David A­D­ Morano
        These macros were written from scratch but inspired by single
        instructions that did the same sort of thing on the VAX!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	BWOPS_INCLUDE
#define	BWOPS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>		/* for unsigned types */


#ifdef	__cplusplus
extern "C" {
#endif

#define	bwtst(w,n)		(((w) >> (n)) & 1)
#define	bwset(w,n)		((w) |= (1 << (n)))
#define	bwclr(w,n)		((w) &= (~ (1 << (n))))

/* char */
#define	bwtstc(w,n)		(((w) >> (n)) & 1)
#define	bwsetc(w,n)		((w) |= (1 << (n)))
#define	bwclrc(w,n)		((w) &= (~ (1 << (n))))

/* short */
#define	bwtsts(w,n)		(((w) >> (n)) & 1)
#define	bwsets(w,n)		((w) |= (1 << (n)))
#define	bwclrs(w,n)		((w) &= (~ (1 << (n))))

/* integer */
#define	bwtsti(w,n)		(((w) >> (n)) & 1)
#define	bwseti(w,n)		((w) |= (1 << (n)))
#define	bwclri(w,n)		((w) &= (~ (1 << (n))))

/* long */
#define	bwtstl(w,n)		(((w) >> (n)) & 1)
#define	bwsetl(w,n)		((w) |= (1 << (n)))
#define	bwclrl(w,n)		((w) &= (~ (1 << (n))))

/* long-long */
#define	bwtstll(w,n)		(((w) >> (n)) & 1)
#define	bwsetll(w,n)		((w) |= (1 << (n)))
#define	bwclrll(w,n)		((w) &= (~ (1 << (n))))

#ifdef	__cplusplus
}
#endif

#endif /* BWOPS_INCLUDE */


