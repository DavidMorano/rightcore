/* findname */


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
*	FUNCTIONAL DESCRIPTION:						
*	'Findname' returns the entry number for the next name which	
*	matches the realname.						
*									
*	PARAMETERS:							
*	realname	real name of user				
*	start		place to start looking in table			
*									
*	RETURNED VALUE:							
*	[n]	entry number in table (if match)			
*	-1	no match						
*									
*	SUBROUTINES CALLED:						
*									


*	Note: ":" added to strpbrk						


************************************************************************/



#include	<sys/types.h>
#include	<string.h>
#include	<stdio.h>

#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* external variables */

extern struct global	g ;




int findname(realname,start,type)
char	*realname ;
int	*start ;
int	type ;
{
	struct table	*aname ;

	int		i, j ;


/* if realname contains a ! then searching for it */
/* in the name database is quite futile, so return a -1  */

	if (strchr(++realname, '!') != NULL) return -1 ;

	realname-- ;

/* translate name if can */

	while (*start < tablelen) {

	    aname = name[*start] ;
#if	DEBUG
	if (strcmp(aname->realname,"w.m.pitio") == 0) {

	logfile_printf(&g.eh,"search name \"%s\" st=%d tt=%d\n",
		realname,type,aname->type) ;

	}
#endif

	    if ((type == aname->type) || (type == TT_ALL)) {

	        if (*realname == '!') {	/* check for login name */

	            if (strcmp(realname + 1,aname->mailaddress) == 0)
	                return *start ;

	        } else if (cmpnames(realname,aname->realname)) {

	            if (*(aname->mailaddress) == ALIAS) {

	                if (strpbrk(aname->mailaddress + 1,"!@:%/") != NULL) {

/* a mail address path has been found */
	                    return *start ;

	                }
	                i = 0 ;
	                j = findname(aname->mailaddress + 1,&i,TT_ALL) ;

	                if (j < 0) return -1 ;

	                *start = i ;
	                return *start ;
	            }

#ifdef	COMMENT
	            if (strchr(aname->mailaddress, NONMAIL ) != NULL)
	                return 0 ;
#endif

	            return *start ;

	        } /* end if */

	    } /* end if */

	    *start += 1 ;

	} /* end while */

	return -1 ;
}
/* end subroutine (findname) */


