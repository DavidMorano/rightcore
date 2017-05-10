/* last modified %G% version %I% */

/* Multiple Index DataBase Fetch */

/*
	1985-07-01, David A.D. Morano

*/



#include	<fcntl.h>

#include	"localmisc.h"
#include	"midb.h"



/* external subroutines */


/* externals within the library */


/* external variables */


int midb_fetch(dbp,qp,buf,len)
midb		*dfp ;
midb_query	*qp ;
char		*buf ;
int		len ;
{
	long	record ;

	int	i, mlen, rlen ;

	char	*dbp ;


	if (fp->magic != MIDB_MAGIC) return (MIDBR_NOTOPEN) ;

/* get a record offset if we have to */

	if (qp->next <= 0) {



	} else
		record = qp->next ;

	lseek(midb->fp,record,SEEK_SET) ;


	rlen = 0 ;
	dbp = buf ;
	while (len > 0) {

	    if (fp->len <= 0) {

	        fp->len = read(fp->fd,fp->buf,BFILE_BUFSIZE) ;

	        if (fp->len < 0) return (- errno) ;

	        fp->bp = fp->buf ;

	        if (fp->len == 0) break ;

	    }

	    mlen = (fp->len < len) ? fp->len : len ;

	    for (i = 0 ; i < mlen ; i++) *dbp++ = *(fp->bp)++ ;

	    fp->len -= mlen ;
	    rlen += mlen ;
	    len -= mlen ;
	}

	fp->offset += rlen ;

	return (rlen) ;
}
/* end of my read routine */


