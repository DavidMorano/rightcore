/* dump */

/* dump data from pipes */

/* 
	This is a cheap way to try and read the junk coming out of two
	pipes without getting into a situation where the sending program
	blocks.
*/

#define	DUMPLEN	50

static void bdump(fd1,fd2)
int	fd1, fd2 ;
{
	int	f_done1, f_done2 ;

	char	buf[DUMPLEN + 1] ;


	f_done1 = f_done2 = FALSE ;
	while ((! f_done1) || (! f_done2)) {

	    if (! f_done1) {

	        if (read(fd1,buf,DUMPLEN) <= 0)
	            f_done1 = TRUE ;

	    }

	    if (! f_done2) {

	        if (bread(fd2,buf,DUMPLEN) <= 0)
	            f_done2 = TRUE ;

	    }

	} /* end while */

}
/* end subroutine (dump) */


