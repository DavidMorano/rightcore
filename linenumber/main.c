/* main (linenumber) */

/* front-end subroutine to number lines in a file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1998-06-01, David A­D­ Morano

	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ linenumber [<file(s)>]


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdio.h>

#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	fgetline(FILE *,char *,int) ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	FILE	*ifp = stdin ;
	FILE	*ofp = stdout ;

	int	rs ;
	int	len, line ;
	int	ex = EX_INFO ;

	char	linebuf[LINEBUFLEN + 1] ;


	line = 1 ;
	while ((rs = fgetline(ifp,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    linebuf[len] = '\0' ;

	    fprintf(ofp,"%5u ",line) ;

	    fprintf(ofp,"%s",linebuf) ;

	    line += 1 ;

	} /* end while */

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

	fclose(ifp) ;

	fclose(ofp) ;

	return ex ;
}
/* end subroutine (main) */



