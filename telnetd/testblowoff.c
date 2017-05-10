/* testblowoff */
/* lang=C89 */

#define	CF_REAL	1

#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<bfile.h>
#include	<filebuf.h>
#include	<localmisc.h>

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX(MAXPATHLEN,2048)
#endif

#define	DFNAME	"here"

extern int	nprintf(const char *,const char *,...) ;
extern int	bufprintf(const char *,...) ;

int main(int argc,const char **argv,const char **envv)
{
	FILEBUF	b ;

	bfile	src, *sfp = &src ;

	const char	fd = 1 ;

	int	rs ;
	int	wlen = 0 ;

	const char	*fn = "/usr/extra/etc/telnetd/blowoff.txt" ;


	{
	    const char	*resp = "hello there\r\n" ;
	    u_write(fd,resp,strlen(resp)) ;
	}

	{
		struct ustat	sb ;
		int	tlen = LINEBUFLEN ;
		int	tl ;
		char	tbuf[LINEBUFLEN+1] ;
	rs = u_stat(fn,&sb) ;
		tl = bufprintf(tbuf,tlen,"rs=%d\r\n",rs) ;
	    u_write(fd,tbuf,tl) ;
	}

#if	CF_REAL
	if ((rs = bopen(sfp,fn,"r",0666)) >= 0) {
	    if ((rs = filebuf_start(&b,fd,0L,512,0)) >= 0) {
		const int	llen = LINEBUFLEN ;
		char		lbuf[LINEBUFLEN+3] ;

		while ((rs = breadline(sfp,lbuf,llen)) > 0) {
		    int	len = rs ;

		    if (lbuf[len-1] == '\n') len -= 1 ;

		    lbuf[len++] = '\r' ;
		    lbuf[len++] = '\n' ;
		    rs = filebuf_write(&b,lbuf,len) ;
		    wlen += rs ;

		    if (rs < 0) break ;
		} /* end while (reading lines) */

		filebuf_finish(&b) ;
	    } /* end if (filebuf) */
	    bclose(sfp) ;
	} /* end if (open source) */
#else /* CF_REAL */
	rs = bopen(sfp,fn,"r",0666) ;
	nprintf(DFNAME,"bopen() rs=%d\n",rs) ;
	if (rs >= 0) {
	    if ((rs = filebuf_start(&b,fd,0L,512,0)) >= 0) {
		const int	llen = LINEBUFLEN ;
		char		lbuf[LINEBUFLEN+3] ;

		while (rs >= 0) {
		    int	len ;
		    rs = breadline(sfp,lbuf,llen) ;
	nprintf(DFNAME,"breadline() rs=%d\n",rs) ;
		    if (rs <= 0) break ;
		    len = rs ;

		    if (lbuf[len-1] == '\n') len -= 1 ;	

		    lbuf[len++] = '\r' ;
		    lbuf[len++] = '\n' ;
		    rs = filebuf_write(&b,lbuf,len) ;
		    wlen += rs ;

		    if (rs < 0) break ;
		} /* end while (reading lines) */

		filebuf_finish(&b) ;
	    } /* end if (filebuf) */
	    bclose(sfp) ;
	} /* end if (open source) */
	    if ((rs = filebuf_start(&b,fd,0L,512,0)) >= 0) {
	    const char	*resp = "hello there\r\n" ;
		    rs = filebuf_write(&b,resp,strlen(resp)) ;
		filebuf_finish(&b) ;
	    }
#endif /* CF_REAL */

	{
	    const char	*resp = "good bye\r\n" ;
	    u_write(fd,resp,strlen(resp)) ;
	}

	return 0 ;
}
/* end subroutine (main) */

