/* bitops */

/* bit operation defines */
/* last modifided %G% version %I% */


/* revision history:

	= 83/02/13, David A.D. Morano

	These macros were written from scratch but inspired by single
	instructions that did the same sort of thing on the VAX !


	= 93/08/27, David A.D. Morano

	I added the support for the 64 bit LONG values.


*/


#ifndef	BITOPS_INCLUDE
#define	BITOPS_INCLUDE	1


/* on char(acter) (byte) organized bit arrays */

#define	BTST(array,bn)	(((array)[(bn) >> 3] >> ((bn) & 7)) & 1)
#define	BSET(array,bn)	((array)[(bn) >> 3] |= (1 << ((bn) & 7)))
#define	BCLR(array,bn)	((array)[(bn) >> 3] &= (~ (1 << ((bn) & 7))))

#define	BTSTC(array,bn)	(((array)[(bn) >> 3] >> ((bn) & 7)) & 1)
#define	BSETC(array,bn)	((array)[(bn) >> 3] |= (1 << ((bn) & 7)))
#define	BCLRC(array,bn)	((array)[(bn) >> 3] &= (~ (1 << ((bn) & 7))))

#define	BTSTB(array,bn)	(((array)[(bn) >> 3] >> ((bn) & 7)) & 1)
#define	BSETB(array,bn)	((array)[(bn) >> 3] |= (1 << ((bn) & 7)))
#define	BCLRB(array,bn)	((array)[(bn) >> 3] &= (~ (1 << ((bn) & 7))))


/* on short word (half word -- 16 bit) organized bit arrays */

#define	BTSTS(array,bn)	(((array)[(bn) >> 4] >> ((bn) & 15)) & 1)
#define	BSETS(array,bn)	((array)[(bn) >> 4] |= (1 << ((bn) & 15)))
#define	BCLRS(array,bn)	((array)[(bn) >> 4] &= (~ (1 << ((bn) & 15))))

#define	BTSTH(array,bn)	(((array)[(bn) >> 4] >> ((bn) & 15)) & 1)
#define	BSETH(array,bn)	((array)[(bn) >> 4] |= (1 << ((bn) & 15)))
#define	BCLRH(array,bn)	((array)[(bn) >> 4] &= (~ (1 << ((bn) & 15))))


/* on integers (currently 32-bit) */

#define	BTSTI(array,bn)	(((array)[(bn) >> 5] >> ((bn) & 31)) & 1)
#define	BSETI(array,bn)	((array)[(bn) >> 5] |= (1 << ((bn) & 31)))
#define	BCLRI(array,bn)	((array)[(bn) >> 5] &= (~ (1 << ((bn) & 31))))


/* on words (32-bit) organized bit arrays */

#define	BTSTW(array,bn)	(((array)[(bn) >> 5] >> ((bn) & 31)) & 1)
#define	BSETW(array,bn)	((array)[(bn) >> 5] |= (1 << ((bn) & 31)))
#define	BCLRW(array,bn)	((array)[(bn) >> 5] &= (~ (1 << ((bn) & 31))))


/* on 64-bit longs */

#define	BTSTL(array,bn)	(((array)[(bn) >> 6] >> ((bn) & 63)) & 1)
#define	BSETL(array,bn)	((array)[(bn) >> 6] |= (1 << ((bn) & 63)))
#define	BCLRL(array,bn)	((array)[(bn) >> 6] &= (~ (1 << ((bn) & 63))))



#endif /* BITOPS_INCLUDE */



