#
#include <stdio.h>

main(argc,argv)
int argc;
char **argv;
{
	char s[256];
	char t[256];
	int i;
	int dtype;

	s[0] = '\0';
	i = 1;
	dtype = 0;
	if(argc > 1 && strncmp(argv[1],"-t",2) == 0)
	{
		sscanf(&argv[1][2],"%d",&dtype);
		i++;
	}
	for(; i < argc; i++)
		sprintf(s + strlen(s),"%s ",argv[i]);

	if( get_date(s,t) < 0)
	{
		fprintf(stderr,"unknown date\n");
		exit(1);
	}

	if(dtype != 0)
	{
		undate(t,t);
		convdt(t,dtype,t);
	}
	fprintf(stdout,"%s\n",t);
	exit(0);

}
