/* progstrmailer */

/* get the "mailer"-name string if we can */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The subroutine was written from scratch but based on previous versions
        of the 'mkmsg' program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Output the "x-mailer" header.

	int progstrmailer(pip)
	struct proginfo	*pip ;


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<buffer.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	readfilestrs(char *,int,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern int progexpcook_subbuf(struct proginfo *,BUFFER *,const char *,int) ;

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* global variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int progstrmailer(struct proginfo *pip,const char *strp)
{
	const int	llen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		strl = -1 ;
	char		lbuf[LINEBUFLEN+1] ;

	if (pip == NULL) return SR_FAULT ;

	if (strp == NULL) strp = STR_MAILER ;

	if (strp == NULL) {
	    const char	*sfname = STRMAILERFNAME ;
	    char	fbuf[MAXPATHLEN+1] ;
	    if (sfname[0] != '/') {
		rs = mkpath2(fbuf,pip->pr,sfname) ;
		sfname = fbuf ;
	    }
	    if (rs >= 0) {
	        strp = lbuf ;
	        rs = readfilestrs(lbuf,llen,sfname) ;
		strl = rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("progstrmailer: readfilestrs() rs=%d\n",rs) ;
#endif
	    }
	}

	if ((rs >= 0) && (strp != NULL)) {
	    BUFFER	b ;
	    const int	bsize = (strl >= 0) ? (2*strl) : 50 ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("progstrmailer: strl=%d\n",strl) ;
	        debugprintf("progstrmailer: s=>%t<\n",
		    strp,strlinelen(strp,strl,50)) ;
	    }
#endif
	    if ((rs = buffer_start(&b,bsize)) >= 0) {
	        if ((rs = progexpcook_subbuf(pip,&b,strp,strl)) >= 0) {
		    const char	*bp ;
		    if ((rs = buffer_get(&b,&bp)) >= 0) {
			const char	**vpp = &pip->hdr_mailer ;
			rs = proginfo_setentry(pip,vpp,bp,rs) ;
		    }
		}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("progstrmailer: procexpcook_subbuf() rs=%d\n",rs) ;
#endif
		strl = buffer_finish(&b) ;
		if (rs >= 0) rs = strl ;
	    } /* end if (buffer) */
	} /* end if (string-expand) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    const char	*s = pip->hdr_mailer ;
	    debugprintf("progstrmailer: mailer=>%t<\n",
		s,strlinelen(s,-1,50)) ;
	    debugprintf("progstrmailer: ret rs=%d\n",rs) ;
	}
#endif

	return rs ;
}
/* end subroutine (progstrmailer) */


