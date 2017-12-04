/* procse */

/* build up a server entry piece-meal as it were */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little object is used to create a server entry and to populate
        aspects of it with different operations on the object. This object is
        used in "server" types of programs. This object is usually created from
        elements taken from the parsing of a server file.


*******************************************************************************/


#define	PROCSE_MASTER		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<varsub.h>
#include	<expcook.h>
#include	<localmisc.h>

#include	"procse.h"


/* local defines */

#ifndef	VBUFLEN
#define	VBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(4 * MAXPATHLEN)
#endif

#undef	BUFLEN
#define	BUFLEN		(10 * MAXPATHLEN)


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;


/* forward references */

static int	process(PROCSE *,EXPCOOK *,const char *,const char **) ;


/* external variables */


/* exported subroutines */


int procse_start(PROCSE *pep,cchar **envv,varsub *vsp,PROCSE_ARGS *esap)
{

	if (pep == NULL) return SR_FAULT ;
	if (esap == NULL) return SR_FAULT ;

	memset(pep,0,sizeof(PROCSE)) ;
	pep->envv = envv ;
	pep->vsp = vsp ;
	pep->ap = esap ;

	return SR_OK ;
}
/* end subroutine (procse_start) */


int procse_finish(PROCSE *pep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pep == NULL) return SR_FAULT ;

	if (pep->a.passfile != NULL) {
	    rs1 = uc_free(pep->a.passfile) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (pep->a.sharedobj != NULL) {
	    rs1 = uc_free(pep->a.sharedobj) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (pep->a.program != NULL) {
	    rs1 = uc_free(pep->a.program) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (pep->a.srvargs != NULL) {
	    rs1 = uc_free(pep->a.srvargs) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (pep->a.username != NULL) {
	    rs1 = uc_free(pep->a.username) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (pep->a.groupname != NULL) {
	    rs1 = uc_free(pep->a.groupname) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (pep->a.options != NULL) {
	    rs1 = uc_free(pep->a.options) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (pep->a.access != NULL) {
	    rs1 = uc_free(pep->a.access) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (pep->a.failcont != NULL) {
	    rs1 = uc_free(pep->a.failcont) ;
	    if (rs >= 0) rs = rs1 ;
	}

	memset(pep,0,sizeof(PROCSE)) ;

	return rs ;
}
/* end subroutine (procse_finish) */


/* process server entry */
int procse_process(PROCSE *pep,EXPCOOK *ecp)
{
	PROCSE_ARGS	*ap ;
	int		rs = SR_OK ;

	if (pep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("procse_process: ent\n") ;
#endif

	ap = pep->ap ;

/* pop them */

#if	CF_DEBUGS
	debugprintf("procse_process: in-program=>%s<\n",
	    ap->program) ;
#endif

	if ((rs >= 0) && (ap->passfile != NULL))
	    rs = process(pep,ecp,ap->passfile,&pep->a.passfile) ;

	if ((rs >= 0) && (ap->sharedobj != NULL))
	    rs = process(pep,ecp,ap->sharedobj,&pep->a.sharedobj) ;

	if ((rs >= 0) && (ap->program != NULL))
	    rs = process(pep,ecp,ap->program,&pep->a.program) ;

#if	CF_DEBUGS
	debugprintf("procse_process: process() rs=%d out-program=>%s<\n",
	    rs,pep->program) ;
#endif

#if	CF_DEBUGS
	debugprintf("procse_process: in-srvargs=>%s<\n",
	    ap->srvargs) ;
#endif

	if ((rs >= 0) && (ap->srvargs != NULL))
	    rs = process(pep,ecp,ap->srvargs,&pep->a.srvargs) ;

#if	CF_DEBUGS
	debugprintf("procse_process: process() rs=%d out-srvargs=>%s<\n",
	    rs,pep->srvargs) ;
#endif

	if ((rs >= 0) && (ap->username != NULL))
	    rs = process(pep,ecp,ap->username,&pep->a.username) ;

	if ((rs >= 0) && (ap->groupname != NULL))
	    rs = process(pep,ecp,ap->groupname,&pep->a.groupname) ;

	if ((rs >= 0) && (ap->options != NULL))
	    process(pep,ecp,ap->options,&pep->a.options) ;

	if ((rs >= 0) && (ap->access != NULL))
	    rs = process(pep,ecp,ap->access,&pep->a.access) ;

	if ((rs >= 0) && (ap->failcont != NULL))
	    rs = process(pep,ecp,ap->failcont,&pep->a.failcont) ;

	return rs ;
}
/* end subroutine (procse_process) */


/* local subroutines */


static int process(PROCSE *pep,EXPCOOK *ecp,cchar *inbuf,cchar **opp)
{
	int		rs = SR_OK ;
	char		fl = 0 ;
	const char	*ccp ;
	const char	*fp ;

#if	CF_DEBUGS
	debugprintf("procse/process: ent inbuf=>%s<\n",inbuf) ;
#endif

	if (opp == NULL) return SR_FAULT ;

	*opp = NULL ;
	if (rs >= 0) {
	    const int	vlen = BUFLEN ;
	    int		vl = 0 ;
	    char	vbuf[BUFLEN + 1] ;
	    if (pep->vsp != NULL) {
	        rs = varsub_expand(pep->vsp,vbuf,vlen,inbuf,-1) ;
	        vl = rs ;
	    } else {
	        rs = sncpy1(vbuf,vlen,inbuf) ;
	        vl = rs ;
	    }
	    if (rs >= 0) {
		const int	elen = BUFLEN ;
		int		el = 0 ;
	        char		ebuf[BUFLEN + 1] ;
	        if (ecp != NULL) {
	            rs = expcook_exp(ecp,0,ebuf,elen,vbuf,vl) ;
	            el = rs ;
	        } else {
	            rs = snwcpy(ebuf,elen,vbuf,vl) ;
	            el = rs ;
	        }
	        if (rs >= 0) {
	            fl = sfshrink(ebuf,el,&fp) ;
	            if ((rs = uc_mallocstrw(fp,fl,&ccp)) >= 0) {
	                *opp = ccp ;
	            }
	        }
	    } /* end if (ok) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("procse/process: ret rs=%d fl=%u\n",rs,fl) ;
#endif

	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (process) */


