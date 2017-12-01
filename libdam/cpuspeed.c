/* cpuspeed */

/* subroutine to load a CPUSPEED module */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-08-13, David A­D­ Morano
	This is an original write.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine loads up a CPUSPEED module.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#define	DEBUGFNAME	"/tmp/cpuspeed.deb"

#ifndef	PROGRAMEOOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#ifndef	OFD
#define	OFD		"S5"
#endif

#define	LIBDNAME	"lib/cpuspeed"

#define	ENTRYNAME	"cpuspeed"
#define	NRUNS		10000000


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	mkpath5(char *,cchar *,cchar *,cchar *,cchar *,cchar *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;


/* local structures */

struct loadfile {
	void		*dhp ;
	int		dlmode ;
	char		fname[MAXPATHLEN + 1] ;
} ;


/* forward references */

static int	loadfile(struct loadfile *,cchar *,cchar *,cchar **) ;


/* local variables */

#if	defined(_LP64)

static const char	*subdirs[] = {
	"sparcv9",
	"sparc",
	"",
	NULL
} ;

#else /* defined(_LP64) */

static const char	*subdirs[] = {
	"sparcv8",
	"sparcv7",
	"sparc",
	"",
	NULL
} ;

#endif /* defined(_LP64) */

static const char	*names[] = {
	"dhry",
	NULL
} ;

static const char	*exts[] = {
	"",
	"so",
	"o",
	NULL
} ;


/* exported suroutines */


int cpuspeed(cchar *pr,cchar *name,int nruns)
{
	struct loadfile	lf ;
	int		rs ;
	int		i ;
	int		speed ;
	int		(*fp)(int) ;
	void		*dhp ;

/* program root */

	if (pr == NULL)
	    pr = PROGRAMROOT ;

#if	CF_DEBUGS
	debugprintf("loader: pr=%s\n",pr) ;
#endif

	if (nruns <= 0)
	    nruns = NRUNS ;

	memset(&lf,0,sizeof(struct loadfile)) ;

#if	CF_PARENT
	lf.dlmode = RTLD_LAZY | RTLD_LOCAL | RTLD_PARENT ;
#else
	lf.dlmode = RTLD_LAZY | RTLD_LOCAL ;
#endif

	rs = SR_NOTFOUND ;
	if ((name == NULL) || (name[0] == '\0')) {
	    for (i = 0 ; names[i] != NULL ; i += 1) {
	        rs = loadfile(&lf,pr,names[i],exts) ;
	        if (rs > 0) break ;
	    } /* end for */
	} else {
	    rs = loadfile(&lf,pr,name,exts) ;
	}

#if	CF_DEBUGS
	debugprintf("loader: search rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	dhp = lf.dhp ;
	if (dhp != NULL) {
#if	CF_DEBUGS
	    cp = dlerror() ;
#endif
	    if ((fp = (int (*)()) dlsym(dhp,ENTRYNAME)) != NULL) {
	        speed = (*fp)(nruns) ;
	    } else {
	        rs = SR_LIBBAD ;
	    }
	    dlclose(dhp) ;
	} else {
	    rs = SR_LIBACC ;
	}
	} /* end if (ok) */

	return (rs >= 0) ? speed : rs ;
}
/* end subroutine (cpuspeed) */


/* local subroutines */


static int loadfile(lfp,pr,name,exts)
struct loadfile	*lfp ;
const char	pr[] ;
const char	name[] ;
const char	*exts[] ;
{
	struct ustat	sb ;
	int		i, j, k ;
	int		fl = 0 ;
	const char	*lp = NULL ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (lfp == NULL)
	    return SR_FAULT ;

	for (i = 0 ; i < 2 ; i += 1) {

	    lp = (i == 0) ? OFD : "" ;
	    for (j = 0 ; subdirs[j] != NULL ; j += 1) {

	        mkpath5(tmpfname,pr,LIBDNAME,lp,subdirs[j],name) ;

	        for (k = 0 ; exts[k] != NULL ; k += 1) {

#if	CF_DEBUGS
	            debugprintf("loader/havefile: ext=%s\n",exts[k]) ;
#endif

	            if (exts[k][0] != '\0') {
	                fl = mkfnamesuf1(lfp->fname,tmpfname,exts[k]) ;
	            } else {
	                fl = sncpy1(lfp->fname,MAXPATHLEN,tmpfname) ;
		    }

#if	CF_DEBUGS
	            debugprintf("loader/havefile: fname=%s\n",lfp->fname) ;
#endif

	            lfp->dhp = NULL ;
	            if ((u_stat(lfp->fname,&sb) >= 0) && (S_ISREG(sb.st_mode)))
	                lfp->dhp = dlopen(lfp->fname,lfp->dlmode) ;

	            if (lfp->dhp != NULL) break ;
	        } /* end for (extensions) */

	        if (lfp->dhp != NULL) break ;
	    } /* end for (subdirs) */

	    if (lfp->dhp != NULL) break ;
	} /* end for (major machine designator) */

	return (lfp->dhp != NULL) ? fl : SR_NOENT ;
}
/* end subroutine (loadfile) */


