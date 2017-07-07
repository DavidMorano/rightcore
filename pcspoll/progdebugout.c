/* progdebugout */

/* write out the output files from the executed program */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Some sort of output procecessing.


*******************************************************************************/

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

extern cchar	*getourenv(cchar **,cchar *) ;


/* forward references */

static int	procline(struct proginfo *,int,const char *,int) ;


/* local variables */

static const char	blanks[] = "        " ;


/* exported subroutines */


int progdebugout(pip,s,fname)
struct proginfo	*pip ;
const char	s[] ;
const char	fname[] ;
{
	bfile		ofile, *ofp = &ofile ;
	int		rs = SR_OK ;
	int		rs1 = SR_OK ;
	int		columns = -1 ;
	int		wlen = 0 ;
	int		f_title = FALSE ;
	const char	*pn = pip->progname ;
	const char	*cp ;

	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

	if (pip->efp == NULL)
	    goto ret0 ;

/* determine the column-width of STDERR */

	if ((cp = getourenv(pip->envv,VARCOLUMNS)) != NULL) {
	    rs1 = cfdeci(cp,-1,&columns) ;
	    if (rs1 < 0) columns = -1 ;
	}

	if (columns < 0)
	    columns = COLUMNS ;

	if (columns == 0)
	    goto ret0 ;

/* perform */

	if ((rs = bopen(ofp,fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    rs1 = SR_OK ;
	    while ((rs = breadline(ofp,lbuf,llen)) > 0) {
	        int	len = rs ;

	        if (lbuf[len - 1] == '\n')
	            lbuf[--len] = '\0' ;

	        if ((! f_title) && (len > 0) && (s != NULL) && (s[0] != '\0')) {
	            f_title = TRUE ;
	            rs1 = shio_printf(pip->efp,"%s: %s>\n",pn,s) ;
	            wlen += rs1 ;
	        }

	        if (rs1 >= 0) {
	            rs1 = procline(pip,columns,lbuf,len) ;
	            wlen += rs1 ;
	        }

	        if (rs1 < 0) break ;
	    } /* end while (reading lines) */
	    if (rs1 < 0) wlen = 0 ;

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (file-processing) */

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progdebugout) */


/* local subroutines */


static int procline(pip,columns,lp,ll)
struct proginfo	*pip ;
int		columns ;
const char	lp[] ;
int		ll ;
{
	SHIO		*efp = pip->efp ;
	const int	indent = 2 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		leadlen ;
	int		textlen ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;

	leadlen = (strlen(pn) + 4) ;
	textlen = (columns - leadlen) ;
	if (textlen > 1) {
	    LINEFOLD	lf ;
	    if ((rs = linefold_start(&lf,textlen,indent,lp,ll)) >= 0) {
	        int		ind = 0 ;
	        int		i ;
	        int		cl ;
		const char	*fmt = "%s: | %t%t\n" ;
	        const char	*cp ;
	        for (i = 0 ; (cl = linefold_get(&lf,i,&cp)) >= 0 ; i += 1) {
	            rs1 = shio_printf(efp,fmt,pn,blanks,ind,cp,cl) ;
	            wlen += rs1 ;
	            if (rs1 < 0) break ;
	            ind = indent ;
	        } /* end for */
	        rs1 = linefold_finish(&lf) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (linefold) */
	} /* end if (big enough) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procline) */


