/* deliv_mbag */



#define	CF_DEBUG	0



/************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
 *		J.Mukerji						
 *									
*

 * deliv_mbag( mailbag ) delivers the contents of a mailbag using
 * any method that it chooses based on the type of the mailbag


***********************************************************************/



#include	<sys/types.h>
#include	<string.h>
#include	<stdio.h>

#include	<baops.h>
#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* external subroutines */


/* external variables */

extern struct global	g ;

extern struct sys_tbl	sname[] ;



deliv_mbag( mailbag )
struct sys_tbl	*mailbag ;
{
	struct table	*aname ;

	int		f_local ;

	char		*dest_host ;


	dest_host = mailbag->sysname ;
	f_local = (strcmp(dest_host,"LOCAL") == 0) ? TRUE : FALSE ;

	aname = mailbag->trans_link ;
	if (f_local) {

/*
		 Ironically, local delivery is the hardest!	
		 i.e.nodeinfo->standard == NO are the hardest. 
		 To do a correct job at notifying delivery	
		 failures it is imparative that each individual
		 be delivered the message separately. Otherwise
		 it is impossible to figure out which delivery
		 failed when delivery to one of a string of	
		 addressees fails. For now we will deliver	
		 local mail one at a time by invoking '/bin/mail'
		 Eventually, a simple mail delivery program 	
		 should be written to replace '/bin/mail'.
								
		 The technique used is as follows:		
			mailit() believes that it has been	
		 given a string of 'anames' linked through	
		 the syslink field. That is how a mailbag is	
		 handed to deliv_mbag() by deliverem(). So	
		 that bulk delivery is trivial. For doing	
		 individual delivery, the mailbag must be fed 
		 to mailit() one message at a time		
*/

/* deliver the messages one at a time	 */

	    struct table	*tname ;

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        logfile_printf(&g.eh,
			"deliv_mbag: local destination in 'deliv_mbag'\n") ;
#endif

	    while (aname != NULL) {

	        tname = aname->syslink ;
	        aname->syslink = NULL ;
	        mailit( tempfile, aname, dest_host ) ;

	        aname->syslink = tname ;
	        aname = tname ;
	    }

	} else {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        logfile_printf(&g.eh,"deliv_mbag: remote delivery 'deliv_mbag'\n") ;
#endif

	    mailit( tempfile, aname, dest_host ) ;

	}

}
/* end subroutine (deliv_mbag) */


