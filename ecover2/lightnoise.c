/* lightnoise */

/* gather light noise */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory allocations */


/* revision history:

	= 1998-05-03, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	Gather light noise.

	Synopsis:
	int lightnoise(PROGINFO *pip,

	$ ecover -d <infile> > <outfile>


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<userinfo.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern uint	hashelf(const char *,int) ;

extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	isdigitlatin(int) ;
extern int	isalnumlatin(int) ;
extern int	isNotAccess(int) ;
extern int	isFailOpen(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*getourenv(cchar **,cchar *) ;


/* external variables */


/* forward references */


/* local structures */


/* local variables */


/* exported subroutines */


int lightnoise(PROGINFO *pip,USERINFO *uip,cchar *ofname)
{
	struct timeval	tod ;
	SBUF		hb ;
	uint		v = 0 ;
	uint		hv = 0 ;
	const int	dlen = BUFLEN ;
	int		rs ;
	int		i ;
	int		bl = 0 ;
	char		dbuf[BUFLEN + 1] ;

	if ((rs = sbuf_start(&hb,dbuf,dlen)) >= 0) {
	    pid_t	pid_parent = getppid() ;

/* get some miscellaneous stuff */

	    if (uip->username != NULL)
	        sbuf_strw(&hb,uip->username,-1) ;

	    if (uip->homedname != NULL)
	        sbuf_strw(&hb,uip->homedname,-1) ;

	    if (uip->nodename != NULL)
	        sbuf_strw(&hb,uip->nodename,-1) ;

	    if (uip->domainname != NULL)
	        sbuf_strw(&hb,uip->domainname,-1) ;

	    sbuf_deci(&hb,(int) uip->pid) ;

	    sbuf_decl(&hb,(long) pip->daytime) ;

	    if (ofname != NULL)
	        sbuf_strw(&hb,ofname,-1) ;

	    sbuf_deci(&hb,(int) pid_parent) ;

	    uc_gettimeofday(&tod,NULL) ;

	    sbuf_buf(&hb,(char *) &tod,sizeof(struct timeval)) ;

	    bl = sbuf_finish(&hb) ;
	    if (rs >= 0) rs = bl ;
	} /* end if (sbuf) */

	    hv = hashelf(dbuf,bl) ;

/* pop in our environment also! */

	for (i = 0 ; pip->envv[i] != NULL ; i += 1) {
	    v ^= hashelf(pip->envv[i],-1) ;
	}
	hv ^= v ;

#ifdef	COMMENT
	if ((cp = getenv("RANDOM")) != NULL) {
	    if (cfdecui(cp,-1,&v) >= 0)
	        hv ^= v ;
	}
#endif /* COMMENT */

	pip->hv ;
	return rs ;
}
/* end subroutine (lightnoise) */


