/* pingtab */

/* object to handle the user's PINGTAB file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1999-11-13, David A­D­ Morano
        This is a new object modules that collects the code that accesses the
        user currency file in one place.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object module accesses a PINGTAB file. The PINGTAB file is used to
        hold the names and PING timeout of hosts that are to be pinged by our
        caller.


*******************************************************************************/


#define	PINGTAB_MASTER		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<char.h>
#include	<field.h>
#include	<localmisc.h>

#include	"pingtab.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		(2 * MAXHOSTNAMELEN)
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local (static) variables */


/* exported subroutines */


int pingtab_open(ptp,fname)
PINGTAB		*ptp ;
const char	fname[] ;
{
	int	rs ;


#if	CF_DEBUGS
	debugprintf("pingtab_open: ent\n") ;
#endif

	if (ptp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_NOENT ;

/* open the user's PINGTAB file (if present) */

	if ((rs = bopen(&ptp->pfile,fname,"r",0664)) >= 0) {
	    struct ustat	sb ;
	    if ((rs = bcontrol(&ptp->pfile,BC_STAT,&sb)) >= 0) {
	        if (! S_ISDIR(sb.st_mode)) {
	            ptp->magic = PINGTAB_MAGIC ;
	        } else
	            rs = SR_ISDIR ;
	    }
	    if (rs < 0)
	        bclose(&ptp->pfile) ;
	} /* end if (file-open) */

#if	CF_DEBUGS
	debugprintf("pingtab_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pingtab_open) */


/* close this object */
int pingtab_close(ptp)
PINGTAB		*ptp ;
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (ptp == NULL) return SR_FAULT ;

	if (ptp->magic != PINGTAB_MAGIC) return SR_NOTOPEN ;

	rs1 = bclose(&ptp->pfile) ;
	if (rs >= 0) rs = rs1 ;

	ptp->magic = 0 ;
	return rs ;
}
/* end subroutine (pingtab_close) */


/* read an entry from the file */
int pingtab_read(ptp,ep)
PINGTAB		*ptp ;
PINGTAB_ENT	*ep ;
{
	const int	llen = LINEBUFLEN ;

	int	rs ;
	int	i ;
	int	len ;

	const char	*cp ;

	char	lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUGS
	char	timebuf[TIMEBUFLEN + 1] ;
#endif


	if (ptp == NULL)
	    return SR_FAULT ;

	if (ptp->magic != PINGTAB_MAGIC)
	    return SR_NOTOPEN ;

	ep->hostname[0] = '\0' ;
	ep->intminping = -1 ;
	ep->timeout = -1 ;
	while ((rs = breadline(&ptp->pfile,lbuf,llen)) > 0) {
	    len = rs ;

	    if (lbuf[len - 1] == '\n') len -= 1 ;
	    lbuf[len] = '\0' ;

	    cp = lbuf ;
	    while (CHAR_ISWHITE(*cp)) cp += 1 ;

	    if ((*cp != '#') && (*cp != '\0')) {
	        FIELD		f ;
	        int		iw ;

	        if ((rs = field_start(&f,cp,-1)) >= 0) {
	            int		fl ;
	            const char	*fp ;

	            for (i = 0 ; i < 3 ; i += 1) {
	                if ((fl = field_get(&f,NULL,&fp)) >= 0) {
	                    if (fl > 0) {
	                        switch (i) {
	                        case 0:
	                            if (fl <= MAXHOSTNAMELEN)
	                                strwcpy(ep->hostname,fp,fl) ;
	                            break ;
	                        case 1:
	                            if (cfdecti(fp,fl,&iw) >= 0)
	                                ep->intminping = iw ;
	                            break ;
	                        case 2:
	                            if (cfdecti(fp,fl,&iw) >= 0)
	                                ep->timeout = iw ;
	                            break ;
	                        } /* end switch */
	                    }
	                } /* end if (field_get) */
	                if (f.term == '#') break ;
	                if (rs < 0) break ;
	            } /* end for */

	            field_finish(&f) ;
	        } /* end if (field) */

	    } /* end if (non-empty) */

	    if (ep->hostname[0] != '\0') break ;
	    if (rs < 0) break ;
	} /* end while (reading PINGTAB file) */

	return rs ;
}
/* end subroutine (pingtab_read) */


/* rewind the file */
int pingtab_rewind(ptp)
PINGTAB		*ptp ;
{
	int	rs ;


	if (ptp == NULL)
	    return SR_FAULT ;

	if (ptp->magic != PINGTAB_MAGIC)
	    return SR_NOTOPEN ;

	rs = bseek(&ptp->pfile,0L,SEEK_SET) ;

	return rs ;
}
/* end subroutine (pingtab_rewind) */


