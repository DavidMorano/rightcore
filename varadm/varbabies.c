/* varbabies */

/* KSH variable implmentation */
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

	These subroutines implement a KSH variable.


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
#include	"varbabies.h"


/* local defines */

#ifndef	VARRANDOM
#define	VARRANDOM	"RANDOM"
#endif

#define	LOCINFO		struct locinfo


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
	int		to ;		/* time-out */
	int		pagesize ;
} ;

static MyNamfun_t {
	Namfun_t	namefun ;
	LOCINFO		li ;
	int		mydata ;
} ;


/* forward references */

/* KSH discipline functions */
static void putval(Namval_t *,const char *,int,Namfun_t *) ;
static char *getval(Namval_t *,Namfun_t *) ;
static double *getnum(Namval_t *,Namfun_t *) ;
static char *setdisc(Namval_t *,const char *,Namval_t *,Namfun_t *) ;
static Namval_t *createf(Namval_t *,const char *,Namfun_t *) ;
static Namval_t *clonef(Namval_t *,Namval_t *,int,Namfun_t *) ;
static char *namef(Namval_t *,Namfun_t *) ;
static Namval_t *nextf(Namval_t *,Dt_t *,Namfun_t *) ;
static Namval_t *typef(Namval_t *,Namfun_t *) ;


/* global variables */

KSHVAR	v_babies = {
	"babies",
	NULL
} ;


/* local variables */

static char	timebuf[TIMEBUFLEN+1] ;

static const char	*varname = "BABIES" ;


/* exported subroutines */

int varbabies_start(KSHVAR *op)
{

}


/* local subroutines */

static void putval(Namval_t *nvp,const char *val,int flags,Namfun_t *nfp)
{


}
/* end subroutine (putval) */


static char *getval(Namval_t *nvp,Namfun_t *nfp)
{

	strdcpy(timebuf,TIMEBUFLEN,"hello world!") ;

	return timebuf ;
}
/* end subroutine (getval) */


static double *getnum(Namval_t *nvp,Namfun_t *mfp)
{
	double	val = 1.2 ;

	return val ;
}
/* end subroutine (getnum) */


static char *namef(Namval_t *nvp,Namfun_t *mfp)
{


	return varname ;
}
/* end subroutine (namef) */


static Namval_t *nextf(Namval_t *nvp,Dt_t *dp,Namfun_t *nfp)
{


	return NULL ;
}
/* end subroutine (nextf) */



