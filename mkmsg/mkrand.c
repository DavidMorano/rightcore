/* mkrand */

/* make some light random data */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* not-switchable debug print-outs */
#define	CF_HEAVY	0		/* use heavy (strong) randomness */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates a unique (?) string no more than MSGBOUND
	characters.

	Synopsis:

	int mkrand(pip)
	PROGINFO	*pip ;

	Arguments:

	pip		pointer to program information

	Returns:

	>=0		length of result
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/time.h>		/* for 'gethrtime(3c)' */
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<buffer.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	RVBUFLEN
#define	RVBUFLEN	(2 * MAXHOSTNAMELEN)
#endif

#ifndef	HEXBUFLEN
#define	HEXBUFLEN	32
#endif

#ifndef	VARRANDOM
#define	VARRANDOM	"RANDOM"
#endif

#ifndef	VARSECONDS
#define	VARSECONDS	"SECONDS"
#endif


/* external subroutines */

extern uint	hashelf(void *,int) ;

extern int	md5calc(ULONG *,void *,int) ;


/* local structures */


/* forward references */

static int	mkrand_light(PROGINFO *) ;

#if	CF_HEAVY
static int	mkrand_heavy(PROGINFO *) ;
#endif /* CF_LIGHT */


/* local variables */


/* exported subroutines */


int mkrand(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->rand == 0) {
	    rs = mkrand_light(pip) ;
#if	CF_HEAVY
	    if (rs >= 0)
	        rs = mkrand_heavy(pip) ;
#endif /* CF_HEAVY */
	}

	return rs ;
}
/* end subroutine (mkrand) */


/* local subroutines */


static int mkrand_light(PROGINFO *pip)
{
	struct timeval	tod ;
	const pid_t	pid = getppid() ;
	ULONG		rv = 0 ;
	ULONG		v ;
	int		rs = SR_OK ;
	int		rs1 ;

#ifdef	COMMENT
	v = gethostid() ;
	rv ^= (v << 32) ;
#endif

	v = pip->uid ;
	rv ^= (v << 32) ;

	v = pid ;
	rv += (v << 16) ;

	v = pip->pid ;
	rv += v ;

	if ((rs1 = uc_gettimeofday(&tod,NULL)) >= 0) {
	    v = tod.tv_sec ;
	    rv ^= (v << 32) ;
	    rv ^= tod.tv_usec ;
	    rv ^= rs1 ;
	} else {
	    v = pip->daytime ;
	    rv ^= (v << 32) ;
	}

#if	SYSHAS_HRTIME
	{
	    hrtime_t	ht = gethrtime() ;
	    rv ^= ht ;
	}
#endif /* SYSHAS_GRTIME */

	v = pip->serial ;
	rv += v ;

	if (rs >= 0)
	    pip->rand += rv ;

	return rs ;
}
/* end subroutine (mkrand_light) */


#if	CF_HEAVY
static int mkrand_heavy(PROGINFO *pip,struct timeval *todp)
{
	BUFFER		hb ;
	ULONG		rv = 0 ;
	int		rs ;

	if ((rs = buffer_start(&hb,RVBUFLEN)) >= 0) {
	    int		i ;
	    int		bl ;
	    const char	*cp ;
	    const char	*bp ;
	    char	*buf ;

/* get some miscellaneous stuff */

	    if (pip->pwd != NULL)
	        buffer_strw(&hb,pip->pwd,-1) ;

	    if (pip->progename != NULL)
	        buffer_strw(&hb,pip->progename,-1) ;

	    if (pip->username != NULL)
	        buffer_strw(&hb,pip->username,-1) ;

	    if (pip->homedname != NULL)
	        buffer_strw(&hb,pip->homedname,-1) ;

	    if (pip->nodename != NULL)
	        buffer_strw(&hb,pip->nodename,-1) ;

	    if (pip->domainname != NULL)
	        buffer_strw(&hb,pip->domainname,-1) ;

	    if (pip->org != NULL)
	        buffer_strw(&hb,pip->org,-1) ;

	    if (pip->name != NULL)
	        buffer_strw(&hb,pip->name,-1) ;

	    if (pip->fullname != NULL)
	        buffer_strw(&hb,pip->fullname,-1) ;

	    if ((cp = getenv(VARRANDOM)) != NULL)
	        buffer_strw(&hb,cp,-1) ;

	    if ((cp = getenv(VARSECONDS)) != NULL)
	        buffer_strw(&hb,cp,-1) ;

/* get this stuff so far */

	    if ((rs = buffer_get(&hb,&buf)) >= 0) {
	        rs = md5calc(&rv,buf,rs) ;
	    }

/* I think we are done with the buffer */

	    bl = buffer_finish(&hb) ;
	    if (rs >= 0) rs = bl ;
	} /* end if (buffer) */

/* pop in our environment also! */

#ifdef	COMMENT
	if (rs >= 0) {
	    for (i = 0 ; pip->envv[i] != NULL ; i += 1) {
	        uint	hv ;
	        hv = hashelf(pip->envv[i],-1) ;
	        rv ^= (((ULONG) hv) << ((i & 1) ? 32 : 0)) ;
	    } /* end for */
	}
#endif /* COMMENT */

	if (rs >= 0)
	    pip->rand += rv ;

	return rs ;
}
/* end subroutine (mkrand_heavy) */
#endif /* CF_HEAVY */


