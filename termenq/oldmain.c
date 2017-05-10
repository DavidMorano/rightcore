/* main */

/* enquire to the terminal for the answer-back message */


#define	CF_DEBUG	0
#define	CF_FAKE		0


/* revision history ;

	= 1998-10-16, David A­D­ Morano
	I wrote this for setting terminal locations in UTMP records.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little program queries the terminal for the answer-back message.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<libff.h>
#include	<uterm.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN	1024
#endif

#define	TO_READ		3


/* external subroutines */


/* exported subroutines */


int main()
{
	int	rs ;
	int	bl, plen ;
	int	fd ;
	int	fm = fm_timed | fm_noecho ;

	char	pbuf[10], buf[BUFLEN + 1] ;


	fd = FD_STDOUT ;

	if (! isatty(fd))
		return EX_NOINPUT ;

	uterm_start(fd) ;

	pbuf[0] = CH_ENQ ;
#if	CF_FAKE
	plen = 0 ;
#else
	plen = 1 ;
#endif

	rs = uterm_reade(fd,fm,
		(long) buf,BUFLEN,TO_READ,0L,(long) pbuf,(long) plen) ;

	uterm_restore(fd) ;

	if (rs > 0) {
		bl = rs ;
		while (bl && isspace(buf[bl - 1])) bl -= 1 ;
		fprintf(stdout,"%s\n",buf,bl) ;
	}

	return 0 ;
}
/* end subroutine (main) */


