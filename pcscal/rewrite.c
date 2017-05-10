#include <stdio.h>
#include "cal.h"

/* This function creates a temporary file of entries changed.  This
   function also eliminates deleted entries.
   The revised temporary calendar is then copied to the user's specified
   calendar
*/
#define BUFSIZE 256

rewrite()
{
	int i;
	int flag;
	int num, onum;
	FILE *fptr,*ftmpc;
	extern char tmpc[], tmpeall[];
	extern char oldcal[];
	extern int outs, didedit;
	extern int elim[MAXEDIT];
	char s[BUFSIZE];

	if ((ftmpc = fopen(tmpc, "w")) == NULL)
	{
		fprintf (stderr,"error: cannot open \"%s\"\n",tmpc);
		return(1);
	}


	if ((fptr = fopen (oldcal, "r")) == NULL)
		{
		fprintf (stderr, "error: cannot open \"%s\"\n",oldcal);
		return(1);
		}

	num = 0;
	while ((fgets (s,BUFSIZE,fptr)) != NULL)
	{
		flag = 0;
		onum = num;
		num = ftell (fptr);
		for ( i=0; i<outs; i++)
		{
			if (onum == elim[i]) flag = 1;
		}

		/*
		put the string into the revised calendar IF it is not an
		edited one
		*/
		if (!flag) fputs (s,ftmpc);

	}

		/* add the newly revised entries to the personal calendar */
		fclose (fptr);
		fclose (ftmpc);
			strcpy (s, "cat ");
			strcat (s, tmpeall);
			strcat (s, ">> ");
			strcat (s, tmpc);
			system (s);

		/* copy the tempfile to calendar file */
		strcpy (s, "cp ");
		strcat (s, tmpc);
		strcat (s, " ");
		strcat (s, oldcal);
		system (s);
}
