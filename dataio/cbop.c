/* last modified %G% version %I% */

/*
	David A.D. Morano
	April 1983
*/




/* set a bit in a longword */
int bset(bit,bf)
unsigned long	bit, *bf ;
{
	register unsigned long	mask, temp ;

	register int	i ;

	i = bit >> 5 ;
	mask = 1L << (bit & 0x0000001FL) ;
	temp = bf[i] & mask ;
	bf[i] |= mask ;
	return (temp ? 1 : 0) ;
}
/* end of subroutine */
/*----------------------------------------------------------------------*/


/* subroutine to clear a bit */
int bclr(bit,bf) 
unsigned long	bit, *bf ;
{
	register unsigned long		mask, temp ;

	register int	i ;

	i = bit >> 5 ;
	mask = 1L << (bit & 0x0000001FL) ;
	temp = bf[i] & mask ;
	mask = ~mask ;
	bf[i] &= mask ;
	return (temp ? 1 : 0) ;
}
/* end of subroutine */
/*-----------------------------------------------------------------*/


/* test a bit */
int btst(bit,bf)
unsigned long	bit, *bf ;
{
	register unsigned long	mask ;

	unsigned long	i ;

	i = bit >> 5 ;
	mask = 1L << (bit & 0x0000001FL) ;
	return ((mask & bf[i]) ? 1 : 0) ;
}
/* end of subroutine */
/*-------------------------------------------------------*/

