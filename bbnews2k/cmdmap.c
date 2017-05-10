/* cmdmap */

/* command mapping management */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_FASTDEF	1		/* use fast-default */


/* revision history:

	= 2009-01-20, David A­D­ Morano
	This was written from scratch.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object manages the mapping between keys and commands.

        Using the "lookup" function, one can provide a key (in the form of a
        KEYSYM value) and this object will return the index of a 'command'.


*******************************************************************************/


#define	CMDMAP_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"cmdmap.h"


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	cfdecui(const char *,int,uint *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpyblanks(char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strncpylc(char *,const char *,int) ;
extern char	*strncpyuc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strnrpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int cmdmap_defmap(CMDMAP *,const CMDMAP_E *) ;

static int vcmpfind(const void *,const void *) ;


/* exported subroutines */


int cmdmap_start(CMDMAP *op,const CMDMAP_E *defmap)
{
	int		rs = SR_OK ;
	int		size ;
	int		opts ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(CMDMAP)) ;

	opts = VECOBJ_OREUSE ;
	size = sizeof(CMDMAP_E) ;
	if ((rs = vecobj_start(&op->map,size,10,opts)) >= 0) {
	    if (defmap != NULL) rs = cmdmap_defmap(op,defmap) ;
	    if (rs >= 0) {
	        op->magic = CMDMAP_MAGIC ;
	    }
	    if (rs < 0)
		vecobj_finish(&op->map) ;
	} /* end if (vecobj-started) */

	return rs ;
}
/* end subroutine (cmdmap_start) */


int cmdmap_finish(CMDMAP *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CMDMAP_MAGIC) return SR_NOTOPEN ;

	rs1 = vecobj_finish(&op->map) ;
	if (rs < 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (cmdmap_finish) */


int cmdmap_load(CMDMAP *op,int key,int cmd)
{
	CMDMAP_E	e, *ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_add = TRUE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CMDMAP_MAGIC) return SR_NOTOPEN ;

	if (key < 0) return SR_INVALID ;

	memset(&e,0,sizeof(CMDMAP_E)) ;
	e.key = key ;
	e.cmd = cmd ;

	if ((rs1 = vecobj_search(&op->map,&e,vcmpfind,&ep)) >= 0) {
	    if (ep->cmd != e.cmd) {
	        rs = vecobj_del(&op->map,rs1) ;
	    } else {
		f_add = FALSE ;
	    }
	} /* end if */

	if ((rs >= 0) && f_add) {
	    op->f.sorted = FALSE ;
	    rs = vecobj_add(&op->map,&e) ;
	}

	return rs ;
}
/* end subroutine (cmdmap_load) */


int cmdmap_lookup(CMDMAP *op,int key)
{
	int		rs = SR_OK ;
	int		cmd = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CMDMAP_MAGIC) return SR_NOTOPEN ;

	if (key < 0) return SR_INVALID ;

	if (! op->f.sorted) {
	    op->f.sorted = TRUE ;
	    rs = vecobj_sort(&op->map,vcmpfind) ;
	}

	if (rs >= 0) {
	    CMDMAP_E	te, *ep ;
	    te.key = key ;
	    if ((rs = vecobj_search(&op->map,&te,vcmpfind,&ep)) >= 0) {
	        if (ep != NULL) {
	            cmd = ep->cmd ;
	        }
	    }
	}

	return (rs >= 0) ? cmd : rs ;
}
/* end subroutine (cmdmap_lookup) */


/* private subroutines */


static int cmdmap_defmap(CMDMAP *op,const CMDMAP_E *defmap)
{
	int		rs = SR_OK ;
	int		i ;

#if	CF_FASTDEF
	{
	CMDMAP_E	*ep ;
	for (i = 0 ; (rs >= 0) && (defmap[i].key >= 0) ; i += 1) {
	    ep = (CMDMAP_E *) (defmap + i) ;
	    rs = vecobj_add(&op->map,ep) ;
	}
	}
#else /* CF_FASTDEF */
	for (i = 0 ; (rs >= 0) && (defmap[i].key >= 0) ; i += 1) {
	    rs = cmdmap_load(op,defmap[i].key,defmap[i].cmd) ;
	} /* end for */
#endif /* COMMENT */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (cmdmap_defmap) */


static int vcmpfind(const void *v1pp,const void *v2pp)
{
	CMDMAP_E	**e1pp = (CMDMAP_E **) v1pp ;
	CMDMAP_E	**e2pp = (CMDMAP_E **) v2pp ;
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            rc = (*e1pp)->key - (*e2pp)->key ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (vcmpfind) */


