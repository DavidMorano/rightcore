
#include	<stdio.h>
#include	<string.h>


#define	LINELEN		100


int main()
{
	int	len ;

	char	linebuf[LINELEN + 1] ;
	char	*s ;


	len = fgetline(stdin,linebuf,LINELEN) ;

	linebuf[len] = '\0' ;

	s = strtok(linebuf,"()\t ") ;

	if (s != NULL) {

		printf("s=>%s<\n",s) ;

	while ((s = strtok(NULL,"()\t ")) != NULL) {

		printf("s=>%s<\n",s) ;

	}

	}

	fclose(stdout) ;

	return 0 ;
}



