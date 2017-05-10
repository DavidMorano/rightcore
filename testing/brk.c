
#include	<stdio.h>


int	here[10] ;


extern int	_end ;


int main()
{
	char	*endp ;


	endp = (char *) sbrk(0) ;

	printf("brk=%p\n",endp) ;

	printf("end=%p\n",&_end) ;

	fclose(stdout) ;

	return 0 ;
}
	

