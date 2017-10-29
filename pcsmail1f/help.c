/* help */


#define	CF_DEBUG	0


/************************************************************************
 *									*
 * The information contained herein is for the use of AT&T Information	*
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	*
 *									*
 *	(c) 1984 AT&T Information Systems				*
 *									*
 * Authors of the contents of this file:				*
 *									*
 *		T.S.Kennedy						*
 *		J.Mukerji						*
		David A.D. Morano
 *									*

 ************************************************************************/



#include	<sys/types.h>
#include	<stdio.h>

#include	"config.h"

#include	"localmisc.h"
#include	"defs.h"



/* external variables */

extern struct global	g ;




void help(i)
int	i ;
{


	if (! g.f.interactive) return ;

	if (i == 0) {

	    putc('\n',stdout) ;

	    printf("%-12s%s\n","send","send mail - prompt for send options") ;
	    printf("%-12s%s\n","review","display message for review") ;
	    printf("%-12s%s\n","check","check recipient names") ;
	    printf("%-12s%s\n","edit","edit message") ;
	    printf("%-12s%s\n","quit","quit without sending message") ;

	} else if (i == 1) {

	    putc('\n',stdout) ;

	    printf("%-12s%s\n","standard","send mail without notification") ;
	    printf("%-12s%s\n","verify","verify that all mail was sent") ;
	    printf("%-12s%s\n","filecopy","place a copy of message in copy mailbox") ;
	    putc('\n',stdout) ;

	} else if (i == 2) {

	    printf("The names can be any of the following:\n\n") ;
	    printf("    %-20s%s\n","name","a person's real name (eg, t.s.kennedy, kennedy)") ;
	    printf("    %-20s%s\n","initials","a person's initials (eg, tsk)") ;
	    printf("    %-20s%s\n","alias","an alias (eg, admin)") ;
	    printf("    %-20s%s\n","~list","a mailing list (eg, ~wagner)") ;
	    printf("    %-20s%s\n","!login","a login name (eg, !tsk)") ;
	    printf("    %-20s%s\n","system!login",
	        "an explicit login name (eg, hocsb!tsk)") ;
	    printf("    %-20s%s\n","login@system",
	        "an explicit login name (eg. jis@mtgzx)") ;
	    printf("    %-20s%s\n","login@system.domain",
	        "an explicit login in a different domain (eg. jishnu@mit-mc.ARPA)") ;
	    printf("\nAll the names, except explicit login names,\n") ;
	    printf("are translated to the correct system and login names.\n\n") ;
	    printf("If the system specified in an explicit login name is not\n") ;
	    printf("directly reachable from your system, a suitable path\n") ;
	    printf("to the system is substituted for the system name in the\n") ;
	    printf("address before sending the message. If no path is known,\n") ;
	    printf("you will be warned.\n\n") ;

	} else if (i == 3) {

	    printf("The subject of the message should be entered.\n") ;
	    printf("The subject is printed with notification of incoming mail\n") ;
	    printf("and is used by other programs.\n\n") ;

	} else if (i == 4) {

	    printf("Any arbitrary text may be entered\n") ;
	    printf("and is terminated by a line containing a single period.\n") ;
	    printf("The text may be edited at a later point in the program.\n\n") ;

	} else if (i==6) {

/* help for disambiguating names */

	    printf(
		"type the line number to select the name on that line\n") ;
	    printf("enter a carriage-return to select none\n\n") ;

	}

}
/* end subroutine (help) */


