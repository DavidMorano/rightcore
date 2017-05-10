/* last modified %G% version %I% */

/* "Basic I/O" package similiar to "stdio" */

/*
	David A.D. Morano
	July 1986
*/


#define		MOVC	0



#include	<fcntl.h>

#include	"localmisc.h"

#include	"bfile.h"


/* external subroutines */

extern long	lseek() ;

extern int	open(), close(), read(), write() ;


/* external variables */

extern int	errno ;


/* defines for 'bopen' */

#define		BO_READ		1
#define		BO_WRITE	2
#define		BO_APPEND	4
#define		BO_CREAT	8
#define		BO_TRUNC	16



int bclose(fp)
bfile	*fp ;
{
	if (fp->magic != BFILE_MAGIC) return (BE_NOTOPEN) ;

	if ((fp->stat & BIOSM_WRITE) && (fp->len > 0))

	    if (write(fp->fd,fp->buf,fp->len) < 0) return (- errno) ;

	fp->magic = 0 ;			/* clear magic number */
	free(fp->buf) ;

#ifdef	COMMENT
	if (fp->fd < 3) return OK ;
#endif

	if (close(fp->fd) < 0) return (- errno) ;

	return OK ;
}
/* end subroutine (midb_close) */


