/* b_testsfread */

/* this is a SHELL built-in version of 'cat(1)' */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_SFGETR	1		/* ? */


#include	<envstandards.h>	/* MUST be first to configure */

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif /* LINEBUFLEN */


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	sfreadline(Sfio_t *,char *,int) ;


/* local variables */


/* exported subroutines */


int b_testsfread(argc,argv,contextp)
int		argc ;
const char	*argv[] ;
void		*contextp ;
{
	Sfio_t	*ifp = sfstdin ;
	Sfio_t	*ofp = sfstdout ;
	const int	llen = LINEBUFLEN ;
	int	rs = SR_OK ;
	int	ex = 0 ;
	int	len ;
	int	wlen = 0 ;

	char	lbuf[LINEBUFLEN + 1] ;


	while ((rs = sfreadline(ifp,lbuf,llen)) > 0) {
	    len = rs ;

	    if ((rs = sfwrite(ofp,lbuf,len)) < 0) rs = SR_PIPE ;
	    wlen = rs ;

	    if (rs < 0) break ;
	} /* end while */

	if (rs >= 0) sfsync(ofp) ;

	if (rs < 0) ex = 1 ;
	return ex ;
}
/* end subroutine (b_testsfread) */


/* local subroutines */


static int sfreadline(fp,rbuf,rlen)
Sfio_t	*fp ;
char	rbuf[] ;
int	rlen ;
{
	int	rl = 0 ;


#if	CF_DEBUGS
	debugprintf("sfreadlinetimed: ent rlen=%d\n",rlen) ;
#endif

	if ((fp == NULL) || (rbuf == NULL))
	    return -1 ;

	rbuf[0] = '\0' ;
	if (rlen == 0) goto ret0 ;

#if	CF_SFGETR
	{
	    const char	*rp = sfgetr(fp,'\n',0) ;
#if	CF_DEBUGS
	debugprintf("sfreadlinetimed: sfgetr() rp=%p\n",rp) ;
#endif
	    if (rp != NULL) {
	        rl = sfvalue(fp) ;
#if	CF_DEBUGS
	debugprintf("sfreadlinetimed: sfvalue() rl=%d\n",rl) ;
#endif
	    } else {
	        rp = sfgetr(fp,'\n',SF_LASTR) ;
#if	CF_DEBUGS
	debugprintf("sfreadlinetimed: sfgetr(LAST) rp=%p\n",rp) ;
#endif
	        if (rp != NULL) {
	            rl = sfvalue(fp) ;
#if	CF_DEBUGS
	debugprintf("sfreadlinetimed: sfvalue() rl=%d\n",rl) ;
#endif
		} else
		    rl = 0 ;
	    } /* end if */
	        if ((rl > 0) && (snwcpy(rbuf,rlen,rp,rl) < 0))
		    rl = -1 ;
	}
#else /* CF_SFGETR */
	{
	    int ch = 0 ;
	    while ((rl < rlen) && ((ch = sfgetc(fp)) >= 0)) {
	        rbuf[rl++] = ch ;
	        if (ch == '\n') break ;
	    }
	    if (ch < 0) rl = -1 ;
	}
#endif /* CF_SFGETR */

ret0:

#if	CF_DEBUGS
	debugprintf("sfreadlinetimed: ret rl=%d\n",rl) ;
#endif

	return rl ;
}
/* end subroutine (sfreadline) */


