/* fnmap */

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

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



#ifdef BBPOST

extern	int	bbcheck() ;
extern	int	bbsetup() ;
extern	int	bbpost() ;
#endif

#ifdef NETNEWS
extern	int	netcheck() ;
extern	int	netsetup() ;
extern	int	netpost() ;
#endif


struct fnentry	functab[] = {

#ifdef BBPOST
	"bb",	bbcheck,	bbsetup,	bbpost,
#endif

#ifdef NETNEWS
	"net",	netcheck,	netsetup,	netpost,
#endif

	"",	0,		0,		0,
} ;



