

#include	<sys/types.h>
#include	<sys/statvfs.h>
#include	<sys/stat.h>
#include	<stdio.h>

#include	<bfile.h>
#include	<exitcodes.h>





int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct statvfs	fsb ;

	bfile	outfile ;

	int	rs ;
	int	cl ;

	char	*cp ;


	if (argc < 2)
		return EX_USAGE ;

	rs = u_statvfs(argv[1],&fsb) ;

	if (rs < 0)
		return EX_NOINPUT ;


	rs = bopen(&outfile,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0)
		return EX_CANTCREAT ;


	cp = fsb.f_basetype ;
	cl = strnlen(cp,FSTYPSZ) ;

	bprintf(&outfile,"basetype=%t\n",cp,cl) ;


	cp = fsb.f_fstr ;
	cl = strnlen(cp,32) ;

	bprintf(&outfile,"fstr=%t\n",cp,cl) ;


	bclose(&outfile) ;

	return EX_OK ;
}
/* end subroutine (main) */



