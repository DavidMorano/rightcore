/* main (mktobase) */

/* transate a character into its base value (0 -> 35) */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This program creates an array that is used to translate a character
	into its base value of between zero (0) and thirthy-five (35).


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<ctype.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<baops.h>
#include	<localmisc.h>


/* local defines */


/* forward references */

static int mktoxc(int) ;


/* local variables */

enum cases {
	case_tolower,
	case_toupper,
	case_overlast
} ;


/* exported subroutines */


int main()
{
	int		i ;
	int		j = 0 ;
	int		ch ;
	int		b ;

	fprintf(stdout,"const unsigned char char_tobase[] = %c\n",
	    CH_LBRACE) ;

	for (i = 0 ; i < 256 ; i += 1) {

	    ch = tolower(i & 0xff) ;

	    b = 0xff ;
	    if ((ch >= '0') && (ch <= '9')) {
		b = (ch - '0') ;
 	    } else if ((ch >= 'a') && (ch <= 'z')) {
		b = (ch - 'a' + 10) ;
	    }

	    if ((i & 7) == 0)
	        fprintf(stdout,"\t") ;

	    if ((i & 7) != 0)
	        fprintf(stdout," ") ;

	    fprintf(stdout,"0x%02x,",b) ;

	    if ((i & 7) == 7)
	        fprintf(stdout,"\n") ;

	} /* end for */

	fprintf(stdout,"%c ;\n",
	    CH_RBRACE) ;

	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */


