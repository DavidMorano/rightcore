/* search */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

 This function executes 'function(mn)' on each mn in the current
  mailbox that matches.  A message matches if the logical expression
  set up by "leparse" evaluates to true.  Each term in the expression
  consists of HEADER:value  where value matches if it is equal to any
  word within the corresponding  .

  Thus  "SUBJECT:rdmail" as an expression matches 
   "SUBJECT: program rdmail now available" .

  The expression is contained in the global variables:
   isop, etoken, hvalpt     which are set up by  leparse.

  Looping is done on external messages since that is what func() expects.
   However, evaluation and matching must be done on internal.
  Returns 0 if can do matching, 1 if expression improperly formed.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<string.h>
#include	<curses.h>
#include	<stdio.h>

#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"mb.h"


/* local defines */


/* forward references */

static int	wmatch() ;

static void	tokeval(struct proginfo *,int,char *) ;







int search(pip,dsp,mbp,func)
struct proginfo	*pip ;
DS		*dsp ;
struct mailbox	*mbp ;
int		(*func)() ;
{
	int	mn ;

	char	boolexp[LINEBUFLEN] ;


/* initial processing */

	tokeval(pip,messord[1],boolexp) ;

	switch (booleval(dsp,boolexp)) {

	case 1:   
	    (*func)(1) ;

	case 0:   
	    break ;

	case -1:  
	    return 1 ;		/* invalid expression */

	} /* end switch */

/* find rest of matches and call func */

	for (mn = 2 ; mn <= mbp->total ; mn += 1) {

	    tokeval(pip,messord[mn],boolexp) ;

	    if (booleval(dsp,boolexp)) 
		(*func)(mn) ;

	} /* end for */

	return 0 ;
}
/* end subroutine (search) */



/* LOCAL SUBROUTINES */



/***************************************************************************

 generates boolean expression of logical expression for this mn.
  ie evaluates matching of the specified field values into 0s and 1s.
  logical expression is in global variables.  
  boolexp returned as character string in boolexp.
  mn is internal mn.


****************************************************************************/

static void tokeval(pip,mn,boolexp)
struct proginfo	*pip ;
int		mn ;
char		boolexp[] ;
{
	int	b, letok ;

	char	mhval[LINEBUFLEN] ;


	b=0 ;

/* just copy through operators */

	for (letok=0; letok < numletok; letok += 1) {

	    if (isop[letok] == 1) {

	        boolexp[b++] = operator[etoken[letok]] ;
	        continue ;		/*next token*/
	    }

/* have HEADER:value token so get value */

	    if (fetchfield(mn,header[etoken[letok]], mhval,LINEBUFLEN) != 0)

/* try special check for non-sendmail From */

	        if (etoken[letok] == 0)
	            fetchfrom(pip,mn,mhval,LINEBUFLEN) ;

	        else {

/* message missing so eval to 0 */

	            boolexp[b++] = '0' ;
	            continue ;	/*next token*/
	        }

/* does message value match logexpr  value? */

	    if (wmatch (hvalpt[letok],mhval))
	        boolexp[b++] = '1' ;

	    else
		boolexp[b++] = '0' ;

	} /* next token */

	boolexp[b] = '\0' ;

}
/* end subroutine (tokeval) */


/* returns true iff word occurs within string */
static int wmatch(word,string)
char	word[], string[] ;
{
	char text[LINEBUFLEN], *tp ;


	strcpy(text,string) ;

/* note -' not delimeters */

	tp = strtok (text," .,:;\"()/?!~*_&[]@");  

	while (tp != NULL)

	    if (strcasecmp(word,tp))
	        return(1) ;

	    else	
		tp = strtok (0," .,:;\"()/?!~*_&[]@") ;

/* special cases exclude delimeters:    .!@ (FROM),  /: (DATE) */

	strcpy (text,string) ;
	tp = strtok (text," ,;\"()?~*_&[]");  /*note  -'  not delimeters*/
	while (tp != NULL)
	    if (strcasecmp(word,tp))
	        return(1) ;
	    else	tp = strtok (0," ,;\"()?~*_&[]") ;

/* no match */

	return 0 ;
}
/* end subroutine (wmatch) */



