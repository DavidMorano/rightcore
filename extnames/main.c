/* main (extnames) */

/* ?? */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2015-09-16, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2015 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is actually a very old program, and in truth, I have no idea what
	this ever did!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif


/* external subroutines */

extern int	isdigitlatin(int) ;


/* exported subroutines */


int main()
{
	int		rs = SR_OK ;
	int		len ;
	int		c ;
	int		i ;
	char		namebuf[REALNAMELEN + 1] ;

	i = 0 ;
	while ((c = fgetc(stdin)) != EOF) {

#if	CF_DEBUGS
		fprintf(stderr,"main: i=%u c=%c\n",i,c) ;
#endif

	    if ((i == 0) && (c == 'd')) {

	        namebuf[i++] = c ;

	    } else if (i > 0) {
		int	ch ;

	        switch (c) {

	        case 's':
	            if (namebuf[i - 1] == 'd') {
	                namebuf[i++] = c ;
	                } else {
	                i = 0 ;
		    }
	            break ;

	        case 'c':
	            if (namebuf[i - 1] == 's') {
	                namebuf[i++] = c ;
		    } else {
	                i = 0 ;
		    }
	            break ;

	        default:
	            if (isdigitlatin(c) && (i >= 3) && (i < (3+5))) {
	                namebuf[i++] = c ;
	            } else {
	                i = 0 ;
		    }
	            break ;

	        case '.':
	            ch = namebuf[i - 1] ;
	            if (isdigitlatin(ch)) {
	                namebuf[i++] = c ;
	            } else {
	                i = 0 ;
		    }
		    break ;

	        case 'j':
	            if (namebuf[i - 1] == '.') {
	                namebuf[i++] = c ;
		    } else {
	                i = 0 ;
		    }
	            break ;

	        case 'p':
	            if (namebuf[i - 1] == 'j') {
	                namebuf[i++] = c ;
		    } else {
	                i = 0 ;
		    }
	            break ;

	        case 'g':
	            if (namebuf[i - 1] == 'p') {
	                namebuf[i++] = c ;
	                namebuf[i] = '\0' ;
	                fprintf(stdout,"%s\n",namebuf) ;
	            }
	                i = 0 ;
	            break ;

	        } /* end switch */

#if	CF_DEBUGS
		if (i > 0) {
			namebuf[i] = '\0' ;
	                fprintf(stderr,"namebuf=%s\n",namebuf) ;
		}
#endif

	    } /* end if */

	} /* end while */

	return 0 ;
}
/* end subroutine (main) */


