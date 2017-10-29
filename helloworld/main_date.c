/* main (date) */


/* revision history:

	= 2017-09-07, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<time.h>
#include	<stdio.h>


#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif



extern int	timestr_log(time_t,char *) ;


int main()
{
	time_t	daytime ;

	char	timebuf[TIMEBUFLEN + 1] ;


	daytime = time(NULL) ;

	timestr_log(daytime,timebuf) ;

	fprintf(stdout,"%s\n",timebuf) ;

	return 0 ;
}
/* end subroutine (main) */


