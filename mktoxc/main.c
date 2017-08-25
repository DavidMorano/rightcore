/* main (mktoxc) */

/* create translation arrays for character case conversion */


/* revision history:

	= 1998-06-29, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This program makes two arrays of characters, each 256 characters long.
        These arrays serve as translation tables for use in converting
        characters either to lower-case or to upper-case (each array serves one
        of these two functions).


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<baops.h>
#include	<localmisc.h>


/* local defines */

#define	CLASSLOWER(ch)	(BATST(case_lower,(ch)))
#define	CLASSUPPER(ch)	(BATST(case_upper,(ch)))


/* forward references */

static int mktoxc(int) ;
static int tolc(int) ;
static int touc(int) ;


/* local variables */

enum cases {
	case_tolower,
	case_toupper,
	case_overlast

} ;

static cchar	case_lower[] = {
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0xFE, 0xFF, 0xFF, 0x07,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0xFF, 0xFF, 0x7F, 0x7F
} ;

static cchar	case_upper[] = {
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0xFE, 0xFF, 0xFF, 0x07,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0xFF, 0xFF, 0x7F, 0x7F,
	    0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{


	mktoxc(case_tolower) ;

	mktoxc(case_toupper) ;

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int mktoxc(int w)
{
	int		i ;
	int		j = 0 ;
	int		rc ;
	const char	*name ;

	if ((w < 0) || (w >= case_overlast))
	    return SR_INVALID ;

	switch (w) {
	case case_tolower:
	    name = "tolc" ;
	    break ;
	case case_toupper:
	    name = "touc" ;
	    break ;
	} /* end switch */

	fprintf(stdout,"const unsigned char char_%s[] = %c\n",
	    name,CH_LBRACE) ;

	for (i = 0 ; i < 256 ; i += 1) {

	    switch (w) {
	    case case_tolower:
	        rc = tolc(i) ;
	        break ;
	    case case_toupper:
	        rc = touc(i) ;
	        break ;
	    } /* end switch */

	    if ((i & 7) == 0)
	        fprintf(stdout,"\t") ;

	    if ((i & 7) != 0)
	        fprintf(stdout," ") ;

	    fprintf(stdout,"0x%02x,",rc) ;

	    if ((i & 7) == 7)
	        fprintf(stdout,"\n") ;

	} /* end for */

	fprintf(stdout,"%c ;\n", CH_RBRACE) ;

	return 0 ;
}
/* end subroutine (mktoxc) */


static int tolc(int c)
{

	return (CLASSUPPER(c & 0xff)) ? (c | 0x20) : c ;
}


static int touc(int c)
{

	return (CLASSLOWER(c & 0xff)) ? (c & (~ 0x20)) : c ;
}


