/* deffile */

/* read in a Solaris "default" file of variables */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-02-05, David A­D­ Morano
        Some code for this subroutine was taken from something that did
        something similar to what we are doing here. The rest was originally
        written for LevoSim.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object reads in the parameter file and makes the parameter pairs
        available thought a key search.


*******************************************************************************/


#define	DEFFILE_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"deffile.h"


/* local defines */

#define	DEFFILE_ICHECKTIME	2	/* file check interval (seconds) */
#define	DEFFILE_ICHANGETIME	2	/* wait change interval (seconds) */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	vecstr_envfile(vecstr *,const char *) ;
extern int	vecstr_envget(vecstr *,const char *,const char **) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int deffile_open(DEFFILE *op,cchar *fname)
{
	int		rs = SR_OK ;
	int		opts ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(DEFFILE)) ;

	if ((rs = uc_mallocstrw(fname,-1,&cp)) >= 0) {
	    struct ustat	sb ;
	    op->fname = cp ;
	    if ((rs = uc_stat(fname,&sb)) >= 0) {
		const int	opts = VECSTR_OSTATIONARY ;
	    	op->ti_filemod = sb.st_mtime ;
	        if ((rs = vecstr_start(&op->vars,10,opts)) >= 0) {
	            if ((rs = vecstr_envfile(&op->vars,fname)) >= 0) {
	                op->magic = DEFFILE_MAGIC ;
	            }
	            if (rs < 0)
	                vecstr_finish(&op->vars) ;
	        } /* end if (vecstr_start) */
	    } /* end if (stat) */
	    if (rs < 0) {
	        uc_free(op->fname) ;
	        op->fname = NULL ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("deffile_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (deffile_open) */


/* free up the resources occupied by a DEFFILE list object */
int deffile_close(DEFFILE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DEFFILE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("deffile_close: ent\n") ;
#endif

	rs1 = vecstr_finish(&op->vars) ;
	if (rs >= 0) rs = rs1 ;

	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (deffile_close) */


/* cursor manipulations */
int deffile_curbegin(DEFFILE *op,DEFFILE_CUR *cp)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DEFFILE_MAGIC) return SR_NOTOPEN ;

	cp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (deffile_curbegin) */


int deffile_curend(DEFFILE *op,DEFFILE_CUR *cp)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DEFFILE_MAGIC) return SR_NOTOPEN ;

	cp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (deffile_curend) */


/* search the parameters for a match */
int deffile_fetch(DEFFILE *op,cchar *key,cchar **rpp)
{
	int		rs ;
	int		vl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (key == NULL) return SR_FAULT ;

	if (op->magic != DEFFILE_MAGIC) return SR_NOTOPEN ;

	if (key[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("deffile_fetch: ent key=%s\n",key) ;
#endif

	rs = vecstr_envget(&op->vars,key,rpp) ;
	vl = rs ;

#if	CF_DEBUGS
	debugprintf("deffile_fetch: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (deffile_fetch) */


/* enumerate the entries */
int deffile_enum(DEFFILE *op,DEFFILE_CUR *curp,char *kbuf,int klen,cchar **rpp)
{
	int		rs ;
	int		i ;
	int		kl ;
	int		vl = 0 ;
	const char	*ep ;

	if (op == NULL) return SR_FAULT ;
	if (kbuf == NULL) return SR_FAULT ;

	if (op->magic != DEFFILE_MAGIC) return SR_NOTOPEN ;

	kbuf[0] = '\0' ;
	if (rpp != NULL) *rpp = NULL ;

	i = 0 ;
	if (curp != NULL) {
	    i = (curp->i >= 0) ? (curp->i + 1) : 0 ;
	}

	for ( ; (rs = vecstr_get(&op->vars,i,&ep)) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;
	    break ;
	} /* end for */

	if (rs >= 0) {
	    const char	*tp = strchr(ep,'=') ;
	    const char	*vp ;
	    if (tp != NULL) {
	        kl = (tp-ep) ;
	        vp = (tp+1) ;
	    } else {
	        kl = strlen(ep) ;
	        vp = (ep+kl) ;
	    }
	    vl = strlen(vp) ;
	    if ((rs = snwcpy(kbuf,klen,ep,kl)) >= 0) {
	        if (rpp != NULL) *rpp = vp ;
	        if (curp != NULL) curp->i = i ;
	    }
	}

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (deffile_enum) */


int deffile_checkint(DEFFILE *op,int intcheck)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DEFFILE_MAGIC) return SR_NOTOPEN ;

	if (intcheck < 1) intcheck = 1 ;
	op->intcheck = intcheck ;
	return SR_OK ;
}
/* end subroutine (deffile_checkint) */


/* check if the parameter file has changed */
int deffile_check(DEFFILE *op,time_t daytime)
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_changed = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DEFFILE_MAGIC) return SR_NOTOPEN ;

	if (daytime == 0)
	    daytime = time(NULL) ;

/* should we even check? */

	if ((daytime - op->ti_check) > op->intcheck) {
	    op->ti_check = daytime ;
	    if ((rs1 = uc_stat(op->fname,&sb)) >= 0) {
	        if (sb.st_mtime > op->ti_filemod) {
	            f_changed = TRUE ;
	            op->ti_filemod = sb.st_mtime ;
	            vecstr_delall(&op->vars) ;
	            rs = vecstr_envfile(&op->vars,op->fname) ;
		}
	    }
	}

#if	CF_DEBUGS
	debugprintf("deffile_check: ret rs=%d changed=%d\n",
	    rs,c_changed) ;
#endif

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (deffile_check) */


