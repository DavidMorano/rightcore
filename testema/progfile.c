/* progfile */

/* process the input files */


#define	CF_DEBUGS 	0		/* compile-time switchable */
#define	CF_DEBUG 	1		/* run-time switchable */


/* revision history:

	= 1994-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Here we process a message for its addresses.

	Synopsis:

	int progfile(pip,ofp,fname)
	struct proginfo	*pip ;
	bfile		*ofp ;
	char		fname[] ;

	Arguments:

	- pip		program information pointer
	- pp		paramter option pointer
	- ofp		output (BIO) file pointer
	- fname		file to process

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<buffer.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"ema.h"


/* local defines */


/* external subroutines */

extern int	progentryinfo(struct proginfo *,bfile *,EMA_ENT *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int progfile_info(struct proginfo *,bfile *,EMA_ENT *,int) ;


/* local variables */


/* exported subroutines */


int progfile(pip,ofp,fname)
struct proginfo	*pip ;
bfile		*ofp ;
const char	fname[] ;
{
	EMA		adds ;
	EMA_ENT	*ep ;

	bfile		infile, *ifp = &infile ;

	const int	llen = LINEBUFLEN ;

	int	rs ;
	int	rs1 ;
	int	i, j ;
	int	len ;
	int	n = 0 ;

	const char	*cp, *hp, *vp ;

	char	lbuf[LINEBUFLEN + 1] ;


	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progfile: entered file=%s\n",fname) ;
#endif

	if ((rs = ema_start(&adds)) >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progfile: ema_start() rs=%d\n",rs) ;
#endif

/* open the file that we are supposed to process */

	    if ((fname[0] == '\0') || (fname[0] == '-')) fname = BFILE_STDIN ;

	    if ((rs = bopen(ifp,fname,"r",0666)) >= 0) {
		BUFFER	b ;
		if ((rs = buffer_start(&b,80)) >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	            debugprintf("progfile: continuing\n") ;
#endif

	        while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

#if	CF_DEBUG 
	if (DEBUGLEVEL(4))
	                debugprintf("progfile: >%t<\n",lbuf,len) ;
#endif

	            rs = buffer_strw(&b,lbuf,len) ;

#if	CF_DEBUG 
	if (DEBUGLEVEL(4))
	                debugprintf("progfile: ema_parse() rs=%d\n",rs) ;
#endif

	            if (rs < 0) break ;
	        } /* end while */

		    if (rs >= 0) {
			const char	*bp ;
			int		bl ;
			if ((rs = buffer_get(&b,&bp)) > 0) {
				bl = rs ;
				rs = ema_parse(&adds,bp,bl) ;
			}
		    }

		    buffer_finish(&b) ;
		} /* end if (buffer) */
/* the theory is that we have all we want from the headers, do it */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progfile: looping printing\n") ;
#endif

	        if (rs >= 0) {
	            for (i = 0 ; ema_get(&adds,i,&ep) >= 0 ; i += 1) {
	                int	spc = 0 ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progfile: ema_get() i=%d ep=%p\n",
				i,ep) ;
#endif

	                if (ep == NULL) continue ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	                    debugprintf("progfile: l=%d orig=>%s<\n",
	                        ep->ol,ep->op) ;
	                    debugprintf("progfile: l=%d address=>%s<\n",
	                        ep->al,ep->ap) ;
	                    debugprintf("progfile: l=%d route=%s\n",
	                        ep->rl,ep->rp) ;
	                    debugprintf("progfile: l=%d comment=>%s<\n",
	                        ep->cl,ep->cp) ;
	                }
#endif

	                if (! pip->f.count) {

	                    if (! pip->f.emainfo) {

	                        if (pip->f.ema_original) {

#if	CF_DEBUG
	                            if (DEBUGLEVEL(4))
	                                debugprintf("progfile: "
						"try_original=>%s<\n",
	                                    ep->op) ;
#endif

	                            if (ep->op != NULL)
	                                bprintf(ofp,"%s",ep->op) ;

	                            spc += 1 ;
	                        }

	                        if (pip->f.ema_best) {

#if	CF_DEBUG
	                            if (DEBUGLEVEL(4))
	                                debugprintf("progfile: try_best\n") ;
#endif

	                            if (ep->rp != NULL) {
	                                bprintf(ofp,"%s",ep->rp) ;

	                            } else if (ep->ap != NULL) {
	                                bprintf(ofp,"%s",ep->ap) ;

	                            } else if (ep->cp != NULL)
	                                bprintf(ofp,"(%s)",ep->cp) ;

	                            spc += 1 ;
	                        }

	                        if (pip->f.ema_any) {

#if	CF_DEBUG
	                            if (DEBUGLEVEL(4))
	                                debugprintf("progfile: try_any\n") ;
#endif

	                            if (ep->ap != NULL) {
	                                bprintf(ofp,"%s",ep->ap) ;

	                            } else if (ep->rp != NULL) {
	                                bprintf(ofp,"%s",ep->rp) ;

	                            } else if (ep->cp != NULL)
	                                bprintf(ofp,"(%s)",ep->cp) ;

	                            spc += 1 ;
	                        }

	                        if (pip->f.ema_address) {

	                            if (spc > 0)
	                                bprintf(ofp,"|") ;

	                            if (ep->ap != NULL) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	                                    debugprintf("progfile: "
						"al=%d ap=>%t<\n",
	                                        ep->al,ep->ap,ep->al) ;
#endif

	                                bprintf(ofp,"%s",ep->ap) ;

	                            }

	                            spc += 1 ;
	                        }

	                        if (pip->f.ema_route) {

#if	CF_DEBUG
	                            if (DEBUGLEVEL(4))
	                                debugprintf("progfile: "
						"try_route=>%s<\n",
	                                    ep->rp) ;
#endif

	                            if (spc > 0)
	                                bprintf(ofp,"|") ;

	                            if (ep->rp != NULL)
	                                bprintf(ofp,"%s",ep->rp) ;

	                            spc += 1 ;
	                        }

	                        if (pip->f.ema_comment) {

#if	CF_DEBUG
	                            if (DEBUGLEVEL(4))
	                                debugprintf("progfile: "
						"try_comment=>%s<\n",
	                                    ep->cp) ;
#endif

	                            if (spc > 0)
	                                bprintf(ofp,"|") ;

	                            if (ep->cp != NULL) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	                                    debugprintf("progfile: "
						"clen=%d comment=>%t<\n",
	                                        ep->cl,ep->cp,ep->cl) ;
#endif

	                                bprintf(ofp,"%s",ep->cp) ;

	                            }

	                            spc += 1 ;
	                        }

	                        bprintf(ofp,"\n") ;

	                    } else {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	                            debugprintf("progfile: "
					"printing information\n") ;
#endif

	                        rs = progfile_info(pip,ofp,ep,0) ;

	                    } /* end if (information) */

	                } /* end if (counting only) */

	                n += 1 ;
	                if (rs < 0) break ;
	            } /* end for */
	        } /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progfile: file-out rs=%d\n",rs) ;
