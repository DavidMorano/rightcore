/* getfield */


#define	DEBUG	0


/************************************************************************
 *                                                                      
 * The information contained herein is for use of   AT&T Information    
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   
 *                                                                      
 *     (c) 1984 AT&T Information Systems                                
 *                                                                      
 * Authors of the contents of this file:                                
 *                                                                      
 *                      Bruce Schatz, Jishnu Mukerji                    
 *									

 ***********************************************************************/



#include	<ctype.h>
#include	<string.h>

#include	"defs.h"



/* get the value of the specified header within specified message */
/* return 0 if header field was found  (fvalue is the value)
   return 1 if message number is too big or too small
   return 2 if that header is not found in the message		*/


int getfield(messnum,fheader,fvalue)
   int messnum;
   char fheader[];
   char fvalue[] ;
{
	int	i ;

	char	field[LINELEN + 1] ;
	char	*c ;
	char	*cp ;


	fvalue[0] = '\0' ;
	if ((messnum > nummess) || (messnum < 1)) return 1 ;

/* assume getting from current mailbox which is already set up */

	fseek(curr.fp,messbeg[messnum],0);

	while (ftell(curr.fp) < messend[messnum]) {

		fgets(field,LINELEN,curr.fp);

		if (*field == '\n') break;

#if	DEBUG
	debugprintf("hmatch looking for : %s\n",fheader) ;
#endif

	        if ((i = hmatch(fheader,field)) > 0) {

#if	DEBUG
	debugprintf("hmatch A\n",fheader) ;
#endif

			strcpy(fvalue,field + i) ;

#if	DEBUG
	debugprintf("hmatch B\n",fheader) ;
#endif

			if ((cp = strrchr(fvalue,'\n')) != NULL) *cp = '\0' ;

#if	DEBUG
	debugprintf("hmatch returns %d\n",i) ;
#endif

			return 0 ; 	/* found */
		}

#if	DEBUG
	debugprintf("hmatch returns %d\n",i) ;
#endif

	}
	return 2 ; /* not found */
}
/* end subroutine (getfield) */


#ifdef	COMMENT

/* Is the initial substring of field the specified fheader?  return 1 if so.
   First character must be in capitals but rest may be either case.
   Thus "SUBJECT:" matches "Subject:".
*/


int hmatch(fheader,field)
char	*fheader, *field;
{

#ifdef	COMMENT
	if (*fheader++ != *field++) return 0 ;
#endif

	for ( ; *fheader; )
		if ((*fheader++ | 040) != (*field++ | 040))
				return 0 ;

	return 1 ;
}
/* end subroutine (hmatch) */

#endif


