/* maintest */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>
#include	<stdio.h>


#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif




int main()
{
	int	i ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	buf[LINEBUFLEN + 1] ;


	strcpy(buf,"DATE (dst)") ;

/* top index line */

	for (i = 0 ; i < LINEBUFLEN ; i += 1) {
	    linebuf[i] = ((i % 10) == 0) ? ('0' + (i / 10)) : ' ' ;
	}

	linebuf[i] = '\0' ;
	fprintf(stdout,"%s\n",linebuf,i) ;

/* bottom index line */

	for (i = 0 ; i < LINEBUFLEN ; i += 1) {
	    linebuf[i] = ('0' + (i % 10)) ;
	}

	linebuf[i] = '\0' ;
	fprintf(stdout,"%s\n",linebuf,i) ;

/* content line */

	fprintf(stdout,"%-30s%-31s%-14s%-5s\n",
	        "   FROM","SUBJECT",buf,"LINES") ;

	fflush(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



