/************************************************************************
 *                                                                      *
 * The information contained herein is for use of   AT&T Information    *
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   *
 *                                                                      *
 *     (c) 1984 AT&T Information Systems                                *
 *                                                                      *
 * Authors of the contents of this file:                                *
 *                                                                      *
 *                      Bruce Schatz, Jishnu Mukerji                    *
 *									*
 ***********************************************************************/

#include  "defs.h"

/* this executes  function(arg1,messnum)  on each messnum in the current
  mailbox that matches.  A message matches if the logical expression
  set up by "leparse" evaluates to true.  Each term in the expression
  consists of HEADER:value  where value matches if it is equal to any
  word within the corresponding message header.
  Thus  "SUBJECT:rdmail" as an expression matches 
   "SUBJECT: program rdmail now available" .
  The expression is contained in the global variables:
   isop, etoken, hvalpt     which are set up by  leparse.
  Looping is done on external messnums since that is what func expects.
   However, evaluation & matching must be done on internal.
  Returns 0 if can do matching, 1 if expression improperly formed.
  Sets the global variables: 
     firstmatch -     1 before the first fun call if was 0.
     lastmatch  -     messnum of last successfully matched message.
     lastundeleted -  messnum of last undeleted matched message.
*/


search (func,arg1)
 int (*func)();
 char arg1[];
{
	int messnum;
	char boolexp[LINELEN];

	/* first message processing */
	tokeval (messord[1],boolexp);
	switch (booleval (boolexp))
	{
	case 1:   if (firstmatch == 0)   firstmatch = 1;
		  lastmatch = 1;
		  if (messdel[messord[1]] == 0)   lastundeleted = 1;
		  (*func) (arg1,1);
	case 0:   break;
	case -1:  return (1);		/* invalid expression */
	}


	/* find rest of matches and call func */
	for (messnum=2; messnum <= nummess; messnum++)
	{
		tokeval (messord[messnum],boolexp);
		if (booleval (boolexp))
		{      
			if (firstmatch == 0)   firstmatch = 1;
			lastmatch = messnum;
			if (messdel[messord[messnum]] == 0)   
				lastundeleted = messnum;
		 	(*func) (arg1,messnum);
		}
	}

	
	return(0);
}








/* generates boolean expression of logical expression for this messnum.
  ie evaluates matching of the specified field values into 0s and 1s.
  logical expression is in global variables.  
  boolexp returned as character string in boolexp.
  messnum is internal messnum.
*/


tokeval (messnum,boolexp)
  int messnum;
  char boolexp[];
{
	int b, letok;
	char mhval[LINELEN];

	b=0;
	for (letok=0; letok < numletok; letok++)
	{	/* just copy through operators */
		if (isop[letok] == 1)
		{
			boolexp[b++] =  operator[etoken[letok]];
			continue;		/*next token*/
		}

		/* have HEADER:value token so get value */
		if (getfield (messnum, header[etoken[letok]], mhval)   !=  0)
			/* try special check for non-sendmail From */
			if (etoken[letok] == 0)
				fetchfrom (messnum,mhval,LINELEN);
			else 	
			{	/* header missing so eval to 0 */
				boolexp[b++] = '0';
				continue;	/*next token*/
			}
		/* does message value match logexpr header value? */
		if (wmatch (hvalpt[letok],mhval))
			boolexp[b++] = '1';
		else	boolexp[b++] = '0';
	}   /*next token*/
	boolexp[b] = NULL;
}
		





/* returns true iff word occurs within string */

			
wmatch (word,string)
 char word[],string[];
{
	char text[LINELEN], *tp;

	strcpy (text,string);
	tp = strtok (text," .,:;\"()/?!~*_&[]@");  /*note  -'  not delimeters*/
	while (tp != NULL)
		if (casecmp(word,tp))
			return(1);
	        else	tp = strtok (0," .,:;\"()/?!~*_&[]@"); 

	/* special cases 
	  exclude delimeters:    .!@ (FROM),  /: (DATE)
	*/
	strcpy (text,string);
	tp = strtok (text," ,;\"()?~*_&[]");  /*note  -'  not delimeters*/
	while (tp != NULL)
		if (casecmp(word,tp))
			return(1);
		else	tp = strtok (0," ,;\"()?~*_&[]"); 
	
	/* no match */
	return (0);
}





 /* returns true iff the two strings exactly match with case folding */

#include <ctype.h>


casecmp (str1,str2)
  char str1[],str2[];
{
	int k=0;
	while (str1[k] != NULL)
	{
		if (tolower(str1[k]) != tolower(str2[k]))
			return(0);
		k++;
	}
	if (str2[k] == NULL)
		return(1);
	else    return(0);
}
