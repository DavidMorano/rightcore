/* getfield */


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
		David A.D. Morano

*									
*	FUNCTIONAL DESCRIPTION:						
*	The 'getfield' reads a file for a key descriptor and returns	
*	the value contained in it					
*									
*	PARAMETERS:							
*	file		filename					
*	key		key name (eg, 'FROM')				
*	value		value in key (must be of size BUFSIZE)
	buflen		length of value buffer above
*									
*	RETURNED VALUE:							
		BAD	no header found
		OK	a header was found
*									
*	SUBROUTINES CALLED:						
		+ mm_getfield

*									
************************************************************************/



#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<baops.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"




/* external subroutines */

extern int	mm_getfield() ;


/* external variables */




int getfield(file,key,value,buflen)
char	file[] ;
char	key[] ;
char	value[] ;
int	buflen ;
{
	struct ustat	stat_f ;

	bfile	mm_file, *fp = &mm_file ;

	int	len, dlen ;
	int	fd ;


#if	CF_DEBUG
	errprintf(
	    "getfield: want to get \"%s\" in file \"%s\"\n",
	    key,file) ;
#endif

	buflen = BUFSIZE ;
	value[0] = '\0' ;

/* open file for reading */

	if (bopen(fp,file,"r",0666) < 0) return BAD ;

#if	CF_DEBUG
	errprintf("getfield: OK 1\n") ;
#endif

	if (bcontrol(fp,BC_STAT,&stat_f) < 0) goto badret ;

#if	CF_DEBUG
	errprintf("getfield: OK 3\n") ;
#endif

	if (mm_getfield(fp,(offset_t) 0,stat_f.st_size,key,value,buflen) < 0)
	    goto badret ;

#if	CF_DEBUG
	errprintf("getfield: got stuff \n\"%s: %s\"\n",key,value) ;
#endif

	bclose(fp) ;

	return OK ;

badret:
	bclose(fp) ;

	return BAD ;
}
/* end subroutine (getfield) */


