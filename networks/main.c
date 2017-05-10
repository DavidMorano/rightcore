/* main */

/* for the NETWORKS program */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This program prints out all of the service entries in the system
        'services' database.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<stdio.h>


int main()
{
	struct netent	*nep ;

	struct in_addr	a4 ;

	int	i ;

	char	strbuf[INET_ADDRSTRLEN + 1] ;

	const char	*cp ;
	const char	*ap ;


	setnetent(1) ;

	while ((nep = getnetent()) != NULL) {

	    if (nep->n_addrtype == AF_INET) {

	        a4 = inet_makeaddr(nep->n_net,0) ;

	        cp = inet_ntoa(a4) ;

	    } else {

	        ap = (char *) &nep->n_net ;
	        cp = inet_ntop(nep->n_addrtype,ap,
		    strbuf,INET6_ADDRSTRLEN) ;

	    }

	    if (cp == NULL)
		cp = "*invalid*" ;

	    fprintf(stdout,"%-16s %-15s",
	        nep->n_name,cp) ;

	    if (nep->n_aliases != NULL) {
	        for (i = 0 ; nep->n_aliases[i] != NULL ; i += 1) {
	            fprintf(stdout," %-16s",nep->n_aliases[i]) ;
	        } /* end for */
	    } /* end if (aliases) */

	    fprintf(stdout,"\n") ;

	} /* end while */

	endnetent() ;

	fflush(stdout) ;

	return 0 ;
}
/* end subroutine (main) */


