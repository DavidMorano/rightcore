/* b_testsfio */

/* this is a SHELL built-in version of 'cat(1)' */
/* last modified %G% version %I% */


#define	CF_DEBUGS	1		/* non-switchable debug print-outs */
#define	CF_DEBUG	1		/* switchable at invocation */
#define	CF_READINTR	0		/* 'sfio_readintr(3sfio)' */


/* revision history:

	= 2004-03-01, David A­D­ Morano

	This subroutine was originally written as a KSH built-in command.


*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ testsfio [<file(s)> ...] [<options>]


*******************************************************************************/


#include	<envstandards.h>	/* must be first to configure */

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
#include	<time.h>

#include	<vsystem.h>
#include	<sigman.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<char.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_testsfio.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct subinfo_flags {
	uint		terminal:1 ;
	uint		bufline:1 ;
} ;

struct subinfo {
	struct subinfo_flags	f ;
	Sfio_t		*efp ;
} ;


/* forward references */

static int	sfio_print(Sfio_t *,const char *,int) ;

static void	sighand_int(int) ;


/* local variables */

static volatile int	if_exit ;
static volatile int	if_int ;

static const int	sigblocks[] = {
	SIGUSR1,
	SIGUSR2,
	SIGHUP,
	SIGCHLD,
	0
} ;

static const int	sigignores[] = {
	SIGPIPE,
	SIGPOLL,
#if	defined(SIGXFSZ)
	SIGXFSZ,
#endif
	0
} ;

static const int	sigints[] = {
	SIGINT,
	SIGTERM,
	SIGQUIT,
	0
} ;


/* exported subroutines */


int b_testsfio(argc,argv,contextp)
int		argc ;
const char	*argv[] ;
void		*contextp ;
{
	struct subinfo	si, *sip = &si ;

	SIGMAN		sm ;

	Sfio_t	*ofp = sfstdout ;
	Sfio_t	*ifp = sfstdin ;

	const int	to = 10 ;
	const int	llen = LINEBUFLEN ;

	int	rs, rs1 ;
	int	ex = EX_OK ;
	int	v ;
	int	len ;
	int	wlen = 0 ;

	char	lbuf[LINEBUFLEN + 1] ;


	if_exit = 0 ;

	rs = sigman_start(&sm,sigblocks,sigignores,sigints,sighand_int) ;
	if (rs < 0) goto badsigman ;

	memset(sip,0,sizeof(struct subinfo)) ;
	sip->efp = sfstderr ;

#if	CF_READINTR
	while ((rs = sfio_readintr(ifp,lbuf,llen,to,&if_exit)) > 0) {
	        len = rs ;
	    if ((! if_exit) && (! if_int)) {
	        rs = sfio_write(ofp,lbuf,len) ;
	        wlen += len ;
	    }
	            if ((rs >= 0) && if_exit) rs = SR_EXIT ;
	            if ((rs >= 0) && if_int) rs = SR_INTR ;
	    if (rs < 0) break ;
	} /* end while */
#else /* CF_READINTR */
	while ((rs = sfio_readlinetimed(ifp,lbuf,llen,to)) > 0) {
	        len = rs ;
	    if ((! if_exit) && (! if_int)) {
	        rs = sfio_write(ofp,lbuf,len) ;
	        wlen += len ;
	    }
	            if ((rs >= 0) && if_exit) rs = SR_EXIT ;
	            if ((rs >= 0) && if_int) rs = SR_INTR ;
	    if (rs < 0) break ;
	} /* end while */
#endif /* CF_READINTR */

	sfsync(ofp) ;

	sfsync(sip->efp) ;

	sigman_finish(&sm) ;

badsigman:
	return ex ;
}
/* end subroutine (b_testsfio) */


/* local subroutines */


static void sighand_int(sn)
int	sn ;
{

	switch (sn) {
	case SIGINT:
	    if_int = TRUE ;
	    break ;
	case SIGKILL:
	    if_exit = TRUE ;
	    break ;
	default:
	    if_exit = TRUE ;
	    break ;
	} /* end switch */

}
/* end subroutine (sighand_int) */



