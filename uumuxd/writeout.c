/* progdebugout */

/* write out the output files from the executed program */


#define	CF_DEBUGS	0		/* compile-time debugging */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<linefold.h>
#include	<bfile.h>

#include	"localmisc.h"
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

static int	procline(struct proginfo *,int,const char *,int) ;


/* local variables */

static const char	blanks[] = "        " ;


/* exported subroutines */


int progdebugout(pip,s,fname)
struct proginfo	*pip ;
const char	s[] ;
const char	fname[] ;
{
	bfile	ofile, *ofp = &ofile ;

	int	rs = SR_OK ;
	int	rs1 = SR_OK ;
	int	len ;
	int	columns = -1 ;
	int	lines = 0 ;
	int	f_title = FALSE ;
	int	wlen = 0 ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	*cp ;


	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

	if (pip->efp == NULL)
	    goto ret0 ;

/* determine the column-width of STDERR */

	if ((cp = getenv(VARCOLUMNS)) != NULL) {
	    rs1 = cfdeci(cp,-1,&columns) ;
	    if (rs1 < 0) columns = -1 ;
	}

	if (columns < 0)
	    columns = COLUMNS ;

	if (columns == 0)
	    goto ret0 ;

/* perform */

	rs = bopen(ofp,fname,"r",0666) ;
	if (rs < 0)
	    goto ret0 ;

	rs1 = SR_OK ;
	while ((rs = breadline(ofp,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    if (linebuf[len - 1] == '\n')
	        linebuf[--len] = '\0' ;

	    if ((! f_title) && (len > 0) && (s != NULL) && (s[0] != '\0')) {
	        f_title = TRUE ;
	        rs1 = shio_printf(pip->efp,"%s: %s>\n",pip->progname,s) ;
	        wlen += rs1 ;
	    }

	    if (rs1 >= 0) {
	        rs1 = procline(pip,columns,linebuf,len) ;
	        wlen += rs1 ;
	    }

	    if (rs1 < 0)
	        break ;

	    lines += 1 ;

	} /* end while (reading lines) */

	if (rs1 < 0)
	   wlen = 0 ;

ret1:
	bclose(ofp) ;

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
	LINEFOLD	folder ;

	SHIO		*fp = pip->efp ;

	const int	indent = 2 ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	leadlen ;
	int	textlen ;
	int	i ;
	int	ind ;
	int	cl ;
	int	wlen = 0 ;

	const char	*pn = pip->progname ;
	const char	*cp ;


	leadlen = (strlen(pn) + 4) ;
	textlen = (columns - leadlen) ;
	if (textlen < 1)
	    goto ret0 ;

	if ((rs = linefold_start(&folder,textlen,indent,lp,ll)) >= 0) {

	    ind = 0 ;
	    for (i = 0 ; (cl = linefold_get(&folder,i,&cp)) >= 0 ; i += 1) {
	        rs1 = shio_printf(fp,"%s: | %t%t\n",pn,blanks,ind,cp,cl) ;
	        wlen += rs1 ;
	        if (rs1 < 0)
	            break ;
	        ind = indent ;
	    } /* end for */

	    linefold_finish(&folder) ;

	} /* end if (linefold) */

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procline) */



