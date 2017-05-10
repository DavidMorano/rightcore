/* makescript */

/* program to aid in spelling corrections */


#include	<envstandards.h>

#include <stdio.h>


#define BOOLEAN int
#define FALSE 0
#define TRUE 1
#define MAXSIZE 400

char first[MAXSIZE], second[MAXSIZE];

BOOLEAN eof = FALSE;


void getline() 
{
	char *fp=first, *sp = second, c='9';
	int count = 0;

	for (;;) {
		c = getchar();
		if( c == EOF || c == '\n' || c == '/' )
			break;

		*fp++ = c;

		if (count++ > MAXSIZE){
			fprintf(stderr," ERROR - first word too long in correction file\n");
			abort();
		}
	}
	eof = (c == EOF);
	if ( eof ) 
		return;

	if ( count == 0 ){
		fprintf(stderr,"No words to correct file\n");
		exit(1);
	}

	*fp = '\0';
	count = 0;

	for (;;) {
		c = getchar();
		if ( c == EOF || c == '\n' )
			break;
		*sp++ = c;
		if (count++ >MAXSIZE) {
			fprintf(stderr,
			" ERROR - second word too long in correction file\n");
			abort();
		}
	}
	*sp = '\0';
	eof = (c == EOF) ;
}
	
int main()
{

	printf("set noic\n");
	while (eof == FALSE) {
		getline();
		if (strcmp(first,second) == 0)
			continue;
		printf("g/\\<%s\\>/p|s//%s/gp\n",first,second);
	}
	printf("w\n");
	printf("q\n");

	return 0 ;
}
/* end subroutine (main) */


