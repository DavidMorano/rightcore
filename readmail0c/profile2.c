/* profile */


#define	CF_DEBUG	0



#include	<sys/utsname.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/errno.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<signal.h>
#include	<string.h>
#include	<curses.h>
#include	<stdio.h>

#include	<logfile.h>
#include	<bfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"profile.h"



/* external subroutines */

extern char	*getenv() ;


/* external vaiables */

extern struct global	g ;



/* boolean function which determines whether the current user profile 
   has set the specified option (desires certain action).  
   returns 1 if set, 0 if not.
   references the global profile structure "userprof".
 */

int profile(opt)
char	opt[];
{
	int	i ;


	i=0;
	while ((int) strlen(userprof[i].name) > 0) {

	    if (strcmp(userprof[i].name,opt) == 0)
	        return (userprof[i].value) ;

	    i += 1 ;

	} /* end if */

/* if we did not have the option specified at all, consider it OFF */

	return 0 ;
}
/* end subroutine (profile) */



/* initializes the various profile options.
   any user-specified ones are contained in the shell variable RDMAILOPTS.
   these options then override the defaults in the global profile structure
    "userprof".  this structure is defined in "defs.c" and declared
    in "config.h" (which is included into every function file).
   set options (+) have value 1, not set (-) have value 0.
 */


int profinit()
{
	int i,val;

	char 	opts[MAXPATHLEN + 1],*opt;


/* check for user overrides */

	opt = getenv("RDMAILOPTS");

	if (( opt == NULL ) || ( *opt == NULL ))
	    strcpy( opts, DRDMAILOPTS );

	else
	    strcpy( opts, opt );

	opt = strtok (opts,":");

	if ( opt != NULL ) {

	    do {

#if	CF_DEBUG
		debugprintf("profinit: given \"%s\"\n",opt) ;
#endif

		val = 1 ;
	        if (*opt == '-') {

	            val = 0;     		/* turn off */
	        	opt += 1 ;

		} else if (*opt == '+')
			opt += 1 ;

#if	CF_DEBUG
		debugprintf("profinit: given \"%s\"\n",opt) ;
#endif

	        i=0;
	        while ((int) strlen(userprof[i].name) > 0) {

#if	CF_DEBUG
		debugprintf("profinit: checking for \"%s\"\n",userprof[i].name) ;
#endif

	            if (strcmp(opt,userprof[i].name) == 0) {

#if	CF_DEBUG
		debugprintf("profinit: got \"%s\"\n",opt) ;
#endif

	                userprof[i].value = val;
			break ;

	            }
	            i += 1 ;

	        } /* end while */

	        opt = strtok (0,":");

	    } while (opt != NULL) ;

	} /* end if */

	return OK ;
}
/* end subroutine (profinit) */



