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

/* parses the specified logical expression, placing the tokens into
  global variables for use by "search" as follows:
  "isop" denotes whether a token is an operator or a HEADER:value.
  Each token is identified by a number in "etoken" which gives
  the operator number (as referenced into "operator") or 
  the header number (as referenced into "header").
  Headers have an associated value which is pointed to by "hvalpt"
  (the actual string space is in "hvalues").
  "numletok" is the number of tokens (max bound of these arrays).

  Returns 0 if successful, 1 if error.
*/


leparse (exp)
 char exp[];
{
	char *valpt, *ep, headstr[50];
	int k;

	valpt = hvalues;	/* start of next value in string space */
	numletok = 0;
	ep = exp;		/* current parsing point */ 

	while (*ep != NULL)
	{
		if ((k = findoperator(*ep))  >  -1)
		{	/* found operator */
			isop [numletok]  =  1;
			etoken [numletok]  =  k;
			numletok++;
			ep++;
		}

		if (findoperator(*ep) > -1)
			continue; 	/* two operators in row, eg &( */
		if (*ep == NULL)
			break;		/* end with operator, eg ) */
		
		/* header build */
		k=0;
		while (*ep != ':')
		{
			if ((findoperator(*ep) > -1)  ||  (*ep == NULL))
			{
				printf("\n invalid header in expression, ");
				printf("please respecify. \n");
				return(1);
			}
			headstr[k++] =  *ep++;
		}
		headstr[k++] = ':';
		headstr[k] = NULL;

		isop [numletok]  =  0;
		etoken [numletok]  =  findheader (headstr);
		if (etoken[numletok] == -1)
		{
			printf("\n invalid header in expression, ");
			printf("please respecify. \n");
			return(1);
		}
		ep++;

		if ((findoperator(*ep) > -1)  ||  (*ep == NULL))
		{
			printf("\n empty header value in expression, ");
			printf("please respecify. \n");
			return(1);
		}

		/* value build */
		hvalpt [numletok]  =  valpt;
		while ((findoperator(*ep) == -1)  &&  (*ep != NULL))
			*valpt++  =  *ep++ ;
		*valpt++  =  NULL;	/* complete & pt to next */
		numletok++;
	}
	return(0);
}







/* finds operator number of character.   returns -1 if not an operator */

findoperator (ch)
 char ch;
{
	int i;
	for (i=0; i<strlen(operator); i++)
		if (ch == operator[i]) 
			return(i);
	return(-1);
}





/* finds header number of string.  returns -1 if not a header.
  match allows case folding (eg  from: matches FROM:).
  match done on initial substring of str (so "FROM:schatz" matches "FROM:").
*/

findheader (str)
 char str[];
{
	int k;
	char headonly[LINELEN];

	for (k=0;  str[k] != ':';  k++)
	{	/* extract leading substring */
		if (str[k] == NULL)    return(-1);      /* no ':' */
		headonly[k] = str[k];
	}
	headonly[k] = ':';
	headonly[++k] = NULL;
	k=0;
	while (strlen(header[k])  > 0)
	{	/* find header */
		if (casecmp (header[k],headonly))
			return (k);
		k++;
	}
	return (-1);
}
