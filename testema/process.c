/* process */

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

	int process(pip,pp,ofp,fname)
	struct proginfo	*pip ;
	PARAMOPT	*pp ;
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
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<msg.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"ema.h"


/* local defines */


/* external subroutines */

extern int	progentryinfo(struct proginfo *,EMA_ENT *,int,bfile *) ;


/* external variables */


/* local structures */


/* forward references */

static int process_info(struct proginfo *, EMA_ENT *,int,bfile *) ;


/* local variables */


/* exported subroutines */


int process(pip,ofp,fname)
struct proginfo	*pip ;
bfile		*ofp ;
const char	fname[] ;
{
	EMA		adds ;
	EMA_ENT		*ep ;
	bfile		infile, *ifp = &infile ;
	int		rs ;
	int		i, j ;
	int		len ;
	int		n = 0 ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	*cp, *hp, *vp ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("process: entered file=%s\n",fname) ;
	    debugprintf("process: ofp=%p \n",ofp) ;
	}
#endif

	rs = ema_init(&adds) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process: ema_init() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

/* open the file that we are supposed to process */

	if (fname[0] != '-') {
	    rs = bopen(ifp,fname,"r",0666) ;
	} else
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	if (rs < 0)
	    goto ret1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process: continuing\n") ;
#endif

	while ((rs = breadline(ifp,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    if (linebuf[len - 1] == '\n')
		len -= 1 ;

	    linebuf[len] = '\0' ;

#if	CF_DEBUG 
	    if (DEBUGLEVEL(2))
	        debugprintf("process: >%w<\n",linebuf,len) ;
#endif

	rs = ema_parse(&adds,linebuf,len) ;

#if	CF_DEBUG 
	    if (DEBUGLEVEL(2))
	        debugprintf("process: ema_parse() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        break ;

	} /* end while */

/* the theory is that we have all we want from the headers, do it */

	if (rs >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("process: looping printing\n") ;
#endif

	    for (i = 0 ; ema_get(&adds,i,&ep) >= 0 ; i += 1) {

	        int	spc = 0 ;


#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: ema_get() i=%d ep=%p\n",i,ep) ;
#endif

	        if (ep == NULL) continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("process: address=>%s<\n",ep->address) ;
	            debugprintf("process: route=%s\n",ep->route) ;
	            debugprintf("process: comment=>%s<\n",ep->comment) ;
	        }
#endif

	        if (! pip->f.count) {

	            if (! pip->f.emainfo) {

	                if (pip->f.ema_original) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("process: original=>%s<\n",
	                            ep->original) ;
#endif

	                    if (ep->original != NULL)
	                        bprintf(ofp,"%s",ep->original) ;

	                    spc += 1 ;
	                }

	                if (pip->f.ema_best) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("process: best\n") ;
#endif

	                    if (ep->route != NULL)
	                        bprintf(ofp,"%s",ep->route) ;

	                    else if (ep->address != NULL)
	                        bprintf(ofp,"%s",ep->address) ;

	                    else if (ep->comment != NULL)
	                        bprintf(ofp,"(%s)",ep->comment) ;

	                    spc += 1 ;
	                }

	                if (pip->f.ema_any) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("process: any\n") ;
#endif

	                    if (ep->address != NULL)
	                        bprintf(ofp,"%s",ep->address) ;

	                    else if (ep->route != NULL)
	                        bprintf(ofp,"%s",ep->route) ;

	                    else if (ep->comment != NULL)
	                        bprintf(ofp,"(%s)",ep->comment) ;

	                    spc += 1 ;
	                }

	                if (pip->f.ema_address) {

	                    if (spc > 0)
	                        bprintf(ofp,"|") ;

	                    if (ep->address != NULL) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
				debugprintf("process: alen=%d address=>%w<\n",
	                                ep->alen,ep->address,ep->alen) ;
#endif

	                        bprintf(ofp,"%s",ep->address) ;

	                    }

	                    spc += 1 ;
	                }

	                if (pip->f.ema_route) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("process: route=>%s<\n",
	                            ep->route) ;
#endif

	                    if (spc > 0)
	                        bprintf(ofp,"|") ;

	                    if (ep->route != NULL)
	                        bprintf(ofp,"%s",ep->route) ;

	                    spc += 1 ;
	                }

	                if (pip->f.ema_comment) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("process: comment=>%s<\n",
	                            ep->comment) ;
#endif

	                    if (spc > 0)
	                        bprintf(ofp,"|") ;

	                    if (ep->comment != NULL) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
				debugprintf("process: clen=%d comment=>%W<\n",
	                                ep->clen,ep->comment,ep->clen) ;
#endif

	                        bprintf(ofp,"%s",ep->comment) ;

	                    }

	                    spc += 1 ;
	                }

	                bprintf(ofp,"\n") ;

	            } else {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	                    debugprintf("process: printing information\n") ;
#endif

	                rs = process_info(pip,ep,0,ofp) ;

	            } /* end if (information) */

	        } /* end if (counting only) */

	        n += 1 ;
	        if (rs < 0)
	            break ;

	    } /* end for */

	} /* end if */

ret2:
	bclose(ifp) ;

ret1:
	ema_free(&adds) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process: ret rs=%d entries=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (process) */


/* local subroutines */


static int process_info(pip,ep,level,ofp)
struct proginfo	*pip ;
EMA_ENT	*ep ;
int		level ;
bfile		*ofp ;
{
	EMA_ENT	*ep2 ;

	int	rs = SR_OK ;
	int	j ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process_info: level=%u\n",level) ;
#endif

	progentryinfo(pip,ep,level,ofp) ;

	if ((ep->type == ematype_group) && (ep->listp != NULL)) {

	    for (j = 0 ; ema_get(ep->listp,j,&ep2) >= 0 ; j += 1) {

	        if (ep2 == NULL)
	            continue ;

	        rs = process_info(pip,ep2,(level + 1),ofp) ;

	        if (rs < 0)
	            break ;

	    } /* end for */

	} /* end if */

	return (rs >= 0) ? j : rs ;
}
/* end subroutine (process_info) */



