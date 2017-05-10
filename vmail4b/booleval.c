/* booleval */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        this evaluates the boolean expression contained in the input string
        "bexp" (contains characters 01&|() ). a boolean expression is thus, for
        example, 1 & 0 . returns 1 (true), 0 (false), -1 (invalid expression).
        the evaluation is done by a table-driven stack machine.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<string.h>

#include	<bfile.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"ds.h"


/* local defines */

/* stack operations */

#define  push(x)  (stack[++sp] = (x))
#define  pop()    (stack[sp--])
#define  stacktop (stack[sp])

/* the input (expression token) */

#define      input	(bexp[k])

/* actions */

#define  BE_PUSH  	1 /* push input onto stack */
#define  BE_EVAL  	2 /* evaluate boolexpr:  stack[sp-1] stack[sp] input */
#define  BE_EVALSTAR 	3 /* evaluate:  stack[sp-2] stack[sp-1] stack[sp] */
#define  BE_BALANCE 	4 /* balanced parentheses, value replaces <ch_lparen> */
#define  BE_ERR 	5 /* error. invalid expression */


/* external subroutines */


/* forward references */

static int	transinput(int), transstack(int) ;

static void	eval() ;


/* action table - stacktop vs input */

static int  actable [4][5]  = {
	{BE_ERR,   BE_PUSH,  BE_ERR,   BE_BALANCE,  BE_EVALSTAR},
	{BE_EVAL,  BE_ERR,   BE_PUSH,    BE_ERR,      BE_ERR   },
	{BE_PUSH,  BE_ERR,   BE_PUSH,    BE_ERR,      BE_ERR   },
	{BE_PUSH,  BE_ERR,   BE_PUSH,    BE_ERR,      BE_ERR   },
} ;


/* the stack itself and its pointer */

static char  stack[LINEBUFLEN] ;
static int   sp ;


/* exported subroutines */


int booleval(bexp)
char	bexp[] ;
{
	int k ;

	char val ;


/* initialize */
	sp = 0 ;
	stacktop = '\0' ;

/* get inputs from bexp and eval expression */

	for (k = 0 ; k <= (int) strlen(bexp) ; k += 1) {

		int	val ;


/* actable handles 'end' */

	    val = actable[transstack(stacktop)][transinput(input)] ;

	    switch (val) {

	    case BE_PUSH:
	        push(input) ;

	        break ;

	    case BE_EVAL:
	        eval(input) ;

	        break ;

	    case BE_BALANCE:
	        val = pop();	 /* have '(x)' so replace <ch_lparen> */

	        stacktop = val;  /* by 'x' and eval if can */

	    case BE_EVALSTAR:
	        val = pop();     /*eval top 3 on  stack */

	        eval(val) ;

	        break ;

	    case BE_ERR:
	        return -1 ;

	    } /* end switch */

	} /* end for */

/* compute value and return */
	if (sp == 1) {

	    if (stacktop == '1')
	        return 1 ;

	    else
	        return 0 ;

	}

/* incompletely formed expression */

	return -1 ;
}
/* end subroutine */


/* evaluate.  the operator is stacktop, the arguments are stacktop-1 and x.
   thus   stack[sp-1] is set to  stack[sp-1] stack[sp] x   and then 
   becomes the stacktop.
 */

#define  AND(a,b)  ((a) == '0' ? '0' : (b))
#define  OR(a,b)   ((a) == '1' ? '1' : (b))

static void eval(x)
char	x ;
{
	char	op ;


	op = pop() ;

	switch (op) {

	case '&':
	    stacktop = AND(stacktop,x) ;
	    break ;

	case '|':
	    stacktop = OR(stacktop,x) ;
	    break ;

	default:
	    push(op) ;

	    push(x);	/* failed so restore and push x */
	}
}
/* end subroutine (eval) */


/* translate stack entries into internal numbers (cf actable) */
static int transstack(int ch)
{
	int		rc = 4 ;

	switch (ch) {
	case '0':
	case '1':
	    rc = 0 ;
	    break ;
	case '&':
	case '|':
	    rc = 1 ;
	    break ;
	case CH_LPAREN:
	    rc = 2 ;
	    break ;
	default:
	    rc = 3 ;
	    break ;
	} /* end switch */

	return rc ;
}
/* end subroutine (transstack) */


/* translate input tokens into internal numbers (cf actable) */
static int transinput(int ch)
{
	int		rc = 4 ;

	switch (ch) {
	case '0':
	case '1':
	    rc = 0 ;
	    break ;
	case '&':
	case '|':
	    rc = 1 ;
	    break ;
	case CH_LPAREN:
	    rc = 2 ;
	    break ;
	case CH_RPAREN:
	    rc = 3 ;
	    break ;
	default:
	    rc = 4 ;
	    break ;
	} /* end switch */

	return rc  ;
}
/* end subroutine (transinput) */



