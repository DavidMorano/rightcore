/* last modified %G% version %I% */

/* "Basic I/O" package similiar to "stdio" */

/*
	David A.D. Morano
	July 1986
*/


#define		MOVC	0



#include	"localmisc.h"

#include	"bfile.h"

#include	<fcntl.h>


/* external subroutines */

extern long	lseek() ;

extern int	open(), close(), read(), write() ;


/* externals within the library */

extern int	bflush_i() ;


/* external variables */

extern int	errno ;


/* routine to write bytes */
/*
	Unlike the standard I/O library, all writes of this library
	are atomic in that the entire portion of each write request
	is actually written to the file as a whole.  Each write
	block is either written to the file as a single block or
	in conjunction with previous write requests, but in no
	way will any single write request be broken up and written
	separately to the file.

	Note that this library can also freely intermix reads and
	writes to a file with the data ending up where it should
	without getting scrambled as in the standard library.

	Both of the above features, as well as some other features
	unique to this library, would normally make this package
	slower than the standard I/O library, but this package is 
	normally faster than most recent versions of the standard 
	package and probably close in performance with some of the
	latest implemtations which use some of the buffer management
	strategies used here.
*/

int bwrite(fp,buf,len)
bfile	*fp ;
char	*buf ;
int	len ;
{
	int	i, mlen, lenr, rlen = 0 ;
	int	llen ;

#ifdef	COMMENT
	int	rs ;
#endif

	char	*dbp ;


	if (fp->magic != BFILE_MAGIC) return BE_NOTOPEN ;

	if (fp->oflag & O_RDONLY) return BE_RDONLY ;

	if (! (fp->stat & BIOSM_WRITE)) {

	    if (! (fp->stat & BIOSM_NOTSEEK)) lseek(fp->fd,fp->offset,0) ;

	    fp->len = 0 ;
	    fp->bp = fp->buf ;
	    fp->stat |= BIOSM_WRITE ;
	}

/* check for a large write */

	if (len > BFILE_BUFSIZE) {

	    if (fp->len > 0) bflush_i(fp) ;

	    if (write(fp->fd,buf,len) < 0) return (- errno) ;

	    fp->offset += len ;

	    return len ;

	} else if ((len > (BFILE_BUFSIZE - fp->len)) && (fp->len > 0)) {

	    bflush_i(fp) ;

	}


	if (len > 0) {

	    dbp = buf ;
	    llen = len ;
	    while (llen > 0) {

	        lenr = BFILE_BUFSIZE - fp->len ;
	        mlen = (llen < lenr) ? llen : lenr ;

#if	MOVC
	        movc(mlen,dbp,fp->bp) ;

	        fp->bp += mlen ;
	        dbp += mlen ;
#else
	        for (i = 0 ; i < mlen ; i += 1) *(fp->bp)++ = *dbp++ ;
#endif

	        fp->len += mlen ;
	        llen -= mlen ;
	        rlen += mlen ;

#ifdef	COMMENT
	        if (fp->len >= BFILE_BUFSIZE) {

	            if (write(fp->fd,fp->buf,BFILE_BUFSIZE) < 0)
			return (- errno) ;

	            fp->bp = fp->buf ;
	            fp->len = 0 ;

	        }
#endif

	    }

	    if (fp->stat & BIOSM_UNBUF) bflush_i(fp) ;

	}

	fp->offset += rlen ;

	return (len) ;
}
/* end of my write routine */


