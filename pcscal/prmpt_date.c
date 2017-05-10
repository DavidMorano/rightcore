#
#include <stdio.h>
#define BUFSIZE 256

main(argc,argv)
int argc;
char **argv; 
{

	char s[BUFSIZE];
	char t[BUFSIZE];

	while(1)
	{

		/* prompt for date */
		fprintf(stderr,"DATE: ");

		/* read date */
		if(gets(s) == NULL) exit(0);

		/* check for end of input */
		if(strcmp(s,".") == 0) exit(0);

		/* check for valid date */

		if(get_date(s,t) >= 0)

		{
			fprintf(stdout,"%s\n",t);
			exit(0);
		}

		/* print error message */
		if(strcmp(s,"?") != 0)
		{
			fprintf(stderr,"Sorry, that is an unintelligible date.\n");
			fprintf(stderr,"Enter ? for more help\n");
		}

		/* print help message */
		if(strcmp(s,"?") == 0)
		{
			fprintf(stderr,"\nThe date may be any standard calendar convention\n");
			fprintf(stderr,"such as '10/15/81', 'October 15, 1981', '15oct81', or 'OCT 15'.\n");
			fprintf(stderr,"Days of the weeks (eg, 'monday', 'tuesday')\n");
			fprintf(stderr,"and terms such as today' and 'tommorrow' are understood.\n");
			fprintf(stderr,"Finally, such combinations as 'monday + 1 month' or\n");
			fprintf(stderr,"Jan 1 - 2 weeks' are also accepatable.\n");
			fprintf(stderr,"If the year is ommited, the year for the closest date is assumed.\n");
			fprintf(stderr,"Words can be abbreviated to 3 letters (eg, 'Jan', 'sat')\n\n");
		}

	}
}
