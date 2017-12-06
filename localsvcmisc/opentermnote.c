/* opentermnote */

/* open a channel to send a note to a terminal */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2013-04-20, David A­D­ Morano
        This subroutine was originally written. It obviously just takes
        advantange of the 'termnote(3dam)' object.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will open a channel for writing a note to a terminal.

	Synopsis:

	int opentermnote(pr,recips,max,opts)
	const char	*pr ;
	const char	**recips ;
	int		max ;
	int		opts ;

	Arguments:

	pr		program-root
	recips		array of username-strings to write to
	max		maximum number of terminals for each user
	opts		options
	wlen		length of bytes to write

	Returns:

	>=0		FD
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<stropts.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<filebuf.h>
#include	<termnote.h>
#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARHOME
#define	VARHOME		"HOME"
#endif

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#define	PROG_OPENPORT1	"/usr/preroot/sbin/opentermnote"
#define	PROG_OPENPORT2	"/usr/extra/sbin/opentermnote"

#ifndef	TERMNOTELEN
#define	TERMNOTELEN	(80 * 24)
#endif

#define	JUNKBUFLEN	200

#ifndef	ENVBUFLEN
#define	ENVBUFLEN	(MAXPATHLEN + 20)
#endif

#define	NENVS		10

#define	TARGS		struct targs


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matkeystr(char **,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct targs {
	const char	**recips ;
	const char	*a ;		/* the allocation */
	const char	*pr ;
	int		max ;
	int		opts ;
	int		rfd ;
} ;


/* forward references */

static int targs_start(TARGS *,const char *,const char **,int,int,int) ;
static int targs_finish(TARGS *) ;

static int termnoter(TARGS *) ;


/* local variables */

static const struct mapex	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;


/* exported subroutines */


int opentermnote(cchar *pr,cchar **recips,int max,int opts)
{
	int		rs ;
	int		pipes[2] ;
	int		fd = -1 ;

	if (pr == NULL) return SR_FAULT ;
	if (recips == NULL) return SR_FAULT ;

	if (recips[0] == NULL) return SR_INVALID ;

	if (max < 1) max = INT_MAX ;

#if	CF_DEBUGS
	for (i = 0 ; recips[i] != NULL ; i += 1)
	    debugprintf("opentermnote: r[%u]=%s\n",i,recips[i]) ;
#endif

	if ((rs = uc_piper(pipes,3)) >= 0) {
	    TARGS	*tap ;
	    const int	size = sizeof(TARGS) ;
	    int		i ;
	    fd = pipes[0] ;
	    if ((rs = uc_malloc(size,&tap)) >= 0) {
	        const int	rfd = pipes[1] ;
	        if ((rs = targs_start(tap,pr,recips,max,opts,rfd)) >= 0) {
	            if ((rs = uc_fork()) == 0) { /* child */
	                int		ex ;
	                u_setsid() ;
	                uc_sigignore(SIGHUP) ; /* just for good measure */
#if	CF_DEBUGS
#else
	                for (i = 0 ; i < 3 ; i += 1) u_close(i) ;
#endif /* CF_DEBUGS */
	                u_close(pipes[0]) ;
	                rs = termnoter(tap) ;
	                u_close(rfd) ;
	                ex = mapex(mapexs,rs) ;
	                u_exit(ex) ;
	            } /* end if (child) */
	            u_close(rfd) ;
	            pipes[1] = -1 ;
	            targs_finish(tap) ;
	        } /* end if (targs) */
	        uc_free(tap) ;
	    } /* end if (memory allocation) */
	    if (rs < 0) {
	        for (i = 0 ; i < 2 ; i += 1) {
	            if (pipes[i] >= 0) u_close(pipes[i]) ;
	        }
	    }
	} /* end if (pipes) */

#if	CF_DEBUGS
	debugprintf("opentermnote: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opentermnote) */


/* local subroutines */


static int targs_start(tap,pr,recips,max,opts,rfd)
TARGS		*tap ;
const char	*pr ;
const char	**recips ;
int		max ;
int		opts ;
int		rfd ;
{
	int		rs = SR_OK ;
	int		size ;
	int		i ;
	int		nrecips ;
	char		*vp = NULL ;
	char		*strp ;

	if (tap == NULL) return SR_FAULT ;

	memset(tap,0,sizeof(TARGS)) ;
	tap->max = max ;
	tap->opts = opts ;
	tap->rfd = rfd ;

	size = (strlen(pr) + 1) ;
	for (i = 0 ; recips[i] != NULL ; i += 1) {
	    size += (strlen(recips[i]) + 1) ;
	}
	nrecips = i ;
	size += ((nrecips + 1) * sizeof(const char *)) ;

	if ((rs = uc_malloc(size,&vp)) >= 0) {
	tap->a = vp ;
	tap->recips = (const char **) vp ;
	{
	    char	*bp = vp ;
	    strp = bp + ((nrecips + 1) * sizeof(const char *)) ;
	}
	for (i = 0 ; recips[i] != NULL ; i += 1) {
	    tap->recips[i] = strp ;
	    strp = strwcpy(strp,recips[i],-1) + 1 ;
	}
	tap->recips[i] = NULL ;
	tap->pr = strp ;
	strp = strwcpy(strp,pr,-1) + 1 ;
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (targs_start) */


static int targs_finish(TARGS *tap)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (tap == NULL) return SR_FAULT ;

	if (tap->a != NULL) {
	    rs1 = uc_free(tap->a) ;
	    if (rs >= 0) rs = rs1 ;
	    tap->a = NULL ;
	}
	tap->pr = NULL ;
	tap->recips = NULL ;
	return rs ;
}
/* end subroutine (targs_finish) */


/* this is the worker subroutines */
static int termnoter(TARGS *tap)
{
	TERMNOTE	tn ;
	int		rs = SR_OK ;
	int		lenr = TERMNOTELEN ;
	int		rs1 ;
	int		rfd ;
	int		max, opts ;
	int		c = 0 ;
	const char	*pr ;
	const char	**recips ;
	char		*buf = NULL ;

#if	CF_DEBUGS
	debugprintf("opentermnote/termnoter: ent\n") ;
#endif

	if (tap == NULL) return SR_FAULT ;

	pr = tap->pr ;
	recips = tap->recips ;
	max = tap->max ;
	opts = tap->opts ;
	rfd = tap->rfd ;

#if	CF_DEBUGS
	{
	    int	i ;
	    debugprintf("opentermnote/termnoter: rfd=%d\n",rfd) ;
	    for (i = 0 ; recips[i] != NULL ; i += 1)
	        debugprintf("opentermnote/termnoter: r[%u]=%s\n",i,recips[i]) ;
	}
#endif

	if ((rs = uc_malloc((lenr+1),&buf)) >= 0) {
	    int		bl = 0 ;
	    int		len = 0 ;
	    char	*bp = buf ;

/* read in the data (that we want) */

	    while ((rs >= 0) && (lenr > 0)) {
#if	CF_DEBUGS
	        debugprintf("opentermnote/termnoter: lenr=%u\n",lenr) ;
#endif
	        rs = u_read(rfd,bp,lenr) ;
	        len = rs ;
	        if ((rs < 0) || (len == 0)) break ;

#if	CF_DEBUGS
	        debugprintf("opentermnote/termnoter: u_read() rs=%d\n",rs) ;
	        debugprintf("opentermnote/termnoter: d=>%t<\n",
	            bp,strlinelen(bp,len,40)) ;
#endif

	        bp += len ;
	        bl += len ;
	        lenr -= len ;

	    } /* end while (reading data) */

/* read in the rest of the data that we throw away */

#if	CF_DEBUGS
	    debugprintf("opentermnote/termnoter: mid1 rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        char	junkbuf[JUNKBUFLEN+1] ;
	        while ((rs = u_read(rfd,junkbuf,JUNKBUFLEN)) > 0) ;
	    }

#if	CF_DEBUGS
	    debugprintf("opentermnote/termnoter: mid2 rs=%d\n",rs) ;
#endif

/* write out the term-note data to the terminals */

	    if (rs >= 0) {
	        if ((rs = termnote_open(&tn,pr)) >= 0) {

	            rs = termnote_write(&tn,recips,max,opts,buf,bl) ;
	            c = rs ;

	            rs1 = termnote_close(&tn) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (termnote) */
	    } /* end if (ok) */

	    uc_free(buf) ;
	} /* end if (memory-allocated buffer) */

#if	CF_DEBUGS
	debugprintf("opentermnote/termnoter: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (termnoter) */


