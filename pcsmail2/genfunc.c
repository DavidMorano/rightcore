/* genfunc */


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

 ************************************************************************/



#include	<stdio.h>
#include	<string.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"




extern struct fnentry	functab[];




int genfunc( fninfo, state )
struct	table	*fninfo;
int	state;
{
    struct	fnentry	*current;
    char	*cur_prefix;

    if ((state < 0) || (state > 2)) {

	printf("genfunc: invalid state %d\n", state);

	return 0 ;
    }

    current = &functab[0] ;
    cur_prefix = current->prefix ;
    while (*cur_prefix != '\0') {

	if (strncmp(fninfo->realname,cur_prefix,strlen(cur_prefix)) != 0) {

		current++;
		cur_prefix = current->prefix;
		continue;
	}

	return ((*current->funct[state])(fninfo));
    }

    printf("%s: unknown mailbox %s\n", g.progname,fninfo->realname);

    return 0 ;
}
/* end subroutine (genfunc) */


