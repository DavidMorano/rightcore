/* cmpnames */


#define		DEBUG	0


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
*
*	FUNCTIONAL DESCRIPTION:						
*	"Cmpnames' compares a test string to a master name and returns	
*	true (1) if there is a match.	Acceptable matches are any	
*	combination of initials or initials and last name.		

	OK man.  Here it comes.  I have been fed up with the above
	initial match algorithm for years !!  Since I am now coding
	in this file, it goes !  Got it !  Finally, some more
	sensible rules about what constitutes a valid name match.

	The new rules (in summary form) will be :

		f.m.l.	matches if all or first and last match
		fml	matches the same as above AFTER it is checked
			to see if it is already a login name

	DAM 01/06/94


*									
*	PARAMETERS:							
*	ss	test string						
*	master	master name (in form 't.s.kennedy')			
*									
*	RETURNED VALUE:							
		0		no match found
		1		match found

*	SUBROUTINES CALLED:						
*

************************************************************************/



#include	<sys/types.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdio.h>

#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



#ifdef	COMMENT

int namecmp(tn,ns)
struct name	*ns ;
char		*tn ;
{


}

#endif


int cmpnames(ss,master)
char	ss[] ;
char	master[] ;
{
	char noinit, nominit;
	char *last, *mlast;
	char fi, si, mfi, msi, mli;


	noinit = nominit = '\0';

/* parse name */

	if ((mlast = strrchr( master, '.' )) == NULL) {

	    mlast = master;
	    nominit++;

	} else mlast++;

	if ((last = strrchr( ss, '.' )) == NULL) {

	    last = ss;
	    noinit++;

	} else last++;

	if (! (*(last + 1))) return 0 ;

	while (*last) if ((*last++ | 040) != (*mlast++ | 040)) {

	    if (! noinit) return 0 ;

	    goto initials ;
	}

	if (*mlast != '\0') return 0 ;

	if (noinit) return 1 ;

	if (nominit) return 0 ;

/* drats, now we have to diddle around with the initials ! */

initials:
	last = ss;
	if ( !noinit ) {

	    fi = *last++ | 040;
	    if (*last++ != '.') return 0 ;

	    si = *last++ | 040;
	    if (*last != '.') {

	        si = '\0';
	    }
	}
	mlast = master;
	mfi = *mlast++ | 040;
	if ( *mlast++ != '.' ) return(0);

	msi = *mlast++ | 040;
	if ( *mlast++ != '.' ) {

	    mli = msi;
	    msi = '\0';

	} else mli = *mlast | 040;

/**********************************************************************

 OK now we have all the initials figured out  	
 Now acceptable combinations are the following:	

  !noinit && fi == mfi && si == msi				
  !noinit && fi == msi && si == NULL				
  !noinit && fi == mfi && msi == NULL				
   noinit &&							
   ((( mfi == ss[0] ) && (mli == ss[1]) && !ss[2]) ||		
   (( mfi == ss[0]) && (msi == ss[1]) && (mli == ss[2]) && !ss[3])
   || ((msi == ss[0]) && (mli == ss[1]) && !ss[2]))		
							
 - o PHEEW!! o -					
							
 So testum and returnum results!			
 Also makeum test case insensitive			

**********************************************************************/

	if ( noinit ) {

/* this covers the case when the ss is initials */

	    if ( mfi == *last ) {

	        last++;
	        if (mli == *last) {

	            last++;
	            if (!(*last)) return 1 ;

/* check case of same middle and last init */

	            if ((msi == mli) && (mli == *last++) && 
	                !(*last)) return 1 ;

	            return 0 ;

	        } else if (msi == *last++)
	            if ((mli == *last++) && !(*last)) return 1 ;

	        return 0 ;
	    }

	    if ((msi == *last++) && (mli == *last++) && !(*last))
	        return 1 ;

	    return 0 ;
	}

	if ((fi == mfi) && ((si == msi) || !si)) return 1 ;

	if (!si && ( fi == msi) ) return 1 ;

	return 0 ;
}
/* end subroutine (cmpnames) */


