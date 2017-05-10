/* getfield */



#include	<sys/types.h>
#include	<sys/unistd.h>
#include	<ctype.h>
#include	<curses.h>
#include	<stdio.h>

#include	<bfile.h>

#include	"mailbox.h"

#include	"config.h"
#include	"defs.h"
#include	"localmisc.h"



/* external varaiables */

extern struct mailbox	mb ;


/* forward references */

static int hmatch() ;



/****************************************************************************

	get the value of the specified  within specified 
	return 0 if  field was found  (fvalue is the value)
 	return 1 if  number is too big or too small
	return 2 if that message is not found in the 



****************************************************************************/


int fetchfield(mn,f,fvalue,len)
int	mn ;
char	f[] ;
char	fvalue[] ;
int	len ;
{
	int	i, l, ml, flen = 0 ;

	char	field[LINELEN + 1], *fp = field ;


	fvalue[0] = '\0' ;

	if ((mn < 0) || (mn >= mb.total)) return 1 ;

/* assume getting from current mailbox which is already set up */

	fseek(curr.fp,messbeg[mn],0) ;

	while (ftell(curr.fp) < messend[mn]) {

	    l = fgetline(curr.fp,field,LINELEN) ;

	    if (field[0] == '\n') break ;

/* check to see if we got a truncated line */

	    if (field[l - 1] == '\n') field[--l] = '\0' ;

	    if (! hmatch(f,field)) continue ;

/* got a match, fast forward to colon and first non-white after that */

	    fp = field ;
	    ml = (l < 78) ? l : 78 ;
	    i = 0 ;
	    while ((i++ < ml) && (*fp != ':')) fp += 1 ;

/* skip this header if there was NO colon character */

	    if (i >= ml) continue ;

/* skip the colon character */

	    fp += 1 ;

/* skip over leading white space */

	    while (ISWHITE(*fp)) fp += 1 ;

	    l = strlen(fp) ;

	    ml = MIN(l,(FIELDLEN - flen)) ;

	    strncpy(fvalue,fp,ml) ;

	    flen += ml ;
	    fvalue[flen] = '\0' ;

/* OK, get more lines until a blank line or non-white 1st character */

	    while ((ftell(curr.fp) < messend[mn]) && (flen < FIELDLEN)) {

	        l = fgetline(curr.fp,field,LINELEN) ;

	        if ((l < 1) || (! ISWHITE(field[0]))) break ;

	        if (field[l - 1] == '\n') field[l - 1] = '\0' ;

	        fp = field ;
	        while (ISWHITE(*fp)) fp += 1 ;

	        if (*fp != '\0') {

	            fvalue[flen++] = ' ' ;

	            l = strlen(fp) ;

	            ml = MIN(l,(FIELDLEN - flen)) ;

	            strncpy(fvalue + flen,fp,ml) ;

	            flen += ml ;
	            fvalue[flen] = '\0' ;

	        }

	    } /* end inner while */

#ifdef	COMMENT
	    if (flen >= FIELDLEN) return BAD ;
#endif

	    return 0 ; /* found */

	} /* end outer while */

	return 2 ; /* not found */
}




/* 
	Is the initial substring of field the specified f?  
	return TRUE if so, else return FALSE .  
	The match is case independent.

*/

static int hmatch(f,field)
char	f[], field[] ;
{
	char	*fp = field, *hp = f ;


	while (*hp && (*hp != ':')) {

	    if (LOWER(*fp) != LOWER(*hp)) return 0 ;

	    fp += 1 ;
	    hp += 1 ;
	}

	while (ISWHITE(*fp)) fp += 1 ;

	if (*fp != ':') return FALSE ;

	return TRUE ;
}


