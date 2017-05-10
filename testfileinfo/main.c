

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<stdio.h>



/* forward references */

static int putinfo(int) ;





int main()
{


	putinfo(0) ;

	putinfo(1) ;

	putinfo(2) ;


	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int putinfo(fd)
int	fd ;
{
	struct ustat	sb ;

	int	rs ;


	fstat(fd,&sb) ;

	printf("FD=%d\n",fd) ;

	printf(" inode=%d\n",sb.st_ino) ;

	printf(" dev=%08x\n",sb.st_dev) ;

	printf(" rdev=%08x\n",sb.st_rdev) ;

	return rs ;
}



