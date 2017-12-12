/* progdebugout */

/* write out the output files from the executed program */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-09-07, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<linefold.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"defs.h"


/* local defines */

#ifndef	VARCOLUMNS
#define	VARCOLUMNS	"COLUMNS"
#endif

#ifndef	COLUMNS
#define	COLUMNS		80
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;


/* forward references */

static int	procline(PROGINFO *,int,const char *,int) ;


/* local variables */

static cchar	blanks[] = "        " ;


/* exported subroutines */


int progdebugout(PROGINFO *pip,cchar *s,cchar *fname)
{
	int		rs = SR_OK ;
	int		rs1 = SR_OK ;
	int		wlen = 0 ;

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	if (pip->efp != NULL) {
	    int		columns = -1 ;
	    char	*cp ;

/* determine the column-width of STDERR */

	    if ((cp = getenv(VARCOLUMNS)) != NULL) {
	        rs1 = cfdeci(cp,-1,&columns) ;
	        if (rs1 < 0) columns = -1 ;
	    }

	    if (columns < 0)
	        columns = COLUMNS ;

	    if (columns > 0) {
	        bfile	ofile, *ofp = &ofile ;

	        if ((rs = bopen(ofp,fname,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
		    int		f_title = FALSE ;
	            int		lines = 0 ;
	            int		len ;
	            cchar	*pn = pip->progname ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            rs1 = SR_OK ;
	            while ((rs = breadline(ofp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n')
	                    lbuf[--len] = '\0' ;

	                if ((! f_title) && (len > 0)) {
	                    if ((s != NULL) && (s[0] != '\0')) {
	                        cchar	*fmt = "%s: %s>\n" ;
	                        f_title = TRUE ;
	                        rs1 = shio_printf(pip->efp,fmt,pn,s) ;
	                        wlen += rs1 ;
	                    }
	                }

	                if (rs1 >= 0) {
	                    rs1 = procline(pip,columns,lbuf,len) ;
	                    wlen += rs1 ;
	                }

	                lines += 1 ;

	                if (rs1 < 0) break ;
	            } /* end while (reading lines) */

	            if (rs1 < 0) wlen = 0 ;

	            bclose(ofp) ;
	        } /* end if (bfile) */

	    } /* end if (positive columns) */

	} /* end if (non-null) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progdebugout) */


/* local subroutines */


static int procline(PROGINFO *pip,int columns,cchar *lp,int ll)
{
	SHIO		*fp = pip->efp ;
	const int	indent = 2 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		leadlen ;
	int		textlen ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;

	leadlen = (strlen(pn) + 4) ;
	textlen = (columns - leadlen) ;
	if (textlen > 0) {
	    LINEFOLD	lf ;
	    if ((rs = linefold_start(&lf,textlen,indent,lp,ll)) >= 0) {
	        int	ind = 0 ;
	        int	i ;
	        int	cl ;
	        cchar	*cp ;
	        for (i = 0 ; (cl = linefold_get(&lf,i,&cp)) >= 0 ; i += 1) {
	            rs1 = shio_printf(fp,"%s: | %t%t\n",pn,blanks,ind,cp,cl) ;
	            wlen += rs1 ;
	            if (rs1 < 0) break ;
	            ind = indent ;
	        } /* end for */
	        linefold_finish(&lf) ;
	    } /* end if (linefold) */
	} /* end if (non-zero) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procline) */


