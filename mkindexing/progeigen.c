/* progeigen */

/* find an EIGEN file */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1999-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This set of subroutines are used to find an EIGEN file.


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<expcook.h>
#include	<localmisc.h>

#include	"defs.h"
#include	"progeigen.h"
#include	"progids.h"


/* local defines */

#define	EIGENLANG	"english"
#define	EIGENSUF	"eign"


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;


/* externals variables */


/* forward references */

static int	progeigen_find(PROGINFO *) ;
static int	loadcooks(PROGINFO *,EXPCOOK *,cchar *) ;
static int	prochavefile(PROGINFO *,cchar *) ;


/* local structures */


/* local variables */

static const char	*eigenfnames[] = {
	"%p/lib/%s/%l.%f",
	"/usr/lib/%s/%l.%f",
	"%p/share/dict/%l.%f",
	"/usr/share/dict/%l.%f",
	"%p/lib/%s/%f",
	"/usr/lib/%s/%f",
	"%p/share/dict/%f",
	"/usr/share/dict/%f",
	NULL
} ;


/* exported subroutines */


int progeigen_begin(PROGINFO *pip)
{
	int		rs = SR_OK ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("progeigen_begin: ent\n") ;
#endif
	if (! pip->open.eigendb) {
	    if (pip->eigenfname == NULL) {
		rs = progeigen_find(pip) ;
	    } else {
	        rs = prochavefile(pip,pip->eigenfname) ;
	    }
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("progeigen_begin: mid rs=%d\n",rs) ;
#endif
	    if (rs >= 0) {
		EIGENDB		*edbp = &pip->eigendb ;
		if ((rs = eigendb_open(edbp,pip->eigenfname)) >= 0) {
		    pip->open.eigendb = TRUE ;
		}
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("progeigen_begin: eigendb_open() rs=%d\n",rs) ;
#endif
	    }
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("progeigen_begin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (progeigen_begin) */


int progeigen_end(PROGINFO *pip)
{
	EIGENDB		*edbp = &pip->eigendb ;
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->open.eigendb) {
	    pip->open.eigendb = FALSE ;
	    rs1 = eigendb_close(edbp) ;
	    if (rs >= 0) rs = rs1 ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("progeigen_end: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (progeigen_end) */


int progeigen_curbegin(PROGINFO *pip,PROGEIGEN_CUR *curp)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	rs = eigendb_curbegin(&pip->eigendb,&curp->cur) ;
	return rs ;
}
/* end subroutine (progeigen_curbegin) */


extern int progeigen_count(PROGINFO *pip)
{
	int		rs = SR_NOTOPEN ;
	if (pip->open.eigendb) {
	    rs = eigendb_count(&pip->eigendb) ;
	}
	return rs ;
}
/* end subroutine (progeigen_count) */


int progeigen_curend(PROGINFO *pip,PROGEIGEN_CUR *curp)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	rs = eigendb_curend(&pip->eigendb,&curp->cur) ;
	return rs ;
}
/* end subroutine (progeigen_curend) */


int progeigen_enum(PROGINFO *pip,PROGEIGEN_CUR *curp,cchar **rpp)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;
	rs = eigendb_enum(&pip->eigendb,&curp->cur,rpp) ;
	return rs ;
}
/* end subroutine (progeigen_enum) */


/* local subroutines */


int progeigen_find(PROGINFO *pip)
{
	EXPCOOK		cooks, *ecp = &cooks ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;
	cchar		*elang = pip->eigenlang ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("progeigen_find: ent lang=%s\n",elang) ;
#endif
	if (elang == NULL) elang = EIGENLANG ;
	if ((rs = expcook_start(ecp)) >= 0) {
	    if ((rs = loadcooks(pip,ecp,elang)) >= 0) {
		const int	elen = MAXPATHLEN ;
		int		efl ;
		int		i ;
		cchar		*efp ;
		char		ebuf[MAXPATHLEN+1] ;
		ebuf[0] = '\0' ;
		for (i = 0 ; eigenfnames[i] != NULL ; i += 1) {
	    	    efp = eigenfnames[i] ;
	            efl = -1 ;
	        	if ((rs = expcook_exp(ecp,0,ebuf,elen,efp,efl)) >= 0) {
			    len = rs ;
			    if ((rs = prochavefile(pip,ebuf)) >= 0) {
				rs = 1 ;
			    } else if (isNotPresent(rs)) {
				len = 0 ;
				rs = SR_OK ;
			    }
			} /* end if (expcook_exp) */
		    if (len > 0) break ;
	    	    if (rs < 0) break ;
		} /* end for */
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("progeigen_find: look1 rs=%d\n",rs) ;
#endif
		if ((rs >= 0) && (len > 0)) {
		    cchar	**vpp = &pip->eigenfname ;
	    	    rs = proginfo_setentry(pip,vpp,ebuf,len) ;
		}
	    } /* end if (loadcooks) */
	    rs1 = expcook_finish(&cooks) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (expcook) */
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("progeigen_find: ret rs=%d len=%u\n",rs,len) ;
#endif
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (progeigen_find) */


static int prochavefile(PROGINFO *pip,cchar *fname)
{
	struct ustat	sb ;
	int		rs ;
	int		rc = 0 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("progeiegen/prochavefile: ent fn=%s\n",fname) ;
#endif
	if (fname == NULL) return SR_FAULT ;
	if (fname[0] == '\0') return SR_INVALID ;
	if ((rs = u_stat(fname,&sb)) >= 0) {
	    if (S_ISREG(sb.st_mode)) {
		rs = progids_sperm(pip,&sb,R_OK) ;
		rc = rs ;
	    } else {
		rs = SR_ISDIR ;
	    }
	} /* end if (stat) */
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("progeiegen/prochavefile: ret rs=%d rc=%u\n",rs,rc) ;
#endif
	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (prochavefile) */


static int loadcooks(PROGINFO *pip,EXPCOOK *ecp,cchar *elang)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	cchar		*keys = "pslf" ;
	cchar		*vp ;
	char		kbuf[2] = "x" ;
	for (i = 0 ; keys[i] != '\0' ; i += 1) {
		const int	kc = MKCHAR(keys[i]) ;
		vp = NULL ;
		switch (kc) {
		case 'p':
		    vp = pip->pr ;
		    break ;
		case 's':
		    vp = pip->searchname ;
		    break ;
		case 'l':
		    if (elang[0] != '\0') vp = elang ;
		    break ;
		case 'f':
		    vp = EIGENSUF ;
		    break ;
		} /* end switch */
		if (vp != NULL) {
		    c += 1 ;
		    kbuf[0] = kc ;
		    rs = expcook_add(ecp,kbuf,vp,-1) ;
		}
	    if (rs < 0) break ;
	} /* end for */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadcooks) */


