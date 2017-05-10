/* opensys_banner (opem-system-banner) */

/* Open the System Banner */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_FILEMAP	1		/* use |filemap(3uc)| */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Write ("date") the current date on the system banner.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<tmtime.h>
#include	<sntmtime.h>
#include	<filebuf.h>
#include	<filemap.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */

#define	SYSBANNER	"/etc/banner"

#ifndef	COLUMNS
#define	COLUMNS		80
#endif


/* external subroutines */

extern int	sfnext(const char *,int,const char **) ;
extern int	filebuf_writeblanks(FILEBUF *,int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnprbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	process(const char *,int,mode_t,const char *,int) ;
static int	procfile(FILEBUF *,int,mode_t,const char *,int) ;
static int	printend(FILEBUF *,const char *,const char *,int) ;
static int	printsub(FILEBUF *,const char *,const char *,int) ;
static int	filebuf_char(FILEBUF *,int) ;


/* local variables */


/* exported subroutines */


int opensys_banner(const char *fname,int of,mode_t om)
{
	TMTIME		tm ;
	const time_t	dt = time(NULL) ;
	int		rs ;
	int		fd = -1 ;
	int		f_top = TRUE ;
	const char	*tspec = "%e %b %T" ;

	if (fname == NULL) return SR_FAULT ;

	if ((rs = tmtime_gmtime(&tm,dt)) >= 0) {
	    char	ds[TIMEBUFLEN+1] ;
	    if ((rs = sntmtime(ds,TIMEBUFLEN,&tm,tspec)) >= 0) {
	        rs = process(fname,of,om,ds,f_top) ;
	        fd = rs ;
	    }
	} /* end if (tmtime_gmtime) */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensys_banner) */


/* local subroutines */


/* ARGSUSED */
static int process(const char *fn,int of,mode_t om,const char *ds,int f_top)
{
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;
	int		pipes[2] ;
	if ((rs = u_pipe(pipes)) >= 0) {
	    FILEBUF	wfile, *wfp = &wfile ;
	    const int	wfd = pipes[1] ;
	    fd = pipes[0] ;
	    if ((rs = filebuf_start(wfp,wfd,0L,0,0)) >= 0) {
	        rs = procfile(wfp,of,om,ds,f_top) ;
	        rs1 = filebuf_finish(wfp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (filebuf) */
	    u_close(wfd) ;
	} /* end if (pipes) */
	if ((rs < 0) && (fd >= 0)) u_close(fd) ;
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (process) */


#if	CF_FILEMAP
/* ARGSUSED */
static int procfile(FILEBUF *wfp,int of,mode_t om,const char *ds,int f_top)
{
	FILEMAP		sysban, *sfp = &sysban ;
	const int	maxsize = (5*1024) ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	const char	*sysbanner = SYSBANNER ;
	if ((rs = filemap_open(sfp,sysbanner,of,maxsize)) >= 0) {
	    int		line = 0 ;
	    int		len ;
	    const char	*lbuf ;

	    while ((rs = filemap_getline(sfp,&lbuf)) > 0) {
	        len = rs ;

	        if (lbuf[len-1] == '\n') len -= 1 ;

	        if (f_top && (line == 0)) {
	            rs = printend(wfp,ds,lbuf,len) ;
		    wlen += rs ;
	        } else if ((! f_top) && (line == 5)) {
	            rs = printsub(wfp,ds,lbuf,len) ;
	            wlen += rs ;
	        } else {
	            rs = filebuf_print(wfp,lbuf,len) ;
	            wlen += rs ;
	        }

	        line += 1 ;
	        if (rs < 0) break ;
	    } /* end while (reading lines) */

	    rs1 = filemap_close(sfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filemap) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfile) */
#else /* CF_FILEMAP */
static int procfile(FILEBUF *wfp,int of,mode_t om,const char *ds,int f_top)
{
	const int	to = -1 ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	const char	*sysbanner = SYSBANNER ;
	if ((rs = u_open(sysbanner,O_RDONLY,0666)) >= 0) {
	    FILEBUF	sysban, *sfp = &sysban ;
	    const int	fd = rs ;
	    if ((rs = filebuf_start(sfp,fd,0L,0,0)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        char		*lbuf ;
	        if ((rs = uc_malloc((llen+1),&lbuf)) >= 0) {
	            int		line = 0 ;
	            int		len ;

	            while ((rs = filebuf_readline(sfp,lbuf,llen,to)) > 0) {
	                len = rs ;

	        	if (lbuf[len-1] == '\n') len -= 1 ;

	                if (f_top && (line == 0)) {
	                    rs = printend(wfp,ds,lbuf,len) ;
		    	    wlen += rs ;
	                } else if ((! f_top) && (line == 5)) {
	                    rs = printsub(wfp,ds,lbuf,len) ;
	                    wlen += rs ;
	                } else {
	                    rs = filebuf_print(wfp,lbuf,len) ;
	                    wlen += rs ;
	                }

	                line += 1 ;
	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            uc_free(lbuf) ;
	        } /* end if (m-a) */
	        rs1 = filebuf_finish(sfp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (filebuf) */
	    u_close(fd) ;
	} /* end if (output-file) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfile) */
#endif /* CF_FILEMAP */


static int printend(FILEBUF *wfp,const char *ds,const char *lbuf,int len)
{
	const int	cols = COLUMNS ;
	const int	dl = strlen(ds) ;
	int		rs = SR_OK ;
	int		breaklen ;
	int		ml ;
	int		i = 0 ;
	int		wlen = 0 ;

	breaklen = (cols - dl) ;

	if ((rs >= 0) && (i < breaklen)) {
	    ml = MIN(len,breaklen) ;
	    rs = filebuf_write(wfp,(lbuf+i),ml) ;
	    wlen += rs ;
	    i += rs ;
	}

	if ((rs >= 0) && (i < breaklen)) {
	    ml = (breaklen-i) ;
	    rs = filebuf_writeblanks(wfp,ml) ;
	    wlen += rs ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = filebuf_write(wfp,ds,dl) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = filebuf_char(wfp,'\n') ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (printend) */


static int printsub(FILEBUF *wfp,cchar *ds,cchar lbuf[],int llen)
{
	int		rs = SR_OK ;
	int		di = 0 ;
	int		ch ;
	int		i ;
	int		wlen = 0 ;

	for (i = 0 ; (i < llen) && lbuf[i] ; i += 1) {
	    ch = MKCHR(lbuf[i]) ;
	    if ((! CHAR_ISWHITE(ch)) && (ds[di] != '\0')) {
	        if (ds[di] != ' ') ch = ds[di] ;
	        di += 1 ;
	    }
	    rs = filebuf_char(wfp,ch) ;
	    wlen += rs ;
	} /* end for */

	if (rs >= 0) {
	    rs = filebuf_char(wfp,'\n') ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (printsub) */


static int filebuf_char(FILEBUF *wfp,int ch)
{
	char	wbuf[2] ;
	wbuf[0] = ch ;
	wbuf[1] = '\0' ;
	return filebuf_write(wfp,wbuf,1) ;
}
/* end subroutine (filebuf_char) */


