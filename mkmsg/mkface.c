/* mkface */

/* retrieve the FACE information for this user */


#define	CF_DEBUG	0		/* not-switchable debug print-outs */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine reads the FACE image for the calling user (if it is
	available) and puts it into the given buffer.

	Synopsis:

	int mkface(pip,rbuf,rlen)
	PROGINFO	*pip ;
	char		*rbuf ;
	int		rlen ;

	Arguments:

	pip		pointer to program information
	rbuf		result buffer
	rlen		length of result buffer

	Returns:

	>0		OK, got a face and returning length of result in buffer
	==0		did *not* get a face
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	FACEFNAME
#define	FACEFNAME	".xface"
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	iseol(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mkface(PROGINFO *pip,char *fbuf,int flen)
{
	int		rs ;
	int		rs1 ;
	int		fl = 0 ;
	const char	*fn = FACEFNAME ;
	char		tbuf[MAXPATHLEN + 1] ;

	if (pip == NULL) return SR_FAULT ;
	if (fbuf == NULL) return SR_FAULT ;

	pip->f.init_face = TRUE ;
	if ((rs = mkpath2(tbuf,pip->homedname,fn)) >= 0) {
	    const int	am = R_OK ;
	    if ((rs = uc_access(tbuf,am)) >= 0) {
	        SBUF	b ;
	        if ((rs = sbuf_start(&b,fbuf,flen)) >= 0) {
	            bfile	xfile, *xfp = &xfile ;

	            if ((rs = bopen(xfp,tbuf,"r",0666)) >= 0) {
	                const int	llen = LINEBUFLEN ;
	                int		len ;
	                int		cl ;
			int		c = 0 ;
	                const char	*cp ;
	                char		lbuf[LINEBUFLEN + 1] ;

	                while ((rs = breadline(xfp,lbuf,llen)) > 0) {
	                    len = rs ;

	                    while (len && iseol(lbuf[len-1])) len -= 1 ;

	                    if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
				    if (c++ > 0) {
	                                rs = sbuf_char(&b,' ') ;
				    }
				    if (rs >= 0) {
	                                rs = sbuf_strw(&b,cp,cl) ;
				    }
	                    }

	                    if (rs < 0) break ;
	                } /* end while */

	                rs1 = bclose(xfp) ;
	                if (rs >= 0) rs = rs1 ;
	            } else if (isNotPresent(rs)) {
	                rs = SR_OK ;
	            } /* end if (face-file) */

	            fl = sbuf_finish(&b) ;
	            if (rs >= 0) rs = fl ;
	        } /* end if (sbuf) */
	    } else if (isNotAccess(rs)) {
	        rs = SR_OK ;
	    }
	} /* end if (mkpath) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mkface: ret rs=%d flen=%u\n",rs,fl) ;
#endif

	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (mkface) */


