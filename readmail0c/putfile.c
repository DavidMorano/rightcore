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
#include "defs.h"

/* takes specified message and copies it into specified file */


putfile (filename,messnum)
   char filename[];
   int messnum; 
{
   	char temp[LINELEN];
   	FILE *fout;

	messnum =  messord [messnum];     /* convert to internal number */

	fseek(curr.fp,messbeg[messnum],0);

	fout =  fopen (filename,"a+");     /* append to file */
	if (fout == NULL)
	{
		printf("\n invalid file, please respecify.\n");
		return(1);
	}

	  /* copy out the message */
	while(ftell(curr.fp)< messend[messnum])
	{
		fgets(temp,LINELEN,curr.fp);
		fprintf(fout,"%s",temp);
	}

	fprintf(fout,"\n");
	fclose (fout);
}
