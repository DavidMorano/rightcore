/* main (testema) */

/* program to test the EMA object */


#define	CF_DEBUGS	1		/* non-switchable debug print-outs */
#define	CF_DEBUG	1		/* run-time */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a test program for the EMA object.

	Synopsis:

	$ testema.x < testfile


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"ema.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	LINEOUTLEN	38


/* external subroutines */

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;

extern int	progfile(struct proginfo *,bfile *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local structures */


/* local variables */

static const char	*types[] = {
	"regular",
	"pcs",
	"lalias",
	"salias",
	"group",
	NULL
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
} ;

static const struct mapex	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ 0, 0 }
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	bfile	outfile, *ofp = &outfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	rs = SR_OK ;
	int	ex = EX_OK ;

	const char	*pr = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFNAME)) == NULL) {
	    if ((cp = getenv(VARDEBUGFD1)) == NULL)
	        cp = getenv(VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting pip(%p)\n",pip) ;
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

#if	CF_DEBUGS
	debugprintf("main: 2 pip(%p)\n",pip) ;
#endif

	pip->verboselevel = 1 ;
	pip->debuglevel = 5 ;

	rs = proginfo_setpiv(pip,pr,&initvars) ;
	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    goto ret1 ;
	}

#if	CF_DEBUGS
	debugprintf("main: 3 pip(%p)\n",pip) ;
#endif

	if ((ofname == NULL) || (ofname[0] == '\0')) ofname = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) >= 0) {

#if	CF_DEBUGS
	debugprintf("main: entered pip(%p)\n",pip) ;
#endif

	pip->f.emainfo = TRUE ;

#if	CF_DEBUGS
	debugprintf("main: progfile() ofp(%p)\n",ofp) ;
#endif

	rs = progfile(pip,ofp,"-") ;

#if	CF_DEBUGS
	debugprintf("main: progfile() rs=%d\n",rs) ;
#endif

	bclose(ofp) ;
	} else
	    ex = EX_CANTCREAT ;

	if ((rs < 0) && (ex == EX_OK))
	    ex = EX_DATAERR ;

retearly:

#if	CF_DEBUGS
	debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

ret1:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_shcat: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */



