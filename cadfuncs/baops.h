/* baops */

/* bit-array operational defines */
/* last modifided %G% version %I% */


/* revision history:

	= 1998-08-27, David A­D­ Morano
        These macros were written from scratch but inspired by single
        instructions that did the same sort of thing on the VAX!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	BAOPS_INCLUDE
#define	BAOPS_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


/* on char(acter) (byte) organized bit arrays */

#define	BASET(array,bn)		((array)[(bn) >> 3] |= (1 << ((bn) & 7)))
#define	BACLR(array,bn)		((array)[(bn) >> 3] &= (~ (1 << ((bn) & 7))))
#define	BATST(array,bn)		(((array)[(bn) >> 3] >> ((bn) & 7)) & 1)

#define	BASETC(array,bn)	((array)[(bn) >> 3] |= (1 << ((bn) & 7)))
#define	BACLRC(array,bn)	((array)[(bn) >> 3] &= (~ (1 << ((bn) & 7))))
#define	BATSTC(array,bn)	(((array)[(bn) >> 3] >> ((bn) & 7)) & 1)

#define	BASETB(array,bn)	((array)[(bn) >> 3] |= (1 << ((bn) & 7)))
#define	BACLRB(array,bn)	((array)[(bn) >> 3] &= (~ (1 << ((bn) & 7))))
#define	BATSTB(array,bn)	(((array)[(bn) >> 3] >> ((bn) & 7)) & 1)


/* on short word (16-bit) organized bit arrays */

#define	BASETS(array,bn)	((array)[(bn) >> 4] |= (1 << ((bn) & 15)))
#define	BACLRS(array,bn)	((array)[(bn) >> 4] &= (~ (1 << ((bn) & 15))))
#define	BATSTS(array,bn)	(((array)[(bn) >> 4] >> ((bn) & 15)) & 1)


/* on integers (currently 32-bit) */

#define	BASETI(array,bn)	((array)[(bn) >> 5] |= (1 << ((bn) & 31)))
#define	BACLRI(array,bn)	((array)[(bn) >> 5] &= (~ (1 << ((bn) & 31))))
#define	BATSTI(array,bn)	(((array)[(bn) >> 5] >> ((bn) & 31)) & 1)


/* on words (32-bit) organized bit arrays */

#define	BASETW(array,bn)	((array)[(bn) >> 5] |= (1 << ((bn) & 31)))
#define	BACLRW(array,bn)	((array)[(bn) >> 5] &= (~ (1 << ((bn) & 31))))
#define	BATSTW(array,bn)	(((array)[(bn) >> 5] >> ((bn) & 31)) & 1)


/* on 64-bit longs */

#define	BASETL(array,bn) \
	((array)[(bn) >> 6] |= (1ULL << ((bn) & 63)))
#define	BACLRL(array,bn) \
	((array)[(bn) >> 6] &= (~ (1ULL << ((bn) & 63))))
#define	BATSTL(array,bn) \
	(((array)[(bn) >> 6] >> ((bn) & 63)) & 1ULL)


#ifdef	__cplusplus
extern "C" {
#endif

extern int	baset(unsigned char *,int) ;
extern int	baclr(unsigned char *,int) ;
extern int	batst(unsigned char *,int) ;

extern int	basetl(ULONG *,int) ;
extern int	baclrl(ULONG *,int) ;
extern int	batstl(ULONG *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* BAOPS_INCLUDE */