#endif

	        bclose(ifp) ;
	    } /* end if (file-open) */

	    rs1 = ema_finish(&adds) ;
	    if (rs >= 0) rs = rs1 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progfile: ema_finish() rs=%d\n",rs) ;
#endif
	} /* end if (ema) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progfile: ret rs=%d entries=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (progfile) */


/* local subroutines */


static int progfile_info(pip,ofp,ep,level)
struct proginfo	*pip ;
EMA_ENT	*ep ;
int		level ;
bfile		*ofp ;
{
	EMA_ENT	*eep ;

	int	rs = SR_OK ;
	int	j ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progfile_info: level=%u ep(%p)\n",level,ep) ;
#endif

	if (ofp == NULL)
	    return SR_OK ;

	if (ep == NULL)
	    return SR_FAULT ;

	if ((rs = progentryinfo(pip,ofp,ep,level)) >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progfile_info: progentryinfo() rs=%d\n",rs) ;
#endif

	    if ((ep->type == ematype_group) && (ep->listp != NULL)) {
	        for (j = 0 ; ema_get(ep->listp,j,&eep) >= 0 ; j += 1) {
	            if (eep != NULL) {
	                rs = progfile_info(pip,ofp,eep,(level + 1)) ;
		    }
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (right type) */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progfile_info: ret rs=%d j=%u\n",rs,j) ;
#endif

	return (rs >= 0) ? j : rs ;
}
/* end subroutine (progfile_info) */



