/* experiment with C++ */



#include	<bfile.h>



int main()
{
	bfile	out, *ofp = &out ;


	bopen(ofp,BFILE_STDOUT,"wct",0666) ;

	bprintf("main: hello there\n") ;

	bclose(ofp) ;

	return 0 ;
}
/* end subroutine (main) */



