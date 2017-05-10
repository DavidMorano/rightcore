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

/* for each messnum in the range expression, this calls
         func (arg1,messnum).
   ranges are of the form 5-10,3   where the delimeter is ',' (not blank). 
	 '$' indicates the last message.
   passes in external messnums since that is what func will expect.
   range returns 0 if can do matching, 1 if expression improperly formed.
   sets the global variables: 
     firstmatch -     1 before the first func call if was 0.
     lastmatch  -     messnum of last successfully matched message.
     lastundeleted -  messnum of last undeleted matched message.
*/


#define  NUMBER case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':


range (func,arg1,exp)
   int (*func)();
   char arg1[];
   char exp[100];        /* this is the range expression */
{
	char c[2];	   /* make length 2 so it can be used with strcat */
	char state='s';
	char number1[10], number2[10];
	int i=0;	   /* index for position in exp */
	int out=0;	   /* normal exit flag */
	int errflag=0;     /* error flag */
	int synerrflag=0;  /* syntax error flag */
	c[1] = NULL;

	strcat(exp,"\n");	/*make sure there is a carriage return */
	while(((c[0]=exp[i]) != NULL)  &&  (synerrflag==0)  &&  
	      (errflag==0)  &&  (out==0))      
        {   i++;
	 switch(state) 
	 {
	 case 's':	/* start state */
		 switch(c[0]) 
		 {
		 NUMBER number1[0] = NULL;     /*initialize */
     		        strcat(number1,c);/*create digitString, convert later*/
		        state='m';	  /* found a number */
		        break;
		 case '\n':  out=1;
			     break;
		 case '$':   sprintf(number1,"%d",nummess);
			     switch (exp[i])   { NUMBER  synerrflag=1; }
			     state='m';
			     break;
		 default:    synerrflag=1;
			     break;
		 }	
		 break;
	 case 'm':   	/* encountered first number */
		 switch(c[0])
		 {
		 NUMBER strcat(number1,c);
		        break;
		 case '\n': errflag = act_on(func,arg1,atoi(number1),-1);
			    out=1;	
			    break;
		 case ',':  errflag = act_on(func,arg1,atoi(number1),-1);
		  	    state='s';		
			    break;
		 case '-':  state='n';
			    break;
		 default:   synerrflag=1;	
			    break;
		 }
		 break;
	 case 'n':   /* encountered number after the dash */
		 switch(c[0])
		 {
		 NUMBER	number2[0] = NULL;
			strcat(number2,c);
			state='o';
			break;
		 case '$':  errflag = act_on(func,arg1,atoi(number1),nummess);
			    if      (exp[i] == ',' )   i++;
			    else if (exp[i] == '\n')   out=1;
			    else                       synerrflag=1;   
			    state='s';
			    break;
		 default:   synerrflag=1;
		 }
		 break;
	 case 'o':   /* encountered more than one digit after dash */
		 switch(c[0]) 
		 {
		 NUMBER strcat(number2,c);
		        break;
		 case ',':  errflag = act_on(func,arg1,atoi(number1),
						      atoi(number2));
			    state='s';
			    break;
		 case '\n': errflag = act_on(func,arg1,atoi(number1),
						      atoi(number2));
			    out=1;
			    break;
		 default:   synerrflag=1;
			    break;
		 }
		 break;
	 }   /*end the big state switch*/
	}   /*end the while (next piece of exp)*/

	if (synerrflag == 1) 
		printf("\n invalid range expression, please respecify.\n");

}






/* invoke the function in the computed range. 
  Returns 0 if successful, 1 if error.
*/

#define  min(a,b)  (((a) <= (b)) ? (a) : (b))


act_on(func,arg1,number1,number2)
   int (*func)();
   char arg1[];
   int number1;
   int number2;
{
   	int j;
	 /* single number */
	if (number2 < 0)
		if (messinrange (number1))
		{
			if (firstmatch == 0)   firstmatch = 1;
			lastmatch = number1;
			if (messdel[messord[number1]] == 0)
				lastundeleted = number1;
			(*func) (arg1,number1);	
		}
		else	return(1);

	 /* full range */
	else	
		if (number1 > number2) 
		{	
			printf("\n invalid range, please respecify.\n");
			return(1);
		}
		else	
			if (messinrange(number1))
			{
				for (j=number1; j<=min(number2,nummess); j++)
				{
					if (firstmatch == 0)  firstmatch = 1;
					if (messdel[messord[j]] == 0)
						lastundeleted = j;
					(*func) (arg1,j);
				}
				lastmatch = j - 1;
			}
			else	return(1);


	return(0);
}



	




/*  boolean function.  returns 1 if message is in valid boundaries, else 0.
   assumes is passed external messnum.
*/

messinrange (messnum)
   int messnum;
{
	if (messnum < 1  ||  messnum > nummess)
	{
		printf("\n message %d out of range, please respecify.\n",
			messnum);
		return(0);
	}
	else   return(1);
}
