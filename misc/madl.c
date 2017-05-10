/* make down load */

#include	"rel.h"

#include	<stdio.h>


/*****************************************************************************

	This program will convert an SREC type input file into a 
	BELLMAC assembly language source file.


******************************************************************************/


#define		EOL	'\n'		/* end of line mark */

main()
{
	extern int	cfhex(), putshort() ;

	int	c, i, nsw, len, state ;

	char	buf[8] ;


/* start of instructions */

	printf("	.text\n") ;
	printf("	.align 4\n") ;
	printf("	.globl code\n") ;
	printf("code:\n") ;

	state = 0 ;

next_line:
	c = getchar() ;				/* read 'S' */
	if (c == EOF) goto bad_exit ;

	if (c != 'S') goto next_line ;

	c = getchar() ;				/* read SREC code digit */
	if (c == EOF) goto bad_exit ;

	if (c != '2') goto exit ;

/* read the two length digits */

	c = getchar() ;
	if (c == EOF) goto bad_exit ;

	buf[0] = c ;

	c = getchar() ;
	if (c == EOF) goto bad_exit ;

	buf[1] = c ;

	if (cfhex(2L,buf,&len)) goto bad_exit ;

/* throw away the address part */

	for (i = 0 ; i < 6 ; i++) {

	c = getchar() ;
	if (c == EOF) goto bad_exit ;

	} ;

/* start transfer */

	len = len - 4 ;
	nsw = len >> 1 ;

	for (i = 0 ; i < nsw ; i++) {

	putshort(&state) ;

	} ; /* end for */

	goto	next_line ;



exit:
	if (state) {

	state = 3 ;
	putshort(&state) ;

	} ;

	return (OK) ;


bad_exit:
	return (BAD) ;

}
/* end of routine */


/* routine to read a short word from input stream and write to output */
putshort(sp)
int	*sp ;
{
	int	c ;

	switch (*sp) {

case 0:
	printf("	.word 0x") ;

	c = getchar() ;
	if (c == EOF) return (BAD) ;

	putchar(c) ;

	c = getchar() ;
	if (c == EOF) return (BAD) ;

	putchar(c) ;

	c = getchar() ;
	if (c == EOF) return (BAD) ;

	putchar(c) ;

	c = getchar() ;
	if (c == EOF) return (BAD) ;

	putchar(c) ;

	*sp = 1 ;

	break ;

case 1:
	c = getchar() ;
	if (c == EOF) return (BAD) ;

	putchar(c) ;

	c = getchar() ;
	if (c == EOF) return (BAD) ;

	putchar(c) ;

	c = getchar() ;
	if (c == EOF) return (BAD) ;

	putchar(c) ;

	c = getchar() ;
	if (c == EOF) return (BAD) ;

	putchar(c) ;

	putchar(EOL) ;

	*sp = 0 ;

	break ;

case 3:
	printf("0000\n") ;

	*sp = 0 ;
	break ;

default:
	printf("** bad switch **") ;

	} ; /* end switch */

	return (OK) ;
}
/* end of subroutine */


/* subroutine to convert a HEX string to a long word */
int cfhex(len,s,rp)
long	len ;
char	*s ;
unsigned long	*rp ;
{
	unsigned long	c, value, count ;

	if (len == 0) goto cfh21 ;

	value = 0 ;
	count = 0 ;

/* top of loop */
cfh10:
	len-- ;
	c = (((unsigned long) s[len]) & 0x007F) ;

	if (c < '0') goto cfh71 ;

	if (c <= '9') goto cfh11 ;

	c = c & 0x00DF ;		/* clear bit 5 for upper case */

	if (c > 'F') goto cfh71 ;

	if (c < 'A') goto cfh71 ;

	c = c - 7 ;

/* continue here if a good character */
cfh11:
	c = c & 0x000F ;
	c = c << (count * 4) ;
	value = value | c ;

	if (len == 0) goto cfh21 ;	/* jump if out */

	count++ ;
	if (count < 8) goto cfh10 ;

/* we are done looping */
cfh21:
	*rp = value ;
	return (OK) ;

cfh71:
	return (BAD) ;

}


