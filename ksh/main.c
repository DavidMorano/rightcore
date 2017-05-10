/* main */

/* generic front-end for SHELL */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_SHOBJ	0		/* experiment? */


/* revision history:

	= 2002-03-01, David A­D­ Morano

	This was written as a small experiement.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ ksh


*******************************************************************************/


#include	<envstandards.h>

#include	<shell.h>

#if	defined(SOLARIS) && (SOLARIS >= 8)
#include	<user_attr.h>
#include	<project.h>
#endif

#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<sbuf.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	lastlogin(char *,uid_t,time_t *,char *,char *) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* global variables */

struct gprog	g ;


/* local structures */


/* forward references */

static void	shmark(int) ;


/* local variables */


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	int	rs = SR_OK ;
	int	ex = EX_INFO ;


	memset(&g,0,sizeof(struct gprog)) ;
	g.envv = envv ;

#if	CF_SHOBJ
	rs = shobj_init(&g.shobjs,&g) ;
#endif


	ex = sh_main(argc,argv,shmark) ;


#if	CF_SHOBJ
	shobj_free(&g.shobjs) ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static void shmark(n)
int	n ;
{
	time_t	daytime = time(NULL) ;


#if	CF_SHOBJ
	shobj_check(&g.shobjs) ;
#endif



}
/* end subroutine (shmark) */


