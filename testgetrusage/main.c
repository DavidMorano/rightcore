

#include	<sys/resource.h>

#include	<stdio.h>



int main()
{
	struct rusage	ru ;

	int	rs ;


 	rs = getrusage(RUSAGE_SELF,&ru);

	fprintf(stdout,"rs=%d sec=%lu usec=%6lu\n",
		rs,ru.ru_utime.tv_sec,ru.ru_utime.tv_usec) ;

	fclose(stdout) ;

	return 0 ;
}


