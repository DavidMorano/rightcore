/* main */


#include	<envstandards.h>
#include	<sys/types.h>
#include	<bfile.h>


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	cchar		*ofn = BFILE_STDOUT ;
	if ((rs = bopen(ofp,ofn,"dwct",0666)) >= 0) {
	    int		i ;
	    for (i = 0 ; (rs >= 0) && (i < argc) ; i += 1) {
		bprintf(ofp,"%u> %s\n",i,argv[i]) ;
	    }
	    bclose(ofp) ;
	} /* end if (o-file) */
	return 0 ;
}
/* end subroutine (main) */


