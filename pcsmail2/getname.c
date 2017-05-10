/* getnames */


#define	CF_DEBUG	0
#define	CF_GECOS	1


/************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
 *		T.S.Kennedy						
 *		J.Mukerji						
 *									
*									
*									
*	FUNCTIONAL DESCRIPTION:						
*	'conname' (convert name) converts a mail address to a real name.	
*	Ie, 'hocsb!tsk' is converted to 't.s.kennedy'.			
*									
*	PARAMETERS:							
*	username	user name (in form 'hocsb!tsk')			
*	realname	real name (in form 't.s.kennedy')		
*									
*	RETURNED VALUE:							
		OK	means succeeded
		BAD	failed
*									
*	SUBROUTINES CALLED:						
*									
************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdio.h>

#include	<logfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"



/* system library externals */


/* local externals */

extern char	*namefromgecos();



int getname(username,realname) 
char *username,*realname;
{
	char	*cp ;


#if	CF_GECOS
    	cp = namefromgecos(username) ;

	if (cp == ((char *) 0)) return -1 ;

	strcpy(realname,cp) ;

	return 0 ;
#else
	extern struct table *name[NAMEDB];
	struct table **this, **last;

	extern int tablelen;

	int	nfound;
	int	i, j;

	char	foundname[LENNAME];


	nfound = 0;
	i = 0;
	this = &name[0];
	last = &name[tablelen-1];

	for (; this <= last; this++ ,i++) {

		if(strcmp(username, (*this)->mailaddress) == 0) {

			strcpy(foundname, (*this)->realname);
			j = i;
			nfound++;
		}
	}

/* This is to make sure that no one is trying to masquarade as someone
 * else in the from line by adding an entry in ones private name list
 * of the form:
 *	
 *	j.doe		mylogin
 *
 * If the number of matches were found to be > 1 that means the user may
 * have tried this trick. In this case the login of the user is returned
 * just as if the user's name was not found.
*/
	if (nfound == 1) {

		strcpy( realname, foundname ) ;

		return j ;
	}
	strcpy(realname,username);

	return -1 ;
#endif

}
/* end subroutine (getname) */



