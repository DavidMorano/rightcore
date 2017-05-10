/* kshvar */

/* KSH variable framework */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGSIG	0		/* debug signal handling */
#define	CF_DEBUGMALL	1		/* debug memory allocation */


/* revision history:

	= 2004-03-01, David A­D­ Morano

	This subroutine was originally written.  


*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	int kshvar_start(KSHVAR *op)


*****************************************************************************/


#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/time.h>		/* for 'gethrtime(3c)' */
#include	<limits.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<sigman.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<field.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshvar.h"


/* local defines */

#define	KSHVAR_MAGIC	0x49736218

#ifndef	VARRANDOM
#define	VARRANDOM	"RANDOM"
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;


/* local structures */

struct locinfo_flags {
	uint		to:1 ;
} ;

struct locinfo {
	struct locinfo_flags	have, f, changed, final ;
	struct locinfo_flags	init, open ;
	vecstr		stores ;
	struct proginfo	*pip ;
	int		to ;		/* time-out */
	int		pagesize ;
} ;


/* forward references */

static int	locinfo_start(struct locinfo *,struct proginfo *) ;
static int	locinfo_utfname(struct locinfo *,const char *) ;
static int	locinfo_flags(struct locinfo *,int,int) ;
static int	locinfo_to(struct locinfo *,int) ;
static int	locinfo_setentry(struct locinfo *,const char **,
			const char *,int) ;
static int	locinfo_defaults(struct locinfo *) ;
static int	locinfo_finish(struct locinfo *) ;

static void	sighand_int(int) ;


/* global variables */

KSHVAR	kshvar ;


/* local variables */


/* exported subroutines */


int kshvar_start(KSHVAR *op)
{
	int	rs ;
	int	size ;


	memset(op,0,sizeof(KSHVAR)) ;

	size = sizeof(KSHVAR_VAR) ;
	rs = vecobj_start(&op->vars,size,0,0) ;
	if (rs < 0) goto bad0 ;

	op->magic = KSHVAR_MAGIC ;

ret0:
	return rs ;

bad0:
	goto ret0 ;
}
/* end subroutine (kshvar_start) */


int kshvar_finish(KSHVAR *op)
{
	KSHVAR_VAR	*ep ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;


	if (op == NULL) return SR_FAULT ;
	if (op->magic != KSHVAR_MAGIC) return SR_NOTOPEN ;

	for (i = 0 ; vecobj_get(&op->vars,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;

	    varname = ep->varname ;

	} /* end for */

	op->magic = 0 ;

ret0:
	return rs ;
}
/* end subroutine (kshvar_finish) */






