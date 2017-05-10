/* getvars */


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
 *		T.S.Kennedy						
 *		J.Mukerji						
 *									

*									
*				getvars					
*									
*************************************************************************
*									
*	FUNCTIONAL DESCRIPTION:						
*	'getvars' gets the environment vairables used by SMAIL	
*									
*	PARAMETERS (accessed in the global area):			
*	namelist	translation tables (SMAILNAMES)			
*	maillist	mailing lists (SMAILLISTS)			
*	mailopts	options	(SMAILOPTS)				
*									
*	RETURNED VALUE:							
*									
*	SUBROUTINES CALLED:						
*									

************************************************************************/



#include	<sys/types.h>
#include	<stdio.h>

#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* external subroutines */

extern char	*getenv() ;



getvars()
{
	char	*env ;


	/* translation tables */
	env = getenv("SMAILNAMES");

	if (env != NULL && strlen(env) != 0)
		strcpy(namelist,env);
	else
		strcpy(namelist, DSMAILNAMES);

/* directories for mailing lists */

	env = getenv("SMAILLISTS") ;

	if (env != NULL && strlen(env) != 0)
		strcpy(maillist,env) ;

	else 
		strcpy(maillist, DSMAILLISTS) ;

	strcat(maillist,":/usr/postlists") ;

/* mailing options */

	env = getenv("SMAILOPTS") ;

	if (env != NULL && strlen(env) != 0)
		strcpy(mailopts,env) ;

	else 
		strcpy(mailopts, DSMAILOPTS) ;

}
/* end subroutine (getvars) */


