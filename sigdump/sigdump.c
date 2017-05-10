/* sigdump */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<netdb.h>



extern int	bufprintf(char *,int,const char *,...) ;



int sigdump(pid_t pid,int s)
{
	int		rs ;
	char		cmd[MAXHOSTNAMELEN + 1] ;

	bufprintf(cmd,MAXHOSTNAMELEN,"psig %d > %s.sig",pid,s) ;

	rs = system(cmd) ;

	return rs ;
}
/* end subroutine (sigdump) */


