/* progexpcook */

/* support building a message without output related subroutines */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The subroutine was written from scratch but based on previous versions
        of the 'mkmsg' program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Output a header.

	int progexpcook(pip,rbuf,rlen,sbuf,slen)
	struct proginfo	*pip ;
	char		rbuf[] ;
	int		rlen ;
	const char	sp[] ;
	int		sl ;


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	pcsgetfacility(const char *,char *,int) ;
extern int	nextfield(const char *,int,const char **) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* global variables */


/* local structures */

static const char	*cooks[] = {
	"varprname",
	"s",
	"v",
	"facility",
	"b",
	NULL
} ;

enum cooks {
	cook_varprname,
	cook_s,
	cook_v,
	cook_facility,
	cook_b,
	cook_overlast
} ;


/* forward references */

static int progexpcook_beginner(struct proginfo *) ;
static int progexpcook_loadcooks(struct proginfo *) ;


/* local variables */


/* exported subroutines */


int progexpcook_begin(struct proginfo *pip)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	return rs ;
}
/* end subroutine (procexpcook_begin) */


int progexpcook_end(struct proginfo *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

	if (pip->open.pec) {
	    pip->open.pec = FALSE ;
	    rs1 = expcook_finish(&pip->pec) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (procexpcook_end) */


int progexpcook_sub(pip,rbuf,rlen,sp,sl)
struct proginfo	*pip ;
char		rbuf[] ;
int		rlen ;
const char	sp[] ;
int		sl ;
{
	int		rs = SR_OK ;
	int		rl = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	if (! pip->open.pec) rs = progexpcook_beginner(pip) ;

	if (rs >= 0) {
	    EXPCOOK	*ecp = &pip->pec ;
	    const int	wch = MKCHAR('¿') ;
	    rs = expcook_exp(ecp,wch,rbuf,rlen,sp,sl) ;
	    rl = rs ;
	} /* end if */

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (progexpcook_sub) */


int progexpcook_subbuf(pip,bufp,sp,sl)
struct proginfo	*pip ;
BUFFER		*bufp ;
const char	sp[] ;
int		sl ;
{
	int		rs = SR_OK ;
	int		rl = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (bufp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progexpcook_subbuf: s=>%t<\n",
		sp,strlinelen(sp,sl,50)) ;
#endif

	if (! pip->open.pec) rs = progexpcook_beginner(pip) ;

	if (rs >= 0) {
	    EXPCOOK	*ecp = &pip->pec ;
	    const int	wch = MKCHAR('¿') ;
	    rs = expcook_expbuf(ecp,wch,bufp,sp,sl) ;
	    rl = rs ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progexpcook_subbuf: ret rs=%d rl=%u\n",rs,rl) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (progexpcook_subbuf) */


/* local subroutines */


static int progexpcook_beginner(struct proginfo *pip)
{
	int		rs = SR_OK ;

	if (! pip->open.pec) {
	    EXPCOOK	*ecp = &pip->pec ;
	    if ((rs = expcook_start(ecp)) >= 0) {
		pip->open.pec = TRUE ;
		rs = progexpcook_loadcooks(pip) ;
		if (rs < 0) {
		    pip->open.pec = FALSE ;
		    expcook_finish(ecp) ;
		}
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progexpcook_beginner: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progexpcook_beginner) */


static int progexpcook_loadcooks(struct proginfo *pip)
{
	EXPCOOK		*ecp = &pip->pec ;
	int		rs = SR_OK ;
	int		i ;
	int		vl ;
	const char	*vp ;

	for (i = 0 ; cooks[i] != NULL ; i += 1) {
	    vl = -1 ;
	    vp = NULL ;
	    switch (i) {
	    case cook_varprname:
		vp = VARPRNAME ;
		break ;
	    case cook_s:
		vp = pip->searchname ;
		break ;
	    case cook_v:
		vp = pip->version ;
		break ;
	    case cook_facility:
		vp = pip->facility ;
		break ;
	    case cook_b:
		vp = pip->banner ;
		break ;
	    } /* end switch */
	    if ((rs >= 0) && (vp != NULL)) {
		rs = expcook_add(ecp,cooks[i],vp,vl) ;
	    }
	    if (rs < 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (progexpcook_loadcooks) */


