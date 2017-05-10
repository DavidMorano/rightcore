/************************************************************************
 *                                                                      *
 * The information contained herein is for use of   AT&T Information    *
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   *
 *                                                                      *
 *     (c) 1984 AT&T Information Systems                                *
 *                                                                      *
 * Authors of the contents of this file:                                *
 *                                                                      *
 *                      Bruce Schatz                                    *
 *									*
 ***********************************************************************/
#include  "defs.h"


/* this evaluates the boolean expression contained in the input string
  "bexp" (contains characters 01&|()  ).
  a boolean expression is thus, for example,  1 & 0  .
  returns  1 (true), 0 (false), -1 (invalid expression).
  the evaluation is done by a table-driven stack machine.
*/


  /* actions */
#define  PUSH  1	/* push input onto stack */
#define  EVAL  2	/* evaluate boolexpr:  stack[sp-1] stack[sp] input */
#define  EVALSTAR 3	/* evaluate:  stack[sp-2] stack[sp-1] stack[sp] */
#define  BALANCE 4	/* balanced parentheses.  value replaces '(' .  */
#define  ERR 5		/* error. invalid expression */


  /* action table.  stacktop vs input.  */
 static int  actable [4][5]  =
 {/*           0,1    &,|     (        )        end  */
  /* 0,1 */   {ERR,   PUSH,  ERR,   BALANCE,  EVALSTAR},
  /* &,| */   {EVAL,  ERR,   PUSH,    ERR,      ERR   },
  /*  (  */   {PUSH,  ERR,   PUSH,    ERR,      ERR   },
  /*empty*/   {PUSH,  ERR,   PUSH,    ERR,      ERR   }
 };



  /* the stack itself and its pointer */
 static char  stack[LINELEN];
 static int   sp;


  /* stack operations */
#define  push(x)  (stack[++sp] = (x))
#define  pop()    (stack[sp--])
#define  stacktop (stack[sp])


  /* the input (expression token) */
#define  input    (bexp[k])





booleval (bexp)
 char bexp[];
{
	int k;
	char val;

	/* initialize */
	sp = 0;
	stacktop = NULL;

	/* get inputs from bexp and eval expression */
	for (k=0; k<=strlen(bexp); k++)		/* actable handles 'end'*/

		switch ( actable [transStack(stacktop)][transInput(input)] )
		{
		case PUSH:      push (input);    break;
		case EVAL:      eval (input);    break;
		case BALANCE:   val = pop();	 /*have '(x)' so replace '(' */
				stacktop = val;  /* by 'x' and eval if can */
		case EVALSTAR:  val = pop();     /*eval top 3 on  stack */
				eval (val);      break;
		case ERR:       printf("\n invalid logical expression, ");
				printf("please respecify.\n");
				return(-1);
		}


	/* compute value and return */
	if (sp == 1)
		if (stacktop == '1')
			return(1);
		else	return(0);

	/* incompletely formed expression */
	printf("\n invalid logical expression, please respecify.\n");
	return(-1);
}







 /* evaluate.  the operator is stacktop, the arguments are stacktop-1 and x.
   thus   stack[sp-1] is set to  stack[sp-1] stack[sp] x   and then 
   becomes the stacktop.
 */

#define  AND(a,b)  ((a) == '0' ? '0' : (b))
#define  OR(a,b)   ((a) == '1' ? '1' : (b))

eval (x)
 char x;
{
	char op;

	op = pop();
	switch (op)
	{
	case '&':  stacktop = AND(stacktop,x);   break;
	case '|':  stacktop = OR(stacktop,x);    break;
	default:   push(op); push(x);	/* failed so restore and push x */
	}
}








/* translate stack entries into internal numbers  (cf actable) */
transStack (ch)
 char ch;
{
	switch (ch)
	{
	case '0':
	case '1':  return(0);
	case '&':
	case '|':  return(1);
	case '(':  return(2);
	case NULL: return(3);	 	       /* empty string */
	default:   printf("\n *** invalid token in boolean expression ***\n");
		   return(3);
	}
}





/* translate input tokens into internal numbers  (cf actable) */
transInput (ch)
 char ch;
{
	switch (ch)
	{
	case '0':
	case '1':  return(0);
	case '&':
	case '|':  return(1);
	case '(':  return(2);
	case ')':  return(3);
	case NULL: return(4);     	 /* end of expression */
	default:   printf("\n *** invalid token in boolean expression ***\n");
		   return(4);
	}
}
