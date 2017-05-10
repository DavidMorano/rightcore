

#include	<sys/types.h>
#include	<stdio.h>

#include	<bfile.h>

#include	"localmisc.h"




int main()
{
	bfile	ofile, *ofp = &ofile ;


	bopen(&ofile,BFILE_STDOUT,"dwct",0666) ;



	printf("hello p=%p\n",main) ;

	bprintf(&ofile,"hello p=%p\n",main) ;




	fclose(stdout) ;

	bclose(&ofile) ;

	return 0 ;
}


