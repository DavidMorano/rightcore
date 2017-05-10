/* testuss */


#include	<bfile.h>


#define	ADDRESS	"/tmp/unix"



extern int	listenuss(const char *,int,int) ;



int main()
{
	bfile	outfile ;

	int	rs ;


	bopen(&outfile,BFILE_STDOUT,"dwct",0666) ;


	rs = listenuss(ADDRESS,0666,0) ;


	bprintf(&outfile,"rs=%d\n",rs) ;

	u_close(rs) ;


	bclose(&outfile) ;

	return 0 ;
}



